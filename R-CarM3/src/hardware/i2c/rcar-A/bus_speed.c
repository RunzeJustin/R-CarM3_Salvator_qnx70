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

static int
find_best_clk(rcar_i2c_dev_t *dev, unsigned speed, unsigned *best_schd, unsigned *best_scld, unsigned *best_smd)
{
    unsigned    uiSchd, uiScld, uiSmd;
    unsigned    scl, best_scl;
    float       err, least_err;
    int         i;

    uiSmd     = 15;
    best_scl  = 0;
    least_err = 0.1;
    scl       = speed;

    if (speed == 50000) {
        uiScld = 323;
        uiSchd = 323;
    } else if (speed == 100000) {
        uiScld = 640;
        uiSchd = 632;
    } else if (speed == 400000) {
        uiScld = 152;
        uiSchd = 135;
    } else {
        uiScld = ((dev->pck/speed - 8 - 2 * uiSmd)) / 2 - 1 ;
        uiSchd = uiScld + 1 ;
        scl    = dev->pck / (8 + 2 * uiSmd + uiScld + uiSchd);
    }

    err = (float)(abs(scl - speed)) / (float)speed;
    i = 0;
    while ((err >= least_err) && (i < 100)) {
        uiScld++;
        uiSchd--;
        scl = dev->pck / (8 + 2 * uiSmd + uiScld + uiSchd);
        err = (float)(abs(scl - speed)) / (float)speed;
        i++;
    }

    if (err < least_err) {
        least_err = err;
        *best_smd = uiSmd;
        *best_scld= uiScld;
        *best_schd= uiSchd;
        best_scl = scl;
    } else {
        if (dev->verbose) {
            rcar_i2c_slogf(dev, VERBOSE_QUIET, "Can not find out best scl clock");
        }
    }

    return best_scl;
}

int
rcar_i2c_set_bus_speed(void *hdl, unsigned int speed, unsigned int *ospeed)
{
    rcar_i2c_dev_t  *dev = hdl;
    unsigned        scld, schd, smd, scl;

    /* Support Fast-Mode and Normal-Mode only */
    if (speed > 400000) {
        rcar_i2c_slogf(dev, VERBOSE_QUIET, "rcar_i2c_set_bus_speed: unsupported speed %dHz", speed);
        return -1;
    }

    if (speed == dev->scl_freq) {
        if (ospeed)
            *ospeed = dev->scl_freq;
        return 0;
    }

    out32(dev->regbase + RCAR_I2C_ICCCR2, 0x7);
    out32(dev->regbase + RCAR_I2C_ICCCR, 0x2);
    scl = find_best_clk(dev, speed, &schd, &scld, &smd);

    if (scl == 0) {
        errno = ERANGE;
        rcar_i2c_slogf(dev, VERBOSE_QUIET, "I2C rcar_i2c_set_bus_speed: scl=0 (errno=ERANGE)");
        return -1;
    }

    if (scl == dev->scl_freq) {
        if (ospeed)
            *ospeed = dev->scl_freq;
        return 0;
    }

    out32(dev->regbase + RCAR_I2C_ICMPR, smd);
    out32(dev->regbase + RCAR_I2C_ICHPR, schd);
    out32(dev->regbase + RCAR_I2C_ICLPR, scld);

    dev->scl_period = 1e9 / scl;
    dev->scl_freq = scl;

    if (ospeed)
        *ospeed = scl;

    return 0;
}

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/hardware/i2c/rcar-A/bus_speed.c $ $Rev: 805407 $")
#endif
