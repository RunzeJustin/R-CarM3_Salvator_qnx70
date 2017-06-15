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

#include <internal.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <hw/inout.h>
#include <sys/mman.h>
#include <rcar.h>

#include <arm/r-car-m3.h>

static int rcar_bs_signal_voltage(sdio_hc_t *hc, int signal_voltage)
{
    sdmmc_bs_ext    *bsext = (sdmmc_bs_ext *)hc->bs_hdl;
    sdio_hc_cfg_t   *cfg = &hc->cfg;
    uint32_t        pocctrl0, tmp;
    int             shift;

    if (signal_voltage == SIGNAL_VOLTAGE_1_8)
        delay(10);

    // Signal IO voltage level
    pocctrl0  = in32(bsext->pfc_base + RCAR_PFC_POCCTRL0);
    tmp = cfg->idx == 0 ? RCAR_SDHI0_POC_MASK : RCAR_SDHI3_POC_MASK;
    if (signal_voltage == SIGNAL_VOLTAGE_3_3) {
        pocctrl0 |=  tmp;
    } else {
        pocctrl0 &= ~tmp;
    }
    // It's not safe to do a read modified write for common registers!
    out32(bsext->pfc_base + RCAR_PFC_PMMR, ~pocctrl0);
    out32(bsext->pfc_base + RCAR_PFC_POCCTRL0, pocctrl0);

    if (in32(bsext->pfc_base + RCAR_PFC_POCCTRL0) != pocctrl0) {
        sdio_slogf(_SLOGC_SDIODI, _SLOG_INFO, hc->cfg.verbosity, 1, "%s: Failed to change signal level %d!", __func__, signal_voltage);
        return (EIO);
    }

    // Interface voltage control VDDQVA_SDx
    shift = cfg->idx == 0 ? RCAR_SDHI0_VDDQVA_PORT : RCAR_SDHI3_VDDQVA_PORT;
    if (signal_voltage == SIGNAL_VOLTAGE_3_3)
        out32(bsext->pwr_base + RCAR_GPIO_OUTDT, in32(bsext->pwr_base + RCAR_GPIO_OUTDT) | (1 << shift));
    else
        out32(bsext->pwr_base + RCAR_GPIO_OUTDT, in32(bsext->pwr_base + RCAR_GPIO_OUTDT) & ~(1 << shift));
    hc->signal_voltage = signal_voltage;

    return (EOK);
}

static int rcar_sdhi_signal_voltage(sdio_hc_t *hc, int signal_voltage)
{
    rcar_sdmmc_t     *sdmmc;
    uint16_t        clkctl;
    uint16_t        info2;
    int             cnt;

    sdmmc = (rcar_sdmmc_t *)hc->cs_hdl;

    hc->signal_voltage = signal_voltage;

    if (signal_voltage == SIGNAL_VOLTAGE_3_3)
        return (rcar_bs_signal_voltage(hc, signal_voltage));

    for (cnt = SDHI_TMOUT; cnt >= 0; --cnt) {
        info2 = sdmmc_read(sdmmc->vbase, MMC_SD_INFO2);
        if (!(info2 & SDH_INFO2_CBSY) && (info2 & SDH_INFO2_SCLKDIVEN))
            break;

        nanospin_ns(1000);
    }
    if (cnt <= 0) {
        sdio_slogf(_SLOGC_SDIODI, _SLOG_INFO, hc->cfg.verbosity, 1, "%s: Busy state! Cannot change the voltage!", __func__);
        return (EAGAIN);
    }

    clkctl = 0x0100;
    if (sdmmc_read(sdmmc->vbase, MMC_SD_CLK_CTRL) & 0x0200)
        clkctl |= 0x0200;

    sdmmc_write(sdmmc->vbase, MMC_SD_CLK_CTRL, ~(clkctl) & sdmmc_read(sdmmc->vbase, MMC_SD_CLK_CTRL));

    /* Check to see if DAT[3:0] is 0000b  */
    if (sdmmc_read(sdmmc->vbase, MMC_SD_INFO2) & 0x0080) {
        sdio_slogf( _SLOGC_SDIODI, _SLOG_ERROR, hc->cfg.verbosity, 1, "%s:  DAT0 is not LOW", __FUNCTION__);
        sdmmc_write(sdmmc->vbase, MMC_SD_CLK_CTRL, sdmmc_read(sdmmc->vbase, MMC_SD_CLK_CTRL) | clkctl);
        return (EIO);
    }

    rcar_bs_signal_voltage(hc, signal_voltage);

    /* Wait for at least 5ms and this depends on card manufacturer */
    delay(20);

    /* Start SDCLK */
    sdmmc_write(sdmmc->vbase, MMC_SD_CLK_CTRL, sdmmc_read(sdmmc->vbase, MMC_SD_CLK_CTRL) | 0x0100);
    cnt = 20;
again:
    /* Wait for 10ms */
    delay(10);

    /* Check to see if DAT[3:0] is 1111b  */
    if (!(sdmmc_read(sdmmc->vbase, MMC_SD_INFO2) & 0x0080)) {
        if(--cnt != 0) goto again;
    }
    if (cnt==0) {
        sdio_slogf(_SLOGC_SDIODI, _SLOG_ERROR, hc->cfg.verbosity, 1, "%s:  DAT0 is not HIGH", __FUNCTION__);
        return (EIO);
    }

    if (clkctl & 0x0200)
        sdmmc_write(sdmmc->vbase, MMC_SD_CLK_CTRL, sdmmc_read(sdmmc->vbase, MMC_SD_CLK_CTRL) | 0x0200);

    return (EOK);
}

