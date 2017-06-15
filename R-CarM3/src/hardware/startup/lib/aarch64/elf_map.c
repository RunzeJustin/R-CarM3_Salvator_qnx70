/*
 * $QNXLicenseC:
 * Copyright 2014, QNX Software Systems. 
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

/*
 * Convert a set of ELF permission bits to AArch64 specific ones and
 * map in the specified memory region with the given permissions.
 */
void
elf_map(uintptr_t vaddr, paddr_t paddr, size_t size, int flags)
{
	unsigned	prot = 0;

	if (flags & PF_R) {
		prot |= PROT_READ;
	}
	if (flags & PF_W) {
		prot |= PROT_WRITE;
	}
	if (flags & PF_X) {
		prot |= PROT_EXEC;
	}
	(void) aarch64_map(vaddr, paddr, size, prot);
}



#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/hardware/startup/lib/aarch64/elf_map.c $ $Rev: 778261 $")
#endif
