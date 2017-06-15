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
 * @file	fdt_get_reg64_cells.c
 * @brief	Convinience functions on top of the standard FDT library.
 */

#include "libfdt_private.h"

/**
 * Extracts a list of "reg" property values under a given node.
 * The reg property may accept multiple address,size pairs within a single
 * value, e.g.,
 *     reg = <0x40000000 0x1000 0x48000000 0x1000>;
 * which, for address-cells and size-cells values of 1 represents an array of
 * two pairs.
 * @param	fdt			Device tree
 * @param	node		The node to look under
 * @param	index		The first value in the array to extract
 * @param	num			Number of values to extract
 * @param	addr		Array of addresses to fill, NULL to just determine
 * 						number of elements in the property's value array
 * @param	size		Array of sizes to fill
 * @param	addr_cells	Number of 32-bit cells in an address (-1 to use node
 * 						setting)
 * @param	size_cells	Number of 32-bit cells in a size (-1 to use node
 * 						setting)
 * @return	Number of extracted elements in the value array, if successful,
 * 			negative error code otherwise
 */
int fdt_get_reg64_cells(const void *fdt, int node, int index, int num,
						uint64_t *addr, uint64_t *size, int addr_cells, int size_cells) {
	int				len;
	const fdt32_t	*regprop;
	int				ent_cells;
	int				entlen;

	// Find "reg" property in node
	regprop = fdt_getprop(fdt, node, "reg", &len);
	if(regprop == NULL) {
		return len;
	}

	// Get the number of 32 bit cells for address and size
	if(addr_cells == -1) {
		// Finding parent requires scanning the complete device tree
		const int	parent = fdt_parent_offset(fdt, node);
		if(parent < 0) {
			return parent;
		}
		addr_cells = fdt_address_cells(fdt, parent);
		size_cells = fdt_size_cells(fdt, parent);
	}

	// Validate the number of 32 bit cells for address and size
	if((addr_cells < 1) || (addr_cells > 2)) {
		return -FDT_ERR_BADNCELLS;
	}
	if((size_cells < 0) || (size_cells > 2)) {
		return -FDT_ERR_BADNCELLS;
	}
	ent_cells = addr_cells + size_cells;

	// Calculate number of entries
	entlen = ent_cells * (int)(sizeof(*regprop));
	if(len % entlen) {
		return -FDT_ERR_BADOFFSET;
	}
	len /= entlen;
	if((index < 0) || (index >= len)) {
		return -FDT_ERR_NOTFOUND;
	}
	regprop += index * ent_cells;
	len -= index;
	if(!addr) {
		return len;
	}

	if((size_cells == 0) != (size == 0)) {
		return -FDT_ERR_BADNCELLS;
	}

	if(num > len) {
		num = len;
	}
	while(num-- > 0) {
		union {
			fdt32_t	cells[2];
			fdt64_t	val;
		} temp;

		// Get reg entries
		switch(addr_cells) {
		case 1:
			*addr = fdt32_to_cpu(regprop[0]);
			break;
		case 2:
			// Extract as two moves to avoid alignment problems
			temp.cells[0] = regprop[0];
			temp.cells[1] = regprop[1];
			*addr = fdt64_to_cpu(temp.val);
			break;
		default:
			// Never reached, addr_cells were checked above and can only be 1 or
			// 2.
			break;
		}
		regprop += addr_cells;
		addr++;

		if(size) {
			switch(size_cells) {
			case 0:
				break;
			case 1:
				*size = fdt32_to_cpu(*((fdt32_t *)regprop));
				break;
			case 2:
				// Extract as two moves to avoid alignment problems
				temp.cells[0] = regprop[0];
				temp.cells[1] = regprop[1];
				*size = fdt64_to_cpu(temp.val);
				break;
			default:
				// Never reached, size_cells were checked above and can only be
				// 0, 1 or 2.
				break;
			}
			regprop += size_cells;
			size++;
		}
	}

	return len;
}

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/hardware/startup/lib/fdt_get_reg64_cells.c $ $Rev: 811485 $")
#endif
