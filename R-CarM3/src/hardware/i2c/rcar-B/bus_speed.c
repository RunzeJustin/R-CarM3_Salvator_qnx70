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

int rcar_i2c_set_bus_speed(void *hdl, unsigned int speed, unsigned int *ospeed)
{
    rcar_i2c_dev_t  *dev = hdl;
    unsigned int    iccl, icch;
    double          l = (double)dev->clockLow;
    double          h = (double)dev->clockHigh;

    if (speed != dev->speed) {
        /* Calculate high and low clock times according to the datasheet */
        iccl = (unsigned int)((((double)dev->pck / (double)speed) * (l / (l + h)) - 1) / 2);
        icch = (unsigned int)((((double)dev->pck / (double)speed) * (h / (l + h)) - 5) / 2);

        /* Check that the values don't overflow ICCL, ICCH or ICIC */
        if (iccl <= 0x1FF && icch <= 0x1FF) {
            /* Save new value */
            dev->iccl = iccl;
            dev->icch = icch;
            dev->speed = speed;
        } else {
            /* Speed is not achievable - ICCL and ICCH are 9 bits (with ICIC bits) */
            return ERANGE;
        }
    }

    /* Return speed if a buffer pointer is provided */
    if (ospeed != NULL)
        *ospeed = dev->speed;

    return 0;
}

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/hardware/i2c/rcar-B/bus_speed.c $ $Rev: 805407 $")
#endif
