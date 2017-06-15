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

#include <hw/inout.h>
#include <inttypes.h>
#include <arm/r-car-m3.h>

typedef enum {
    RCAR_H3,
    RCAR_M3,
    ID_NUM
} product_id_t;

typedef enum {
    REV_1_0,
    REV_1_1,
    REV_2_0,
    REV_3_0,
    REV_UNKNOWN,
    REV_NUM
} product_rev_t;

char * soc_name[] = {
    "R-Car product unknown",
    "R-Car H3 rev 1.0",
    "R-Car H3 rev 1.1",
    "R-Car H3 rev 2.0",
    "R-Car H3 rev 3.0",
    "R-Car H3 rev unknown",
    "R-Car M3 rev 1.0",
    "R-Car M3 rev 1.1",
    "R-Car M3 rev 2.0",
    "R-Car M3 rev 3.0",
    "R-Car M3 rev unknown"
};

char *get_soc_name()
{
    uint32_t product_id = ID_NUM;
    uint32_t product_rev = REV_NUM;
    uint32_t reg = in32(RCAR_PRODUCT_REGISTER);

    switch(RCAR_PRODUCT_ID(reg)) {
        case PRODUCT_ID_RCAR_H3:
            product_id = RCAR_H3;
            break;
        case PRODUCT_ID_RCAR_M3:
            product_id = RCAR_M3;
            break;
        default:
            return soc_name[0];
    }

    switch(RCAR_PRODUCT_REV(reg)) {
        case PRODUCT_REV_1_0:
            product_rev = REV_1_0;
            break;
        case PRODUCT_REV_1_1:
            product_rev = REV_1_1;
            break;
        case PRODUCT_REV_2_0:
            product_rev = REV_2_0;
            break;
        case PRODUCT_REV_3_0:
            product_rev = REV_3_0;
            break;
        default:
            product_rev = REV_UNKNOWN;
            break;
    }

    return soc_name[REV_NUM*(product_id) + (product_rev) + 1];
}

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/hardware/startup/boards/rcar_m3/soc_name.c $ $Rev: 812929 $")
#endif
