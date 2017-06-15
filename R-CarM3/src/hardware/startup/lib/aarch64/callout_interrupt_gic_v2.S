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
 * GICv2 interrupt controller callouts.
 *
 * See callout.ah for description of special registers for interrupt callouts.
 */

#include "callout.ah"
#include "aarch64/gic_v2.h"

/*
 * Patch interrupt callouts to access rw data.
 * The first call will also map the GIC distributor and CPU interfaces.
 *
 * Patcher routine takes the following arguments:
 *	x0 - syspage paddr
 *	x1 - vaddr of callout
 *	x2 - offset from start of syspage to start of callout routine
 *	x3 - offset from start of syspage to rw storage
 *	x4 - patch data
 *	x5 - callout_rtn
 */

rw_size: .word	16						// 16 bytes of storage
#define	OFF_GICD	0					// holds pointer to GICD registers
#define	OFF_GICC	8					// holds pointer to GICC registers

rw_patch:
	sub		sp, sp, #16
	stp		x19, x30, [sp]

	add		x2, x0, x2					// x2 = address of callout routine
	and		x3, x3, #0xffff
	add		x19, x0, x3					// x19 = address of rw data

	/*
	 * Patch callout with RW offset in w3
	 */
	CALLOUT_PATCH_RW	x2, w3, w6, w7

	/*
	 * Check if we have already mapped registers
	 */
	adr		x6, .Lmapped
	ldr		w7, [x6]
	cbnz	w7, 0f
	add		w7, w7, #1
	str		w7, [x6]

	/*
	 * Map GICD
	 */
	mov		x0, #0x1000
	adr		x1, gic_gicd
	ldr		x1, [x1]
	bl		callout_io_map
	str		x0, [x19, #OFF_GICD]

	/*
	 * Map GICC
	 */
	mov		x0, #0x2000
	adr		x1, gic_gicc
	ldr		x1, [x1]
	bl		callout_io_map
	str		x0, [x19, #OFF_GICC]

0: 	ldp		x19, x30, [sp]
	add		sp, sp, #16
	ret

.Lmapped:	.word	0

/*
 * -----------------------------------------------------------------------
 * Identify interrupt source
 *
 *	x20 - syspage pointer
 * -----------------------------------------------------------------------
 */
CALLOUT_START(interrupt_id_gic_v2, rw_size, rw_patch)
	mov		w7, 0xabcd				// offset to rw data (patched)
	add		x7, x7, x20				// address of rw data
	ldr		x6, [x7, #OFF_GICC]

	/*
	 * Get interrupt ID and handle special cases:
	 * ID0    - used for IPIs
	 * ID1022 - spurious interrupt
	 * ID1023 - spurious interrupt
	 */
	ldr		w0, [x6, #ARM_GICC_IAR]
	and		w19, w0, #ARM_GICC_IAR_IDMASK
	cbz		w19, 0f
	cmp		w19, #1022
	bhi		1f

	/*
	 * Mask interrupt
	 */
	ldr		x5, [x7, #OFF_GICD]
	add		x5, x5, #ARM_GICD_ICENABLERn
	and		w0, w19, #0x1f
	mov		w1, #1
	lsl		w1, w1, w0					// bit to set = 1 << (id % 32)
	lsr		w0, w19, #5					// index = id / 32
	str		w1, [x5, x0, lsl #2]		// ICENABLERn[index] = bit
	b		2f

	/*
	 * IPI interrupt (ID0) - acknowledge using full SRCID and exit
	 */
0:	str		w0, [x6, #ARM_GICC_EOIR]
	b		2f


	/*
	 * Spurious interrupt - set id to -1
	 */
1:	mov		x19, #-1

	/*
	 * Done - interrupt id is in x19
	 */
2:
CALLOUT_END(interrupt_id_gic_v2)


/*
 * -----------------------------------------------------------------------
 * Acknowledge specified interrupt
 *
 *	x19 - contains interrupt id
 *	x20 - contains syspage pointer (INTR_GENFLAG_LOAD_SYSPAGE was used)
 *	x22 - contains intr mask count (INTR_GENFLAG_LOAD_INTRMASK was used)
 * -----------------------------------------------------------------------
 */
CALLOUT_START(interrupt_eoi_gic_v2, rw_size, rw_patch)
	mov		w7, 0xabcd				// offset to rw data (patched)

	/*
	 * Skip ID0 because we EOI in the id callout and never mask it
	 */
	cbz		w19, 0f

	add		x7, x7, x20				// address of rw data
	ldr		x5, [x7, #OFF_GICD]
	ldr		x6, [x7, #OFF_GICC]

	/*
	 * Send EOI
	 */
	str		w19, [x6, #ARM_GICC_EOIR]

	/*
	 * Unmask interrupt if mask count is zero.
	 */
	cbnz	w22, 0f
	add		x5, x5, #ARM_GICD_ISENABLERn
	and		w0, w19, #0x1f
	mov		w1, #1
	lsl		w1, w1, w0						// bit to set = 1 << (id % 32)
	lsr		w0, w19, #5						// index = id / 32
	str		w1, [x5, x0, lsl #2]			// ISENABLERn[index] = bit
0:
CALLOUT_END(interrupt_eoi_gic_v2)


/*
 * -----------------------------------------------------------------------
 * Mask specified interrupt
 *
 *	x0 - _syspage_ptr
 *	x1 - vector number (relative to controller)
 * -----------------------------------------------------------------------
 */
CALLOUT_START(interrupt_mask_gic_v2, rw_size, rw_patch)
	mov		w7, 0xabcd				// offset to rw data (patched)
	add		x7, x7, x0				// address of rw data
	ldr		x5, [x7, #OFF_GICD]

	/*
	 * Mask interrupt
	 */
	add		x5, x5, #ARM_GICD_ICENABLERn
	and		w0, w1, #0x1f
	mov		w2, #1
	lsl		w2, w2, w0						// bit to set = 1 << (id % 32)
	lsr		w0, w1, #5						// index = id / 32
	str		w2, [x5, x0, lsl #2]			// ICENABLERn[index] = bit

	mov		x0, #0
	ret
CALLOUT_END(interrupt_mask_gic_v2)


/*
 * -----------------------------------------------------------------------
 * Unmask specified interrupt
 *
 *	x0 - _syspage_ptr
 *	x1 - vector number (relative to controller)
 * -----------------------------------------------------------------------
 */
CALLOUT_START(interrupt_unmask_gic_v2, rw_size, rw_patch)
	mov		w7, 0xabcd				// offset to rw data (patched)
	add		x7, x7, x0				// address of rw data
	ldr		x5, [x7, #OFF_GICD]

	/*
	 * Unmask interrupt
	 */
	add		x5, x5, #ARM_GICD_ISENABLERn
	and		w0, w1, #0x1f
	mov		w2, #1
	lsl		w2, w2, w0						// bit to set = 1 << (id % 32)
	lsr		w0, w1, #5						// index = id / 32
	str		w2, [x5, x0, lsl #2]			// ISENABLERn[index] = bit

	mov		x0, #0
	ret
CALLOUT_END(interrupt_unmask_gic_v2)

/*
 * -----------------------------------------------------------------------
 * Configure interrupt flags for a specified interrupt vector
 *
 *	x0 - syspage pointer
 *	x1 - intrinfo_entry pointer
 *	x2 - vector number (relative to controller)
 * -----------------------------------------------------------------------
 */
CALLOUT_START(interrupt_config_gic_v2, 0, 0)
	/*
	 * Use ID0 for IPI
	 */
	cmp		x2, #0
	mov		w0, #INTR_CONFIG_FLAG_IPI
	csel	w0, w0, wzr, eq
	ret
CALLOUT_END(interrupt_config_gic_v2)

#ifdef __QNXNTO__
#ifdef __USESRCVERSION
.section .ident, "SM",%progbits,1;
.asciz "$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/hardware/startup/lib/aarch64/callout_interrupt_gic_v2.S $ $Rev: 812970 $";
.previous
#endif
#endif
