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
 * @file	fdt_get_str.c
 * @brief	Convinience functions on top of the standard FDT library.
 */

#include "libfdt_private.h"

/**
 * Extracts a string value from a property.
 * The string is verified to be well-formed.
 * @param	fdt		Device tree
 * @param	node	Node ID
 * @param	name	Property name
 * @param	pp		Holds a pointer to the string, upon successful return
 * @return	0 if successful, negative error code otherwise
 */
int fdt_get_str(const void *fdt, int node, const char *name, const char **pp) {
	int				len;
	const char		*p;

	p = fdt_getprop(fdt, node, name, &len);
	if(p == NULL) {
		return len;
	}
	if((len < 1) || (p[len-1] != '\0')) {
		return -FDT_ERR_BADSTRUCTURE;
	}
	*pp = p;
	return 0;
}

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/hardware/startup/lib/fdt_get_str.c $ $Rev: 811485 $")
#endif
