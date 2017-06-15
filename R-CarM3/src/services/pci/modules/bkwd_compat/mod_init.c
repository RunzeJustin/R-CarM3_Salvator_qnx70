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
#include <sys/resmgr.h>
#include <sys/iofunc.h>
#include <dlfcn.h>

#include <pci/pci.h>
#include <pci/bkwd_compat_mod_api.h>

#include "bkwdmod_lib.h"
#include "private/pci_lib.h"

/*
 ===============================================================================
 bkwd_compat_mod_access

 This structure is a table of function pointers for handling message from older
 device driver software. If the BKWD_COMPAT_MODULE_INITFN does not exist, this
 symbol will be searched for. If BKWD_COMPAT_MODULE_INITFN does exist and
 returns PCI_ERR_OK, this symbol will not be searched for
*/
const bkwd_compat_mod_api_t BKWD_COMPAT_MODULE_ACCESS =
{
	.struct_size = sizeof(BKWD_COMPAT_MODULE_ACCESS),
	.attach_device = pci_bkwd_attach_device,
	.detach_device = pci_bkwd_detach_device,
	.device_find = pci_bkwd_device_find,
	.read_config_bus = pci_bkwd_read_config_bus,
	.write_config_bus = pci_bkwd_write_config_bus,
	.read_config = pci_bkwd_read_config,
	.write_config = pci_bkwd_write_config,
	.rescan_bus = pci_bkwd_rescan_bus,
	.pci_present = pci_bkwd_pci_present,
};

/*
 ===============================================================================
 BKWD_COMPAT_MODULE_INITFN

 This optional function will be called (if it exists) after the backward
 compatibility module is opened and prior to the search for the
 'bkwd_compat_mod_api_t' function pointer table.

 If this function exists, it accepts as a parameter a pointer to a
 'bkwd_compat_mod_api_t' pointer into which it can store a pointer to the
 backward compatibility 'bkwd_compat_mod_api_t' structure.

 It returns PCI_ERR_OK on success, or one of the defined errors as follows

 	 PCI_ERR_EAGAIN - this init function should be called again

*/
pci_err_t BKWD_COMPAT_MODULE_INITFN(volatile const bkwd_compat_mod_api_t **mod_p)
{
	pci_runtime_flags |= pci_runtime_flags_e_BKWD_COMPAT_MODULE;
	*mod_p = &BKWD_COMPAT_MODULE_ACCESS;
	return PCI_ERR_OK;
}

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/services/pci/modules/bkwd_compat/mod_init.c $ $Rev: 798837 $")
#endif
