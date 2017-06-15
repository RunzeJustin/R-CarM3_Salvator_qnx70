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
 * Implement PSCI invocation via hvc or smc call.
 * x0-x3 contain input parameters
 * x0 returns status
 *
 * The invocation is indirected via a function pointer, psci_call.
 * This defaults to using psci_smc to invoke PSCI functions.
 *
 * Platform specific code may change this to psci_hvc if required.
 */

	.data
	.align	4
	.global	psci_call

psci_call:	.xword	psci_smc

	.text
	.align 2

	.global	psci_hvc
	.global	psci_smc

psci_hvc:
	hvc	#0
	ret

	.type	psci_hvc, function
	.size	psci_hvc, . - psci_hvc

psci_smc:
	smc	#0
	ret

	.type	psci_smc, function
	.size	psci_smc, . - psci_smc

#ifdef __QNXNTO__
#ifdef __USESRCVERSION
.section .ident, "SM",%progbits,1;
.asciz "$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/hardware/startup/lib/aarch64/psci_call.S $ $Rev: 780681 $";
.previous
#endif
#endif
