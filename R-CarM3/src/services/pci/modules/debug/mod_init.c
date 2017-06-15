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
#include "private/dbgmod_api.h"
#include "debug.h"

/*
 ===============================================================================
 dbgmod_access

 This structure is a table of function pointers which provide access to the
 system logging facilities. If the DEBUG_MODULE_INITFN does not exist, this
 symbol will be searched for. If DEBUG_MODULE_INITFN does exist and returns
 PCI_ERR_OK, this symbol will not be searched for
*/
const dbgmod_api_t DEBUG_MODULE_ACCESS =
{
	.struct_size = sizeof(DEBUG_MODULE_ACCESS),
	.log = dbgmod_log,
	.mod_version = mod_version,
	.mod_compat = mod_compat,
};

/*
 ===============================================================================
 DEBUG_MODULE_INITFN

 This optional function will be called (if it exists) after the DEBUG module
 is opened and prior to the search for the 'dbgmod_api_t' function pointer
 table.

 If this function exists, it accepts as a parameter a pointer to a 'dbgmod_api_t'
 pointer into which it can store a pointer to the DEBUG 'dbgmod_api_t'
 structure.

 It returns PCI_ERR_OK on success, or one of the defined errors as follows

 	 PCI_ERR_EAGAIN - this init function should be called again

*/
pci_err_t DEBUG_MODULE_INITFN(volatile const dbgmod_api_t **dbg_p)
{
	*dbg_p = &DEBUG_MODULE_ACCESS;
	return PCI_ERR_OK;
}

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/services/pci/modules/debug/mod_init.c $ $Rev: 798837 $")
#endif
