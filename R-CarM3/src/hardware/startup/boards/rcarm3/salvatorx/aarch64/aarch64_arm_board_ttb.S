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


/*
 * aarch64_arm_board_ttb.S
 *
 */

#define SDRAM_BASE  0x40000000
#define SDRAM_SIZE  ((1024 - 32) * 1024 * 1024)

    .data
    .globl  aarch64_arm_board_ttb

    .align  14
aarch64_arm_board_ttb:
    /* Uncacheable */
    .set    XCount, ((1 << 0) | (1 << 10))
    .rept   (SDRAM_BASE >> 29)
    .dword  XCount
    .set    XCount, XCount + 0x20000000
    .endr

    /* SDRAM, cacheable and bufferable */
    .set    XCount, (XCount | (4 << 2))
    .rept   (SDRAM_SIZE >> 29)
    .dword   XCount
    .set    XCount, XCount + 0x20000000
    .endr

    /* Uncacheable */
    .set    XCount, (XCount & ~(4 << 2))
    .rept   (0x2000 - ((SDRAM_BASE + SDRAM_SIZE) >> 29))
    .dword   XCount
    .set    XCount, XCount + 0x20000000
    .endr

#ifdef __QNXNTO__
#ifdef __USESRCVERSION
.section .ident, "SM",%progbits,1;
.asciz "$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/hardware/startup/boards/rcar_m3/aarch64/aarch64_arm_board_ttb.S $ $Rev: 807455 $";
.previous
#endif
#endif
