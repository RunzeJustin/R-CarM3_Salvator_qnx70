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

#include "startup.h"
#include <libfdt.h>
#include <libfdt_private.h>

#define REG_CHUNK	8
int
init_raminfo_fdt(void) {
    if(fdt == NULL) {
		return 0;
	}

	// Find memory node and add ram
	int node = fdt_node_offset_by_prop_value(fdt, -1, "device_type", "memory", sizeof("memory"));
	if(node < 0) {
		return 0;
	}
	int idx = 0;
	for(;;) {
		uint64_t	addr[REG_CHUNK], size[REG_CHUNK];
		int			i, nreg;

		nreg = fdt_get_reg64_cells(fdt, node, idx, REG_CHUNK, addr, size, -1, -1);
		if(nreg <= 0) {
			break;
		}
		for(i = 0; i < nreg; i++) {
	        add_ram(addr[i], size[i]);
		}
		idx += nreg;
	}

	// alloc reserved memory
	int nrsv = fdt_num_mem_rsv(fdt);
	while(nrsv-- > 0) {
		uint64_t		start, size;

		if(fdt_get_mem_rsv(fdt, nrsv, &start, &size) >= 0) {
		    alloc_ram(start, size, 1);
		}
	}
	return 1;
}

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/hardware/startup/lib/init_raminfo_fdt.c $ $Rev: 810394 $")
#endif
