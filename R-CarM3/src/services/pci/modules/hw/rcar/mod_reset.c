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

#include <pci/pci.h>
#include <pci/pci_ccode.h>

#include "private/hwmod_api.h"
#include "hw_lib.h"
#include "private/pci_slog.h"
#include "private/pci_hw.h"

/*
 ===============================================================================
 hw_reset

 HW dependent reset callout.

 This function will be called for all resets to all devices. It is only required
 if the controller loses any configuration on downstream bus resets

 The current example implementation is the same as leaving the callout entry NULL
 See also hwmod_api.h for details

*/
__attribute__ ((visibility ("internal")))
pci_err_t hw_reset(const pci_resetType_e reset_type, const _pci_resetPhase_e phase, const pci_bdf_t bdf, uintptr_t extra)
{
    pci_err_t r = (phase == _pci_resetPhase_e_CHECK_SUPPORTED) ? PCI_ERR_ENOTSUP : PCI_ERR_OK;

    if (reset_type == pci_resetType_e_FUNCTION) r = PCI_ERR_OK;
    else if (reset_type == pci_resetType_e_BUS)
    {
        r = PCI_ERR_OK;

        if (bdf == PCI_BDF(0,0,0)) {
            static pci_cmd_t cmd = 0;

            switch(phase){
                case _pci_resetPhase_e_1:
                    _pci_device_cfg_rd16(bdf, 4, &cmd);
                    break;

                case _pci_resetPhase_e_2:
                    break;

                case _pci_resetPhase_e_3:
                    _pci_device_cfg_wr16(bdf, 4, cmd);
                    break;

                default:
                    break;
            }
        }
    }

    return r;
}

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL$ $Rev$")
#endif
