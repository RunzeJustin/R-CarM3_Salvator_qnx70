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

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <ctype.h>

#include <pci/pci.h>
#include "hw_lib.h"
#include "hw_cfg.h"
#include "private/pci_slog.h"



static hw_cfg_t hw_cfg;

static pthread_once_t once = PTHREAD_ONCE_INIT;

/*
 ===============================================================================
 load_hwcfg_file

 This function is called once to attempt to open the HW configuration file
 identified by the environment variable PCI_HW_CONFIG_FILE. If the file exists,
 it is parsed and the 'hw_cfg' variable is initialized.

*/
static void load_hwcfg_file(void)
{
	const char * const cfg_fname = pci_getenv(PCI_ENVVAR_HW_CONFIG);
	if (cfg_fname != NULL)
	{
		memset(&hw_cfg, 0, sizeof(hw_cfg));
		parse_config_file(cfg_fname, &hw_cfg);
	}
}

/*
 ===============================================================================
 force_load_hwcfg_file

 Normally, a hardware configuration file is demand loaded (and parsed) when its
 needed. This is normally driven by the extcfg_*() entry functions below for
 handling of common configuration file parameters (ex. slots and interrupts).

 This function will allow loading (and parsing) of the HW configuration file
 to be triggered arbitrarily by the HW specific module, whether any common
 sections are needed or not.

 An example where this would be used is if there are hardware initialization
 parameters within a HW specific section of the configuration file which are
 needed when the HW specific module is first loaded. In this case we cannot wait
 for an external trigger from some other code path.

 This function would generally only ever need to be called if the HW specific
 module is installing its own section processing handler into the
 'process_section_hw_specific' function pointer and the above condition (or one
 similar) was true however it can also be used to provide override processing of
 the common HW sections.

*/
__attribute__ ((visibility ("internal")))
void force_load_hwcfg_file(void)
{
	pthread_once(&once, load_hwcfg_file);
}

/*
 ===============================================================================
 extcfg_intpin_to_irq

 This function will utilize the [interrupts] section of the HW configuration
 file (if one exists) for PCI Interrupt Pin to IRQ mapping.

 Upon a successful <bdf> and <pci_intpin_c> match this function returns the
 'pci_irq_t' corresponding to the IRQ field of the interrupt mapping section
 in the HW config file. HW specific modules may place their own interpretation
 on this field.

 <pci_intpin_c> must be a valid PCI pin based interrupt A, B, C, D to be
 included in the match criteria. It may also be '?' in order to be excluded from
 the match criteria. Any other value is invalid and will result in a failed
 return. Note also that the format of the configuration file allows for the
 'Pin' field to be '-'. In this case, <pci_intpin_c> will be ignored, This
 allows the 'Pin' field to be wildcarded from either the HW dependent module or
 the hardware configuration file

 -1 is returned if not match is found

*/
__attribute__ ((visibility ("internal")))
pci_irq_t extcfg_intpin_to_irq(pci_bdf_t bdf, const char _pci_intpin_c)
{
	pin_to_irq_map_t *p;
	bool_t match_any_intpin = false;
	char pci_intpin_c = _pci_intpin_c;

	if ((pci_intpin_c != '?') && ((pci_intpin_c < 'A') || (pci_intpin_c > 'D'))) return -1;
	else if (pci_intpin_c == '?') match_any_intpin = true;

	/* trigger a load of the HW config file (if its not already been done) */
	pthread_once(&once, load_hwcfg_file);

	p = hw_cfg.irq_list;
	while (p != NULL)
	{
		if ((match_any_intpin) || (p->intpin == '-')) pci_intpin_c = p->intpin;

		if ((p->bdf == bdf) && (p->intpin == pci_intpin_c)) return p->irq;
		else p = p->next;
	}
	return -1;		// not found
}

/*
 ===============================================================================
 extcfg_find_csd_assignment

 This function will utilize the [slots] section of the HW configuration file
 (if one exists) for chassis, slot and device number assignment and override

 Repeated calls to extcfg_find_csd_assignment() may be made in order to retrieve
 each of the assigned chassis, slot and device entries for <vid>, <did>, and
 <idx>. When no (additional) entries are found, PCI_CSD_NONE is returned.

 The first call to extcfg_find_csd_assignment() should be made with <csd> set to
 PCI_CSD_NONE. Subsequent calls should pass the returned 'pci_csd_t' (if not
 PCI_CSD_NONE) for <csd> in order to retrieve the next entry.

 If <vid>, <did> or <idx> values change then <csd> should be reset to
 PCI_CSD_NONE to start a new sequence

*/
__attribute__ ((visibility ("internal")))
pci_csd_t extcfg_find_csd_assignment(pci_csd_t csd, pci_vid_t vid, pci_did_t did, uint_t idx)
{
	csd_assignment_t *p;
	bool_t found_last = (csd == PCI_CSD_NONE) ? true : false;

	/* trigger a load of the HW config file (if its not already been done) */
	pthread_once(&once, load_hwcfg_file);

	p = hw_cfg.csd_list;
	while (p != NULL)
	{
		if ((p->vid == vid) && (p->did == did) && (p->idx == idx))
		{
			if (found_last) return p->csd;
			else if (csd == p->csd) found_last = true;
		}
		p = p->next;
	}
	return PCI_CSD_NONE;		// not found
}

