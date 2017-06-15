/*
 * $QNXLicenseC:
 * Copyright 2008, QNX Software Systems. 
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
#include <hw/8250.h>

/*
 * Drive an 8250 style serial chip.
 */

/*
 * aspace[]
 * this array allows us to record, on a per 'dbg_dvice[]' basis, the address
 * space to use for the access.
 * It is intentionally not part of the 'chip_info' structure. Refer to the
 * code review for discussion
 */
static unsigned aspace[NUM_ELTS(dbg_device)] = {0};

static void
parse_line(unsigned channel, const char *line, unsigned long *baud, unsigned long *clk, unsigned long *div) {
	
    if(*line != '.' && *line != '\0') {
		dbg_device[channel].base = strtopaddr(line, (char **)&line, 16);
		if(*line == '^') dbg_device[channel].shift = strtoul(line+1, (char **)&line, 0);
	}
    if(*line == '.') ++line;
    if(*line != '.' && *line != '\0') *baud = strtoul(line, (char **)&line, 0);
    if(*line == '.') ++line;
    if(*line != '.' && *line != '\0') *clk = strtoul(line, (char **)&line, 0);
    if(*line == '.') ++line;
    if(*line != '.' && *line != '\0') *div = strtoul(line, (char **)&line, 0);
}

void
init_8250(unsigned channel, const char *init, const char *defaults) {
	unsigned long 	baud;
	unsigned long	div;
	unsigned long	clk;

	baud = 0;
	parse_line(channel, defaults, &baud, &clk, &div);
	parse_line(channel, init, &baud, &clk, &div);

	init_8250_common(channel, baud, clk, div);
}

void
init_8250_common(unsigned const channel, unsigned long const baud, unsigned long const clk,
	unsigned long const div)
{
	paddr_t			base;
	unsigned		shift;

	/*
	 * FIX ME
	 * For now, a base address >= 64K is considered an MMIO space access
	 * otherwise, we default to I/O accesses
	 */
	aspace[channel] = (dbg_device[channel].base >= (paddr_t)(64 * 1024));

	base = dbg_device[channel].base;
	shift = dbg_device[channel].shift;

	/*
	 * FIX ME
	 * For some reason the remainder of this function doesn't work for the
	 * memory mapped UART devices on the Apollo Lake/Broxton platforms that
	 * use MMIO accesses. This needs more investigation
	 */
	if (aspace[channel] == 0)
	{
		chip_access(base, shift, aspace[channel], REG_MS);

		// Wait for all pending characters to be output...
		do {
		} while(!(chip_read8(REG_LS) & LSR_TSRE));

		if(baud != 0) {
			unsigned count = clk / (baud * div);

			// Program divisor latch
			chip_write8(REG_LC, LCR_DLAB);
			chip_write8(REG_DL0, count & 0xff);
			chip_write8(REG_DL1, count >> 8);
			chip_write8(REG_LC, 0x03);
		}
	
		// 8 bits no parity
		chip_write8(REG_MC, (chip_read8(REG_MC) & 0xE4) | MCR_DTR|MCR_RTS|MCR_OUT2);
		chip_done();
	}
}

void
put_8250(int c) {
	const paddr_t	base = dbg_device[0].base;
	const unsigned	shift = dbg_device[0].shift;

	chip_access(base, shift, aspace[0], REG_MS);
	do {
	} while(!(chip_read8(REG_LS) & LSR_TXRDY));
	chip_write8(REG_TX, c);
	chip_done();
}

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/hardware/startup/lib/hw_ser8250.c $ $Rev: 803983 $")
#endif
