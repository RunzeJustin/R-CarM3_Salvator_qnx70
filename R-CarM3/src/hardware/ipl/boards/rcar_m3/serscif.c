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

#include "ipl.h"
#include <hw/inout.h>
#include <arm/r-car.h>
#include <arm/scif.h>

extern unsigned scif_base;

void my_delay(int dly)
{
    volatile int i = dly;

    while (i--)
        ;
}

void serscif_putchar(unsigned char c)
{

    while (!(in16(scif_base + SCIF_SCFSR_OFF) & 0x60));
    
	out8(scif_base + SCIF_SCFTDR_OFF, c);
    
	out16(scif_base + SCIF_SCFSR_OFF, in16(scif_base + SCIF_SCFSR_OFF) & ~0x60);    /* TEND,TDFE clear */
    
}

unsigned char serscif_pollkey(void)
{
    if (in16(scif_base + SCIF_SCFSR_OFF) & 0x90)
        out16(scif_base + SCIF_SCFSR_OFF, in32(scif_base + SCIF_SCFSR_OFF) & ~0x90);

    if (in16(scif_base + SCIF_SCFDR_OFF) & 0x1F)
        return 1;
    else
        return 0;
}

unsigned char serscif_getchar(void)
{
    unsigned char c;

    while (serscif_pollkey() == 0)
        ;

    c = in8(scif_base + SCIF_SCFRDR_OFF);

    out16(scif_base + SCIF_SCFSR_OFF, in32(scif_base + SCIF_SCFSR_OFF) & ~0x03);

    return c;
}

void
init_serscif(unsigned address, unsigned baud, unsigned clk)
{
    in16(address + SCIF_SCLSR_OFF);             /* dummy read */
    out16(address + SCIF_SCLSR_OFF, 0x0000);    /* clear ORER bit */
    out16(address + SCIF_SCFSR_OFF, 0x0000);    /* clear all error bit */

    out16(address + SCIF_SCSCR_OFF, 0x0000);    /* clear SCR.TE & SCR.RE*/
    out16(address + SCIF_SCFCR_OFF, 0x0006);    /* reset tx-fifo, reset rx-fifo. */

    // FIXME!!!
    // The baud rate is fixed to 38400
    out16(address + SCIF_SCSCR_OFF, 0x0002);    /* external clock, SC_CLK pin used for input pin */
    out16(address + SCIF_SCSMR_OFF, 0x0000);    /* 8bit data, no-parity, 1 stop, Po/1 */
    my_delay(100);
    out16(address + SCIF_DL_OFF, 0x0008);       /* 14.7456MHz / (38400*16) = 24 */
    out16(address + SCIF_CKS_OFF, 0x0000);      /* select scif_clk	 */
    my_delay(100);
    out16(address + SCIF_SCFCR_OFF, 0x0000);    /* reset-off tx-fifo, rx-fifo. */
    out16(address + SCIF_SCSCR_OFF, 0x0032);    /* enable TE, RE; SC_CLK=input */
    my_delay(100);

    scif_base = address;

}

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL$ $Rev$")
#endif
