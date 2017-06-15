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
 * requests PSCI:SYSTEM_RESET (0x84000009) to reset the system
 */

#include "callout.ah"

CALLOUT_START(reboot_psci_hvc, 0, 0)
	tbz		w1, #0,do_reboot_hvc	// w1 == abnormal reboot flag
	ldurh	w2, [x0, #SYSPAGE_SYSTEM_PRIVATE] // offset to system_private
	add		x0, x0, x2			  // system_private address
	ldr		x2, [x0, #SP_PRIVATE_FLAGS]
	tst		x2, #SYSTEM_PRIVATE_FLAG_ABNORMAL_REBOOT
	beq		spin_hvc		// spin if ABNORMAL_REBOOT is NOT set

do_reboot_hvc:
	mov		x0,#0x0009
	movk	x0,#0x8400, lsl #16
	hvc		#0
	// spin in case something goes wrong with request
spin_hvc:
	b		spin_hvc
CALLOUT_END(reboot_psci_hvc)

CALLOUT_START(reboot_psci_smc, 0, 0)
	tbz		w1, #0, do_reboot_smc	// w1 == abnormal reboot flag
	ldurh	w2, [x0, #SYSPAGE_SYSTEM_PRIVATE] // offset to system_private
	add		x0, x0, x2			  // system_private address
	ldr		x2, [x0, #SP_PRIVATE_FLAGS]
	tst		x2, #SYSTEM_PRIVATE_FLAG_ABNORMAL_REBOOT
	beq		spin_smc		// spin if ABNORMAL_REBOOT is NOT set

do_reboot_smc:
	mov		x0,#0x0009
	movk	x0,#0x8400, lsl #16
	smc		#0
	// spin in case something goes wrong with request
spin_smc:
	b		spin_smc
CALLOUT_END(reboot_psci_smc)

#ifdef __QNXNTO__
#ifdef __USESRCVERSION
.section .ident, "SM",%progbits,1;
.asciz "$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/hardware/startup/lib/aarch64/callout_reboot_psci.S $ $Rev: 810941 $";
.previous
#endif
#endif