static int rcar_bs_pwr(sdio_hc_t *hc, int vdd)
{
    sdmmc_bs_ext    *bsext = (sdmmc_bs_ext *)hc->bs_hdl;
    sdio_hc_cfg_t   *cfg = &hc->cfg;
    int             shift;

    shift = cfg->idx == 0 ? RCAR_SDHI0_VDD_PORT : RCAR_SDHI3_VDD_PORT;

    if (vdd != 0) {
        out32(bsext->pwr_base + RCAR_GPIO_OUTDT, in32(bsext->pwr_base + RCAR_GPIO_OUTDT) | (1 << shift));
        delay(20); //need delay to reset and power up stably
    } 
    else {
        out32(bsext->pwr_base + RCAR_GPIO_OUTDT, in32(bsext->pwr_base + RCAR_GPIO_OUTDT) & ~(1 << shift));
        delay(10);
    }

    bsext->rcar_pwr(hc, vdd);

    return (EOK);
}

static int rcar_bs_dinit(sdio_hc_t *hc)
{
    sdmmc_bs_ext *bsext = (sdmmc_bs_ext *)hc->bs_hdl;

    munmap_device_io(bsext->pwr_base, 0x1000);
    munmap_device_io(bsext->pfc_base, 0x1000);

    free(bsext);
    hc->bs_hdl = NULL;

    return rcar_sdmmc_dinit(hc);
}

static int rcar_bs_init(sdio_hc_t *hc)
{
    sdmmc_bs_ext   *bsext;
    sdio_hc_cfg_t  *cfg = &hc->cfg;

    if ((cfg->idx != 0) && (cfg->idx != 3))
        return (ENODEV);

    if (!(bsext = calloc(1, sizeof(sdmmc_bs_ext))))
        return ENOMEM;

    if (cfg->idx == 3) {
        if ((bsext->pwr_base = mmap_device_io(0x1000, RCAR_GPIO3_BASE)) == (uintptr_t)MAP_FAILED)
            return (ENODEV);
    } else {
        if ((bsext->pwr_base = mmap_device_io(0x1000, RCAR_GPIO5_BASE)) == (uintptr_t)MAP_FAILED)
            return (ENODEV);
    }
    if ((bsext->pfc_base = mmap_device_io(0x1000, RCAR_PFC_BASE)) == (uintptr_t)MAP_FAILED)
        goto fail0;

    hc->caps |= HC_CAP_SV_1_8V | HC_CAP_SV_3_3V;
    hc->caps |= HC_CAP_XPC_1_8V | HC_CAP_XPC_3_3V;

    hc->ocr   = OCR_VDD_17_195 | OCR_VDD_32_33 | OCR_VDD_33_34;

    if (rcar_sdmmc_init(hc) != EOK)
        goto fail1;

    bsext->rcar_pwr          = hc->entry.pwr;
    hc->entry.signal_voltage = rcar_sdhi_signal_voltage;
    hc->entry.dinit          = rcar_bs_dinit;
    hc->entry.pwr            = rcar_bs_pwr;
    hc->bs_hdl               = (void *)bsext;

    return EOK;

fail0:
    munmap_device_io(bsext->pwr_base, 0x1000);
fail1:
    munmap_device_io(bsext->pfc_base, 0x1000);

    free(bsext);

    return (ENODEV);
}

sdio_product_t	sdio_fs_products[] = {
    { SDIO_DEVICE_ID_WILDCARD, 0, 0, "RCar SDHI", rcar_bs_init },
};

sdio_vendor_t	sdio_vendors[] = {
    { SDIO_VENDOR_ID_WILDCARD, "Renesas", sdio_fs_products },
    { 0, NULL, NULL }
};

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/hardware/devb/sdmmc/aarch64/rcar_sdhi.le/bs.c $ $Rev: 810496 $")
#endif
