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
 * _start.S
 *
 * This code runs as the first instructions after control has transferred
 * from any bootup monitor.  This module defines a minimal bootup stack,
 * and sets SP to this stack, and then starts running C code in _main().
 *
 * For the ARMv8 Fast Model, all processors execute from cold reset in EL3.
 * This code sets up minimal EL2/EL3 state for all cpus and switches to EL1
 * to continue execution:
 * - cpu0 jumps to cstart to perform the startup initialisation
 * - other cpus spin until they are woken up by board_smp_start
 */

    .text
    .align 2

/*
 * EL3 needs to perform some initial GIC configuration
 */
#define GIC_BASE            0xf1000000
#define GICC_PMR_OFF        0x2004
#define GICD_IGROUPR_OFF    0x1080

    .extern cstart
    .extern _start_el1
    .global _start

_start:
    /*
     * Set up EL3 state if necessary
     */
    mrs     x0, CurrentEL
    cmp     x0, #0xc                // EL3
    bne     0f
    bl      setup_el3
0:

    /*
     * Model starts all cores at the ELF entry point.
     * Secondary cores wait until the boot cpu wakes them up.
     *
     * FIXME_AARCH64: this will need work to handle multiple clusters.
     */
    mrs     x0, mpidr_el1
    and     x0, x0, #0xff           // FIXME_AARCH64: assumes only one cluster
    cbnz    x0, 1f
    /*
     * Switch to EL1 and then call cstart
     */
    bl      _start_el1
    b       cstart

    /*
     * FIXME: increment the number of processors so board_smp_num_cpus()
     *        knows how many processors are present.
     *        This assumes all secondary processors get through this code
     *        before board_smp_num_cpus() executes on the boot processor.
     */
1:  adr     x1, board_smp_max_cpu
0:  ldaxr   w2, [x1]
    add     w2, w2, #1
    stlxr   w3, w2, [x1]
    cbnz    w3, 0b

    /*
     * Wait until board_smp_start sets secondary_cpu to our core number.
     */
    adr     x1, secondary_start
    adr     x2, secondary_cpu
0:  wfe
    dmb     sy
    ldr     x3, [x2]
    cmp     x3, x0
    bne     0b
    ldr     x3, [x1]
    br      x3

    /*
     * Setup EL3 state:
     * - return to EL1 at caller's return address
     * - set EL1/EL2 to AArch64
     * - set minimal GIC state
     */
setup_el3:
    mov     x1, #((1<<10)|(1<<0))   // SCR_EL3.RW and NS set
    msr     scr_el3, x1             // Set EL2/EL1/EL0 to AArch64

    /*
     * Set Priority Mask to enable forwarding of interrupts to cpu interface.
     * Configure interrupts as insecure Group1 interrupts.
     */
    mov     x1, #GIC_BASE
    mov     w2, #0xf0
    str     w2, [x1, #GICC_PMR_OFF]
    mov     w2, #0xffffffff
    str     w2, [x1, #GICD_IGROUPR_OFF]
    str     w2, [x1, #GICD_IGROUPR_OFF+4]
    str     w2, [x1, #GICD_IGROUPR_OFF+8]

    /*
     * Set default generic timer frequency
     */
    ldr     w1, cntfrq_default
    msr     cntfrq_el0, x1

    /*
     * Return to caller, executing at EL2
     */
    msr     elr_el3, x30
    mov     x1, #0x3c9              // DAIF + EL2h
    msr     spsr_el3, x1
    eret

cntfrq_default: .word   100000000   // bogus default cntfrq_el0 value

#ifdef __QNXNTO__
#ifdef __USESRCVERSION
.section .ident, "SM",%progbits,1;
.asciz "$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/hardware/startup/boards/rcar_m3/aarch64/_start.S $ $Rev: 807455 $";
.previous
#endif
#endif
