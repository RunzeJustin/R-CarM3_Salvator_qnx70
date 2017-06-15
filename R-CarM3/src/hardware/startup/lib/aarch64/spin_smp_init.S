#
# Copyright 2016, QNX Software Systems.
#
# Licensed under the Apache License, Version 2.0 (the "License"). You
# may not reproduce, modify or distribute this software except in
# compliance with the License. You may obtain a copy of the License
# at: http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" basis,
# WITHOUT WARRANTIES OF ANY KIND, either express or implied.
#
# This file may contain contributions from others, either as
# contributors under the License or as licensors under other terms.
# Please review this entire file for other proprietary rights or license
# notices, as well as the QNX Development Suite License Guide at
# http://licensing.qnx.com/license-guide/ for other information.
#

/*
 * spin_smp_start.S
 *
 * - cpu0 returns to perform the startup initialisation
 * - other cpus spin until they are woken up by board_smp_start
 */

	.text
	.align 2

	.global spin_smp_init

spin_smp_init:
	/* NOTE:
	   	Do NOT modify registers X0-X3 before returning on the bootstrap
		processor.
		These registers may contain information provided by the IPL and
		cstart will save them in the boot_regs variable for later perusal
		by other portions of startup.
	*/
	mrs		x9, mpidr_el1
	tbnz	x9, #30, bootstrap	// return if a uniprocessor system
	bfi		x9, xzr, #24, #8	// Clear the MT & M bits
	ldr		x8, spin_bootstrap_id
	cmp		x9, x8
	beq		bootstrap
	/*
	 * increment the number of processors so spin_smp_num_cpus()
	 * knows how many processors are present.
	 * This assumes all secondary processors get through this code
	 * before spin_smp_num_cpus() executes on the boot processor.
	 */
	adr		x1, spin_num_cpu
0:	ldaxr	w9, [x1]
	add		w2, w9, #1
	stlxr	w3, w2, [x1]
	cbnz	w3, 0b

	/*
	 * Wait until spin_smp_start sets spin_start_addr to our core index
	 */
	adr		x1, spin_start_addr
0:	wfe
	dmb		sy
	ldr		x2, [x1]
	lsr		x3, x2, #56	// extract the cpu index
	cmp		x3, x9
	bne		0b
	str		xzr, [x1]			// tell bootstrap CPU that we've noticed the start addr
	bfi		x2, xzr, #56, #8	// Clear the cpu index
	br		x2

bootstrap:
	ret

#ifdef __QNXNTO__
#ifdef __USESRCVERSION
.section .ident, "SM",%progbits,1;
.asciz "$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/hardware/startup/lib/aarch64/spin_smp_init.S $ $Rev: 805507 $";
.previous
#endif
#endif
