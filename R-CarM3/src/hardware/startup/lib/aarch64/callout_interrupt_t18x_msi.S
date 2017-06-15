/*
 * $QNXLicenseC:
 * Copyright 2015, QNX Software Systems.
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


#include "callout.ah"

#define T18X_MSI_STATUS		0x6C
#define T18X_MSI_ENABLE		0x8C


/*
  These callouts handle the 256 MSI interrupts
*/
/*
 * -----------------------------------------------------------------------
 * Patch interrupt callouts to access rw data.
 * The first call will also map the uart.
 *
 * Patcher routine takes the following arguments:
 *
 *	x0 - syspage paddr
 *	x1 - vaddr of callout
 *	x2 - offset from start of syspage to start of callout routine
 *	x3 - offset from start of syspage to rw storage
 *	x4 - patch data
 *	x5 - callout_rtn
 * -----------------------------------------------------------------------
 */
.text
.align 4

patch_intr:
	sub		sp, sp, #16
	stp		x19, x30, [sp]

	add		x19, x0, x2				/* x19 = address of callout routine */
	/*
	 * Check if we have already mapped registers
	 */
	adr		x6, Lvaddr
	ldr		x0, [x6]
	cbnz	x0, 0f

	/*
	 * Map the T18x AFI Base address
	 */
	mov		w0, #0x800			/* Size of the AFI */
	ldr		x1, [x4]
	bl		callout_io_map
	adr		x6, Lvaddr
	str		x0, [x6]

0:
	/*
	 * Patch the callout routine
	 * - patch argument in x0
	 * - location to patch specified as first parameter
	 * - remaining 2 parameters are scratch registers
	 */
	CALLOUT_PATCH	x19, w6, w7

 	ldp		x19, x30, [sp]
	add		sp, sp, #16
	ret

/*
 * -----------------------------------------------------------------------
 * Identify interrupt source
 *
 * On entry
 * - syspage pointer in x20
 *
 * On exit
 * - interrupt id returned in x19
 * -----------------------------------------------------------------------
 */

CALLOUT_START(interrupt_id_t18x_msi, 0, patch_intr)
	/*
	 * Get the AFI base address (patched)
	 */
	mov		x7, #0x0000
	movk	x7, #0x0000, lsl #16
	movk	x7, #0x0000, lsl #32
	movk	x7, #0x0000, lsl #48

	mov		w19, #-1

	/*
	 * Read status/enable registers
	 */
	mov		x3, #0
0:
	ldr		w0, [x7, #T18X_MSI_STATUS]
	ldr		w1, [x7, #T18X_MSI_ENABLE]
	ands	w2, w0, w1
	bne		1f
	add		w3, w3, #4
	cmp		w3, #0x1c	// 8 sets of 32 bits registers
	bgt		2f
	add		x7, x7, #4
	b		0b

1:
	rbit	w2, w2
	clz		w19, w2

	mov		w2, #1
	lsl		w2, w2, w19

	/*
	 * Mask the interrupt
	 */
	bic		w1, w1, w2
	str		w1, [x7, #T18X_MSI_ENABLE]

	/*
	 * Clear status
	 */
	str		w2, [x7, #T18X_MSI_STATUS]

	add		w19, w19, w3, lsl #3

2:
CALLOUT_END(interrupt_id_t18x_msi)

/*
 * -----------------------------------------------------------------------
 * Acknowledge specified interrupt
 *
 * On entry
 * - x19 - contains interrupt id
 * - x20 - contains syspage pointer (INTR_GENFLAG_LOAD_SYSPAGE was used)
 * - x22 - contains the interrupt mask count (INTR_GENFLAG_LOAD_INTRMASK was used)
 * -----------------------------------------------------------------------
 */
CALLOUT_START(interrupt_eoi_t18x_msi, 0, patch_intr)
	/*
	 * Get the AFI base address (patched)
	 */
	mov		x7, #0x0000
	movk	x7, #0x0000, lsl #16
	movk	x7, #0x0000, lsl #32
	movk	x7, #0x0000, lsl #48

	/*
	 * Only unmask interrupt if mask count is zero
	 */
	cbnz	x22, 0f

	/*
	 * Unmask the interrupt
	 */
	and		w2, w19, #0x1F
	lsr		w0, w19, #5
	add		x7, x7, x0, lsl #2

	mov		w1, #1
	ldr		w0, [x7, #T18X_MSI_ENABLE]
	lsl		w1, w1, w2
	orr		w0, w0, w1
	str		w0, [x7, #T18X_MSI_ENABLE]
0:
CALLOUT_END(interrupt_eoi_t18x_msi)

/*
 * -----------------------------------------------------------------------
 * Mask specified interrupt
 *
 * On entry:
 * - x0 - _syspage_ptr
 * - x1 - vector number (relative to controller)
 *
 * Returns:
 * - x0 - error status
 * -----------------------------------------------------------------------
 */
CALLOUT_START(interrupt_mask_t18x_msi, 0, patch_intr)
	/*
	 * Get the AFI base address (patched)
	 */
	mov		x7, #0x0000
	movk	x7, #0x0000, lsl #16
	movk	x7, #0x0000, lsl #32
	movk	x7, #0x0000, lsl #48

	/*
	 * Mask the interrupt
	 */
	and		w2, w1, #0x1F
	lsr		w3, w1, #5
	add		x7, x7, x3, lsl #2

	mov		w3, #1
	ldr		w0, [x7, #T18X_MSI_ENABLE]
	lsl		w3, w3, w2
	bic		w0, w0, w3
	str		w0, [x7, #T18X_MSI_ENABLE]

	mov		x0, #0

	ret
CALLOUT_END(interrupt_mask_t18x_msi)

/*
 * -----------------------------------------------------------------------
 * Unmask specified interrupt
 *
 * On entry:
 * - x0 - _syspage_ptr
 * - x1 - vector number (relative to controller)
 *
 * Returns:
 * - x0 - error status
 * -----------------------------------------------------------------------
 */
CALLOUT_START(interrupt_unmask_t18x_msi, 0, patch_intr)
	/*
	 * Get the AFI base address (patched)
	 */
	mov		x7, #0x0000
	movk	x7, #0x0000, lsl #16
	movk	x7, #0x0000, lsl #32
	movk	x7, #0x0000, lsl #48

	/*
	 * Unmask the interrupt
	 */
	and		w2, w1, #0x1F
	lsr		w3, w1, #5
	add		x7, x7, x3, lsl #2

	mov		w3, #1
	ldr		w0, [x7, #T18X_MSI_ENABLE]
	lsl		w3, w3, w2
	orr		w0, w0, w3
	str		w0, [x7, #T18X_MSI_ENABLE]

	mov		x0, #0

	ret
CALLOUT_END(interrupt_unmask_t18x_msi)

.data
.align 3
Lvaddr:	.word	0,0	/* 8 bytes of storage for a uintptr_t */


#ifdef __QNXNTO__
#ifdef __USESRCVERSION
.section .ident, "SM",%progbits,1;
.asciz "$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/hardware/startup/lib/aarch64/callout_interrupt_t18x_msi.S $ $Rev: 795130 $";
.previous
#endif
#endif
