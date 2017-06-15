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
#include <sys/syspage.h>
#include <sys/rsrcdbmgr.h>
#include <sys/rsrcdbmsg.h>
#include <string.h>
#include <hw/sysinfo.h>
#include <assert.h>

#include "private/hwmod_api.h"
#include "hw_lib.h"
#include "private/pci_slog.h"
#include "private/pci_debug.h"


/*
 ===============================================================================
 syspage_get_ram_info

 This function is called once to obtain the address space information for the
 RAM installed in the system. The information is obtained from the system page
 'asinfo' entries.

 The <as_ram> parameter is filled in with information which can be used to set
 up inbound address translation hardware for transfers from device to memory

 The 'asinfo' entries will be checked to find the root 'ram' entry which should
 contain the address range for all of system RAM

 Note that we don't care if there are discontiguous regions, they will be
 treated as contiguous for the purpose of the range of RAM. This may need to be
 revisited in the future.

*/
__attribute__ ((visibility ("internal")))
pci_err_t syspage_get_ram_info(_pci_asmap_t *as_ram)
{
	struct asinfo_entry *entry = SYSPAGE_ENTRY(asinfo);
	const uint_t num_entries = SYSPAGE_ENTRY_SIZE(asinfo) / sizeof(*entry);
	pci_err_t r = PCI_ERR_ENOMEM;
	uint_t i;
	uint64_t start = UINT64_MAX;
	uint64_t end = 0;

	for (i=0; i<num_entries; i++)
	{
		const char * const name = __hwi_find_string (entry[i].name);

		if ((name != NULL) && (strcmp(name, "sysram") == 0))
		{
			/* found some */
			if (entry[i].start < start) start = entry[i].start;
			if (entry[i].end > end) end = entry[i].end;

			r = PCI_ERR_OK;
		}
	}
	if (r == PCI_ERR_OK)
	{
		as_ram->ba.addr = start;
		as_ram->ba.size = end - start + 1;
		as_ram->ba.type = pci_asType_e_MEM;
		as_ram->ba.attr = pci_asAttr_e_ALIGN_MEM_MIN |
						  pci_asAttr_e_PREFETCH |
						  pci_asAttr_e_CONTIG |
						  pci_asAttr_e_ENABLED;
		if ((as_ram->ba.addr > UINT32_MAX) || ((as_ram->ba.addr + as_ram->ba.size) > UINT32_MAX)) {
			as_ram->ba.attr |= pci_asAttr_e_64BIT;
		} else {
			as_ram->ba.attr |= pci_asAttr_e_32BIT;
		}
		slog_info(0, "%s() RAM found from %"PRIx64" - %"PRIx64" (0x%"PRIx64" bytes)", __FUNCTION__,
				as_ram->ba.addr, as_ram->ba.addr + as_ram->ba.size - 1, as_ram->ba.size);
	}
	return r;
}

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/services/pci/modules/hw/src/syspage/syspage_get_ram_info.c $ $Rev: 798837 $")
#endif
