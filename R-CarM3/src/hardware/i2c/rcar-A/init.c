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

#include "proto.h"
#include <time.h>
#include <sys/mman.h>
#include <sys/neutrino.h>
#include <math.h>

static void force_stop(rcar_i2c_dev_t *dev, int num);

void *
rcar_i2c_init(int argc, char *argv[])
{
    rcar_i2c_dev_t  *dev;

    if (-1 == ThreadCtl(_NTO_TCTL_IO_PRIV, 0)) {
        perror("ThreadCtl");
        return NULL;
    }

    dev = calloc(sizeof(rcar_i2c_dev_t), 1);
    if ((!dev)) {
        rcar_i2c_slogf(dev, VERBOSE_QUIET, "%s: fail malloc of dev", __FUNCTION__);
        return NULL;
    }

    if (-1 == rcar_i2c_parse_options(dev, argc, argv)) {
        fprintf(stderr, "Incorrect arguments. Check the use message. \n");
        rcar_i2c_slogf(dev, VERBOSE_QUIET, "%s: fail rcar_i2c_parse_options", __FUNCTION__);
        goto fail;
    }

    dev->regbase = mmap_device_io(RCAR_I2C_SIZE, dev->physbase);
    if (dev->regbase == (uintptr_t)MAP_FAILED) {
        rcar_i2c_slogf(dev, VERBOSE_QUIET, "%s: fail mmap_device_io %x, %d", __FUNCTION__, dev->physbase, RCAR_I2C_SIZE);
        perror("mmap_device_io");
        goto fail;
    }

    /* clear bus if necessary */
    force_stop(dev, 10);

    /* reset slave interface */
    out32(dev->regbase + RCAR_I2C_ICSIER, 0);
    out32(dev->regbase + RCAR_I2C_ICSCR, 0);
    out32(dev->regbase + RCAR_I2C_ICSAR, 0);
    out32(dev->regbase + RCAR_I2C_ICSSR, 0);

    /* reset master interface */
    out32(dev->regbase + RCAR_I2C_ICMIER, 0);
    out32(dev->regbase + RCAR_I2C_ICMCR, 0);
    out32(dev->regbase + RCAR_I2C_ICMAR, 0);
    out32(dev->regbase + RCAR_I2C_ICMSR, 0);

    /* clock */
    out32(dev->regbase + RCAR_I2C_ICCCR, 0);
    out32(dev->regbase + RCAR_I2C_ICCCR2, 0);

    /* initialize to 50k Baud */
    rcar_i2c_set_bus_speed(dev, dev->busSpeed, NULL);

    /* Initialize interrupt handler */
    SIGEV_INTR_INIT(&dev->intrEvent);

    dev->intr = InterruptAttach(dev->irq, (void *)rcar_i2c_intr, dev, 0, _NTO_INTR_FLAGS_TRK_MSK);
    if (dev->intr == -1) {
        rcar_i2c_slogf(dev, VERBOSE_QUIET, "%s: fail InterruptAttach %x", __FUNCTION__, dev->irq);
        perror("InterruptAttach");
        goto fail;
    }

    return dev;

fail:
    free(dev);
    return NULL;
}

void
rcar_i2c_fini(void *hdl)
{
    rcar_i2c_dev_t  *dev = hdl;

    rcar_i2c_slogf(dev, VERBOSE_LEVEL8, "I2C rcar_i2c_fini: ");

    out32(dev->regbase + RCAR_I2C_ICMCR, 0);
    out32(dev->regbase + RCAR_I2C_ICSCR, 0);

    munmap_device_io(dev->regbase, RCAR_I2C_SIZE);

    InterruptDetach(dev->intr);

    free(hdl);
}

static void
force_stop(rcar_i2c_dev_t *dev, int num)
{
    unsigned msr;
    unsigned uiSchd, uiScld, uiSmd;
    unsigned long scl_period;
    unsigned retry_count = 0x1000000;
    int i;
    struct timespec	t_spec = { 0, 0 };

    rcar_i2c_slogf(dev, VERBOSE_LEVEL8, "I2C force_stop: ");

    if (in32(dev->regbase + RCAR_I2C_ICMCR) & RCAR_ICMCR_FSDA) {
        out32(dev->regbase + RCAR_I2C_ICMCR, RCAR_ICMCR_MIE | RCAR_ICMCR_FSB | RCAR_ICMCR_MDBS);
        out32(dev->regbase + RCAR_I2C_ICMSR, 0);

        msr = in32(dev->regbase + RCAR_I2C_ICMSR);
        if (msr & RCAR_ICMSR_MDR) {
            while (in32(dev->regbase + RCAR_I2C_ICMSR) & RCAR_ICMSR_MDR) {
                in32(dev->regbase + RCAR_I2C_ICRXD);
                out32(dev->regbase + RCAR_I2C_ICMSR, 0);
            }
        }

        while ((retry_count--) && !(in32(dev->regbase + RCAR_I2C_ICMSR) & RCAR_ICMSR_MST));
    }

    if (in32(dev->regbase + RCAR_I2C_ICMCR) & RCAR_ICMCR_FSDA) {
        uiSmd = in32(dev->regbase + RCAR_I2C_ICMPR);
        uiSchd = in32(dev->regbase + RCAR_I2C_ICHPR);
        uiScld = in32(dev->regbase + RCAR_I2C_ICLPR);
        scl_period = (8 + 2 * uiSmd + uiScld + uiSchd) * 1e9 / dev->pck;

        for (i = 0; i < num; ++i) {
            /* stop condition: keep SCL high, make SDA go low->high */
            t_spec.tv_nsec = scl_period;
            out32(dev->regbase + RCAR_I2C_ICMCR, RCAR_ICMCR_MIE | RCAR_ICMCR_OBPC | RCAR_ICMCR_FSDA);
            nanosleep(&t_spec, NULL);
            t_spec.tv_nsec = scl_period;
            out32(dev->regbase + RCAR_I2C_ICMCR, RCAR_ICMCR_MIE | RCAR_ICMCR_OBPC);
            nanosleep(&t_spec, NULL);
        }
    }
}

void rcar_i2c_reset(rcar_i2c_dev_t *dev)
{
    /* make sure any data transfer is complete before reset */
    force_stop(dev, 10);

    /* initialize to 50k Baud */
    dev->scl_freq = 0;
    rcar_i2c_set_bus_speed(dev, dev->busSpeed, NULL);

    rcar_i2c_slogf(dev, VERBOSE_LEVEL8, "I2C DEBUG reset \r\n");
}

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/hardware/i2c/rcar-A/init.c $ $Rev: 805407 $")
#endif
