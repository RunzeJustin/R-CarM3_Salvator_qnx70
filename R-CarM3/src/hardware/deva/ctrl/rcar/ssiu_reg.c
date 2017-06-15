/*
 * $QNXLicenseC:
 * Copyright 2014, 2016 QNX Software Systems.
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

#include <arm/r-car-m3.h>
#include <audio_driver.h>
#include "ssiu_reg.h"
#include "ssiu.h"
#include "rcar_support.h"

static ssiu_reg_t* ssiu_reg = MAP_FAILED;
static ssiu_common_reg_t* ssiu_common_reg = MAP_FAILED;
static ssi_reg_t* ssi_reg = MAP_FAILED;

#define SSI_MODULE_SIZE 0x40

int ssiu_mem_map(void)
{
    /* SSIU registers */
    ssiu_reg = ado_device_mmap (RCAR_SSIU_BASE, 0x1000);
    if (ssiu_reg == MAP_FAILED )
    {
        ado_error ("%s: SSIU register map failed", __func__);
        return ENOMEM;
    }

    ssiu_common_reg = (ssiu_common_reg_t *)((uintptr_t)ssiu_reg + 0x800);

    ssi_reg = (ssi_reg_t *)ado_device_mmap (RCAR_SSI_BASE, SSI_CHANNEL_NUM * SSI_MODULE_SIZE);
    if (ssi_reg == MAP_FAILED )
    {
        ado_error ("%s: SSI register map failed", __func__);
        return ENOMEM;
    }

    return EOK;
}

int ssiu_mem_unmap(void)
{
    ssiu_common_reg = MAP_FAILED;

    ado_device_munmap((void*)ssiu_reg, 0x1000);
    ssiu_reg = MAP_FAILED;

    ado_device_munmap((void*)ssi_reg, SSI_CHANNEL_NUM * SSI_MODULE_SIZE);
    ssi_reg = MAP_FAILED;

    return EOK;
}

ssi_reg_t* get_ssi_reg(uint32_t ssi_idx)
{
    if( ssi_reg == MAP_FAILED ) {
        return NULL;
    }

    if( ssi_idx >= SSI_CHANNEL_NUM ) {
        return NULL;
    }

    return ssi_reg + ssi_idx;
}

ssiu_busif_reg_t* get_ssiu_busif_reg(uint32_t ssi_idx, uint32_t ssi_subchan_idx)
{
    if( ssiu_reg == MAP_FAILED ) {
        return NULL;
    }

    if( !rcar_ssi_supported(ssi_idx) || ssi_subchan_idx > SSI_SUB_CHANNEL_NUM ) {
        return NULL;
    }

    return &((ssiu_reg + ssi_idx*SSI_SUB_CHANNEL_NUM + ssi_subchan_idx)->busif);
}

ssiu_ssi_reg_t* get_ssiu_ssi_reg(uint32_t ssi_idx)
{
    if( ssiu_reg == MAP_FAILED ) {
        return NULL;
    }

    if( !rcar_ssi_supported(ssi_idx) ) {
        return NULL;
    }

    return &((ssiu_reg + ssi_idx*SSI_SUB_CHANNEL_NUM)->ssi);
}

ssiu_common_reg_t* get_ssiu_common_reg(void)
{
    if( ssiu_common_reg == MAP_FAILED ) {
        return NULL;
    }

    return ssiu_common_reg;
}

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/hardware/deva/ctrl/rcar/ssiu_reg.c $ $Rev: 812827 $")
#endif

