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

void gpio_add_driver(const char *dll, const char *args) {
    unsigned hwi_off = hwidev_add("gpio,driver", 0, HWI_NULL_OFF);

    hwitag_add_optstr(hwi_off, dll);
    if(args && args[0] != '\0') {
        hwitag_add_optstr(hwi_off, args);
    }
}

void gpio_add_native_named_port(const char *device, const char *name, unsigned port) {
    unsigned hwi_off = hwidev_add("gpio,native", 0, HWI_NULL_OFF);

    hwitag_add_optstr (hwi_off, device);
    hwitag_add_optstr (hwi_off, name);
    hwitag_add_regname(hwi_off, "port", port);
}

void gpio_add_virtual_named_port(const char *device, const char *name, unsigned port, unsigned lsb, unsigned msb) {
    unsigned hwi_off = hwidev_add("gpio,virtual", 0, HWI_NULL_OFF);

    hwitag_add_optstr (hwi_off, device);
    hwitag_add_optstr (hwi_off, name);
    hwitag_add_regname(hwi_off, "port", port);
    hwitag_add_regname(hwi_off, "lsb",  lsb);
    hwitag_add_regname(hwi_off, "msb",  msb);
}

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/hardware/startup/lib/gpio-framework.c $ $Rev: 783974 $")
#endif
