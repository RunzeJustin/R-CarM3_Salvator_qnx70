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
 * @file	fdt_get_reg64fv.c
 * @brief	Convinience functions on top of the standard FDT library.
 */

#include "libfdt_private.h"

/**
 * Extracts the value of a reg property into the given 64-bit address and size
 * pointers.
 * @param	fdt		Device tree
 * @param	node	The node to look for the reg propery under
 * @param	index	For a multiple value property, the index of the value to
 * 					extract
 * @param	addr	Holds the address portion of the value, upon successful return
 * @param	size	Holds the size portion of the value, upon successful return
 * @return	0 if addr is non-NULL and the function is successful
 * 			>=0 if addr is NULL, in which case the returned value is the number
 * 			of values associated with the property
 * 			<0 in case of an error
 */
int fdt_get_reg64(const void *fdt, int node, int index, uint64_t *addr, uint64_t *size) {
	const int	r = fdt_get_reg64_cells(fdt, node, index, 1, addr, size, -1, -1);

	if(r < 0) {
		return r;
	}
	return addr ? 0 : r;
}

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/hardware/startup/lib/fdt_get_reg64.c $ $Rev: 811485 $")
#endif
