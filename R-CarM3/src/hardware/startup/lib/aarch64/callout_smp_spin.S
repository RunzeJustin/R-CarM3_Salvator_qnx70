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

#include "callout.ah"

/*
 * This code is used as the virtual entry point for secondary processors.
 *
 * As the kernel initialises each processor, it will store the address
 * of its secondary processor initialisation code into the syspage smp
 * entry's start_addr field.
 *
 * We atomically exchange this so that only one processor at a time will
 * pick up the address and jump to it.
 *
 * x0 contains the _syspage_ptr
 * w1 contains the cpu number
 */

CALLOUT_START(smp_spin, 0, 0)
	ldrh	w2, [x0, #SYSPAGE_SMP]	// smp.entry_off
	add		x2, x0, x2

	/*
	 * Set smp.pending to 0 to tell transfer_aps() to start the next cpu.
	 * cpu0 is still running with MMU/caches disabled so we need to flush
	 * the data cache line for smp.pending so cpu0 sees the write.
	 */
	add		x3, x2, #SMP_PENDING
	str		wzr, [x3]
	dc		cvac, x3
	dsb		sy

	/*
	 * Wait until syspage->smp.start_addr is non-zero then atomically
	 * exchange it with 0 so only this cpu comes out of the loop
	 */
	add		x3, x2, #SMP_START_ADDR
0:	ldaxr	x4, [x3]
	cbnz	x4, 1f
	wfe
	b		0b
1:	stlxr	w5, xzr, [x3]
	cbnz	w5, 0b

	/*
	 * Jump to start_addr.
	 * If it is -1, the kernel wants us to shut down so put the -1 back
	 * and wake up any cores that in the wfe in the loop above.
	 */
	cmn		x4, #1
	beq		2f
	br		x4

2:	str		x4, [x3]
	dsb		sy
	sev
3:	wfi
	b		3b
CALLOUT_END(smp_spin)

#ifdef __QNXNTO__
#ifdef __USESRCVERSION
.section .ident, "SM",%progbits,1;
.asciz "$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/hardware/startup/lib/aarch64/callout_smp_spin.S $ $Rev: 780681 $";
.previous
#endif
#endif
