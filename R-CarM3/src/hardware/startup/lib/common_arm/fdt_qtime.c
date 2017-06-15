/*
 * $QNXLicenseC:
 * Copyright 2015, QNX Software Systems.
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
#include <libfdt.h>

#if defined(__aarch64__)
	#define COMPAT_STRING "arm,armv8-timer"
#else
	#define COMPAT_STRING "arm,armv7-timer"
#endif

int
fdt_qtime(unsigned *freqp, unsigned *intrp)
{
	unsigned            cntfrq = 0;
	unsigned            intr = 0;

	void					*f = fdt;
	int						node;

	if(fdt == NULL) return -1;

	if((node = fdt_node_offset_by_compatible(fdt, -1, COMPAT_STRING)) > 0) {
		const struct fdt_property	*prop;
		int							len;

		if((prop = fdt_get_property(f, node, "interrupts", &len))) {
			const fdt32_t	*mem = (const fdt32_t *)prop->data;

			if(len >= 9 * sizeof *mem) {
				intr = fdt32_to_cpu(mem[7]) + 16;	// Skip over SGIs
				if(fdt32_to_cpu(mem[6]) == 0) {
					intr += 16;	// Add 16 for SPIs
				}
			} else {
				crash("no valid interrupt information!\n");
			}
		} else {
			crash("no interrupts property!\n");
		}

		if((prop = fdt_get_property(f, node, "clock-frequency", &len))) {
			if(len >= sizeof(fdt32_t)) {
				const fdt32_t *cnt = (const fdt32_t *)prop->data;
				cntfrq = fdt32_to_cpu(cnt[0]);
			} else {
				crash("no valid clock-frequency property!\n");
			}
		} else {
			crash("no clock-frequency property!\n");
		}

	} else {
		crash("no timer found!\n");
	}

	*freqp = cntfrq;
	*intrp = intr;
	return 0;
}

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/hardware/startup/lib/common_arm/fdt_qtime.c $ $Rev: 810941 $")
#endif
