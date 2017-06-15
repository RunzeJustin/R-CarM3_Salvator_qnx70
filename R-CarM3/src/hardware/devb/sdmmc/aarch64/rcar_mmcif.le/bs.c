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

// Module Description:  board specific interface

#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <hw/inout.h>
#include <sys/mman.h>
#include <bs.h>

#include <rcar.h>
#include <arm/r-car-m3.h>

static int rcar_bs_cd(sdio_hc_t *hc)
{
    return (CD_INS);
}

static void rcar_cpg_config(uintptr_t cpg_base, int reg, uint32_t val)
{
	out32(cpg_base + RCAR_CPG_CPGWPR, ~val);	
	out32(cpg_base + reg, val);
}

static int rcar_mmcif_bs_init(sdio_hc_t *hc)
{
    sdio_hc_cfg_t *cfg = &hc->cfg;
    uintptr_t cpg_base;
    uint32_t tmp;

    if (cfg->idx != 2)
        return (ENODEV);

    /* Reset host */
    if ((cpg_base = mmap_device_io(0x1000, RCAR_CPG_BASE)) != (uintptr_t)MAP_FAILED) {
        /* Reset MMCIF0 = SDHI1&2  */
        tmp  = in32(cpg_base + RCAR_CPG_SRCR3);
        tmp |= ((1 << 12) | (1 << 13));
        rcar_cpg_config(cpg_base, RCAR_CPG_SRCR3, tmp);	
        delay(1);
        tmp  = in32(cpg_base + RCAR_CPG_SRSTCLR3);
        tmp |= ((1 << 12) | (1 << 13));
        rcar_cpg_config(cpg_base, RCAR_CPG_SRSTCLR3, tmp);	
        delay(2);
        munmap_device_io(cpg_base, 0x1000);
    }
 
    hc->flags|= HC_FLAG_DEV_MMC;
    hc->caps |= HC_CAP_SV_1_8V;
    hc->caps |= HC_CAP_XPC_1_8V;
    hc->ocr   = OCR_VDD_33_34;

    if (rcar_sdmmc_init(hc) != EOK)
        return (ENODEV);

    hc->entry.cd = rcar_bs_cd;

    return EOK;
}

sdio_product_t	sdio_fs_products[] = {
    { SDIO_DEVICE_ID_WILDCARD, 0, 0, "RCar MMCIF", rcar_mmcif_bs_init },
};

sdio_vendor_t	sdio_vendors[] = {
    { SDIO_VENDOR_ID_WILDCARD, "Renesas", sdio_fs_products },
    { 0, NULL, NULL }
};

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/hardware/devb/sdmmc/aarch64/rcar_mmcif.le/bs.c $ $Rev: 810496 $")
#endif
