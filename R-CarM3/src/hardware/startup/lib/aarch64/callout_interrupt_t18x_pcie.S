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

#define T18X_INTR_MASK		0xB4
#define T18X_INTR_CODE		0xB8
#define T18X_INTR_ENABLE0	0xC8

/*
  These callouts handle all of the non MSI PCIe interrupts
  The sideband messages associated with the INTPINs and other error related
  interrupts (INT_CODE 6) is further demux'd with a second level cascade
  callouts in callout_interrupt_t18x_pcie_ic6.S
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
CALLOUT_START(interrupt_id_t18x_pcie, 0, patch_intr)
	/*
	 * Get the AFI base address (patched)
	 */
	mov		x7, #0x0000
	movk	x7, #0x0000, lsl #16
	movk	x7, #0x0000, lsl #32
	movk	x7, #0x0000, lsl #48

	ldr		w3, [x7, #T18X_INTR_MASK]
	bic		w3, w3, #1
	str		w3, [x7, #T18X_INTR_MASK]

	ldr		w3, [x7, #T18X_INTR_CODE]
	subs	w19, w3, #1
	blt		0f

	/*
	 * Mask the interrupt
	 *
	 * For interrupt codes 1 thru 5 (interrupt numbers 0 - 4), the bit position
	 * is the interrupt number. For codes 7 thru 14, we need an extra subtraction.
	 * Interrupt code 6 (interrupt number 5) is special and does not have an enable
	 * bit in the ENABLE0 register. It is cascaded out to be further demultiplexed
	*/
	cmp		w19, #5
	beq		0f
	bgt		2f
	mov		w3, w19
	b		1f
2:
	sub		w3, w19, #1
1:
	/* mask the interrupt code and clear it */
	mov		w2, #1
	ldr		w1, [x7, #T18X_INTR_ENABLE0]
	lsl		w2, w2, w3
	bic		w1, w1, w2
	str		w1, [x7, #T18X_INTR_ENABLE0]

0:
CALLOUT_END(interrupt_id_t18x_pcie)

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
CALLOUT_START(interrupt_eoi_t18x_pcie, 0, patch_intr)
	/*
	 * Get the AFI base address (patched)
	 */
	mov		x7, #0x0000
	movk	x7, #0x0000, lsl #16
	movk	x7, #0x0000, lsl #32
	movk	x7, #0x0000, lsl #48

    /* EOI is issued by writing 0 to the INTR_CODE register */
	mov		w2, #0
	str		w2, [x7, #T18X_INTR_CODE]

	ldr		w3, [x7, #T18X_INTR_MASK]
	orr		w3, w3, #1
	str		w3, [x7, #T18X_INTR_MASK]

	/* Only unmask interrupt if mask count is zero */
	cbnz	x22, 0f

	/*
	 * Unmask the interrupt
	 *
	 * For interrupt codes 1 thru 5 (interrupt numbers 0 - 4), the bit position
	 * is the interrupt number. For codes 7 thru 14, we need an extra subtraction.
	 * Interrupt code 6 (interrupt number 5) is special and does not have an enable
	 * bit in the ENABLE0 register. It is cascaded out to be further demultiplexed
	*/
	cmp		w19, #5
	beq		0f
	bgt		2f
	mov		w3, w19
	b		1f
2:
	sub		w3, w19, #1
1:

	mov		w2, #1
	ldr		w1, [x7, #T18X_INTR_ENABLE0]
	lsl		w2, w2, w3
	orr		w1, w1, w2
	str		w1, [x7, #T18X_INTR_ENABLE0]

0:
CALLOUT_END(interrupt_eoi_t18x_pcie)

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
CALLOUT_START(interrupt_mask_t18x_pcie, 0, patch_intr)
	/*
	 * Get the AFI base address (patched)
	 */
	mov		x7, #0x0000
	movk	x7, #0x0000, lsl #16
	movk	x7, #0x0000, lsl #32
	movk	x7, #0x0000, lsl #48
 
	/*
	 * Mask the interrupt
	 *
	 * For interrupt codes 1 thru 5 (interrupt numbers 0 - 4), the bit position
	 * is the interrupt number. For codes 7 thru 14, we need an extra subtraction.
	 * Interrupt code 6 (interrupt number 5) is special and does not have an enable
	 * bit in the ENABLE0 register. It is cascaded out to be further demultiplexed
	*/
	cmp		w1, #5
	beq		0f
	bgt		2f
	mov		w3, w1
	b		1f
2:
	sub		w3, w1, #1
1:

	mov		w2, #1
	ldr		w1, [x7, #T18X_INTR_ENABLE0]
	lsl		w2, w2, w3
	bic		w1, w1, w2
	str		w1, [x7, #T18X_INTR_ENABLE0]

0:
	mov		x0, #0

	ret
CALLOUT_END(interrupt_mask_t18x_pcie)

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
CALLOUT_START(interrupt_unmask_t18x_pcie, 0, patch_intr)
	/*
	 * Get the AFI base address (patched)
	 */
	mov		x7, #0x0000
	movk	x7, #0x0000, lsl #16
	movk	x7, #0x0000, lsl #32
	movk	x7, #0x0000, lsl #48

	/*
	 * Unmask the interrupt
	 *
	 * For interrupt codes 1 thru 5 (interrupt numbers 0 - 4), the bit position
	 * is the interrupt number. For codes 7 thru 14, we need an extra subtraction.
	 * Interrupt code 6 (interrupt number 5) is special and does not have an enable
	 * bit in the ENABLE0 register. It is cascaded out to be further demultiplexed
	*/
	cmp		w1, #5
	beq		0f
	bgt		2f
	mov		w3, w1
	b		1f
2:
	sub		w3, w1, #1
1:

	mov		w2, #1
	ldr		w1, [x7, #T18X_INTR_ENABLE0]
	lsl		w2, w2, w3
	orr		w1, w1, w2
	str		w1, [x7, #T18X_INTR_ENABLE0]

0:
	mov		x0, #0

	ret
CALLOUT_END(interrupt_unmask_t18x_pcie)


.data
.align 3
Lvaddr:	.word	0,0	/* 8 bytes of storage for a uintptr_t */


#ifdef __QNXNTO__
#ifdef __USESRCVERSION
.section .ident, "SM",%progbits,1;
.asciz "$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/hardware/startup/lib/aarch64/callout_interrupt_t18x_pcie.S $ $Rev: 795130 $";
.previous
#endif
#endif