/*
 ===============================================================================
 extcfg_check_aspace_filter

 This function will utilize the [aspace] section of the HW configuration file
 (if one exists) for address space region filtering.

 When syspage_load_asinfo() is called, it will add unused address space to the
 resource data base (rsrcdb) as determined from the 'asinfo' section of the
 syspage. This function will be called during that process to determine whether
 or not any regions to be added (as identified by <asmap>) overlap a filtered
 address space section (as described in the [aspace] section of the HW config
 file).

 'true' or 'false' is returned depending on whether an overlap exists.
 If <asmap_adjusted> != NULL, then it must point to an array of 2 '_pci_asmap_t'
 structures that will be set to contain the non overlapping range(s) of <asmap>.
 Two structures are required in case the filtered region lies entirely within
 <asmap> and a effective split is required. If the 'size' field of either
 <asmap_adjusted> entry is 0, then the entry should be ignored by the caller.

 Currently, the other fields of <asmap>->ba are not considered in the filter
 check however alignment probably should be

 There are 4 overlap scenarios

 1. the filtered range covers the entire <asmap>
    - return true
    - set asmap_adjusted[0].ba.size = 0
    - set asmap_adjusted[1].ba.size = 0

 2. the filtered range overlaps the start of <asmap>
    - return true
    - set asmap_adjusted[0].ba.addr = filter_end
    - set asmap_adjusted[0].ba.size = region_end - filter_end
    - set asmap_adjusted[1].ba.size = 0

 3. the filtered range overlaps the end of <asmap>
    - return true
    - set asmap_adjusted[0].ba.size = filter_start - region_start
    - set asmap_adjusted[1].ba.size = 0

 4. the filtered range lies within <asmap>
    - return true
    - set asmap_adjusted[0].ba.size = filter_start - region_start;
    - set asmap_adjusted[1].ba.addr = filter_end
    - set asmap_adjusted[1].ba.size = region_end - filter_end

*/
__attribute__ ((visibility ("internal")))
bool_t extcfg_check_aspace_filter(const _pci_asmap_t *asmap, _pci_asmap_t *asmap_adjusted)
{
	bool_t overlap = false;
	pci_ba_val_t region_start = asmap->ba.addr;
	pci_ba_val_t region_end = region_start + asmap->ba.size - 1;
	_pci_asmap_t _asmap_adjusted[2];
	const char *memType[] = {[pci_asType_e_MEM] = "MEM", [pci_asType_e_IO] = "IO"};

	/* trigger a load of the HW config file (if its not already been done) */
	pthread_once(&once, load_hwcfg_file);

	/* if required, point to local storage so we don't need to test each time */
	if (asmap_adjusted == NULL) asmap_adjusted = _asmap_adjusted;

	/* start with a full copy so we only need to adjust specific fields */
	asmap_adjusted[0] = *asmap;
	asmap_adjusted[1] = *asmap;

	aspace_filter_t *p = hw_cfg.aspace_filter_list;
	while (p != NULL)
	{
		if (asmap->ba.type == p->asmap.ba.type)
		{
			const pci_ba_val_t align = (uint64_t)1 << (p->asmap.ba.attr & pci_asAttr_e_ALIGN);
			const pci_ba_val_t filter_start = ROUND_DN(p->asmap.ba.addr, align);
			const pci_ba_val_t filter_end = ROUND_UP(filter_start + p->asmap.ba.size, align) - 1;

			/* the filtered range covers the entire <asmap> */
			if ((filter_start <= region_start) && (filter_end >= region_end))
			{
				overlap = true;
				asmap_adjusted[0].ba.size = 0;
				asmap_adjusted[1].ba.size = 0;

				slog_info(1, "\t%s filter %"PRIx64" -> %"PRIx64" overlaps entire region %"PRIx64" -> %"PRIx64"",
							memType[asmap->ba.type], filter_start, filter_end, region_start, region_end);
			}
			/* the filtered range overlaps the start of <asmap> */
			else if ((filter_start <= region_start) && (filter_end > region_start))
			{
				assert(filter_end < region_end);

				overlap = true;
				asmap_adjusted[0].ba.addr = filter_end + 1;
				asmap_adjusted[0].ba.size = region_end - filter_end;
				asmap_adjusted[1].ba.size = 0;

				slog_info(1, "\t%s filter %"PRIx64" -> %"PRIx64" overlaps start of region %"PRIx64" -> %"PRIx64""
							"\n\t\t\t\t\t\t\t\tadjusting to %"PRIx64" -> %"PRIx64"",
							memType[asmap->ba.type], filter_start, filter_end, region_start, region_end,
							asmap_adjusted[0].ba.addr, asmap_adjusted[0].ba.addr + asmap_adjusted[0].ba.size - 1);

				/* update the region start */
				region_start = asmap_adjusted[0].ba.addr;
			}
			/* the filtered range overlaps the end of <asmap> */
			else if ((filter_start < region_end) && (filter_end >= region_end))
			{
				assert(region_start < filter_start);

				overlap = true;
				asmap_adjusted[0].ba.size = filter_start - region_start;
				asmap_adjusted[1].ba.size = 0;

				slog_info(1, "\t%s filter %"PRIx64" -> %"PRIx64" overlaps end of region %"PRIx64" -> %"PRIx64""
							"\n\t\t\t\t\t\t\t\tadjusting to %"PRIx64" -> %"PRIx64"",
							memType[asmap->ba.type], filter_start, filter_end, region_start, region_end,
							asmap_adjusted[0].ba.addr, asmap_adjusted[0].ba.addr + asmap_adjusted[0].ba.size - 1);

				/* update the region start */
				region_end = region_start + asmap_adjusted[0].ba.size - 1;
			}
			/* the filtered range lies within <asmap> */
			else if ((filter_start > region_start) && (filter_end < region_end))
			{
				assert(region_start < filter_start);
				assert(filter_end < region_end);

				overlap = true;
				asmap_adjusted[0].ba.size = filter_start - region_start;
				asmap_adjusted[1].ba.addr = filter_end + 1;
				asmap_adjusted[1].ba.size = region_end - filter_end;

				slog_info(1, "\t%s filter %"PRIx64" -> %"PRIx64" lies within region %"PRIx64" -> %"PRIx64""
							"\n\t\t\t\t\t\t\t\tsplitting: section 1 %"PRIx64" -> %"PRIx64", section 2 %"PRIx64" -> %"PRIx64"",
							memType[asmap->ba.type], filter_start, filter_end, region_start, region_end,
							asmap_adjusted[0].ba.addr, asmap_adjusted[0].ba.addr + asmap_adjusted[0].ba.size - 1,
							asmap_adjusted[1].ba.addr, asmap_adjusted[1].ba.addr + asmap_adjusted[1].ba.size - 1);

				region_start = asmap_adjusted[0].ba.addr;
				region_end = asmap_adjusted[1].ba.addr + asmap_adjusted[1].ba.size -1;
			}
			else
			{
				slog_info(1, "\t%s filter %"PRIx64" -> %"PRIx64" does not affect region %"PRIx64" -> %"PRIx64"",
							memType[asmap->ba.type], filter_start, filter_end, region_start, region_end);
			}
		}
		else
		{
			slog_info(1, "\t%s filter does not apply to %s region", memType[p->asmap.ba.type], memType[asmap->ba.type]);
		}
		p = p->next;
	}
	return overlap;
}

