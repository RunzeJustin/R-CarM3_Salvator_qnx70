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

#include "startup.h"
#include <arm/r-car-m3.h>

static void irqc_irq_unmask(uint32_t irq)
{
    uint32_t base;

    base = (irq >= 32) ? RCAR_IRQC1_BASE : RCAR_IRQC0_BASE;
    out32(base + RCAR_INTEN_SET0, in32(base + RCAR_INTEN_SET0) | (1 << (irq & 0x1F)));
}

void init_intrinfo()
{
    unsigned    i;
    paddr_t     irqc_base0 = RCAR_IRQC0_BASE;
    paddr_t     irqc_base1 = RCAR_IRQC0_BASE;

    gic_v2_init(RCAR_GIC_DIST_BASE, RCAR_GIC_CPU_BASE);

    /* Enable IRQC & INTC-SYS clock */
    out32(RCAR_CPG_BASE + RCAR_CPG_SMSTPCR4,
            in32(RCAR_CPG_BASE + RCAR_CPG_SMSTPCR4) & ~((1 << 7) | (1 << 8)));

    /* Mask IRQC interrupts */
    out32(irqc_base0 + RCAR_INTEN_STS0, 0x0000001F);
    out32(irqc_base1 + RCAR_INTEN_STS0, 0x0000001F);

    /* Clear IRQC interrupts */
    out32(irqc_base0 + RCAR_DETECT_STATUS, 0x0000001F);
    out32(irqc_base1 + RCAR_DETECT_STATUS, 0x0000001F);

    /* Enable IRQC 1: Network interrupt */
    irqc_irq_unmask(1);
    irqc_irq_unmask(0);

    /* All interrupts high-level */
    for (i = 0; i < 5; i += 1) {
        out32(irqc_base0 + RCAR_CONFIG_00 + i*4, (in32(irqc_base0 + RCAR_CONFIG_00 + i*4) & ~0x3F) | 0x2);
        out32(irqc_base1 + RCAR_CONFIG_00 + i*4, (in32(irqc_base1 + RCAR_CONFIG_00 + i*4) & ~0x3F) | 0x2);
    }
}

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/hardware/startup/boards/rcar_m3/aarch64/init_intrinfo.c $ $Rev: 810496 $")
#endif
