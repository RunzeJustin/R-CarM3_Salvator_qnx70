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

#include "proto.h"

int
rcar_i2c_set_slave_addr(void *hdl, unsigned int addr, i2c_addrfmt_t fmt)
{
    rcar_i2c_dev_t  *dev = hdl;

    /* check supported format */
    if (fmt != I2C_ADDRFMT_7BIT) {
        rcar_i2c_slogf(dev, VERBOSE_QUIET, "rcar_i2c_set_slave_addr: unsupported format (%08x)", fmt);
        return -1;
    }

    dev->slave_addr_fmt = fmt;
    dev->slave_addr     = addr;

    rcar_i2c_slogf(dev, VERBOSE_LEVEL8, "rcar_i2c_set_slave_addr: slave_addr[%02x] fmt[%08x]", dev->slave_addr, dev->slave_addr_fmt);

    return 0;
}

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/hardware/i2c/rcar-A/slave_addr.c $ $Rev: 805407 $")
#endif
