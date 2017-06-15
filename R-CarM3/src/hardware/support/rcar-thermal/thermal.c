/*
 * $QNXLicenseC:
 * Copyright 2016, QNX Software Systems.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"). You
 * may not reproduce, modify or distribute this software except in
 * compliance with the License. You may obtain a copy of the License
 * at: http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTIES OF ANY KIND, either express or implied.
 *
 * This file may contain contributions from others, either as
 * contributors under the License or as licensors under other terms.
 * Please review this entire file for other proprietary rights or license
 * notices, as well as the QNX Development Suite License Guide at
 * http://licensing.qnx.com/license-guide/ for other information.
 * $
 */

#include <stdlib.h>
#include <stdio.h>

#include <stdarg.h>
#include <sys/slog.h>
#include <sys/slogcodes.h>
#include <pthread.h>

#include <stdint.h>
#include <sys/mman.h>
#include <sys/neutrino.h>
#include <hw/inout.h>
#include <sys/resmgr.h>
#include <sys/iofunc.h>
#include <sys/dispatch.h>
#include <errno.h>
#include <string.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <inttypes.h>
#include <devctl.h>

#include <atomic.h>
#include <sys/procmgr.h>

#include "thermal.h"
#include "io.h"

typedef struct ths_civm_dev{
    iofunc_attr_t       hdr;
    dispatch_t          *dpp;
    dispatch_context_t  *ctp;

    int                 irq;        /* interrupt number */
    int                 id;         /* interrupt ID */
    int                 tid;
    int                 chid;
    int                 coid;
    struct sigevent     event;

    ths_civm_t          *ths_civm;  /*Structure contain registers if the THS/CIVM */
}ths_civm_dev_t;


/* our connect functions */
resmgr_connect_funcs_t connect_funcs;
resmgr_io_funcs_t io_funcs;
/* our dispatch, resource manager and iofunc variables */
resmgr_attr_t rattr;
ths_civm_dev_t *dev;
int intr_pulse_code;

/* getopt vars */
char* pcProgname;

char* progname = NULL;
extern int verbose ;
static volatile unsigned done = 0;

void threshold_setting(ths_civm_dev_t *dev);
void ths_civm_init(ths_civm_dev_t *dev);
void ths_civm_interrupt_process(ths_civm_dev_t *dev);

void exit_signal(int signo)
{
    atomic_set(&done, 1);
    return;
}
//  This routine handles the command line options.
int options(int argc, char* argv[])
{
    int c;
    int ret = 0;

    pcProgname = argv[0];
    while ((c = getopt(argc, argv, "vh:")) != -1)
    {
        switch (c)
        {
            case 'v':
                verbose++;
                break;
            default:
                ret = -1;
                break;
        }

    }
    return ret;
}

int
intr_pulse_handler (message_context_t *ctp, int code, unsigned flags, void *handle)
{
    ths_civm_dev_t *dev = (ths_civm_dev_t *)handle;

    if(code == intr_pulse_code) {
        ths_civm_interrupt_process(dev);
        InterruptUnmask(dev->irq, dev->id);
    } else  {
        thermal_slogf(VERBOSE_LEVEL1,"%s: Received an unknown pulse: %d",
                      __FUNCTION__, code);
    }
    return 0;
}

/*
 * Single_instance
 * Ensure that the driver is not already running
 * Request a minor number of 0
 * If we get it, we're the first instantiation.
 */
void single_instance(char* resmgr_name)
{
    name_attach_t* attach;

    // Create a local name
    if ((attach = name_attach(NULL, resmgr_name, 0)) == NULL)
    {
        thermal_slogf(VERBOSE_QUIET,"\nError: Is '%s' already started? \n", resmgr_name);
        exit(EXIT_FAILURE);
    }
}

