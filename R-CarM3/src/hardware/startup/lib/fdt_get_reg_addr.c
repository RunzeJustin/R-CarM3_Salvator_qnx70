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
 * @file	fdt_get_reg_addr.c
 * @brief	Convinience functions on top of the standard FDT library.
 */

#include <stdlib.h>
#include <limits.h>
#include "libfdt_private.h"

/**
 * Extracts a value for the given node from either a reg property or its name.
 * If a reg proprety exists, the first value in that property is
 * used. Otherwise, if the name of the node is of the form 'str@num' then the
 * numeric part of the name is used.
 * @param	fdt		Device tree
 * @param	node	The node to extract a value for
 * @param	addr	Holds the extracted value upon successful return
 * @return	0 if successful, negative error code otherwise
 */
int fdt_get_reg_addr(const void *fdt, int node, unsigned *addr) {
	int				r;
	uint64_t		addr64;

	r = fdt_get_reg64_cells(fdt, node, 0, 1, &addr64, 0, -1, -1);
	if(r < 0) {
		const char			*name;
		char				*p;
		int					len;

		if(r != -FDT_ERR_NOTFOUND) {
			return r;
		}

		if(!(name = fdt_get_name(fdt, node, &len))) {
			return len;
		}
		if(!(name = memchr(name, '@', len))) {
			return r;
		}
		addr64 = strtoull(++name, &p, 16);
		if(*p) {
			return r;
		}
		r = 0;
	}
	if(r >= 0) {
		if(addr64 > UINT_MAX) {
			r = -FDT_ERR_BADSTRUCTURE;
		} else {
			*addr = (unsigned)addr64;
		}
	}
	return r;
}

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/hardware/startup/lib/fdt_get_reg_addr.c $ $Rev: 811485 $")
#endif
