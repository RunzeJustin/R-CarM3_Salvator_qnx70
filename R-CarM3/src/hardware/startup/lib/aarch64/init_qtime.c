/*
 * $QNXLicenseC:
 * Copyright 2014, QNX Software Systems. 
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

#include "startup.h"

static const struct callout_slot	timer_callouts[] = {
	{ CALLOUT_SLOT( timer_load, _armv8) },
	{ CALLOUT_SLOT( timer_value, _armv8) },
	{ CALLOUT_SLOT( timer_reload, _armv8) },
};

void
init_qtime()
{
	struct qtime_entry	*qtime;
	
	qtime = alloc_qtime();

	/*
	 * Set up qtime timer rate/scale
	 */
	if (timer_freq == 0) {
		__asm__ __volatile__("mrs	%0, cntfrq_el0" : "=r"(timer_freq));
	}
	invert_timer_freq(qtime, timer_freq);

	/*
	 * Each core has its own generic timer registers with the timer interrupt
	 * routed to GIC PPI #27, so we force the system timer to cpu0's timer.
	 */
	qtime->flags |= QTIME_FLAG_TIMER_ON_CPU0;
	qtime->cycles_per_sec = timer_freq;
	qtime->intr = 27;

	/*
	 * Disable CNTV interrupt
	 */
	__asm__ __volatile__("msr	cntv_ctl_el0, %0" : : "r"(0));

	add_callout_array(timer_callouts, sizeof(timer_callouts));
}



#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/hardware/startup/lib/aarch64/init_qtime.c $ $Rev: 778261 $")
#endif
