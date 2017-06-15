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

#define T18X_INTR_MSG0			0x190
#define T18X_INTR_MSG1			0x194
#define T18X_SM_INTR_ENABLE0	0xC4

/*
  These callouts handle all of the non MSI PCIe INT CODE 6 interrupts
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
CALLOUT_START(interrupt_id_t18x_pcie_ic6, 0, patch_intr)
	/*
	 * Get the AFI base address (patched)
	 */
	mov		x7, #0x0000
	movk	x7, #0x0000, lsl #16
	movk	x7, #0x0000, lsl #32
	movk	x7, #0x0000, lsl #48

    /*
     * Read AFI_MSG0 register (controllers 0 and 1) or AFI_MSG_1_0 (controller 2)
     * It is layed out as follows
     * Controller 0/2 interrupts are in bits 15:0
     * Controller 1 interrupts are in bits 31:16
     * We'll just do a simple prioritization from MSb to LSb
     */
    mov		w4, #0	/* ID offset for controllers 0 and 1 */
	ldr		w2, [x7, #T18X_INTR_MSG0]
	cbnz	w2, 5f
	/* not controller 0 or 1, try controller 2 */
	mov		w4, #16	/* ID offset for controller 2 */
	ldr		w2, [x7, #T18X_INTR_MSG1]
	cbnz	w2, 5f
	/* not controller 2, treat as spurious */
	mov		x19, #-1
	b		0f
5:
	rbit	w2, w2
	clz		w19, w2

	/* do I have to write down the bits in MSG0 ? */
#	mov		w1, #1
#	bic		w2, w2, w1, lsl w19
#	str		w2, [x7, #T18X_INTR_MSG0]

    /*
    * The interrupt source ID is in x19
    * We now need to mask the specific interrupt but of course the
    * T18X_SM_INTR_ENABLE0 and T18X_INTR_MSG0 registers use different bits for
    * the same source.
    * T18X_INTR_MSG0 is as follows (for controller 1/(0 and 2) respectively)
    *	bit 31/15 - Fatal Error
    *	bit 30/14 - Non-Fatal Error
    *	bit 29/13 - Correctable Error
    *	bit 28/12 - Root Port Interrupt
    *	bit 27/11 - INTPIN D
    *	bit 26/10 - INTPIN C
    *	bit 25/09 - INTPIN B
    *	bit 24/08 - INTPIN A
    *	bit 23/07 - Hotplug Event
    *	bit 22/06 - unassigned
    *	bit 21/05 - unassigned
    *	bit 20/04 - Power Management Event (PME)
    *
    * T18X_SM_INTR_ENABLE0 is as follows
    *	bit 0 - INTPIN A assertion
    *	bit 1 - INTPIN B assertion
    *	bit 2 - INTPIN C assertion
    *	bit 3 - INTPIN D assertion
    *	bits 4:7 - INTPIN deassertions (don't care, always 0)
    *	bit 8 - Correctable Error
    *	bit 9 - Non-Fatal Error
    *	bit 10 - Fatal Error
    *	bit 11 - Power Management Event (PME)
    *	bit 12 - Hotplug Event
    *	bit 13 - Root Port Interrupt assertion
    *	bit 14 - Root Port Interrupt deassertion (don't care, always 0)
    *
    * Because it is the T18X_SM_INTR_ENABLE0 register which is manipulated the
    * most frequently by the callouts, we will use it to establish the ID's
    * so that the incoming interrupt number (modulo 16) will represent the bit
    * position within the T18X_SM_INTR_ENABLE0 register.
    *
    * This therefore means that this callout needs to map to that value
    *
	* Normalize the interrupt source for the purpose of clearing the enable bit
	*/
	cmp		w19, #16
	bge		4f
	mov		w3, w19
	b		3f
4:
	sub		w3, w19, #16
3:
    /*
    * w3 now contains the normalized MSG0 bit position of the highest interrupt
    * We now map that to the corresponding bit position in the T18X_SM_INTR_ENABLE0
    * register
    */
    cmp		w3, #13
    blt		5f
    sub		w3, w3, #5
    b		1f
5:
    cmp		w3, #12
    bne		4f
    add		w3, w3, #1
    b		1f
4:
    cmp		w3, #7
    bne		3f
    add		w3, w3, #5
    b		1f
3:
    cmp		w3, #4
    bne		2f
    add		w3, w3, #7
    b		1f
2:
    /* adjust for 8, 9, 10 or 11, bail with error on remaining values */
    subs	w3, w3, #8
    bge		1f
    /* spurious */
	mov		x19, #-1
	b		0f
1:
	/* mask the interrupt code and clear it */
	mov		w2, #1
	ldr		w1, [x7, #T18X_SM_INTR_ENABLE0]
	lsl		w2, w2, w3
	bic		w1, w1, w2
	/* and x1 x1, #0x3F0F */
	mov		w2, #0x3F0F
	and		w1, w1, w2
	str		w1, [x7, #T18X_SM_INTR_ENABLE0]

	/* use the adjusted interrupt number in w3 and controller offset in w4 */
	cmp		w19, #16
	add		w19, w3, #16
	add		w19, w19, w4
	bge		0f
	mov		w19, w3

0:
CALLOUT_END(interrupt_id_t18x_pcie_ic6)

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
CALLOUT_START(interrupt_eoi_t18x_pcie_ic6, 0, patch_intr)
	/*
	 * Get the AFI base address (patched)
	 */
	mov		x7, #0x0000
	movk	x7, #0x0000, lsl #16
	movk	x7, #0x0000, lsl #32
	movk	x7, #0x0000, lsl #48

	/*
	   Only unmask interrupt if mask count is zero
	 */
	cbnz	x22, 0f

	cmp		w19, #16
	sub		w3, w19, #16
	bge		1f
	mov		w3, w19
1:

	mov		w2, #1
	ldr		w1, [x7, #T18X_SM_INTR_ENABLE0]
	lsl		w2, w2, w3
	orr		w1, w1, w2
	mov		w2, #0x3F0F
	and		w1, w1, w2
	str		w1, [x7, #T18X_SM_INTR_ENABLE0]
0:
CALLOUT_END(interrupt_eoi_t18x_pcie_ic6)

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
CALLOUT_START(interrupt_mask_t18x_pcie_ic6, 0, patch_intr)
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
	cmp		w1, #16
	blt		1f
	sub		w1, w1, #16
1:

	mov		w2, #1
	ldr		w3, [x7, #T18X_SM_INTR_ENABLE0]
	lsl		w2, w2, w1
	bic		w3, w3, w2
	mov		w2, #0x3F0F
	and		w3, w3, w2
	str		w3, [x7, #T18X_SM_INTR_ENABLE0]

0:
	mov		x0, #0

	ret
CALLOUT_END(interrupt_mask_t18x_pcie_ic6)

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
CALLOUT_START(interrupt_unmask_t18x_pcie_ic6, 0, patch_intr)
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
	cmp		w1, #16
	blt		1f
	sub		w1, w1, #16
1:

	mov		w2, #1
	ldr		w3, [x7, #T18X_SM_INTR_ENABLE0]
	lsl		w2, w2, w1
	orr		w3, w3, w2
	mov		w2, #0x3F0F
	and		w3, w3, w2
	str		w3, [x7, #T18X_SM_INTR_ENABLE0]

0:
	mov		x0, #0

	ret
CALLOUT_END(interrupt_unmask_t18x_pcie_ic6)


.data
.align 3
Lvaddr:	.word	0,0	/* 8 bytes of storage for a uintptr_t */


#ifdef __QNXNTO__
#ifdef __USESRCVERSION
.section .ident, "SM",%progbits,1;
.asciz "$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/hardware/startup/lib/aarch64/callout_interrupt_t18x_pcie_ic6.S $ $Rev: 795130 $";
.previous
#endif
#endif