/*
 ===============================================================================
 extcfg_check_rbar_override

 This function will utilize the [rbar] section of the HW configuration file
 (if one exists) for Resizable BARs and provide overrides for applicable devices

 If the [rbar] section does not exist, or the device identified by <bdf> cannot
 be found in the 'rbar_override_list', PCI_ERR_ENODEV will be returned.

 If the device <bdf> was found in the 'rbar_override_list' but an entry for
 <bar_num> could not be found, PCI_ERR_ENOENT will be returned.

*/
__attribute__ ((visibility ("internal")))
pci_err_t extcfg_check_rbar_override(const pci_bdf_t bdf, const uint_t bar_num, pci_ba_val_t *size, uintptr_t extra)
{
	/* trigger a load of the HW config file (if its not already been done) */
	pthread_once(&once, load_hwcfg_file);

	pci_err_t r = PCI_ERR_ENODEV;
	rbar_override_t *p = hw_cfg.rbar_override_list;

	if (p != NULL)
	{
		pci_vid_t vid = PCI_VID_ANY;
		pci_did_t did = PCI_DID_ANY;
		r = pci_device_read_vid(bdf, &vid);

		if (r == PCI_ERR_OK) r = pci_device_read_did(bdf, &did);
		if (r == PCI_ERR_OK)
		{
			pci_bdf_t bdf_found;
			uint_t idx = 0;

			/* find what index of vid/did <bdf> is */
			r = PCI_ERR_ENODEV;
			while ((bdf_found = pci_device_find(idx, vid, did, PCI_CCODE_ANY)) != PCI_BDF_NONE)
			{
				if (bdf_found == bdf)
				{
					r = PCI_ERR_OK;
					break;
				}
				else ++idx;
			}

			if (r == PCI_ERR_OK)
			{
				r = PCI_ERR_ENODEV;
				while (p != NULL)
				{
					if ((p->vid == vid) && (p->did == did) && (p->idx == idx))
					{
						r = PCI_ERR_ENOENT;

						uint_t i;
						for (i=0; i<NELEMENTS(p->bar); i++)
						{
							if (p->bar[i].num == bar_num)
							{
								*size = p->bar[i].size;
								r = PCI_ERR_OK;
								break;
							}
						}
						if (r == PCI_ERR_OK) break;
					}
					p = p->next;
				}
			}
		}
	}
	return r;
}

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/services/pci/modules/hw/src/hw_cfg.c $ $Rev: 811187 $")
#endif
