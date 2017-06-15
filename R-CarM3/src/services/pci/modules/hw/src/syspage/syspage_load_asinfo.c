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

typedef struct
{
	uint64_t start;
	uint64_t end;
} range_entry_t;

static void _syspage_load_asinfo(uint_t type);

/*
 ===============================================================================
 syspage_load_asinfo

 This function is called once to read the system page address space information
 and place it into the rsrcdb so that PCI address spaces can be
 allocated/released.

 What it will do is look at all the system page ASINFO entries and determine
 where the holes are and make that space available for use for mapping PCI
 device memory. This memory is the available PCI address space from the CPU's
 perspective (ie. it is the translated outbound PCI address space).

*/
__attribute__ ((visibility ("internal")))
void syspage_load_asinfo(void)
{
	if (pci_runtime_flags & pci_runtime_flags_e_SERVER)
	{
		_syspage_load_asinfo(RSRCDBMGR_PCI_MEMORY);
		_syspage_load_asinfo(RSRCDBMGR_IO_PORT);
	}
}


/*
 ===============================================================================
 find_root_owner

 Traverse the 'owner' (ie. parent) linkage of <entry> until the root entry is
 found.

*/
static struct asinfo_entry *as_off2info(unsigned off)
{
	struct asinfo_entry *first_entry = SYSPAGE_ENTRY(asinfo);
	return((struct asinfo_entry *)((uintptr_t)first_entry + off));
}

static struct asinfo_entry *find_root_owner(struct asinfo_entry *entry)
{
	while (entry != NULL)
	{
		if (entry->owner == AS_NULL_OFF) return entry;
		else entry = as_off2info(entry->owner);
	}
	return NULL;
}

/*
 ===============================================================================
 remove_range

 Given the <range> list, remove the section from <start> to <end>. This may
 require splitting/collapsing the range list

*/
static uint_t remove_range(range_entry_t **range, uint_t num_range_entries, uint64_t start, uint64_t end)
{
	uint_t i;

	for (i=0; i<num_range_entries; i++)
	{
		range_entry_t *range_p = &(*range)[i];

		if ((start > range_p->start) && (end < range_p->end))
		{
			*range = realloc(*range, ++num_range_entries * sizeof(*range_p));
			range_p = &(*range)[i];
			memmove(range_p + 1, range_p, (num_range_entries - 1 - i) * sizeof(*range_p));
			(range_p + 1)->start = end + 1;
			(range_p + 1)->end = range_p->end;
			range_p->end = start - 1;
			break;
		}
		else if ((start == range_p->start) && (end == range_p->end))
		{
			memmove(range_p, range_p + 1, (num_range_entries - 1 - i) * sizeof(*range_p));
			*range = realloc((*range), --num_range_entries * sizeof(*range_p));
			break;
		}
		else if ((start == range_p->start) && (end < range_p->end))
		{
			range_p->start = end + 1;
			break;
		}
		else if ((start > range_p->start) && (end == range_p->end))
		{
			range_p->end = start - 1;
			break;
		}
	}
	return num_range_entries;
}

/*
 ===============================================================================
 _syspage_load_asinfo

*/
static void _syspage_load_asinfo(uint_t type)
{
	struct asinfo_entry *entry = SYSPAGE_ENTRY(asinfo);
	const uint_t num_entries = SYSPAGE_ENTRY_SIZE(asinfo) / sizeof(*entry);
	uint_t num_range_entries = 1;
	range_entry_t *range = calloc(num_range_entries, sizeof(*range));
	const char * const asinfo_name = (type == RSRCDBMGR_PCI_MEMORY) ? "memory" : "io";

	assert((type == RSRCDBMGR_PCI_MEMORY) || (type == RSRCDBMGR_IO_PORT));

	if (range != NULL)
	{
		uint_t i;
		struct asinfo_entry *base_entry = NULL;

		/* find the entry which describes the range of addressable address space */
		for (i=0; i<num_entries; i++)
		{
			const char * const name = __hwi_find_string (entry[i].name);
			if ((name != NULL) && (strcmp(name, asinfo_name) == 0) && (entry[i].owner == AS_NULL_OFF))
			{
				base_entry = &entry[i];
				range->start = base_entry->start;
				range->end = base_entry->end;
				break;
			}
		}
		if ((base_entry == NULL) || (range->end == 0)) return;	// the entry was not found or is invalid

		/* now, for each entry (excluding the base_entry), remove from range_min/max, the occupied space */
		for (i=0; i<num_entries; i++)
		{
			const char * const name = __hwi_find_string (entry[i].name);
			if (&entry[i] == base_entry) continue;
			else if ((name == NULL) || (strstr(name, "ram") == NULL)) continue;	// ignore anything not "ram"
			/* we only remove entries rooted by 'base_entry' */
			else if (find_root_owner(&entry[i]) == base_entry)
			{
				/* this entry represents an occupied address space of the 'base_entry' so remove it */
				num_range_entries = remove_range(&range, num_range_entries, entry[i].start, entry[i].end);
			}
		}

		/* add all of the ranges to the rsrcdb */
		for (i=0; i<num_range_entries; i++)
		{
			_pci_asmap_t as_map =
			{
				.ba.addr = range[i].start,
				.ba.size = range[i].end - range[i].start + 1,
				.ba.type = (type == RSRCDBMGR_PCI_MEMORY) ? pci_asType_e_MEM : pci_asType_e_IO,
				.ba.attr = ilog2(range[i].end - range[i].start + 1),
			};
			/* set attributes */
			if (as_map.ba.type == pci_asType_e_MEM)
			{
				if (as_map.ba.addr > UINT32_MAX) as_map.ba.attr |= pci_asAttr_e_64BIT;
				else as_map.ba.attr |= pci_asAttr_e_32BIT;
			}
			else if (as_map.ba.type == pci_asType_e_IO)
			{
				if (as_map.ba.addr > UINT32_MAX) as_map.ba.attr |= pci_asAttr_e_64BIT;
				else as_map.ba.attr |= pci_asAttr_e_32BIT;
			}
			rsrcdb_as_add(&as_map);
		}
		free(range);
	}
}

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/services/pci/modules/hw/src/syspage/syspage_load_asinfo.c $ $Rev: 798837 $")
#endif
