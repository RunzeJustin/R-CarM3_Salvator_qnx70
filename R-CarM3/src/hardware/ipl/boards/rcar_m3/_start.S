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
 * _start.S
 *
 * This code runs as the first instructions after control has transferred
 * from any bootup monitor.  This module defines a minimal bootup stack,
 * and sets SP to this stack, and then starts running C code in _main().
 *
 * For the ARMv8 Fast Model, all processors execute from cold reset in EL3.
 * This code sets up minimal EL2/EL3 state for all cpus and switches to EL1
 * to continue execution:
 * - cpu0 jumps to cstart to perform the startup initialisation
 * - other cpus spin until they are woken up by board_smp_start
 */

	.data
	.globl	stack

stack:

	.text
	.align 2

	.extern	main
	.global	_start

_start:
	/*
	 * Load stack pointer and jump to C
	 */
	 
 	adr		x0, stack
	mov		sp, x0
	bl		main
	
cntfrq_default:	.word	100000000	// bogus default cntfrq_el0 value

#ifdef __QNXNTO__
#ifdef __USESRCVERSION
.section .ident, "SM",%progbits,1;
.asciz "$URL: http://svn.ott.qnx.com/product/mainline/hardware/startup/boards/armv8_fm/_start.S $ $Rev: 781181 $";
.previous
#endif
#endif
