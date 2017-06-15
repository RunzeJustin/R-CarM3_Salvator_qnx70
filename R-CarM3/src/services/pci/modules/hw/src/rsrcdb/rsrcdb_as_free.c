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
#include <sys/rsrcdbmgr.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>

#include <pci/pci.h>

#include "private/hwmod_api.h"
#include "hw_lib.h"


/*
 ===============================================================================
 rsrcdb_as_free

 Un-reserve previously allocated or reserved address space.

 This function is only called by the PCI server. It will typically only be
 called when the PCI server exists however it could also be called in response
 to changing address space requirements due to the live insertion or removal of
 devices.

 Although it can be overridden in a target specific module, all attempts not to
 do this should be made even if it means tweaking this common HW dependent
 module API.

*/
__attribute__ ((visibility ("internal")))
pci_err_t rsrcdb_as_free(_pci_asmap_t *as_map)
{
	pci_err_t r = PCI_ERR_EINVAL;
	rsrc_request_t req = {0};

	switch(as_map->ba.type)
	{
		case pci_asType_e_MEM:
		{
			assert((as_map->ba.attr & pci_asAttr_e_ALIGN) >= pci_asAttr_e_ALIGN_MEM_MIN);

			req.flags = RSRCDBMGR_PCI_MEMORY;
			req.name = PCI_RSRC_NAME_MEM_ASPACE;

			r = PCI_ERR_OK;
			break;
		}
		case pci_asType_e_IO:
		{
			assert((as_map->ba.attr & pci_asAttr_e_ALIGN) >= pci_asAttr_e_ALIGN_IO_MIN);

			req.flags = RSRCDBMGR_PCI_MEMORY;
			req.name = PCI_RSRC_NAME_IO_ASPACE;

			r = PCI_ERR_OK;
			break;
		}
		default:
			break;
	}

	if (r == PCI_ERR_OK)
	{
		req.length = as_map->ba.size;
		req.flags |= (RSRCDBMGR_FLAG_RANGE| RSRCDBMGR_FLAG_ALIGN);
		req.start = as_map->ba.addr;
		req.end = as_map->ba.addr + as_map->ba.size - 1;
		req.align = (uint64_t)1 << (as_map->ba.attr & pci_asAttr_e_ALIGN);
		if (req.name != NULL) req.flags |= RSRCDBMGR_FLAG_NAME;

		/* unreserve the address space */
		if (rsrcdbmgr_detach(&req, 1) == -1) r = PCI_ERR_ASPACE_CFG;
	}
	return r;
}

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/services/pci/modules/hw/src/rsrcdb/rsrcdb_as_free.c $ $Rev: 798837 $")
#endif
