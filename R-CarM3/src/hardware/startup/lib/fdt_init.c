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

void
fdt_init(paddr_t paddr) {
	void	*header = 0;

	fdt_paddr = paddr;

	header = startup_memory_map(sizeof(struct fdt_header), fdt_paddr, PROT_READ);

	if(fdt_check_header(header) == 0) {
		fdt_size = fdt_totalsize(header);
	}

	startup_memory_unmap(header);

	if(fdt_size != 0) {
		fdt = startup_memory_map(fdt_size, fdt_paddr, PROT_READ);
	}
}


void
fdt_asinfo(void) {
	if(fdt != NULL) {
		as_add_containing(fdt_paddr, fdt_paddr + fdt_size - 1, AS_ATTR_ROM, "fdt", "memory");
		alloc_ram(fdt_paddr, fdt_size, 1);
	}
}

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/hardware/startup/lib/fdt_init.c $ $Rev: 810394 $")
#endif
