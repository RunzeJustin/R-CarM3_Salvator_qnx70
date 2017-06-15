#
# Copyright 2015, QNX Software Systems. 
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
 * aarch64_cache_flush.S
 *
 * Clean and invalidate data caches
 *
 * This is the example code from the ARMv8 Architecture Reference Manual.
 * ARM DDI 0487 Section D3.4 Cache Support.
 */
 	
	.text
	.align	2

	.global aarch64_cache_flush

aarch64_cache_flush:
	mrs		x0, clidr_el1
	and		w3, w0, #0x07000000
	lsr		w3, w3, #23
	cbz		w3, done
	mov		w10, #0
	mov		w8, #1
loop1:
	add		w2, w10, w10, lsr #1
	lsr		w1, w0, w2
	and		w1, w1, #7
	cmp		w1, #2
	blt		skip
	msr		csselr_el1, x10
	isb
	mrs		x1, ccsidr_el1
	and		w2, w1, #7
	add		w2, w2, #4
	ubfx	w4, w1, #3, #10
	clz		w5, w4
	lsl		w9, w4, w5
	lsl		w16, w8, w5
loop2:
	ubfx	w7, w1, #13, #15
	lsl		w7, w7, w2
	lsl		w17, w8, w2
loop3:
	orr		w11, w10, w9
	orr		w11, w11, w7
	dc		cisw, x11
	subs	w7, w7, w17
	bge		loop3
	subs	x9, x9, x16
	bge		loop2
skip:
	add		w10, w10, #2
	cmp		w3, w10
	dsb		 sy
	bgt		loop1
	dsb		sy
done:
	ret

#ifdef __QNXNTO__
#ifdef __USESRCVERSION
.section .ident, "SM",%progbits,1;
.asciz "$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/hardware/startup/lib/aarch64/aarch64_cache_flush.S $ $Rev: 780681 $";
.previous
#endif
#endif
