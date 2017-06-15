/*
 * $QNXLicenseC:
 * Copyright (c) 2012, 2016 QNX Software Systems. All Rights Reserved.
 *
 * You must obtain a written license from and pay applicable license fees to QNX
 * Software Systems before you may reproduce, modify or distribute this software,
 * or any work that includes all or part of this software.   Free development
 * licenses are available for evaluation and non-commercial purposes.  For more
 * information visit http://licensing.qnx.com or email licensing@qnx.com.
 *
 * This file may contain contributions from others.  Please review this entire
 * file for other proprietary rights or license notices, as well as the QNX
 * Development Suite License Guide at http://licensing.qnx.com/license-guide/
 * for other information.
 * $
 */

#include <stddef.h>
#include <pthread.h>
#include <stdlib.h>
#include <setjmp.h>

#include <pci/pci.h>

#include "private/hwmod_api.h"
#include "hw_lib.h"

/*
 ===============================================================================
 bdf_checked
 set_bdf_checked
 set_bdf_doesnt_exist

 The following functions are used to mark the existence or not of any possible
 BDF so that we don't have to continually take bus faults if some misbehaved
 program does accesses to random BDF's

 The '_bdf_checked[]' array identifies whether or not a BDF has been probed.
 The '_bdf_doesnt_exist[]' array identifies whether the BDF exists or not and is
 only valid if the same bit in '_bdf_checked[]' is set

 '_bdf_doesnt_exist[]' is checked in mod_rdwr.c hence its visibility

*/
__attribute__ ((visibility ("internal")))
uint32_t _bdf_doesnt_exist[(PCI_MAX_BUSES * PCI_BDF_MAX_DEVS * PCI_BDF_MAX_FUNCS) / (sizeof(uint32_t) * 8)];
static uint32_t _bdf_checked[(PCI_MAX_BUSES * PCI_BDF_MAX_DEVS * PCI_BDF_MAX_FUNCS) / (sizeof(uint32_t) * 8)];

static inline bool_t bdf_checked(const pci_bdf_t bdf)
{
    const uint_t idx = bdf / 32;
    const uint_t bit = (PCI_BDF(0, PCI_DEV(bdf), PCI_FUNC(bdf))) % 32;
    return (((_bdf_checked[idx] >> bit) & 1) == 1);
}
static inline void set_bdf_checked(const pci_bdf_t bdf)
{
    const uint_t idx = bdf / 32;
    const uint_t bit = (PCI_BDF(0, PCI_DEV(bdf), PCI_FUNC(bdf))) % 32;
    _bdf_checked[idx] |= (1u << bit);
}
static inline void set_bdf_doesnt_exist(const pci_bdf_t bdf)
{
    const uint_t idx = bdf / 32;
    const uint_t bit = (PCI_BDF(0, PCI_DEV(bdf), PCI_FUNC(bdf))) % 32;
    _bdf_doesnt_exist[idx] |= (1u << bit);
}

/*
 ===============================================================================
 busfault

*/
static sigjmp_buf env;

__attribute__ ((visibility ("internal")))
void busfault(const int sig_num)
{
    if (sig_num == SIGBUS) siglongjmp(env, sig_num);
}

/*
 ===============================================================================
 ONLY_ONE_LINK_DEVICE

 This macro if defined, enables preloading of the '_bdf_checked[]' and
 '_bdf_doesnt_exist[]' arrays to avoid probes for devices we know don't exist.

 In this case, we ensure that devices 1 thru 31 on bus 1 are non-existent
 because during testing with a single function endpoint device (Intel NIC), the
 1:0:0 device was responding to probes for 1:1:0 thru 1:31:0 (ie. aliasing) and
 this caused a great deal of havoc.
 Since the iMX6 RC does not support ARI, the device number (or extended function
 number if ARI was supported) can never be > 0. This fact is also what allows
 for a static iATU configuration for Type 0 config space accesses.

*/
#define ONLY_ONE_LINK_DEVICE
#ifdef ONLY_ONE_LINK_DEVICE
static void only_one_link_device(void)
{
    pci_bdf_t bdf;

    for (bdf=PCI_BDF(1, 1, 0); bdf<=PCI_BDF(1, 31, 7); bdf++)
    {
        set_bdf_checked(bdf);
        set_bdf_doesnt_exist(bdf);
    }
}
#endif  /* ONLY_ONE_LINK_DEVICE */

