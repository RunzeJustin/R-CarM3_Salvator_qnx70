#
# Copyright 2014, QNX Software Systems. 
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
 * Entry point for secondary processors.
 *
 * The board-specific board_smp_start() arranges for each secondary
 * processor to begin execution here.
 */

	.text
	.align	2

	.extern	_start_el1
	.extern aarch64_cache_flush
	.global	smp_start

smp_start:
	/*
	 * Switch to EL1 if necessary
	 */
	bl		_start_el1

	/*
	 * Install a default exception vector table loops on exception entry.
	 * This allows us to inspect the elr_el1 and esr_el1 registers from a
	 * debugger to see what exception occurred.
	 */
	adr		x0, vbar_default
	msr		vbar_el1, x0

	/*
	 * Clean and invalidate caches.
	 */
	bl		aarch64_cache_flush

	/*
	 * Set up stack
	 */
	adr		x0, stack
	mov		sp, x0

	/*
	 * Get our cpu number and save in w19 for later
	 */
	adr		x0, cpu_starting
	ldr		w0, [x0]
	sub		w0, w0, #1
	bl		board_smp_adjust_num
	mov		w19, w0

	/*
	 * Perform any custom cpu startup operations
	 */
	bl		cpu_startup

	/*
	 * Initialise cpuinfo
	 */
	mov		w0, w19
	bl		init_one_cpuinfo

	/*
	 * Tell bootstrap cpu start_aps() that it can start the next cpu
	 */
	adr		x0, cpu_starting
	str		wzr, [x0]
	dmb		sy

	/*
	 * Wait for bootstrap cpu to finish initialising the system page
	 */
	adr		x0, syspage_available
1:	ldr		w1, [x0]
	cmp		w1, w19
	bne		1b

	adr		x1, smp_spin_vaddr
	ldr		x0, [x1]
	mov		w1, w19
	bl		cpu_startnext

	/*
	 * We should not return from cpu_startnext
	 */
0:	wfi
	b		0b

.data
	.align	4
	.space	512
stack:

#ifdef __QNXNTO__
#ifdef __USESRCVERSION
.section .ident, "SM",%progbits,1;
.asciz "$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/hardware/startup/lib/aarch64/smp_start.S $ $Rev: 780681 $";
.previous
#endif
#endif
