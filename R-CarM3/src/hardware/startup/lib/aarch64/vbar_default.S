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
 * Stub exception vector table.
 */

	.text
	.align	2

	.global	vbar_default

.macro ventry
	.align	7
0:	b	0b
.endm

	.align	11

vbar_default:
	ventry		// 0x000
	ventry		// 0x080
	ventry		// 0x100
	ventry		// 0x180
	.align 7	// 0x200

	// synchronous exceptions with ELx, x > 0
	ldr		w0, trying_hvc
0:	cbz		w0, 0b
	// We were trying to execute an HVC instruction, but it's been disabled
	// somewhere. Just skip it.
	mrs		x0, elr_el1
	add		x0, x0, #4
	msr		elr_el1, x0
	eret

	ventry		// 0x280
	ventry		// 0x300
	ventry		// 0x380

	.align	7	// 0x400
	// This entry is for synchronous exceptions from a lower exception
	// level using aarch64. The only reason we'd start executing here during
	// startup is due to a HVC instruction when we're setting up for a
	// hypervisor. X1 holds a function to execute and X0 holds a parm for it.
	mrs		x2, sp_el1
	mov		sp, x2
	blr		x1
	eret

	ventry		// 0x480
	ventry		// 0x500
	ventry		// 0x580
	ventry		// 0x600
	ventry		// 0x680
	ventry		// 0x700
	ventry		// 0x780

#ifdef __QNXNTO__
#ifdef __USESRCVERSION
.section .ident, "SM",%progbits,1;
.asciz "$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/hardware/startup/lib/aarch64/vbar_default.S $ $Rev: 812970 $";
.previous
#endif
#endif
