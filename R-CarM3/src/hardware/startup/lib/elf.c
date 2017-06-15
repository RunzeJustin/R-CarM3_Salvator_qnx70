/*
 * $QNXLicenseC:
 * Copyright 2008, QNX Software Systems. 
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
#include <sys/elf.h>


int
is_elf64(paddr_t addr)
{
	Elf64_Ehdr	*p, hdr, *hp=&hdr;

	p = startup_memory_map(sizeof(hdr), addr, PROT_READ);
	hdr = *p;
	startup_memory_unmap(p);

	if (memcmp(hp->e_ident, ELFMAG, SELFMAG)
		|| hp->e_ident [EI_DATA] != ELFDATANATIVE
		|| hp->e_ident[EI_CLASS] != ELFCLASS64
		|| hp->e_phnum == 0
		|| hp->e_phentsize != sizeof(Elf64_Phdr)) {
		return 0;
	}
	return 1;
}

int
is_elf32(paddr_t addr)
{
	Elf32_Ehdr	*p, hdr, *hp=&hdr;

	p = startup_memory_map(sizeof(hdr), addr, PROT_READ);
	hdr = *p;
	startup_memory_unmap(p);

	if (memcmp(hp->e_ident, ELFMAG, SELFMAG)
		|| hp->e_ident [EI_DATA] != ELFDATANATIVE
		|| hp->e_ident[EI_CLASS] != ELFCLASS32
		|| hp->e_phnum == 0
		|| hp->e_phentsize != sizeof(Elf32_Phdr)) {
		return 0;
	}
	return 1;
}

int
is_elf(paddr_t addr)
{
	if (is_elf32(addr)) {
		return 32;
	}
	if (is_elf64(addr)) {
		return 64;
	}
	return 0;
}


uintptr_t
load_elf(paddr_t addr)
{
	if (is_elf32(addr)) {
		return load_elf32(addr);
	}
	if (is_elf64(addr)) {
		return load_elf64(addr);
	}
	return (uintptr_t)-1;
}

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/hardware/startup/lib/elf.c $ $Rev: 791709 $")
#endif