int main(int argc, char *argv[])
{
    char    *name;
    int status;

    pcProgname = NULL;
    verbose = 0;
    // parse the command-line options
    status = options(argc, argv);
    if(status)
    {
        thermal_slogf(VERBOSE_QUIET,"%s: Invalid option\n", progname);
        exit(EXIT_FAILURE);
    }

    if (-1 == ThreadCtl(_NTO_TCTL_IO, 0))
    {
        thermal_slogf(VERBOSE_QUIET,"%s: ThreadCtl failed: %s\n", progname, strerror(errno));
        exit(EXIT_FAILURE);
    }
    dev=(ths_civm_dev_t *) malloc(sizeof(ths_civm_dev_t ));

    // make sure the driver isn't already running
    if ((name = malloc(sizeof(DEV_NAME) + 2)) == NULL)
    {
        thermal_slogf(VERBOSE_QUIET,"%s: malloc failed: %s\n", progname, strerror(errno));
        exit(EXIT_FAILURE);
    }

    single_instance(RESMGR_NAME);

    // allocate and initialize a dispatch structure for use by our main loop
    dev->dpp = dispatch_create();
    if (dev->dpp == NULL)
    {
        thermal_slogf(VERBOSE_QUIET,"%s:  couldn't dispatch_create: %s\n",progname, strerror (errno));
        exit(EXIT_FAILURE);
    }

    /*
     * Set up the resource manager attributes structure, we'll
     * use this as a way of passing information to resmgr_attach().
     * For now, we just use defaults.
     */

    memset(&rattr, 0, sizeof (rattr));
    rattr.msg_max_size = MAX_MSG_SIZE;  // max size of a message

    /*
     * Intialize the connect functions and I/O functions tables to
     * their defaults and then override the defaults with the
     * functions that we are providing.
     */
    iofunc_func_init(_RESMGR_CONNECT_NFUNCS, &connect_funcs,
            _RESMGR_IO_NFUNCS, &io_funcs);

    // use our own devctl
    io_funcs.devctl = io_devctl;

    /*
     * Set the mode to 0666 (R/W)
     * Note leading 0 for octal -------v
     */
    iofunc_attr_init(&dev->hdr, S_IFCHR | 0666, NULL, NULL);

    /*
     * Call resmgr_attach to register our prefix with the
     * process manager, and also to let it know about our connect and I/O functions.
     * On error, returns -1 and errno is set.
     */
    dev->id= resmgr_attach(dev->dpp, &rattr, DEV_NAME, _FTYPE_ANY, 0, &connect_funcs, &io_funcs, &dev->hdr);
    if (dev->id == -1)
    {
        thermal_slogf(VERBOSE_QUIET,"%s:  couldn't attach pathname: %s\n", progname, strerror(errno));
        exit(1);
    }
    free(name);

    /* THS_CIVM mapping */
    if ((dev->ths_civm = (ths_civm_t *)mmap_device_io(sizeof(ths_civm_t), RCAR_THS_TSC1)) == MAP_FAILED) {
        thermal_slogf(VERBOSE_QUIET,"%s:  couldn't mapping for ths 0: %s\n", __FUNCTION__, strerror(errno));
        exit(1);
    }
    ths_civm_init(dev);

    intr_pulse_code = pulse_attach(dev->dpp, MSG_FLAG_ALLOC_PULSE, 0, intr_pulse_handler, dev);
    /* create a connection to the channel that our resource manager is receiving on */
    dev->coid = message_connect (dev->dpp, MSG_FLAG_SIDE_CHANNEL);
    if (dev->coid == -1) {
        thermal_slogf(VERBOSE_QUIET,"%s: unable to connect to side channel", __FUNCTION__);
        exit(1);
    }

    SIGEV_PULSE_INIT(&dev->event, dev->coid, 21, intr_pulse_code, NULL);

    /* Interrupt attach */
    dev->irq = RCAR_INTCSYS_THERMAL2;
    if ((dev->id = InterruptAttachEvent(dev->irq, &dev->event,
                                        _NTO_INTR_FLAGS_TRK_MSK|_NTO_INTR_FLAGS_END)) == -1)
        thermal_slogf(VERBOSE_QUIET,"%s: Interrupt attach failed:%s \n", __FUNCTION__, strerror(errno));

    /* allocate a context structure */

    dev->ctp = dispatch_context_alloc(dev->dpp);
    /*  Start the resource manager loop */
    while (!done)
    {
        if (dev->ctp == dispatch_block(dev->ctp))
            dispatch_handler(dev->ctp);
        else if (errno != EFAULT)
            atomic_set(&done, 1);
    }

    free(dev);
    return 0;
}

void threshold_setting(ths_civm_dev_t *dev)
{
    uint16_t cur_temp = 0;

    ths_civm_get_temperature(&cur_temp);

    if((cur_temp < CTEMP_MIN_TEMP)  || (cur_temp > CTEMP_MAX_TEMP))
    {
        thermal_slogf(VERBOSE_QUIET,"Error temp %d is out of range \n", cur_temp);
    }

    dev->ths_civm->IRQSTR = 0x00; // Reset all Interrupt Status

}

/*
 * Initialize THS/CIVM module
 */
