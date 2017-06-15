#
# Copyright 2015, QNX Software Systems.
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
 * _start_el1.S
 *
 * Helper function for _start() to switch to EL1 if we began execution at EL2.
 *
 * This code can be overridden by a board specific startup if necessary.
 */

	.text
	.align 2


	.global	_start_el1
_start_el1:
	mrs		x0, CurrentEL
	bic		x0, x0, #1				// clear SPSel bit
	cmp		x0, #0xc				// check for EL3
	beq		from_el3
	cmp		x0, #0x8				// check for EL2
	beq		from_el2

	// At EL1 already
	msr		daifset, #0xf			// disable interrupts etc.
	ret


from_el3:
	/*
	 * Set up minimal EL3 state required for EL1 execution
	 */

	/*
	 * set up for return to caller, executing at EL1
	 */
	msr		elr_el3, x30
	mov		x0, #0x3c5				// DAIF + EL1h
	msr		spsr_el3, x0

	mov		x1, #((1<<10)|(1<<8)|(1<<0))	// SCR_EL3.RW, HCE and NS set
	msr		scr_el3, x1				// Set EL2/EL1/EL0 to AArch64

	mrs		x1,scr_el3
	tst		x1,#(1<<8)			// check to see if SCR_EL3.HCE actually got set
	bne		setup_el2

	// SCR_EL3.HCE is RES0, which means that EL2 isn't supported. Just
	// drop down to EL1
	eret

from_el2:
	/*
	 * Set up minimal EL2 state required for EL1 execution
	 */

	/*
	 * set up for return to caller, executing at EL1
	 */
	msr		elr_el2, x30
	mov		x0, #0x3c5				// DAIF + EL1h
	msr		spsr_el2, x0

setup_el2:
	mrs		x0, sctlr_el2
	mov		x1, #((1<<0)|(1<<2)|(1<<12))
	tst		x0, x1
	beq		1f
	// if MMU, data or instruction cache are on, turn them off
	bic		x0, x0, x1
	msr		sctlr_el2, x0
	isb
	bl		aarch64_cache_flush
1:
	adr		x0,	have_el2			// indicate that the chip supports EL2
	mov		w1, #1
	str		w1, [x0]

	adr		x0, vbar_default		// initialize an EL2 exception table so
	msr		vbar_el2, x0			// we can set up for a hypervisor later

	mov		x0, #(1 << 31)			// HCR_EL2.RW - execute EL1 in AArch64
	msr		hcr_el2, x0				// HVC instruction is enabled

	mrs		x0, midr_el1
	msr		vpidr_el2, x0			// set MIDR_EL1 value read by EL1

	mrs		x0, mpidr_el1
	msr		vmpidr_el2, x0			// set MPIDR_EL1 value read by EL1

	msr		cntvoff_el2, xzr		// reset CNTVOFF_EL2

	mov		x0, #0x3
	msr		cnthctl_el2, x0			// enable access to physical timer by EL0/1

	msr		vttbr_el2, xzr			// clear VMID

	eret

#ifdef __QNXNTO__
#ifdef __USESRCVERSION
.section .ident, "SM",%progbits,1;
.asciz "$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/hardware/startup/lib/aarch64/_start_el1.S $ $Rev: 812970 $";
.previous
#endif
#endif
