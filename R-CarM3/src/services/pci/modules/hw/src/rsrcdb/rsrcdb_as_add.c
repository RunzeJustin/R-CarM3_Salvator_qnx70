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
#include "private/pci_slog.h"

/*
 ===============================================================================
 rsrcdb_as_add

*/
static pci_err_t _rsrcdb_as_add(const _pci_asmap_t * const as_map)
{
	rsrc_alloc_t as_item;
	int r;
//	const uint_t type = (as_map->ba.type == pci_asType_e_MEM) ? RSRCDBMGR_PCI_MEMORY : RSRCDBMGR_IO_PORT;
	const uint_t type = RSRCDBMGR_PCI_MEMORY;

	memset(&as_item, 0, sizeof(as_item));

	slog_info(1, "RSRCDB: +++ Adding type %u resource +++", type);

	as_item.start = as_map->ba.addr;
	as_item.end = as_map->ba.addr + as_map->ba.size - 1;
	as_item.flags = type | RSRCDBMGR_FLAG_NOREMOVE;
	as_item.name = (as_map->ba.type == pci_asType_e_MEM) ? PCI_RSRC_NAME_MEM_ASPACE : PCI_RSRC_NAME_IO_ASPACE;
	if (as_item.name != NULL) as_item.flags |= RSRCDBMGR_FLAG_NAME;

	slog_info(2, "RSRCDB: type %u (%s), %"PRIx64" to %"PRIx64", flags: %x",
					type, (as_item.name == NULL) ? "" : as_item.name,
					as_item.start, as_item.end, as_item.flags);

	r = rsrcdbmgr_create(&as_item, 1);

	if (r == EOK) slog_info(1, "RSRCDB: +++ Resources added successfully +++");
	else slog_error(0, "RSRCDB: +++ Resources not added, errno: %u +++", errno);

	return (r == EOK) ? PCI_ERR_OK : PCI_ERR_ASPACE_INVALID;
}

__attribute__ ((visibility ("internal")))
pci_err_t rsrcdb_as_add(const _pci_asmap_t * const as_map)
{
	pci_err_t r;
	_pci_asmap_t asmap_adjusted[2];

	if (!extcfg_check_aspace_filter(as_map, asmap_adjusted)) r = _rsrcdb_as_add(as_map);
	else
	{
		pci_err_t r1 = PCI_ERR_OK, r2 = PCI_ERR_OK;
		if (asmap_adjusted[0].ba.size > 0) r1 = _rsrcdb_as_add(&asmap_adjusted[0]);
		if (asmap_adjusted[1].ba.size > 0) r2 = _rsrcdb_as_add(&asmap_adjusted[1]);

		r = (r1 != PCI_ERR_OK) ? r1 : r2;
	}
	return r;
}


#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/services/pci/modules/hw/src/rsrcdb/rsrcdb_as_add.c $ $Rev: 798837 $")
#endif
