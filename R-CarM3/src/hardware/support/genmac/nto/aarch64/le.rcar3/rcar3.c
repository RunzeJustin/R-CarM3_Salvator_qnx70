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

#include <stdint.h>
#include "variant.h"

unsigned get_uid(chip_uid_info *chipuid)
{
    chipuid->uid_size = RCAR_UID_SIZE;
    chipuid->uid = (uint8_t*) malloc(chipuid->uid_size);
    if (chipuid->uid == NULL)
    {
        fprintf(stderr, "genmac: malloc failed: %s\n", strerror(errno));
        return false;
    }

    /* Retrieve 4 bytes from RCAR_GEN3_PRR */
    memcpy(chipuid->uid, (uint8_t *)(chipuid->id_base), 4);

    return true;
}


void free_uid_resources(chip_uid_info *chipuid)
{
    free(chipuid->uid);
    return;
}

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/hardware/support/genmac/nto/aarch64/le.rcar3/rcar3.c $ $Rev: 813367 $")
#endif
