#
# Copyright 2016, QNX Software Systems.
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
 * 8250/16x50 polled serial I/O
 */

#include "callout.ah"

/*
 * Patch interrupt callouts to access rw data.
 * The first call will also map the uart.
 *
 * Patcher routine takes the following arguments:
 *	x0 - syspage paddr
 *	x1 - vaddr of callout
 *	x2 - offset from start of syspage to start of callout routine
 *	x3 - offset from start of syspage to rw storage
 *	x4 - patch data
 *	x5 - callout_rtn
 */

patch_debug:
	sub		sp, sp, #16
	stp		x19, x30, [sp]

	add		x19, x0, x2				// x19 = address of callout routine

	/*
	 * Patch callout routine with LSR register address
	 */
	ldr		w10, [x4, #DDI_SHIFT]		// x4 has dbg_device[...]
	mov		x0, #REG_LS
	lsl		w0, w0, w10
	ldr		w1, [x19, #0x10]
	bfi		w1, w0, #5, #16
	str		w1, [x19, #0x10]

	/*
	 * Map UART using patch_data parameter
	 */
	mov		x0, #0x1000
	ldr		x1, [x4]
	bl		callout_io_map

	/*
	 * Patch callout with mapped virtual address in x0
	 */
	CALLOUT_PATCH	x19, w6, w7
 	ldp		x19, x30, [sp]
	add		sp, sp, #16
	ret

/*
 * -----------------------------------------------------------------------
 * void	display_char_8250(struct sypage_entry *, char)
 * -----------------------------------------------------------------------
 */
CALLOUT_START(display_char_8250, 0, patch_debug)
	mov		x7, #0xabcd				// UART base address (patched)
	movk	x7, #0xabcd, lsl #16
	movk	x7, #0xabcd, lsl #32
	movk	x7, #0xabcd, lsl #48

	mov		x8, #0x1234		// LSR register offset

	/*
	 * Wait for TXRDY
	 */

	/*
	 * In SMP system, it happens that someone is writing via this callout while the devc
	 * is being initialized and the UART controller is temporarily disabled
	 * For whatever reason while the callout is polling the LSR register while the
	 * devc driver can't have a chance to be scheduled to enable the UART again, it loops forever
	 *
	 * 0x1500 should be enough for a system with 50MHz/9600Baud to output one character
	 */
	mov		x2, #0x1500

0:	subs	x2, x2, #1
	beq		1f
	ldrb	w3, [x7,x8]
	tst		w3, #LSR_TXRDY
	beq		0b

	/*
	 * Output character
	 */
	strb	w1, [x7]

1:
	ret
CALLOUT_END(display_char_8250)


/*
 * -----------------------------------------------------------------------
 * char	display_char_8250(struct sypage_entry *)
 * -----------------------------------------------------------------------
 */
CALLOUT_START(poll_key_8250, 0, patch_debug)
	mov		x7, #0xabcd				// UART base address (patched)
	movk	x7, #0xabcd, lsl #16
	movk	x7, #0xabcd, lsl #32
	movk	x7, #0xabcd, lsl #48

	mov		x8, #0x1234		// LSR register offset

	/*
	 * If RXRDY, read character, otherwise return -1
	 */
	ldrb	w1, [x7,x8]
	mov		x0, #-1		// assume no character
	tst		w1, #LSR_RXRDY
	beq		1f
	ldrb	w0, [x7]
1:
	ret
CALLOUT_END(poll_key_8250)


/*
 * -----------------------------------------------------------------------
 * int break_detect(struct syspage_entry *)
 *
 * Return 1 if break detected, 0 otherwise
 * -----------------------------------------------------------------------
 */
CALLOUT_START(break_detect_8250, 0, patch_debug)
	mov		x7, #0xabcd				// UART base address (patched)
	movk	x7, #0xabcd, lsl #16
	movk	x7, #0xabcd, lsl #32
	movk	x7, #0xabcd, lsl #48

	mov		x8, #0x1234		// LSR register offset

	/*
	 * Check for break indicator
	 */
	ldrb	w0, [x7,x8]
	ands	w0, w0, #LSR_BI
	beq		1f

	/*
	 * Eat the null character
	 */
	ldrb	w0, [x7]
	mov		x0, #1

1:
	ret
CALLOUT_END(break_detect_8250)


#ifdef __QNXNTO__
#ifdef __USESRCVERSION
.section .ident, "SM",%progbits,1;
.asciz "$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/hardware/startup/lib/aarch64/callout_debug_8250.S $ $Rev: 803672 $";
.previous
#endif
#endif
