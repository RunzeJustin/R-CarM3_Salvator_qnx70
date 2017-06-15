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
 * cstart.S
 *
 *	Initial code to get Neutrino started
 *
 * This code runs as the first instructions after control has transferred
 * from any bootup monitor.  This module defines a minimal bootup stack,
 * and sets SP to this stack, and then starts running C code in _main().
 */

	.data
	.globl	stack
	.globl	boot_args

boot_args:
	.ascii	"ddpvbskr"	/* signature for mkifs */

	.align	4
stack_end:
	.space	3*1024
stack:

	.text
	.align 2

	.extern	_main
	.extern	aarch64_cache_flush
	.global	cstart

cstart:
	/*
	 * Make sure interrupts are disabled
	 */
	msr		daifset, #2

	/*
	 * Save x0-x3 in boot_regs[]
	 */
	ldr		x19, =boot_regs
	stp		x0, x1, [x19]
	stp		x2, x3, [x19, #16]

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
	 * Load stack pointer and jump to C
	 */
	adr		x0, stack
	mov		sp, x0
	bl		_main

	/*
	 * _main should not return
	 */
0:	wfi
	b		0b

#ifdef __QNXNTO__
#ifdef __USESRCVERSION
.section .ident, "SM",%progbits,1;
.asciz "$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/hardware/startup/lib/aarch64/cstart.S $ $Rev: 803732 $";
.previous
#endif
#endif
