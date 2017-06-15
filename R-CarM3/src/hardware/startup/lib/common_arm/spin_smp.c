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

unsigned volatile	spin_num_cpu = 1;
uintptr_t volatile	spin_start_addr;	// start address for cores waiting in spin_smp_start.S

unsigned
spin_smp_num_cpu(void) {
	// The "yield" is to give any other cores a chance to run the
	// code in spin_smp_init.S.
	asm("yield");
	return spin_num_cpu;
}


int
spin_smp_start(unsigned cpu, void (*start)(void)) {
	/*
	 * Secondary cores will be spinning in spin_smp_init.S watching for
	 * spin_start_addr to indicate this core should wake up.
	 */
#if defined(__aarch64__)
	spin_start_addr = (uintptr_t)start | ((uintptr_t)cpu << 56);
#else
	extern unsigned _btext[];
	spin_start_addr = ((uintptr_t)start - (uintptr_t)_btext)
						| ((uintptr_t)cpu << 24);
#endif
	__asm__ __volatile__(
		"dsb sy\n"
		"sev\n"
		: : : "memory"
	);

	// Wait for other processor to notice
	while(spin_start_addr != 0) {
		asm("yield");
	}

	return 1;
}


#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/hardware/startup/lib/common_arm/spin_smp.c $ $Rev: 805440 $")
#endif
