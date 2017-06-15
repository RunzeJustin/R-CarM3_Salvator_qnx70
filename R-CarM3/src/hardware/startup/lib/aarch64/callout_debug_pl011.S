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
 * PrimeCell PL011 UART specific polled serial I/O
 *
 * This should be usable by any board that uses a PrimeCell UART.
 */

#include "callout.ah"
#include <arm/pl011.h>

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
 * void	display_char_pl011(struct sypage_entry *, char)
 *
 * x0: syspage pointer
 * x1: character
 * -----------------------------------------------------------------------
 */
CALLOUT_START(display_char_pl011, 0, patch_debug)
	mov		x7, #0xabcd				// UART base address (patched)
	movk	x7, #0xabcd, lsl #16
	movk	x7, #0xabcd, lsl #32
	movk	x7, #0xabcd, lsl #48

0:	ldr		w2, [x7, #PL011_FR]
	tst		w2, #PL011_FR_TXFF
	b.ne	0b
	and		w0, w1, #0xff
	str		w0, [x7, #PL011_DR]
	ret
CALLOUT_END(display_char_pl011)


/*
 * -----------------------------------------------------------------------
 * char	display_char_pl011(struct sypage_entry *)
 *
 * x0: syspage pointer
 * -----------------------------------------------------------------------
 */
CALLOUT_START(poll_key_pl011, 0, patch_debug)
	mov		x7, #0xabcd				// UART base address (patched)
	movk	x7, #0xabcd, lsl #16
	movk	x7, #0xabcd, lsl #32
	movk	x7, #0xabcd, lsl #48

	ldr		w1, [x7, #PL011_FR]
	tst		w1, #PL011_FR_RXFE
	b.ne	0f
	ldr		w0, [x7, #PL011_DR]
	ret
0:	mov		w0, #-1
	ret
CALLOUT_END(poll_key_pl011)


/*
 * -----------------------------------------------------------------------
 * int break_detect(struct syspage_entry *)
 *
 * x0: syspage pointer
 *
 * Return 1 if break detected, 0 otherwise
 * -----------------------------------------------------------------------
 */
CALLOUT_START(break_detect_pl011, 0, patch_debug)
	mov		x7, #0xabcd				// UART base address (patched)
	movk	x7, #0xabcd, lsl #16
	movk	x7, #0xabcd, lsl #32
	movk	x7, #0xabcd, lsl #48

	/*
	 * FIXME: need to implement
	 */
	mov		w0, #0
	ret
CALLOUT_END(break_detect_pl011)

#ifdef __QNXNTO__
#ifdef __USESRCVERSION
.section .ident, "SM",%progbits,1;
.asciz "$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/hardware/startup/lib/aarch64/callout_debug_pl011.S $ $Rev: 781370 $";
.previous
#endif
#endif
