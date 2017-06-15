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
#include "aarch64/psci.h"
/*
 * FIXME_AARCH64: not sure how to get the number of cores implemented.
 *                For now, _start increments board_smp_max_cpu for each
 *                secondary processor that executes that code.
 */
unsigned	board_smp_max_cpu = 2;

/*
 * FIXME_AARCH64: this will need work to handle multiple clusters.
 */

uintptr_t	secondary_start;	// start address for cores waiting in cstart.S
long		secondary_cpu;		// cpu being woken up

unsigned
board_smp_num_cpu()
{
	kprintf("board_smp_num_cpu: %d cores\n", board_smp_max_cpu);
	return board_smp_max_cpu;
}
extern void board_disable_mmu();
void
board_smp_init(struct smp_entry *smp, unsigned num_cpus)
{
	smp->send_ipi = (void *)&sendipi_gic_v2;
}

int
board_smp_start(unsigned cpu, void (*start)(void))
{
    board_disable_mmu();
    psci_cpu_on(cpu, (uint64_t)start, 0);

	/*
	 * Secondary cores will be spinning in _start.S watching for
	 * secondary_cpunum to indicate this core should wake up.
	 */
	secondary_start = (uintptr_t)start;
	secondary_cpu = cpu;
	__asm__ __volatile__(
		"dsb sy\n"
		"sev\n"
		: : : "memory"
	);

	return 1;
}

unsigned
board_smp_adjust_num(unsigned cpu)
{
	return cpu;
}

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/hardware/startup/boards/rcar_m3/aarch64/board_smp.c $ $Rev: 810496 $")
#endif