/*
 ===============================================================================
 hw_mem_rd
 hw_mem_wr

 These functions implement the PCI configuration space read/write operations
 using memory mapped accesses

 The current implementation is generalized for an arbitrary size (which needs
 to be handled) however we could dereference the target address as 1, 2, 4 or 8
 byte accesses for corresponding specific values of <size>.

 <cfgmem_base> has already been established depending on whether an access will
 be to the controllers internal registers, the immediate downstream link or
 links further downstream.

*/
__attribute__ ((visibility ("internal")))
pci_err_t hw_mem_rd(void *cfgmem_base, const pci_bdf_t bdf, const uint_t offset, const uint_t size, uint8_t *buf)
{
#ifdef ONLY_ONE_LINK_DEVICE
    /*
     * this simulates the code in enumerate_bus() which prevents checking D1 thru D31
     * devices if we're on a PCIe link. For some reason, these devices are aliases
     * of the D0 device. Need more investigation but for now working around the problem
     * in the HW dependent code instead of making the change to pci-server until I
     * get a chance to test on more platforms
     */
    static pthread_once_t once = PTHREAD_ONCE_INIT;
    pthread_once(&once, only_one_link_device);
#endif  /* ONLY_ONE_LINK_DEVICE */

    if (offset >= 4096) return PCI_ERR_EINVAL;
    else if (!bdf_checked(bdf))
    {
        if (PCI_BUS(bdf) == 0)
        {
            if ((PCI_DEV(bdf) > 0) || (PCI_FUNC(bdf) > 0))
            {
                /* Only have a B0:D0:F0. All other Bus 0's are non-existent */
                set_bdf_doesnt_exist(bdf);
                set_bdf_checked(bdf);
                return PCI_ERR_ENODEV;
            }
        }
        else    /* bus > 0 */
        {
            /* setup to catch a possible bus fault, then let the access proceed */
            if (sigsetjmp(env, SIGBUS) == SIGBUS)
            {
                set_bdf_doesnt_exist(bdf);
                config_reg_unlock(NULL);
                return PCI_ERR_ENODEV;
            }
        }
        set_bdf_checked(bdf);
    }

    volatile uint32_t *cfg_mem = (volatile uint32_t *)cfgmem_base;
    uint_t i;
    uint_t pos = 0;

    for (i=0; i<size; i+=4)
    {
        uint_t j;
        uint32_t val = *cfg_mem;

        for(j=(offset + pos) % sizeof(val); (j < sizeof(val)) && (pos < size); j++) {
            buf[pos++] = ((uint8_t *)&val)[j];
        }
        ++cfg_mem;
    }
    return PCI_ERR_OK;
}

__attribute__ ((visibility ("internal")))
pci_err_t hw_mem_wr(void *cfgmem_base, const pci_bdf_t bdf, const uint_t offset, const uint_t size, uint8_t *buf)
{
    if (offset >= 4096) return PCI_ERR_EINVAL;
    else
    {
        assert(bdf_checked(bdf));

        volatile uint32_t *cfg_mem = (volatile uint32_t *)cfgmem_base;
        uint_t i;
        uint_t pos = 0;

        for (i=0; i<size; i+=4)
        {
            uint_t j;
            uint32_t val = *cfg_mem;

            for(j=(offset + pos) % sizeof(val); (j < sizeof(val)) && (pos < size); j++) {
                ((uint8_t *)&val)[j] = buf[pos++];
            }

            *cfg_mem = val;
            ++cfg_mem;
        }
        return PCI_ERR_OK;
    }
}

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL$ $Rev$")
#endif
