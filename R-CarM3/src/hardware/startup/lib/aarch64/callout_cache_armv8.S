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
 * ARMv8 generic cache operations
 */

#include "callout.ah"

#define	WHOLE_CACHE	1024
#define	MAX_LINES	32

/*
 * unsigned control(paddr32_t base,
 *					unsigned num_lines,
 *					int flags,
 *					struct cacheattr_entry *cache,
 *					volatile struct syspage_entry * )
 */
CALLOUT_START(cache_armv8_icache, 0, 0)
	/*
	 * For a huge request, just invalidate the whole cache
	 */
	cmp		x1, #WHOLE_CACHE
	b.lo	0f
	ic		ialluis
	dsb		sy
	isb
	mov		x0, #0				// indicate whole cache flushed
	ret

	/*
	 * Get minimum line size from CTR
	 */
0:	mrs		x4, ctr_el0
	and		x4, x4, #0xf
	mov		x3, #4
	lsl		x3, x3, x4

	/*
	 * Limit to MAX_LINES in case we get preempted.
	 */ 
	cmp		x1, #MAX_LINES
	mov		x4, #MAX_LINES
	csel	x1, x1, x4, ls
	mov		x2, x0
	mov		x0, x1				// return value is number of lines processed
	
0:	ic		ivau, x2
	add		x2, x2, x3
	subs	x1, x1, #1
	b.ne	0b
	dsb		sy
	isb
	ret
CALLOUT_END(cache_armv8_icache)

/*
 * unsigned control(paddr32_t base,
 *					unsigned num_lines,
 *					int flags,
 *					struct cacheattr_entry *cache,
 *					volatile struct syspage_entry * )
 */
CALLOUT_START(cache_armv8_dcache, 0, 0)
	/*
	 * Get minimum line size from CTR
	 */
	mrs		x4, ctr_el0
	lsr		x4, x4, #16
	and		x4, x4, #0xf
	mov		x3, #4
	lsl		x3, x3, x4

	/*
	 * Limit to MAX_LINES in case we get preempted.
	 */ 
	cmp		x1, #MAX_LINES
	mov		x4, #MAX_LINES
	csel	x1, x1, x4, ls
	tst		x2, #MS_INVALIDATE
	mov		x2, x0
	mov		x0, x1				// return value is number of lines processed
	b.ne	1f
	
	/*
	 * Clean by VA to PoC
	 */
0:	dc		cvac, x2
	add		x2, x2, x3
	subs	x1, x1, #1
	b.ne	0b
	dsb		sy
	isb
	ret
	
	/*
	 * Clean and invalidate by VA to PoC
	 */
1:	dc		civac, x2
	add		x2, x2, x3
	subs	x1, x1, #1
	b.ne	1b
	dsb		sy
	isb
	ret

CALLOUT_END(cache_armv8_dcache)

#ifdef __QNXNTO__
#ifdef __USESRCVERSION
.section .ident, "SM",%progbits,1;
.asciz "$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/hardware/startup/lib/aarch64/callout_cache_armv8.S $ $Rev: 780681 $";
.previous
#endif
#endif
