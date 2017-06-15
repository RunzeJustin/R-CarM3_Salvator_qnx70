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
 * @file	fdt_get_intr.c
 * @brief	Convinience functions on top of the standard FDT library.
 */

#include "libfdt_private.h"

/**
 * Extracts the values stored in an "interrupts" property within the given node.
 * @param	fdt			Device tree
 * @param	node		Node that holds the property
 * @param	index		For a multi-value property, the first value to use
 * @param	intr		Array of interrupt descriptors to fill
 * @param	num			Number of elements in the @intr array
 * @param	controller
 */
int fdt_get_intr(const void *fdt, int node, int index, fdtintr_t intr[], int num, int *controller) {
	int				parent;
	int				intr_cells;
	int				len;
	const fdt32_t	*intprop;
	int				i;
	int				r;

	// Check for "interrupts" first, if it's not there, don't do extra work.
	intprop = fdt_getprop(fdt, node, "interrupts", &len);
	if(intprop == NULL) {
		return len;
	}

	intr_cells = fdt_get_intr_cells(fdt, node, &parent);
	if(intr_cells < 0) {
		return intr_cells;
	}
	if(intr_cells >= FDT_MAX_NCELLS) {
		return -FDT_ERR_BADSTRUCTURE;
	}

	// @@@ interrupt controller, we don't support interrupt nexus "interrupt-map" yet
	if(!fdt_getprop(fdt, parent, "interrupt-controller", 0)) {
		return -FDT_ERR_BADSTRUCTURE;
	}
	if(controller) {
		*controller = parent;
	}

	if(len % (intr_cells * (int)(sizeof(*intprop))) != 0) {
		return -FDT_ERR_BADSTRUCTURE;
	}
	len /= intr_cells * (int)(sizeof(*intprop));
	len -= index;
	if(len <= 0) {
		return 0;
	}
	if(!intr) {
		return len;
	}

	intprop += index * intr_cells;
	for(i = 0; i < num; i++) {
		for(r = 0; r < intr_cells; r++) {
			if(i < len) {
				intr[i][r] = (int)fdt32_to_cpu(*intprop++);
			} else {
				intr[i][r] = 0;
			}
		}
		while(r < FDT_MAX_NCELLS) {
			intr[i][r++] = 0;
		}
	}
	if(num < len) {
		return num;
	}
	return len;
}

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/hardware/startup/lib/fdt_get_intr.c $ $Rev: 811485 $")
#endif
