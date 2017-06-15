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
 * @file	fdt_find_node.c
 * @brief	Convinience functions on top of the standard FDT library.
 */

#include "libfdt_private.h"

/**
 * Finds a node matching a device type property compatible string.
 * @param	fdt			Device tree
 * @param	node		The node to start the scan from
 * @param	type		Value of a "device_type" property to match, NULL to
 * 						accept any
 * @param	compatible	Compatible string to match, NULL to accept any
 * @return	The first node matching the given criteria if found, negative error
 * 			value otherwise
 */
int fdt_find_node(const void *fdt, int node, const char *type, const char *compatible) {
	if(!compatible) {
		// Added for completeness.
		// The function does not expect both type and compatible to be NULL, but
		// if they are, just return the next node to adhere to the established
		// semantics.
		if(!type) {
			return fdt_next_node(fdt, node, NULL);
		}
		return fdt_node_offset_by_prop_value(fdt, node, "device_type", type, strlen(type) + 1);
	}

	for (;;) {
		node = fdt_node_offset_by_compatible(fdt, node, compatible);
		if (node < 0) {
			break;
		}

		if (type == NULL) {
			// No need to check device_type, node matches.
			break;
		}

		const char	*p;
		const int	r = fdt_get_str(fdt, node, "device_type", &p);
		if (r == -FDT_ERR_NOTFOUND) {
			continue;
		}

		if (r < 0) {
			return r;
		}

		if (strcasecmp(p, type) == 0) {
			break;
		}
	}

	return node;
}


#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/hardware/startup/lib/fdt_find_node.c $ $Rev: 811485 $")
#endif
