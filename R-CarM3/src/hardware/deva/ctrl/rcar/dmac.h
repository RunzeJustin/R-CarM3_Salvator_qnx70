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

#ifndef _R_Car_DMAC_H
#define _R_Car_DMAC_H

#include <hw/dma.h>
#include <arm/r-car-m3.h>

typedef enum
{
    AUDIO_PERIPHERAL_SSI_0,
    AUDIO_PERIPHERAL_SSI_1,
    AUDIO_PERIPHERAL_SSI_2,
    AUDIO_PERIPHERAL_SSI_3,
    AUDIO_PERIPHERAL_SSI_4,
    AUDIO_PERIPHERAL_SSI_5,
    AUDIO_PERIPHERAL_SSI_6,
    AUDIO_PERIPHERAL_SSI_7,
    AUDIO_PERIPHERAL_SSI_8,
    AUDIO_PERIPHERAL_SSI_9,
    AUDIO_PERIPHERAL_SSI_BUSIF_0_0,
    AUDIO_PERIPHERAL_SSI_BUSIF_0_1,
    AUDIO_PERIPHERAL_SSI_BUSIF_0_2,
    AUDIO_PERIPHERAL_SSI_BUSIF_0_3,
    AUDIO_PERIPHERAL_SSI_BUSIF_1_0,
    AUDIO_PERIPHERAL_SSI_BUSIF_1_1,
    AUDIO_PERIPHERAL_SSI_BUSIF_1_2,
    AUDIO_PERIPHERAL_SSI_BUSIF_1_3,
    AUDIO_PERIPHERAL_SSI_BUSIF_2_0,
    AUDIO_PERIPHERAL_SSI_BUSIF_2_1,
    AUDIO_PERIPHERAL_SSI_BUSIF_2_2,
    AUDIO_PERIPHERAL_SSI_BUSIF_2_3,
    AUDIO_PERIPHERAL_SSI_BUSIF_3,
    AUDIO_PERIPHERAL_SSI_BUSIF_4,
    AUDIO_PERIPHERAL_SSI_BUSIF_5,
    AUDIO_PERIPHERAL_SSI_BUSIF_6,
    AUDIO_PERIPHERAL_SSI_BUSIF_7,
    AUDIO_PERIPHERAL_SSI_BUSIF_8,
    AUDIO_PERIPHERAL_SSI_BUSIF_9_0,
    AUDIO_PERIPHERAL_SSI_BUSIF_9_1,
    AUDIO_PERIPHERAL_SSI_BUSIF_9_2,
    AUDIO_PERIPHERAL_SSI_BUSIF_9_3,
    AUDIO_PERIPHERAL_SCUSRC_0,
    AUDIO_PERIPHERAL_SCUSRC_1,
    AUDIO_PERIPHERAL_SCUSRC_2,
    AUDIO_PERIPHERAL_SCUSRC_3,
    AUDIO_PERIPHERAL_SCUSRC_4,
    AUDIO_PERIPHERAL_SCUSRC_5,
    AUDIO_PERIPHERAL_SCUSRC_6,
    AUDIO_PERIPHERAL_SCUSRC_7,
    AUDIO_PERIPHERAL_SCUSRC_8,
    AUDIO_PERIPHERAL_SCUSRC_9,
    AUDIO_PERIPHERAL_SCUCMD_0,
    AUDIO_PERIPHERAL_SCUCMD_1,
    AUDIO_PERIPHERAL_MLM_0,
    AUDIO_PERIPHERAL_MLM_1,
    AUDIO_PERIPHERAL_MLM_2,
    AUDIO_PERIPHERAL_MLM_3,
    AUDIO_PERIPHERAL_MLM_4,
    AUDIO_PERIPHERAL_MLM_5,
    AUDIO_PERIPHERAL_MLM_6,
    AUDIO_PERIPHERAL_MLM_7,
    AUDIO_PERIPHERAL_DTCPC_0,
    AUDIO_PERIPHERAL_DTCPC_1,
    AUDIO_PERIPHERAL_DTCPP_0,
    AUDIO_PERIPHERAL_DTCPP_1,
    AUDIO_PERIPHERAL_NUM
} audio_peripheral_t;

