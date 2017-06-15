/*
 * $QNXLicenseC:
 * Copyright 2014, 2016 QNX Software Systems.
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
#include <arm/scif.h>

static void parse_line(unsigned channel, const char *line, unsigned long *baud,
			unsigned long *clk, unsigned long *div, unsigned *extclk) {
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

void init_scif(unsigned channel, const char *init, const char *defaults)
{
	unsigned long	baud;
	unsigned long	div;
	unsigned long	clk;
	unsigned	n, extclk;

	baud = extclk = n = 0;
	dbg_device[channel].base = R_CAR_M3_SCIF_BASE2;
	parse_line(channel, defaults, &baud, &clk, &div, &extclk);
	parse_line(channel, init, &baud, &clk, &div, &extclk);

	// wait for one bit interval, then turn on send/receive
	for (n = 0; n < clk / 10000; ++n) {
		if ((n & 0x3ff) == 0)
			mdriver_check();
	}
}

void put_scif(int c)
{
	unsigned base = dbg_device[0].base;
	while( !(in16(base + SCIF_SCFSR_OFF) & SCIF_SCSSR_TDFE) );
	out8(base + SCIF_SCFTDR_OFF, c);
	out16(base + SCIF_SCFSR_OFF, 0);
	while( !(in16(base + SCIF_SCFSR_OFF) & SCIF_SCSSR_TEND) );
}

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/hardware/startup/lib/common_arm/hw_serscif.c $ $Rev: 807382 $")
#endif
