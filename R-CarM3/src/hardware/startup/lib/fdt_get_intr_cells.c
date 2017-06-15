/*
 * $QNXLicenseC:
 * Copyright 2016, QNX Software Systems.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"). You
 * may not reproduce, modify or distribute this software except in
 * compliance with the License. You may obtain a copy of the License
 * at: http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTIES OF ANY KIND, either express or implied.
 *
 * This file may contain contributions from others, either as
 * contributors under the License or as licensors under other terms.
 * Please review this entire file for other proprietary rights or license
 * notices, as well as the QNX Development Suite License Guide at
 * http://licensing.qnx.com/license-guide/ for other information.
 * $
 */

/**
 * @file	fdt_get_intr_cells.c
 * @brief	Convinience functions on top of the standard FDT library.
 */

#include "libfdt_private.h"

#define CACHE_PARENT	// Simple caching of interrupt parent. not multi-thread safe

/**
 * Determines the number of 32-bit cells in an "interrupts" property within a
 * given node.
 * @warning	Not thread-safe!
 * @param	fdt		Device tree
 * @param	node	The node holding the property
 * @param	parent	Holds the parent node, upon successful return
 * @return	Number of cells in the property value if successful, negative error
 * 			code otherwise
 */
int fdt_get_intr_cells(const void *fdt, int node, int *parent) {
	uint32_t		phandle;
	int				intr_cells;
	int				r;
#ifdef CACHE_PARENT
	static const void 	*cache_fdt;
	static uint32_t 	cache_phandle;
	static int 			cache_parent;
	static int 			cache_intr_cells;
#endif

	for(;;) {
		r = fdt_get_u32(fdt, node, 0, "interrupt-parent", &phandle);
		if(r == 0) {
#ifdef CACHE_PARENT
			if((cache_fdt == fdt) && (phandle == cache_phandle)) {
				if(parent) {
					*parent = cache_parent;
				}
				return cache_intr_cells;
			}
#endif
			// Lookup phandle to find parent
			r = fdt_node_offset_by_phandle(fdt, phandle);
		} else {
			if(r == -FDT_ERR_NOTFOUND) {
#ifdef CACHE_PARENT
				phandle = (uint32_t)-1;
#endif
				// Use device tree parent
				r = fdt_parent_offset(fdt, node);
			}
		}
		if(r < 0) {
			break;
		}
		node = r;
		r = fdt_get_int(fdt, node, 0, "#interrupt-cells", &intr_cells);
		if(r >= 0) {
			if((r != 0) || (intr_cells <= 0) || (intr_cells > FDT_MAX_NCELLS)) {
				return -FDT_ERR_BADNCELLS;
			}
#ifdef CACHE_PARENT
			if(phandle != (uint32_t)-1) {
				cache_fdt = fdt;
				cache_phandle = phandle;
				cache_parent = node;
				cache_intr_cells = intr_cells;
			}
#endif
			if(parent) {
				*parent = node;
			}
			return intr_cells;
		}
		if(r != -FDT_ERR_NOTFOUND) {
			break;
		}
		// The ePAPR spec requires #interrupt-cells in the parent, but broken device trees
		// require checking the parent.
	}
	return r;
}

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/hardware/startup/lib/fdt_get_intr_cells.c $ $Rev: 811485 $")
#endif
