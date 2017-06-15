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
#include "libfdt.h"
#if defined(__arch64_)
#include <aarch64/psci.h>
#else
#include <arm/psci.h>
#endif

int
fdt_psci_configure(void) {
	if (psci_cpu_on_cmd != -1) {
		// already set
		return 1;
	}
	void *f = fdt;
	if(f == NULL) {
		// no FDT table
		return 0;
	}

	int const node = fdt_node_offset_by_compatible(f, -1, "arm,psci");
	if(node <= 0) {
		return 0;
	}
	const struct fdt_property	*prop;
	prop = fdt_get_property(f, node, "cpu_on", 0);
	if(fdt32_to_cpu(prop->len) < sizeof(fdt32_t)) {
		return 0;
	}
	const fdt32_t				*data;
	data = (const fdt32_t *)prop->data;
	psci_cpu_on_cmd = fdt32_to_cpu(data[0]);

	prop = fdt_get_property(f, node, "method", 0);
	if(fdt32_to_cpu(prop->len) == sizeof("smc") && !strncmp(prop->data, "smc", sizeof("smc"))) {
		psci_call = &psci_smc;
	} else if(fdt32_to_cpu(prop->len) == sizeof("hvc") && !strncmp(prop->data, "hvc", sizeof("hvc"))) {
		psci_call = &psci_hvc;
	} else {
		return 0;
	}
	return 1;
}

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/hardware/startup/lib/common_arm/fdt_psci_configure.c $ $Rev: 805440 $")
#endif
