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

#include "private/hwmod_api.h"
#include "hw_lib.h"
#include "private/pci_slog.h"


/*
 ===============================================================================
 syspage_load_intrinfo

 This function is called once to read the system page interrupt information and
 place it into the rsrcdb so that interrupt vectors can be allocated/released.

 The startup specified MSI vectors to use will be indicated in the system page
 intrinfo_entry->flags field by the inclusion of the INTR_FLAG_MSI bit

 We place the message interrupt vectors the rsrcdb so that we can use the length
 and alignment request attributes to allocate a suitable block of vectors (as
 required by MSI for example). The vectors are mapped back to IRQ's by
 find_vector()

 This function is not required if the IRQ information will be obtained from an
 alternate source

*/
__attribute__ ((visibility ("internal")))
void syspage_load_intrinfo(void)
{
	if (pci_runtime_flags & pci_runtime_flags_e_SERVER)
	{
		struct intrinfo_entry *entry = SYSPAGE_ENTRY(intrinfo);
		const uint_t num_entries = SYSPAGE_ENTRY_SIZE(intrinfo) / sizeof(*entry);

		if (entry != NULL)
		{
			uint_t i;

			for (i=0; i<num_entries; i++)
			{
				if (entry[i].cpu_intr_stride > 1) continue;	// we don't handle strides > 1. Ok if its not set

				if ((entry[i].num_vectors > 0) && (entry[i].flags & INTR_FLAG_MSI))
				{
					rsrc_alloc_t irq_item;
					int r;

					slog_info(2, "RSRCDB: +++ Adding type %u resources +++", RSRCDBMGR_IRQ);

					irq_item.start = entry[i].cpu_intr_base;
					irq_item.end = irq_item.start + entry[i].num_vectors - 1;
					irq_item.flags = RSRCDBMGR_IRQ | RSRCDBMGR_FLAG_NOREMOVE;
					irq_item.name = PCI_RSRC_NAME_MSGVEC;
					if (irq_item.name != NULL) irq_item.flags |= RSRCDBMGR_FLAG_NAME;

					slog_info(2, "RSRCDB: type %u (%s), %"PRIx64" to %"PRIx64", flags: %x",
								RSRCDBMGR_IRQ, (irq_item.name == NULL) ? "" : irq_item.name,
								irq_item.start, irq_item.end, irq_item.flags);

					r = rsrcdbmgr_create(&irq_item, 1);

					if (r == EOK) slog_info(2, "RSRCDB: +++ Resources added successfully +++");
					else slog_error(0, "RSRCDB: +++ Resources not added, errno: %u +++", errno);
				}
			}
		}
	}
}

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/services/pci/modules/hw/src/syspage/syspage_load_intrinfo.c $ $Rev: 798837 $")
#endif