void ths_civm_init(ths_civm_dev_t *dev)
{
    uintptr_t   SMSTPCR5_val = mmap_device_io(4, RCAR_CPG_BASE + RCAR_CPG_SMSTPCR5);

    thermal_slogf(VERBOSE_LEVEL1,"%s", __FUNCTION__);

    /* supply clock for thermal sensor */
    out32(SMSTPCR5_val, in32(SMSTPCR5_val) & ~(1<<22));

    /* Power-on mode
     * This mode operates automatically at start up
     */

    /* Correction of THS & CIVM
     * TODO: Refer to Application Notes
     */

    /* Setting for Normal mode */
    if (!((dev->ths_civm->CTSR) & THSCIVM_CTSR_PONSEQSTOP))
    {
        /* Write H'0 to CTSR register */
        dev->ths_civm->CTSR = 0x0;
        do
        {
            /* Wait 1ms, until CTSR.PONSEQSTOP = 1 */
            delay(1);
        } while (!((dev->ths_civm->CTSR) & THSCIVM_CTSR_PONSEQSTOP));
    }
    /* Switch mode */
    dev->ths_civm->CTSR = 0x100;
    /*Edge detection */
    dev->ths_civm->IRQCTL = 0x3F; // Detect all status (TEMP_CODE [11:0] falls/exceeds IRQTEMP1,2,3)
    /* Set mask register */
    dev->ths_civm->IRQMSK = 0x3F; // All mask clear (pass all interrupt)
    /* Set IRQTEMP# */
    dev->ths_civm->IRQTEMP1 = 1000;
    dev->ths_civm->IRQTEMP2 = 2000;
    dev->ths_civm->IRQTEMP3 = 3000;

    /* Set IQREN */
    dev->ths_civm->IRQEN = 0x3F; // Enable all interrupt
    /*Enable analog circuit*/
    dev->ths_civm->CTSR = 0x1B0;

    /* Wait 100us */
    usleep(100);
    /*Enable digital circuit*/
    dev->ths_civm->CTSR = 0x1B3;

    /* Clear Interrupt Status */
    dev->ths_civm->IRQSTR = 0;

    //threshold_setting(dev);

    munmap_device_io(SMSTPCR5_val, 4);
}

/*
 * Processs thermal interrupt
 */
void ths_civm_interrupt_process(ths_civm_dev_t *dev)
{
    uint32_t irq_status;

    thermal_slogf(VERBOSE_LEVEL1,"%s", __FUNCTION__);

    irq_status = dev->ths_civm->IRQSTR;
    switch (irq_status)
    {
        case THSCIVM_IRQSTR_TEMP3_STR:
            thermal_slogf(VERBOSE_LEVEL1,"TEMP_CODE [11:0] exceeds IRQTEMP3");
            break;
        case THSCIVM_IRQSTR_TEMP2_STR:
            thermal_slogf(VERBOSE_LEVEL1,"TEMP_CODE [11:0] exceeds IRQTEMP2");
            break;
        case THSCIVM_IRQSTR_TEMP1_STR:
            thermal_slogf(VERBOSE_LEVEL1,"TEMP_CODE [11:0] exceeds IRQTEMP1");
            break;
        case THSCIVM_IRQSTR_TEMPD3_STR:
            thermal_slogf(VERBOSE_LEVEL1,"TEMP_CODE [11:0] falls below IRQTEMP3");
            break;
        case THSCIVM_IRQSTR_TEMPD2_STR:
            thermal_slogf(VERBOSE_LEVEL1,"TEMP_CODE [11:0] falls below IRQTEMP2");
            break;
        case THSCIVM_IRQSTR_TEMPD1_STR:
            thermal_slogf(VERBOSE_LEVEL1,"TEMP_CODE [11:0] falls below IRQTEMP1");
            break;
        default:
            break;
    }
    dev->ths_civm->IRQSTR = 0; // Clear interrupt status
}

/*
 * Convert Digital Code to Temperature Code:
 * temp = (THCODE - 2536.7) / 7.468
 * _rount_temp takes in temp * 10 to account for rounding
 */
#define TEMP_CONVERT(ctemp) \
    ((10L * ((1000L * ctemp) - 2536700L)) / 7468L)

static int _round_temp(int i)
{
    int tmp1, tmp2;
    int result = 0;

    tmp1 = abs(i) % 10;
    tmp2 = abs(i) / 10;
    if (tmp1 < 5)
        result = tmp2;
    else
        result = tmp2 + 1;

    return ((i < 0) ? (result*(-1)) : result);
}

/*
 * Get current temperature of thermal:
 */
int ths_civm_get_temperature(uint16_t *temperature)
{
    uint32_t ret = 0;
    uint32_t ctemp;

    thermal_slogf(VERBOSE_LEVEL1, "%s", __FUNCTION__);

    ctemp = dev->ths_civm->TEMP & 0xFFF; // Read TEMP_CODE[11:0]
    *temperature = _round_temp(TEMP_CONVERT(ctemp));

    return ret;
}

/*
 * Get current voltage of LSI:
 */
int ths_civm_get_voltage(uint16_t *voltage)
{
    uint32_t ret = 0;
    uint32_t tmp;

    thermal_slogf(VERBOSE_LEVEL1, "%s", __FUNCTION__);

    /* TODO: We don't have the formula to convert digital code to voltage yet, returning raw code */
    tmp = dev->ths_civm->VOLT & 0x3FF; //Read VOLT_CODE[9:0]
    *voltage = tmp;

    return ret;
}

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/hardware/support/rcar-thermal/thermal.c $ $Rev: 811478 $")
#endif
