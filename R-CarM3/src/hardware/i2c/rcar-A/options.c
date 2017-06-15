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
rcar_i2c_parse_options(rcar_i2c_dev_t *dev, int argc, char *argv[])
{
    int c;
    int prev_optind;
    int done = 0;

    optind = 0;

    dev->physbase = RCAR_I2C0_BASE;
    dev->irq      = RCAR_INTCSYS_I2C0;
    dev->pck      = I2C_DEFAULT_PCLK;
    dev->busSpeed = I2C_DEFAULT_BAUD;

    while (!done) {
        prev_optind = optind;
        c = getopt(argc, argv, "p:i:v");
        switch (c) {
            case 'p':
                dev->physbase = strtoul(optarg, NULL, 0);
                break;
            case 'i':
                dev->irq      = strtoul(optarg, NULL, 0);
                break;

            case '?':
                if (optopt == '-') {
                    ++optind;
                    break;
                }
                return -1;

            case -1:
                if (prev_optind < optind) /* -- */
                    return -1;

                if (argv[optind] == NULL) {
                    done = 1;
                    break;
                }

                if (*argv[optind] != '-') {
                    ++optind;
                    break;
                }
                return -1;

            case 'v':
                dev->verbose++;
                break;

            default:
                return -1;
        }
    }

    if (dev->verbose){
        rcar_i2c_slogf(dev, VERBOSE_LEVEL1, "rcar_i2c_parse_options: dev->physbase = %x", dev->physbase);
        rcar_i2c_slogf(dev, VERBOSE_LEVEL1, "rcar_i2c_parse_options: dev->irq = H'%x", dev->irq);
        rcar_i2c_slogf(dev, VERBOSE_LEVEL1, "rcar_i2c_parse_options: dev->pck = %dHz", dev->pck);
    }

    return 0;
}

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/hardware/i2c/rcar-A/options.c $ $Rev: 805407 $")
#endif