#define AUDIO_PERIPHERAL_SSI_NUM (AUDIO_PERIPHERAL_SSI_BUSIF_0_0 - AUDIO_PERIPHERAL_SSI_0)
#define AUDIO_PERIPHERAL_SSI_SUBCHAN_NUM (AUDIO_PERIPHERAL_SSI_BUSIF_1_0 - AUDIO_PERIPHERAL_SSI_BUSIF_0_0)
#define AUDIO_PERIPHERAL_SCUSRC_NUM (AUDIO_PERIPHERAL_SCUCMD_0 - AUDIO_PERIPHERAL_SCUSRC_0)
#define AUDIO_PERIPHERAL_SCUCMD_NUM (AUDIO_PERIPHERAL_MLM_0 - AUDIO_PERIPHERAL_SCUCMD_0)
#define AUDIO_PERIPHERAL_MLM_NUM (AUDIO_PERIPHERAL_DTCPC_0 - AUDIO_PERIPHERAL_MLM_0)
#define AUDIO_PERIPHERAL_DTCPC_NUM (AUDIO_PERIPHERAL_DTCPP_0 - AUDIO_PERIPHERAL_DTCPC_0)
#define AUDIO_PERIPHERAL_DTCPP_NUM (AUDIO_PERIPHERAL_NUM - AUDIO_PERIPHERAL_DTCPP_0)

#define AUDIO_PERIPHERAL_SSI(idx) (AUDIO_PERIPHERAL_SSI_0 + idx)
#define AUDIO_PERIPHERAL_SSI_BUSIF(idx,sub_idx) (idx < 3 ? AUDIO_PERIPHERAL_SSI_BUSIF_0_0 + idx * AUDIO_PERIPHERAL_SSI_SUBCHAN_NUM + sub_idx : \
                                                 idx < 9 ? AUDIO_PERIPHERAL_SSI_BUSIF_3 + idx - 3 : \
                                                 AUDIO_PERIPHERAL_SSI_BUSIF_9_0 + sub_idx)
#define AUDIO_PERIPHERAL_SCUSRC(idx) (AUDIO_PERIPHERAL_SCUSRC_0 + idx)
#define AUDIO_PERIPHERAL_SCUCMD(idx) (AUDIO_PERIPHERAL_SCUCMD_0 + idx)
#define AUDIO_PERIPHERAL_SCUMLM(idx) (AUDIO_PERIPHERAL_MLM_0 + idx)
#define AUDIO_PERIPHERAL_DTCPC(idx) (AUDIO_PERIPHERAL_DTCPC_0 + idx)
#define AUDIO_PERIPHERAL_DTCPP(idx) (AUDIO_PERIPHERAL_DTCPP_0 + idx)

typedef struct {
   uint32_t addr;
   uint32_t mid_rid; /* as defined as MID and RID bit fields of the DMARS register */
} audio_dmac_config_t;

typedef struct {
   uint32_t src_addr;
   uint32_t dst_addr;
   uint32_t chcr;
} audio_dmac_pp_config_t;

typedef struct {
   uint32_t audiodma_irq;
   void* audiodma_chn;
   void* audiodma_pp_chn;
} audio_dmac_context_t;

int audio_dmac_init(audio_dmac_context_t* tx_context, audio_dmac_context_t* rx_context);
void audio_dmac_init_cleanup(audio_dmac_context_t* tx_context, audio_dmac_context_t* rx_context);
void audio_dmac_deinit(audio_dmac_context_t* tx_context, audio_dmac_context_t* rx_context);

int audio_dmac_count_register_get(void *audiodma_chn, uint32_t * tc_val);

/* memory to peripheral transfer */
int audio_dmac_mp_get_config(audio_peripheral_t dst, audio_dmac_config_t* dmac_config);
int audio_dmac_mp_setup(void *audiodma_chn, audio_peripheral_t dst, off64_t mem_addr, int len);

/* peripheral to memory transfer */
int audio_dmac_pm_get_config(audio_peripheral_t src, audio_dmac_config_t* dmac_config);
int audio_dmac_pm_setup(void *audiodma_chn, audio_peripheral_t src, off64_t mem_addr, int len);

/* peripheral to memory and memory to peripheral transfer start/stop/clean */
int audio_dmac_start(void *audiodma_chn);
int audio_dmac_stop(void *audiodma_chn);
int audio_dmac_cleanup(void *audiodma_chn);

/* peripheral to peripheral transfer */
int audio_dmac_pp_get_config(audio_peripheral_t src, audio_peripheral_t dest, audio_dmac_pp_config_t* dmac_pp_config);
int audio_dmac_pp_setup(void *audiodma_chn, audio_peripheral_t src, audio_peripheral_t dst);

/* peripheral to peripheral transfer start/stop/clean */
int audio_dmac_pp_start(void *audiodma_chn);
int audio_dmac_pp_stop(void *audioppdma_chn);
int audio_dmac_pp_cleanup(void *audioppdma_chn);

#endif /* _R_Car_DMAC_H */

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/hardware/deva/ctrl/rcar/dmac.h $ $Rev: 812827 $")
#endif
