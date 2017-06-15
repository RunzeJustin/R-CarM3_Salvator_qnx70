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
#include "aarch64/mmu.h"

/*
 * Initialize CPU specific pointers in the system page.
 * This code is hardware independant and should not have to be
 * changed by end users.
 */
void
cpu_init_syspage_memory()
{
	set_syspage_section(&lsp.cpu.aarch64_gic_map, sizeof(_Uint32t) * PROCESSORS_MAX);
}

struct syspage_entry *
cpu_alloc_syspage_memory(PADDR_T *cpupagep, PADDR_T *syspagep, unsigned spsize)
{
	struct syspage_entry		*sp = lsp.syspage.p;
	struct system_private_entry	*private;
	unsigned					size;
	unsigned					cpsize;
	paddr_t						syspage_paddr;

	#define	SP_OFFSET(field)	PTR_DIFF(lsp.cpu.field.p, sp)
	#define	INIT_ENTRY(_cpu, _field) \
		sp->un._cpu._field.entry_size = lsp.cpu._cpu##_##_field.size; \
		sp->un._cpu._field.entry_off  = SP_OFFSET(_cpu##_##_field)

	spsize = ROUND(spsize, sizeof(uint64_t));
	cpsize = sp->num_cpu * sizeof(struct cpupage_entry);

	/*
	 * Allocate the system page (and cpupage entries) and save it away.
	 * The system page must be 4K aligned in a virtual system
	 */
	size = spsize + cpsize;

	syspage_paddr = alloc_ram(NULL_PADDR, size, lsp.system_private.p->pagesize);
	if (syspage_paddr == NULL_PADDR) {
		crash("could not allocate 0x%x bytes for syspage/cpupage\n", size);
	}

	private = lsp.system_private.p;

	private->kern_syspageptr = TOPTR(aarch64_map(~0L, syspage_paddr, size, PROT_READ|PROT_WRITE|PROT_EXEC));
	private->kern_cpupageptr = TOPTR((uintptr_t)private->kern_syspageptr + spsize);

	private->user_syspageptr = TOPTR(aarch64_map(~0L, syspage_paddr, size, PROT_READ|PROT_EXEC|PROT_USER));
	private->user_cpupageptr = TOPTR((uintptr_t)private->user_syspageptr + spsize);

	private->cpupage_spacing = sizeof(struct cpupage_entry);

	*syspagep = syspage_paddr;
	*cpupagep = syspage_paddr + spsize;

	/*
	 * Set up aarch64 specific syspage section
	 */
	sp->un.aarch64.L0_vaddr = L0_vaddr;
	INIT_ENTRY(aarch64, gic_map);

	return((void *)syspage_paddr);
}

void
cpu_write_syspage_memory(paddr_t sysp_paddr, unsigned sysp_size, unsigned callout_size)
{
	/*
	 * WARNING: currently run startup with MMU disabled.
	 *			If we ever run with MMU enabled, we need to flush the data
	 *			cache to ensure that callout code is flushed to memory.
	 */
}



#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/hardware/startup/lib/aarch64/cpu_syspage_memory.c $ $Rev: 795738 $")
#endif
