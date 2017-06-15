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
 * APE6 system reboot
 */

#include "callout.ah"
#include <arm/r-car-m3.h>

/*
 * -----------------------------------------------------------------------
 * Routine to patch callout code
 *
 * On entry:
 *  r0 - physical address of syspage
 *  r1 - virtual  address of syspage
 *  r2 - offset from start of syspage to start of the callout routine
 *  r3 - offset from start of syspage to read/write data used by callout
 * -----------------------------------------------------------------------
 */
patch_reboot:
    sub     sp, sp, #16
    stp     x19, x30, [sp]

    add     x19, x0, x2     // x19 = address of callout routine

    /*
     * Map reset control registers
     */
    mov     x0, #0x100
    adr     x1, Lpaddr
    bl      callout_io_map_indirect

    /*
     * Patch the callout routine
     */
    CALLOUT_PATCH   x19, w4, w5

    ldp     x19, x30, [sp]
    add     sp, sp, #16
    ret

            .align 3
Lpaddr:     .quad    RCAR_RWDT_BASE


CALLOUT_START(reboot_rcarm3, 0, patch_reboot)
    /*
     * Get the Watchdog Timer base address (patched)
     */
    mov     x5, #0xabcd             // UART base address (patched)
    movk    x5, #0xabcd, lsl #16
    movk    x5, #0xabcd, lsl #32
    movk    x5, #0xabcd, lsl #48

_do_reboot:
    movz    w2, #0xFFFF
    movk    w2, #0x5A5A, lsl #16
    str     w2, [x5, #RCAR_WDT_CNT]

    movz    w2, #0xA580
    movk    w2, #0xA5A5, lsl #16
    str     w2, [x5, #RCAR_WDT_CSRA]

0:  b       0b
CALLOUT_END(reboot_rcarm3)

#ifdef __QNXNTO__
#ifdef __USESRCVERSION
.section .ident, "SM",%progbits,1;
.asciz "$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/hardware/startup/boards/rcar_m3/aarch64/callout_reboot_rcarm3.S $ $Rev: 807455 $";
.previous
#endif
#endif
