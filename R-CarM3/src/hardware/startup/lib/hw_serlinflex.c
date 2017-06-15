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
 * polled serial operations for LinFlexD controller which is used by the 
 * NXP S32V234, NXP S32V232
 *
 * NOTE - this code requires that the bootloader previously initialized the LINFLEXD module
 * and enabled FIFO mode (this code does not support buffer mode)
 */

#include "startup.h"
#include <hw/linflexd.h>

static void parse_line(unsigned channel, const char *line, unsigned *baud, unsigned *clk)
{
	/*
	 * Get device base address and register stride
	 */
	if (*line != '.' && *line != '\0') {
		dbg_device[channel].base = strtoul(line, (char **)&line, 16);
		if (*line == '^')
			dbg_device[channel].shift = strtoul(line+1, (char **)&line, 0);
	}

	/*
	 * Get baud rate
	 */
	if (*line == '.')
		++line;
	if (*line != '.' && *line != '\0')
		*baud = strtoul(line, (char **)&line, 0);

	/*
	 * Get clock rate
	 */
	if (*line == '.')
		++line;
	if (*line != '.' && *line != '\0')
		*clk = strtoul(line, (char **)&line, 0);
}

/*
 * Initialise one of the serial ports
 */

void init_linflexd(unsigned channel, const char *init, const char *defaults)
{
	unsigned baud, clk;

	/*
	 * Default perpherial clock rate
	 */
	clk = 66666667;

	parse_line(channel, defaults, &baud, &clk);
	parse_line(channel, init, &baud, &clk);

	if (baud == 0)
		return;
}

/*
 * Send a character
 */
void put_linflexd(int data)
{
	unsigned base = dbg_device[0].base;

	// Ensure TX FIFO is empty
	while (in8(base + LINFLEXD_UARTSR) & LINFLEXD_UARTSR_DTFTFF)
		;

	// Send character to UART to transmit
	out8(base + LINFLEXD_BDRL, (char)data);
}

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/hardware/startup/lib/hw_serlinflex.c $ $Rev: 801787 $")
#endif
