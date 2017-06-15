/*
 * $QNXLicenseC:
 * Copyright 2016, QNX Software Systems.
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

     .text
    .align  2

    .globl board_startup_mmu
    .globl board_disable_mmu

board_startup_mmu:
    sub     sp, sp, #8
    str     x30, [sp]

    // Check if D-Cache is already enabled
    mrs     x0, sctlr_el1
    tbnz    x0, #2, done

    // Set Translate Table Base
    ldr     x0, =aarch64_arm_board_ttb
    msr     ttbr0_el1, x0

    // TCR_EL1 Configuration
    mov     x0, #22             // 42 bits virtual address
    orr     x0, x0, #1 << 14    // AARCH64_TCR_EL1_TG0_64K
    orr     x0, x0, #1 << 10    // AARCH64_TCR_EL1_ORGN0_WBWA
    orr     x0, x0, #1 << 8     // AARCH64_TCR_EL1_IRGN0_WBWA
    orr     x0, x0, #3 << 32    // 42 bits physical address
    msr     tcr_el1, x0

    /*
     * Default MAIR value.
     * Index  MAIR  ARMv8 meaning
     * -----  ----  ------------- -------------
     *     0  0x00  device-nGnRnE strongly-ordered
     *     1  0x04  device-nGnRE  shared-device
     *     2  0x0C  device-GRE    shared-device
     *     3  0x44  normal-NC     normal-NC
     *     4  0xff  normal-NT     normal-NT
     *     5  0x00  reserved      reserved
     *     6  0x00  normal        normal
     *     7  0x00  normal        normal
     */
    ldr     x0, =0x000000ff440C0400
    msr     mair_el1, x0
    isb

    // Enable MMU, Dcache, Icache
    mrs     x0, sctlr_el1
    orr     x0, x0, #(1 << 12)  // Icache
    orr     x0, x0, #(1 << 2)   // Dcache
    orr     x0, x0, #(1 << 0)   // MMU
    msr     sctlr_el1, x0
    isb

    b       done

board_disable_mmu:
    sub     sp, sp, #8
    str     x30, [sp]

    // Check if MMU is enabled
    mrs     x0, sctlr_el1
    tbz     x0, #0, done

    // Disable Dcache and MMU
    mrs     x0, sctlr_el1
    bic     x0, x0, #(1 << 0)   // MMU
    bic     x0, x0, #(1 << 2)   // DCache
    msr     sctlr_el1, x0
    isb

    // Cache Flush
    bl      aarch64_cache_flush

    // Invalidate TLB All
    ic      iallu
    tlbi    vmalle1
    dsb     sy

done:
    ldr     x30, [sp]
    add     sp, sp, #8
    ret

#ifdef __QNXNTO__
#ifdef __USESRCVERSION
.section .ident, "SM",%progbits,1;
.asciz "$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/hardware/startup/boards/rcar_m3/aarch64/board_mmu.S $ $Rev: 807455 $";
.previous
#endif
#endif
