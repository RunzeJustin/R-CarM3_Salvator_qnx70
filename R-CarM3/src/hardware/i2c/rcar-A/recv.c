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


i2c_status_t
rcar_i2c_recv(void *hdl, void *buf, unsigned int len, unsigned int stop)
{
    rcar_i2c_dev_t  *dev = hdl;

    rcar_i2c_slogf(dev, VERBOSE_LEVEL8, "I2C rcar_i2c_recv: ");

    if (len == 0) {
        rcar_i2c_slogf(dev, VERBOSE_LEVEL4, "rcar_i2c_recv: I2C_STATUS_DONE");
        dev->status = I2C_STATUS_DONE;
        return 0;
    }

    if (dev->slave_addr_fmt != I2C_ADDRFMT_7BIT) {
        rcar_i2c_slogf(dev, VERBOSE_QUIET, "rcar_i2c_recv: unsupported format[%08x]",dev->slave_addr_fmt);
        errno = EINVAL;
        return -1;
    }

    dev->mode   = CMODE_RECV | (stop ? CMODE_DOSTOP : 0);
    dev->status = 0;
    dev->buf    = buf;
    dev->len    = len;

    out32(dev->regbase + RCAR_I2C_ICMAR, (dev->slave_addr << 1) | 1);
    out32(dev->regbase + RCAR_I2C_ICMIER, RCAR_ICMIER_MNRE | RCAR_ICMIER_MALE | RCAR_ICMIER_MSTE | RCAR_ICMIER_MATE | RCAR_ICMIER_MDRE);
    out32(dev->regbase + RCAR_I2C_ICMSR, 0);
    out32(dev->regbase + RCAR_I2C_ICMCR, RCAR_ICMCR_MIE | RCAR_ICMCR_MDBS | RCAR_ICMCR_ESG);

    rcar_i2c_wait_complete(dev);

    return dev->status;
}

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/hardware/i2c/rcar-A/recv.c $ $Rev: 805407 $")
#endif
