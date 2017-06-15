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
 * RCar-M3 specific WDT support.
 * RWDT is used.
 */

#include "startup.h"
#include <arm/r-car-m3.h>


int wdt_enable()
{
    volatile int i = 0;

    /* Supply clock to WDT */
    out32(RCAR_CPG_BASE + RCAR_CPG_SMSTPCR4, in32(RCAR_CPG_BASE + RCAR_CPG_SMSTPCR4) & ~(1 << 2));

    /* Enable Generating internal reset when RWDT & SWDT overflow */
    out32(RCAR_RESET_BASE + RCAR_WDTRSTCR, (0xA55A << 16) | 0x0002);

    /* for RWDT */
   out32(RCAR_RWDT_BASE + RCAR_WDT_CSRA, (0xA5A5A5 << 8) | (in8(RCAR_RWDT_BASE + RCAR_WDT_CSRA) & ~(1 << 7))); // stop timer
   out32(RCAR_RWDT_BASE + RCAR_WDT_CSRA, (0xA5A5A5 << 8) | (in8(RCAR_RWDT_BASE + RCAR_WDT_CSRA) & ~(1 << 4)) | 0x7);
   out32(RCAR_RWDT_BASE + RCAR_WDT_CSRB, (0xA5A5A5 << 8) | 0x21);  // overflow after 1 minute
   while (in8(RCAR_RWDT_BASE + RCAR_WDT_CSRA) & (1 << 5))
       i++;

    /* for SWDT */
    // out32(RCAR_SWDT_BASE + RCAR_WDT_CSRA, (0xA5A5A5 << 8) | (in8(RCAR_SWDT_BASE + RCAR_WDT_CSRA) & ~(1 << 7))); // stop timer
    // out32(RCAR_SWDT_BASE + RCAR_WDT_CSRA, (0xA5A5A5 << 8) | (in8(RCAR_SWDT_BASE + RCAR_WDT_CSRA) & ~(1 << 4)) | 0x7);
    // out32(RCAR_SWDT_BASE + RCAR_WDT_CSRB, (0xA5A5A5 << 8) | 0x21);  // overflow after 1 minute
    // while (in8(RCAR_SWDT_BASE + RCAR_WDT_CSRA) & (1 << 5))
    //     i++;

   out32(RCAR_RWDT_BASE + RCAR_WDT_CSRA, (0xA5A5A5 << 8) | (in8(RCAR_RWDT_BASE + RCAR_WDT_CSRA) | (1 << 7)));  // start RWDT
    // out32(RCAR_SWDT_BASE + RCAR_WDT_CSRA, (0xA5A5A5 << 8) | (in8(RCAR_SWDT_BASE + RCAR_WDT_CSRA) | (1 << 7)));  // start SWDT

    return 1;
}

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/hardware/startup/boards/rcar_m3/init_wdt.c $ $Rev: 805185 $")
#endif
