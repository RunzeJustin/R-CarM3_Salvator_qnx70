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
 find_vec_entry

 Starting from the intrinfo entry AFTER <start>, locate the intrinfo_entry
 associated with <vector> using <type> to qualify the search.

 Since mutliple entries may qualify as a match, the caller can do additional
 checks on the entry and pass the returned value as <start> for additional
 searches. NULL can be passed to (re)start the search from the beginning of the
 intrinfo section.

 Returns the entry on success or NULL is not found

*/
__attribute__ ((visibility ("internal")))
struct intrinfo_entry *find_vec_entry(struct intrinfo_entry *start, const uint_t vector, const intrinfo_entry_type_e type)
{
	struct intrinfo_entry *entry = SYSPAGE_ENTRY(intrinfo);

	if (entry != NULL)
	{
		const uint_t start_entry_idx = (start == NULL) ? 0 : (start - entry) + 1;
		const uint_t num_entries = SYSPAGE_ENTRY_SIZE(intrinfo) / sizeof(*entry);
		uint_t i;

		for (i=start_entry_idx; i<num_entries; i++)
		{
			if (entry[i].num_vectors > 0)
			{
				switch (type)
				{
					case intrinfo_entry_type_e_BASE:
					{
						 if (vector == entry[i].vector_base) return &entry[i];
						 else break;
					}
					case intrinfo_entry_type_e_BASE_RANGE:
					{
						const uint_t first_vec = entry[i].vector_base;
						const uint_t last_vec = first_vec + entry[i].num_vectors - 1;

						if ((vector >= first_vec) && (vector <= last_vec)) return &entry[i];
						else break;
					}
					case intrinfo_entry_type_e_CPU:
					{
						 if (vector == entry[i].cpu_intr_base) return &entry[i];
						 else break;
					}
					case intrinfo_entry_type_e_CPU_RANGE:
					{
						const uint_t first_vec = entry[i].cpu_intr_base;
						const uint_t last_vec = first_vec + entry[i].num_vectors - 1;

						if ((vector >= first_vec) && (vector <= last_vec)) return &entry[i];
						else break;
					}
					case intrinfo_entry_type_e_CASCADE:
					{
						 if (vector == entry[i].cascade_vector) return &entry[i];
						 else break;
					}
					default:
						break;
				}
			}
		}
	}
	return NULL;
}

/*
 ===============================================================================
 find_vector

 This function will translate the system defined <vector> into the startup
 assigned IRQ. Message based vectors (ie. MSI's) are flagged with INTR_FLAG_MSI

 The system page intrinfo entries are searched for the entry which contains
 <vector> and if found, the corresponding IRQ is returned.

 Note that the current assumption is that the 'cpu_intr_stride' field of the
 entry is 1 (we don't take it into account).

 Return: on success, the IRQ associated with <vector> otherwise -1
*/
__attribute__ ((visibility ("internal")))
int find_vector(uint_t vector, _pci_irqType_e irq_type)
{
	if (irq_type == _pci_irqType_e_MSG)
	{
		struct intrinfo_entry *entry = NULL;

		do
		{
			entry = find_vec_entry(entry, vector, intrinfo_entry_type_e_CPU_RANGE);

			if ((entry != NULL) && ((entry->flags & INTR_FLAG_MSI) != 0))
			{
				return entry->vector_base + (vector - entry->cpu_intr_base);
			}
		} while (entry != NULL);
	}
	return -1;
}

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/services/pci/modules/hw/src/syspage/syspage_intrinfo.c $ $Rev: 798837 $")
#endif
