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
#include <hw/pci.h>

/*
 * Drive an 8250 style serial chip whose registers are inside a PCI device's BAR.
 */

struct ser8250_pci_data {
	unsigned long	baud;
	unsigned long	clk;
	unsigned long	div;
	unsigned		pci_loc[4];
};

static void
enable_aspace(const unsigned bus, const unsigned dev, const unsigned func, const unsigned bit)
{
	const _Uint16t bit_mask = (1u << bit);
	const _Uint16t cmd = pci_read_cfg16(bus, dev, func, 0x4);

	if ((cmd & bit_mask) == 0) pci_write_cfg16(bus, dev, func, 0x4, cmd | bit_mask);
}
static inline void
enable_mem_aspace(const unsigned bus, const unsigned dev, const unsigned func)
{
	enable_aspace(bus, dev, func, 1);
}
static inline void
enable_io_aspace(const unsigned bus, const unsigned dev, const unsigned func)
{
	enable_aspace(bus, dev, func, 0);
}

static void
parse_line(unsigned channel, const char *line, struct ser8250_pci_data * const data)
{
    if(*line != '.' && *line != '\0') {
		unsigned i;
		for (i = 0; i < 4; ++i) {
			data->pci_loc[i] = (unsigned)strtoul(line, (char **)&line, 0);
			if ((*line == '^') || (*line == '.') || (*line == '\0')) break;
			++line;
		}

		unsigned const bus = data->pci_loc[0];
		unsigned const dev = data->pci_loc[1];
		unsigned const func = data->pci_loc[2];
		unsigned const bar = data->pci_loc[3];
		paddr_t bar_addr = pci_read_cfg32(bus, dev, func, 0x10 + bar*4);

		if (PCI_IS_MEM(bar_addr)) {
			if (PCI_IS_MMAP64(bar_addr)) {
				bar_addr |= (paddr_t)pci_read_cfg32(bus, dev, func, 0x14 + bar*4) << 32;
			}
			bar_addr = PCI_MEM_ADDR(bar_addr);
			enable_mem_aspace(bus, dev, func);
		} else {
			bar_addr = PCI_IO_ADDR(bar_addr);
			enable_io_aspace(bus, dev, func);
		}

		dbg_device[channel].base = bar_addr;

		if(*line == '^') dbg_device[channel].shift = strtoul(line+1, (char **)&line, 0);
	}
    if(*line == '.') ++line;
    if(*line != '.' && *line != '\0') data->baud = strtoul(line, (char **)&line, 0);
    if(*line == '.') ++line;
    if(*line != '.' && *line != '\0') data->clk = strtoul(line, (char **)&line, 0);
    if(*line == '.') ++line;
    if(*line != '.' && *line != '\0') data->div = strtoul(line, (char **)&line, 0);
}

void
init_8250_pci(unsigned channel, const char *init, const char *defaults) {
	struct ser8250_pci_data data = {
		.baud = 0
	};
	parse_line(channel, defaults, &data);
	parse_line(channel, init, &data);

	init_8250_common(channel, data.baud, data.clk, data.div);
}

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/hardware/startup/lib/hw_ser8250_pci.c $ $Rev: 810472 $")
#endif
