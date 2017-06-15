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
 * @file	fdt_get_int.c
 * @brief	Convinience functions on top of the standard FDT library.
 */

#include <limits.h>
#include "libfdt_private.h"

/**
 * Extracts a signed 32-bit value from a property.
 * The @offset parameter is typically 0. If the property holds a value that is
 * bigger than 32 bits, this paramater can be used to access the next 32-bit
 * word of that value, by feeding the function the result of a previous call.
 * @param	fdt		Device tree
 * @param	node	Node ID
 * @paran	offset	Number of bytes into the value to look at
 * @param	name	Property name
 * @param	pp		Holds the value, upon successful return
 * @return	0 if successful and the value is 32-bit wide
 * 			>0 if successful and the value is more than 32-bits wide, in which
 * 			   case the result can be used as the offset in a subsequent call
 * 			<0 on error
 */
int fdt_get_int(const void *fdt, int node, int offset, const char *name, int *pp) {
#if INT_MAX == INT32_MAX
	return fdt_get_u32(fdt, node, offset, name, (uint32_t *)pp);
#elif INT_MAX == INT64_MAX
	int32_t				val;

	const int	r = fdt_get_u32(fdt, node, offset, name, (uint32_t *)&val);

	if(r >= 0) {
		*pp = val;
	}
	return r;
#else
#error INT_MAX not 32 or 64
#endif
}

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/hardware/startup/lib/fdt_get_int.c $ $Rev: 811485 $")
#endif
