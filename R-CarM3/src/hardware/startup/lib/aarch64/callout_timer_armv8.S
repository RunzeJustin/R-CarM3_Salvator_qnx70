/*
 * $QNXLicenseC:
 * Copyright 2007, 2008, QNX Software Systems.
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
 

/*
 * ARMv8 Generic Timer
 */

#include "callout.ah"

/*
 * --------------------------------------------------------------------------
 * Set clock resolution, and enable interrupt triggering as a side effect.
 *
 * On entry:
 *	r0 - pointer to syspage_entry
 *	r1 - pointer to qtime_entry
 * --------------------------------------------------------------------------
 */
CALLOUT_START(timer_load_armv8, 0, 0)
	/*
	 * Disable timer interrupt matching
	 */
	msr		cntv_ctl_el0, xzr

	/*
	 * Set match register to (counter + qtp->timer_load)
	 */
	ldr		w3, [x1, #QT_TIMER_LOAD]
	mrs		x2, cntvct_el0
	add		x2, x2, x3
	msr		cntv_cval_el0, x2

	/*
	 * Enable timer
	 */
	mov		x2, #1
	msr		cntv_ctl_el0, x2

	ret
CALLOUT_END(timer_load_armv8)


/*
 * --------------------------------------------------------------------------
 * Read the current timer value, relative to the last clock tick
 *
 * On entry:
 *	r0 - pointer to syspage_entry
 *	r1 - pointer to qtime_entry
 * --------------------------------------------------------------------------
 */
CALLOUT_START(timer_value_armv8, 0, 0)
	/*
	 * Return counter - (match - qtime->timer_load)
	 */
	ldr		w0, [x1, #QT_TIMER_LOAD]
	isb
	mrs		x1, cntvct_el0
	add		x0, x0, x1
	mrs		x1, cntv_cval_el0
	sub		x0, x0, x1
	ret
CALLOUT_END(timer_value_armv8)


/*
 * --------------------------------------------------------------------------
 * Clear timer interrupt.
 *
 * On entry:
 *	r0 - pointer to syspage_entry
 *	r1 - pointer to qtime_entry
 * --------------------------------------------------------------------------
 */
CALLOUT_START(timer_reload_armv8, 0, 0)
	/*
	 * Disable interrupt generation
	 */
	mrs		x2, cntv_ctl_el0
	orr		x0, x2, #2
	msr		cntv_ctl_el0, x0

	/*
	 * Set match = match + qtime->timer_load and re-enable interrupt generation
	 */
	ldr		w0, [x1, #QT_TIMER_LOAD]
	mrs		x1, cntv_cval_el0
	add		x1, x1, x0
	msr		cntv_cval_el0, x1
	msr		cntv_ctl_el0, x2

	/*
	 * Return 1 to indicate this was a real timer callout
	 */
	mov		x0, #1
	ret
CALLOUT_END(timer_reload_armv8)

#ifdef __QNXNTO__
#ifdef __USESRCVERSION
.section .ident, "SM",%progbits,1;
.asciz "$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/hardware/startup/lib/aarch64/callout_timer_armv8.S $ $Rev: 780681 $";
.previous
#endif
#endif
