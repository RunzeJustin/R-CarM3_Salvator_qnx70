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
 * vstart.S
 *
 * Enable MMU and jump to entry point of next bootstrap executable
 */

	.text
	.align	2

	.global	vstart
	.extern	sctlr_clr
	.extern	sctlr_el1
	.extern	mair_el1

/*
 * void	vstart(uintptr_t syspageptr, uintptr_t entry_point, unsigned cpunum)
 * On entry:
 *	x0 - syspageptr
 *	x1 - entry_point
 *	x2 - cpunum
 *
 * Enable the mmu and jump to the next program's entry point
 * The next program is responsible for tearing down the 1-1 section
 * mapping of the startup program set up by init_mmu().
 */ 
vstart:
	/*
	 * Invalidate Icache and TLBs
	 */
	ic		iallu
	tlbi	vmalle1
	dsb		sy

	/*
	 * Global MAIR_EL1 settings for all processors
	 */
	adr		x3, mair_el1
	ldr		x3, [x3]
	msr		mair_el1, x3

	/*
	 * Global TCR_EL1 settings for all processors
	 */
	adr		x3, tcr_el1
	ldr		x3, [x3]
	msr		tcr_el1, x3

	/*
	 * Global TTBR0 for all processors.
	 * This is holds an identity mapping of the physical address of this code
	 * so instruction fetches work correctly across the transition to MMU on.
	 */
	adr		x3, ttbr0
	ldr		x3, [x3]
	msr		ttbr0_el1, x3

	/*
	 * Per-processor TTBR1 value from ttbr_el1[cpunum].
	 */
	adr		x3, ttbr1
	ldr		x3, [x3, x2, lsl #3]
	msr		ttbr1_el1, x3

	/*
	 * Enable MMU and jump to next program's entry point
	 */
	adr		x3, sctlr_el1
	ldr		w3, [x3]
	isb
	msr		sctlr_el1, x3
	isb

	/*
	 * Call entry_point(_syspage_ptr, cpunum)
	 */
	mov		x3, x1
	mov		w1, w2
	br		x3

#ifdef __QNXNTO__
#ifdef __USESRCVERSION
.section .ident, "SM",%progbits,1;
.asciz "$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/hardware/startup/lib/aarch64/vstart.S $ $Rev: 780681 $";
.previous
#endif
#endif
