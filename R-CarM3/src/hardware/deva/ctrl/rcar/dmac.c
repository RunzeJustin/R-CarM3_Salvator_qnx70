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

#include <audio_driver.h>
#include <string.h>
#include "dmac.h"
#include "rcar_support.h"

#define DMA_OPT_LEN    64

/* AUDIO_DMAC definitions and register configuration */

uint8_t AUDIO_DMAC_MIDRID[AUDIO_PERIPHERAL_NUM][2] =
{
    /* tx,  rx */
    { 0x1,  0x2 }, // AUDIO_PERIPHERAL_SSI_0
    { 0x3,  0x4 }, // AUDIO_PERIPHERAL_SSI_1
    { 0x5,  0x6 }, // AUDIO_PERIPHERAL_SSI_2
    { 0x7,  0x8 }, // AUDIO_PERIPHERAL_SSI_3
    { 0x9,  0xA }, // AUDIO_PERIPHERAL_SSI_4
    { 0xB,  0xC }, // AUDIO_PERIPHERAL_SSI_5
    { 0xD,  0xE }, // AUDIO_PERIPHERAL_SSI_6
    { 0xF,  0x10 }, // AUDIO_PERIPHERAL_SSI_7
    { 0x11, 0x12 }, // AUDIO_PERIPHERAL_SSI_8
    { 0x13, 0x14 }, // AUDIO_PERIPHERAL_SSI_9
    { 0x15, 0x16 }, // AUDIO_PERIPHERAL_SSI_BUSIF_0_0
    { 0x35, 0x36 }, // AUDIO_PERIPHERAL_SSI_BUSIF_0_1
    { 0x37, 0x38 }, // AUDIO_PERIPHERAL_SSI_BUSIF_0_2
    { 0x47, 0x48 }, // AUDIO_PERIPHERAL_SSI_BUSIF_0_3
    { 0x49, 0x4A }, // AUDIO_PERIPHERAL_SSI_BUSIF_1_0
    { 0x4B, 0x4C }, // AUDIO_PERIPHERAL_SSI_BUSIF_1_1
    { 0x57, 0x58 }, // AUDIO_PERIPHERAL_SSI_BUSIF_1_2
    { 0x59, 0x5A }, // AUDIO_PERIPHERAL_SSI_BUSIF_1_3
    { 0x63, 0x64 }, // AUDIO_PERIPHERAL_SSI_BUSIF_2_0
    { 0x67, 0x68 }, // AUDIO_PERIPHERAL_SSI_BUSIF_2_1
    { 0x6B, 0x6C }, // AUDIO_PERIPHERAL_SSI_BUSIF_2_2
    { 0x6D, 0x6E }, // AUDIO_PERIPHERAL_SSI_BUSIF_2_3
    { 0x6F, 0x70 }, // AUDIO_PERIPHERAL_SSI_BUSIF_3
    { 0x71, 0x72 }, // AUDIO_PERIPHERAL_SSI_BUSIF_4
    { 0x73, 0x74 }, // AUDIO_PERIPHERAL_SSI_BUSIF_5
    { 0x75, 0x76 }, // AUDIO_PERIPHERAL_SSI_BUSIF_6
    { 0x79, 0x7A }, // AUDIO_PERIPHERAL_SSI_BUSIF_7
    { 0x7B, 0x7C }, // AUDIO_PERIPHERAL_SSI_BUSIF_8
    { 0x7D, 0x7E }, // AUDIO_PERIPHERAL_SSI_BUSIF_9_0
    { 0x7F, 0x80 }, // AUDIO_PERIPHERAL_SSI_BUSIF_9_1
    { 0x81, 0x82 }, // AUDIO_PERIPHERAL_SSI_BUSIF_9_2
    { 0x83, 0x84 }, // AUDIO_PERIPHERAL_SSI_BUSIF_9_3
    { 0x85, 0x9A }, // AUDIO_PERIPHERAL_SCUSRC_0
    { 0x87, 0x9C }, // AUDIO_PERIPHERAL_SCUSRC_1
    { 0x89, 0x9E }, // AUDIO_PERIPHERAL_SCUSRC_2
    { 0x8B, 0xA0 }, // AUDIO_PERIPHERAL_SCUSRC_3
    { 0x8D, 0xB0 }, // AUDIO_PERIPHERAL_SCUSRC_4
    { 0x8F, 0xB2 }, // AUDIO_PERIPHERAL_SCUSRC_5
    { 0x91, 0xB4 }, // AUDIO_PERIPHERAL_SCUSRC_6
    { 0x93, 0xB6 }, // AUDIO_PERIPHERAL_SCUSRC_7
    { 0x95, 0xB8 }, // AUDIO_PERIPHERAL_SCUSRC_8
    { 0x97, 0xBA }, // AUDIO_PERIPHERAL_SCUSRC_9
    { 0,    0xBC }, // AUDIO_PERIPHERAL_SCUCMD_0
    { 0,    0xBE }, // AUDIO_PERIPHERAL_SCUCMD_1
    { 0xDB, 0xDC }, // AUDIO_PERIPHERAL_MLM_0
    { 0xE3, 0xE4 }, // AUDIO_PERIPHERAL_MLM_1
    { 0xE5, 0xE6 }, // AUDIO_PERIPHERAL_MLM_2
    { 0xE7, 0xE8 }, // AUDIO_PERIPHERAL_MLM_3
    { 0xF3, 0xF4 }, // AUDIO_PERIPHERAL_MLM_4
    { 0xF5, 0xF6 }, // AUDIO_PERIPHERAL_MLM_5
    { 0xF7, 0xF8 }, // AUDIO_PERIPHERAL_MLM_6
    { 0xF9, 0xFA }, // AUDIO_PERIPHERAL_MLM_7
    { 0xD7, 0xD8 }, // AUDIO_PERIPHERAL_DTCPC_0
    { 0xD9, 0xDA }, // AUDIO_PERIPHERAL_DTCPC_1
    { 0xBF, 0xC0 }, // AUDIO_PERIPHERAL_DTCPP_0
    { 0xD5, 0xD6 } // AUDIO_PERIPHERAL_DTCPP_1
};

#define AUDIO_DMAC_ADDR_SSI_BASE 0xEC241000
#define AUDIO_DMAC_ADDR_SSI_TX(idx) (AUDIO_DMAC_ADDR_SSI_BASE + (idx<<6) + 0x8) /* SSITDRx registers */
#define AUDIO_DMAC_ADDR_SSI_RX(idx) (AUDIO_DMAC_ADDR_SSI_BASE + (idx<<6) + 0xC) /* SSIRDRx registers */

#define AUDIO_DMAC_ADDR_SSI_BUSIF_BASE 0xEC100000
#define AUDIO_DMAC_ADDR_SSI_BUSIF(idx, sub_idx) (AUDIO_DMAC_ADDR_SSI_BUSIF_BASE + (idx<<12) + (sub_idx<<10))

#define AUDIO_DMAC_ADDR_SCUSRC_RX_BASE 0xEC004000 /* SCUSRCOx registers */
#define AUDIO_DMAC_ADDR_SCUSRC_RX(idx) (AUDIO_DMAC_ADDR_SCUSRC_RX_BASE + (idx<<10))

#define AUDIO_DMAC_ADDR_SCUSRC_TX_BASE 0xEC000000 /* SCUSRCIx registers */
#define AUDIO_DMAC_ADDR_SCUSRC_TX(idx) (AUDIO_DMAC_ADDR_SCUSRC_TX_BASE + (idx<<10))

#define AUDIO_DMAC_ADDR_SCUCMD_BASE 0xEC008000
#define AUDIO_DMAC_ADDR_SCUCMD(idx) (AUDIO_DMAC_ADDR_SCUCMD_BASE + (idx<<10))

#define AUDIO_DMAC_ADDR_DTCPPP_BASE 0xEC120000
#define AUDIO_DMAC_ADDR_DTCPPP(idx) (AUDIO_DMAC_ADDR_DTCPPP_BASE + (idx<<10))

#define AUDIO_DMAC_ADDR_DTCPCP_BASE 0xEC124000
#define AUDIO_DMAC_ADDR_DTCPCP(idx) (AUDIO_DMAC_ADDR_DTCPCP_BASE + (idx<<10))

#define AUDIO_DMAC_ADDR_MLM_BASE 0xEC020000
#define AUDIO_DMAC_ADDR_MLM(idx) (AUDIO_DMAC_ADDR_MLM_BASE + (idx<<10))

int audio_dmac_mp_get_config(audio_peripheral_t dst, audio_dmac_config_t* dmac_config)
{
    uint32_t dst_addr = 0;
    uint32_t idx, sub_idx;

    if( dmac_config == NULL ) {
        return EINVAL;
    }

    if( dst >= AUDIO_PERIPHERAL_SSI_BUSIF_0_0 && dst <= AUDIO_PERIPHERAL_SSI_BUSIF_9_3 ) {
        if( dst >= AUDIO_PERIPHERAL_SSI_BUSIF_9_0 ) {
            idx = 9;
            sub_idx = dst - AUDIO_PERIPHERAL_SSI_BUSIF_9_0;
        } else if( dst <= AUDIO_PERIPHERAL_SSI_BUSIF_2_3 ) {
            idx = (dst - AUDIO_PERIPHERAL_SSI_BUSIF_0_0) / AUDIO_PERIPHERAL_SSI_SUBCHAN_NUM;
            sub_idx = dst - AUDIO_PERIPHERAL_SSI_BUSIF_0_0 - idx * AUDIO_PERIPHERAL_SSI_SUBCHAN_NUM;
        } else {
            idx = 3 + dst - AUDIO_PERIPHERAL_SSI_BUSIF_3;
            sub_idx = 0;
        }
        dst_addr = AUDIO_DMAC_ADDR_SSI_BUSIF(idx, sub_idx);
    } else if( dst >= AUDIO_PERIPHERAL_SSI_0 && dst <= AUDIO_PERIPHERAL_SSI_9 ) {
        idx = dst - AUDIO_PERIPHERAL_SSI_0;
        dst_addr = AUDIO_DMAC_ADDR_SSI_TX(idx);
    } else if( dst >= AUDIO_PERIPHERAL_SCUSRC_0 && dst <= AUDIO_PERIPHERAL_SCUSRC_9 ) {
        idx = dst - AUDIO_PERIPHERAL_SCUSRC_0;
        dst_addr = AUDIO_DMAC_ADDR_SCUSRC_TX(idx);
    }
#ifdef RCAR_DTCP_SUPPORT
    if( dst >= AUDIO_PERIPHERAL_DTCPC_0 && dst <= AUDIO_PERIPHERAL_DTCPC_1 ) {
        idx = dst - AUDIO_PERIPHERAL_DTCPC_0;
        dst_addr = AUDIO_DMAC_ADDR_DTCPCP(idx);
    } else if( dst >= AUDIO_PERIPHERAL_DTCPP_0 && dst <= AUDIO_PERIPHERAL_DTCPP_1 ) {
        idx = dst - AUDIO_PERIPHERAL_DTCPP_0;
        dst_addr = AUDIO_DMAC_ADDR_DTCPPP(idx);
    }
#endif
#ifdef RCAR_MLP_SUPPORT
    if( dst >= AUDIO_PERIPHERAL_MLM_0 && dst <= AUDIO_PERIPHERAL_MLM_9 ) {
        idx = dst - AUDIO_PERIPHERAL_MLM_0;
        dst_addr = AUDIO_DMAC_ADDR_MLM(idx);
    }
#endif
    if( dst_addr == 0 ) {
       return EINVAL;
    }

    dmac_config->addr = dst_addr;
    dmac_config->mid_rid = AUDIO_DMAC_MIDRID[dst][0];

    return EOK;
}

int audio_dmac_pm_get_config(audio_peripheral_t src, audio_dmac_config_t* dmac_config)
{
    uint32_t src_addr = 0;
    uint32_t idx, sub_idx;

    if( dmac_config == NULL ) {
        return EINVAL;
    }

    if( src >= AUDIO_PERIPHERAL_SSI_BUSIF_0_0 && src <= AUDIO_PERIPHERAL_SSI_BUSIF_9_3 ) {
        if( src >= AUDIO_PERIPHERAL_SSI_BUSIF_9_0 ) {
            idx = 9;
            sub_idx = src - AUDIO_PERIPHERAL_SSI_BUSIF_9_0;
        } else if( src <= AUDIO_PERIPHERAL_SSI_BUSIF_2_3 ) {
            idx = (src - AUDIO_PERIPHERAL_SSI_BUSIF_0_0) / AUDIO_PERIPHERAL_SSI_SUBCHAN_NUM;
            sub_idx = src - AUDIO_PERIPHERAL_SSI_BUSIF_0_0 - idx * AUDIO_PERIPHERAL_SSI_SUBCHAN_NUM;
        } else {
            idx = 3 + src - AUDIO_PERIPHERAL_SSI_BUSIF_3;
            sub_idx = 0;
        }
        src_addr = AUDIO_DMAC_ADDR_SSI_BUSIF(idx, sub_idx);
    } else if( src >= AUDIO_PERIPHERAL_SSI_0 && src <= AUDIO_PERIPHERAL_SSI_9 ) {
        idx = src - AUDIO_PERIPHERAL_SSI_0;
        src_addr = AUDIO_DMAC_ADDR_SSI_RX(idx);
    } else if( src >= AUDIO_PERIPHERAL_SCUSRC_0 && src <= AUDIO_PERIPHERAL_SCUSRC_9 ) {
        idx = src - AUDIO_PERIPHERAL_SCUSRC_0;
        src_addr = AUDIO_DMAC_ADDR_SCUSRC_RX(idx);
    } else if( src >= AUDIO_PERIPHERAL_SCUCMD_0 && src <= AUDIO_PERIPHERAL_SCUCMD_1 ) {
        idx = src - AUDIO_PERIPHERAL_SCUCMD_0;
        src_addr = AUDIO_DMAC_ADDR_SCUCMD(idx);
    }
#ifdef RCAR_DTCP_SUPPORT
    if( src >= AUDIO_PERIPHERAL_DTCPC_0 && src <= AUDIO_PERIPHERAL_DTCPC_1 ) {
        idx = src - AUDIO_PERIPHERAL_DTCPC_0;
        src_addr = AUDIO_DMAC_ADDR_DTCPCP(idx);
    } else if( src >= AUDIO_PERIPHERAL_DTCPP_0 && src <= AUDIO_PERIPHERAL_DTCPP_1 ) {
        idx = src - AUDIO_PERIPHERAL_DTCPP_0;
        src_addr = AUDIO_DMAC_ADDR_DTCPPP(idx);
    }
#endif
#ifdef RCAR_MLP_SUPPORT
    if( src >= AUDIO_PERIPHERAL_MLM_0 && src <= AUDIO_PERIPHERAL_MLM_9 ) {
        idx = src - AUDIO_PERIPHERAL_MLM_0;
        src_addr = AUDIO_DMAC_ADDR_MLM(idx);
    }
#endif
    if( src_addr == 0 ) {
       return EINVAL;
    }

    dmac_config->addr = src_addr;
    dmac_config->mid_rid = AUDIO_DMAC_MIDRID[src][1];

    return EOK;
}

/* AUDIO_DMAC_PP definitions and register configuration */

/* values for source and destination request source as used in the PDMACHCR register */
#define AUDIO_DMAC_PP_RS_SSI_BASE 0
#define AUDIO_DMAC_PP_RS_SSI(idx, sub_idx) ( idx < 3 ? \
                                             AUDIO_DMAC_PP_RS_SSI_BASE + idx * AUDIO_PERIPHERAL_SSI_SUBCHAN_NUM + sub_idx : \
                                             ( idx < 9 ? idx - 3 + 0xC : 0x12 + sub_idx ) )
#define AUDIO_DMAC_PP_RS_SCUSRC_BASE 0x2D
#define AUDIO_DMAC_PP_RS_SCUSRC(idx) (AUDIO_DMAC_PP_RS_SCUSRC_BASE + idx)

#define AUDIO_DMAC_PP_RS_CMD_BASE 0x37
#define AUDIO_DMAC_PP_RS_CMD(idx) (AUDIO_DMAC_PP_RS_CMD_BASE + idx)

#define AUDIO_DMAC_PP_RS_DTCPPP_BASE 0x16
#define AUDIO_DMAC_PP_RS_DTCPPP(idx) (AUDIO_DMAC_PP_RS_DTCPPP_BASE + idx)

#define AUDIO_DMAC_PP_RS_DTCPCP_BASE 0x18
#define AUDIO_DMAC_PP_RS_DTCPCP(idx) (AUDIO_DMAC_PP_RS_DTCPCP_BASE + idx)

#define AUDIO_DMAC_PP_RS_MLM_BASE 0x25
#define AUDIO_DMAC_PP_RS_MLM(idx) (AUDIO_DMAC_PP_RS_MLM_BASE + idx)

/* values for source and destination addresses for PP DMA transfers */
#define AUDIO_DMAC_PP_ADDR_SSI_BASE 0xEC400000
#define AUDIO_DMAC_PP_ADDR_SSI(idx, sub_idx) (AUDIO_DMAC_PP_ADDR_SSI_BASE + (idx<<12) + (sub_idx<<10))

#define AUDIO_DMAC_PP_ADDR_SCUSRCO_BASE 0xEC304000
#define AUDIO_DMAC_PP_ADDR_SCUSRCO(idx) (AUDIO_DMAC_PP_ADDR_SCUSRCO_BASE + (idx<<10))

#define AUDIO_DMAC_PP_ADDR_SCUSRCI_BASE 0xEC300000
#define AUDIO_DMAC_PP_ADDR_SCUSRCI(idx) (AUDIO_DMAC_PP_ADDR_SCUSRCI_BASE + (idx<<10))

#define AUDIO_DMAC_PP_ADDR_CMD_BASE 0xEC308000
#define AUDIO_DMAC_PP_ADDR_CMD(idx) (AUDIO_DMAC_PP_ADDR_CMD_BASE + (idx<<10))

#define AUDIO_DMAC_PP_ADDR_DTCPPP_BASE 0xEC420000
#define AUDIO_DMAC_PP_ADDR_DTCPPP(idx) (AUDIO_DMAC_PP_ADDR_DTCPPP_BASE + (idx<<10))

#define AUDIO_DMAC_PP_ADDR_DTCPCP_BASE 0xEC424000
#define AUDIO_DMAC_PP_ADDR_DTCPCP(idx) (AUDIO_DMAC_PP_ADDR_DTCPCP_BASE + (idx<<10))

#define AUDIO_DMAC_PP_ADDR_MLM_BASE 0xEC320000
#define AUDIO_DMAC_PP_ADDR_MLM(idx) (AUDIO_DMAC_PP_ADDR_MLM_BASE + (idx<<10))

int audio_dmac_pp_get_config(audio_peripheral_t src, audio_peripheral_t dst, audio_dmac_pp_config_t* dmac_pp_config)
{
    uint32_t src_addr = 0;
    uint32_t dst_addr = 0;
    uint32_t src_rs = 0;
    uint32_t dst_rs = 0;
    uint32_t idx = 0;
    uint32_t sub_idx = 0;

    if( dmac_pp_config == NULL ) {
        return EINVAL;
    }

    if( src >= AUDIO_PERIPHERAL_SSI_BUSIF_0_0 && src <= AUDIO_PERIPHERAL_SSI_BUSIF_9_3 ) {
        if( src >= AUDIO_PERIPHERAL_SSI_BUSIF_9_0 ) {
            idx = 9;
            sub_idx = src - AUDIO_PERIPHERAL_SSI_BUSIF_9_0;
        } else if( src <= AUDIO_PERIPHERAL_SSI_BUSIF_2_3 ) {
            idx = (src - AUDIO_PERIPHERAL_SSI_BUSIF_0_0) / AUDIO_PERIPHERAL_SSI_SUBCHAN_NUM;
            sub_idx = src - AUDIO_PERIPHERAL_SSI_BUSIF_0_0 - idx * AUDIO_PERIPHERAL_SSI_SUBCHAN_NUM;
        } else {
            idx = 3 + src - AUDIO_PERIPHERAL_SSI_BUSIF_3;
            sub_idx = 0;
        }
        src_addr = AUDIO_DMAC_PP_ADDR_SSI( idx, sub_idx );
        src_rs = AUDIO_DMAC_PP_RS_SSI(idx, sub_idx);
    } else if( src >= AUDIO_PERIPHERAL_SCUSRC_0 && src <= AUDIO_PERIPHERAL_SCUSRC_9 ) {
        idx = src - AUDIO_PERIPHERAL_SCUSRC_0;

        src_addr = AUDIO_DMAC_PP_ADDR_SCUSRCO(idx);
        src_rs = AUDIO_DMAC_PP_RS_SCUSRC(idx);
    } else if( src >= AUDIO_PERIPHERAL_SCUCMD_0 && src <= AUDIO_PERIPHERAL_SCUCMD_1 ) {
        idx = src - AUDIO_PERIPHERAL_SCUCMD_0;

        src_addr = AUDIO_DMAC_PP_ADDR_CMD(idx);
        src_rs = AUDIO_DMAC_PP_RS_CMD(idx);
    }

    if( dst >= AUDIO_PERIPHERAL_SSI_BUSIF_0_0 && dst <= AUDIO_PERIPHERAL_SSI_BUSIF_9_3 ) {
        if( dst >= AUDIO_PERIPHERAL_SSI_BUSIF_9_0 ) {
            idx = 9;
            sub_idx = dst - AUDIO_PERIPHERAL_SSI_BUSIF_9_0;
        } else if( dst <= AUDIO_PERIPHERAL_SSI_BUSIF_2_3 ) {
            idx = (dst - AUDIO_PERIPHERAL_SSI_BUSIF_0_0) / AUDIO_PERIPHERAL_SSI_SUBCHAN_NUM;
            sub_idx = dst - AUDIO_PERIPHERAL_SSI_BUSIF_0_0 - idx * AUDIO_PERIPHERAL_SSI_SUBCHAN_NUM;
        } else {
            idx = 3 + dst - AUDIO_PERIPHERAL_SSI_BUSIF_3;
            sub_idx = 0;
        }
        dst_addr = AUDIO_DMAC_PP_ADDR_SSI( idx, sub_idx );
        dst_rs = AUDIO_DMAC_PP_RS_SSI(idx, sub_idx);
    } else if( dst >= AUDIO_PERIPHERAL_SCUSRC_0 && src_rs <= AUDIO_PERIPHERAL_SCUSRC_9 ) {
        idx = dst - AUDIO_PERIPHERAL_SCUSRC_0;

        dst_addr = AUDIO_DMAC_PP_ADDR_SCUSRCI(idx);
        dst_rs = AUDIO_DMAC_PP_RS_SCUSRC(idx);
    }

#ifdef RCAR_DTCP_SUPPORT
    if( src >= AUDIO_PERIPHERAL_DTCPC_0 && src <= AUDIO_PERIPHERAL_DTCPC_1 ) {
        idx = src - AUDIO_PERIPHERAL_DTCPC_0;

        src_addr = AUDIO_DMAC_PP_ADDR_DTCPCP(idx);
        src_rs = AUDIO_DMAC_PP_RS_DTCPCP(idx);
    } else if( src >= AUDIO_PERIPHERAL_DTCPP_0 && src <= AUDIO_PERIPHERAL_DTCPP_1 ) {
        idx = src - AUDIO_PERIPHERAL_DTCPP_0;

        src_addr = AUDIO_DMAC_PP_ADDR_DTCPPP(idx);
        src_rs = AUDIO_DMAC_PP_RS_DTCPPP(idx);
    }
    if( dst >= AUDIO_PERIPHERAL_DTCPC_0 && dst <= AUDIO_PERIPHERAL_DTCPC_1 ) {
        idx = dst - AUDIO_PERIPHERAL_DTCPC_0;

        dst_addr = AUDIO_DMAC_PP_ADDR_DTCPCP(idx);
        dst_rs = AUDIO_DMAC_PP_RS_DTCPCP(idx);
    } else if( dst >= AUDIO_PERIPHERAL_DTCPP_0 && dst <= AUDIO_PERIPHERAL_DTCPP_1 ) {
        idx = dst - AUDIO_PERIPHERAL_DTCPP_0;

        dst_addr = AUDIO_DMAC_PP_ADDR_DTCPPP(idx);
        dst_rs = AUDIO_DMAC_PP_RS_DTCPPP(idx);
    }
#endif
#ifdef RCAR_MLP_SUPPORT
    if( src >= AUDIO_PERIPHERAL_MLM_0 && src <= AUDIO_PERIPHERAL_MLM_9 ) {
        idx = src - AUDIO_PERIPHERAL_MLM_0;

        src_addr = AUDIO_DMAC_PP_ADDR_MLM(idx);
        src_rs = AUDIO_DMAC_PP_RS_MLM(idx);
    }
    if( dst >= AUDIO_PERIPHERAL_MLM_0 && dst <= AUDIO_PERIPHERAL_MLM_9 ) {
        idx = dst - AUDIO_PERIPHERAL_MLM_0;

        dst_addr = AUDIO_DMAC_PP_ADDR_MLM(idx);
        dst_rs = AUDIO_DMAC_PP_RS_MLM(idx);
    }
#endif
    if( src_addr == 0 || dst_addr == 0 ) {
        ado_error("Failed to compute src and dest addr: src=%x, dst=%x", src_addr, dst_addr);
        return EINVAL;
    }

    dmac_pp_config->src_addr = src_addr;
    dmac_pp_config->dst_addr = dst_addr;
    dmac_pp_config->chcr = (src_rs << 24) | (dst_rs << 16);

    return EOK;
}

extern int get_audioppdmafuncs(dma_functions_t *functable, int tabsize);

static dma_functions_t  audiodmafuncs;
static dma_functions_t  audiodmappfuncs;

int audio_dmac_init(audio_dmac_context_t* tx_context, audio_dmac_context_t* rx_context)
{
    char attach_opt[DMA_OPT_LEN];
    char ver_opt[DMA_OPT_LEN];
    dma_channel_query_t chinfo;
    int ret;

    /* one of tx_context and rx_context can be NULL */
    if ( !tx_context && !rx_context ) {
        return EINVAL;
    }

    /*
     * Audio DMAC
     */
    if (get_dmafuncs(&audiodmafuncs, sizeof (dma_functions_t)) == -1)
    {
        ado_error( "%s: failed to get DMA lib functions", __func__ );
        ret = EPROCUNAVAIL;
        goto dmac_init_fail;
    }

    audiodmafuncs.init(NULL);

    switch(rcar_version_get()) {
        case RCAR_VERSION_H2:
            strlcpy(ver_opt, "ver=h2", sizeof(ver_opt));
            break;
        case RCAR_VERSION_M2:
            strlcpy(ver_opt, "ver=m2", sizeof(ver_opt));
            break;
        case RCAR_VERSION_E2:
            strlcpy(ver_opt, "ver=e2", sizeof(ver_opt));
            break;
        case RCAR_VERSION_V2:
            strlcpy(ver_opt, "ver=v2", sizeof(ver_opt));
            break;
        case RCAR_VERSION_H3:
            strlcpy(ver_opt, "ver=h3", sizeof(ver_opt));
            break;
		case RCAR_VERSION_M3:
            strlcpy(ver_opt, "ver=m3", sizeof(ver_opt));
            break;
        default:
            ado_error( "%s: invalid rcar version", __func__);
            return EINVAL;
    }

    strlcpy(attach_opt, "dma=audio,desc=2,", sizeof(attach_opt));
    strlcat(attach_opt, ver_opt, sizeof(attach_opt));

    /* Playback DMA channel */
    if( tx_context ) {
        tx_context->audiodma_chn =
            audiodmafuncs.channel_attach(attach_opt, NULL, NULL, 0, DMA_ATTACH_ANY_CHANNEL | DMA_ATTACH_EVENT_PER_SEGMENT);

        if ( !tx_context->audiodma_chn )
        {
            ado_error( "%s: unable to attach to audio DMA playback channel", __func__ );
            ret = EAGAIN;
            goto dmac_init_fail;
        }

        audiodmafuncs.query_channel( tx_context->audiodma_chn, &chinfo );
        tx_context->audiodma_irq = chinfo.irq;
    }
    /* Capture DMA channel */
    if( rx_context ) {
        rx_context->audiodma_chn =
            audiodmafuncs.channel_attach(attach_opt, NULL, NULL, 0, DMA_ATTACH_ANY_CHANNEL | DMA_ATTACH_EVENT_PER_SEGMENT);
        if ( !rx_context->audiodma_chn )
        {
            ado_error( "%s: unable to attach to audio DMA capture channel", __func__ );
            ret = EAGAIN;
            goto dmac_init_fail;
        }

        audiodmafuncs.query_channel( rx_context->audiodma_chn, &chinfo );
        rx_context->audiodma_irq = chinfo.irq;
    }

    /*
     * Audio PP DMAC
     */
    if (get_audioppdmafuncs (&audiodmappfuncs, sizeof (dma_functions_t)) == -1)
    {
        ado_error( "%s: failed to get audio DMA pp lib functions", __func__ );
        ret = EPROCUNAVAIL;
        goto dmac_init_fail;
    }

    audiodmappfuncs.init(NULL);

    /* Playback DMA PP channel */
    if( tx_context ) {
        tx_context->audiodma_pp_chn =
            audiodmappfuncs.channel_attach(ver_opt, NULL, NULL, 0, DMA_ATTACH_ANY_CHANNEL);

        if( !tx_context->audiodma_pp_chn ) {
            ado_error( "%s: unable to attach to audio pp DMA playback channel", __func__ );
            ret = EAGAIN;
            goto dmac_init_fail;
        }
    }

    /* Capture DMA PP channel */
    if( rx_context ) {
        rx_context->audiodma_pp_chn =
            audiodmappfuncs.channel_attach(ver_opt, NULL, NULL, 0, DMA_ATTACH_ANY_CHANNEL);

        if( !rx_context->audiodma_pp_chn ) {
            ado_error( "%s: unable to attach to audio pp DMA capture channel", __func__ );
            ret = EAGAIN;
            goto dmac_init_fail;
        }
    }

    return EOK;

dmac_init_fail:
    audio_dmac_init_cleanup(tx_context, rx_context);

    return ret;
}

void audio_dmac_init_cleanup(audio_dmac_context_t* tx_context, audio_dmac_context_t* rx_context)
{
    if( !tx_context && !rx_context ) {
        return;
    }

    if ( tx_context && tx_context->audiodma_chn ) {
        audiodmafuncs.channel_release( tx_context->audiodma_chn );
    }
    if ( rx_context && rx_context->audiodma_chn ) {
        audiodmafuncs.channel_release( rx_context->audiodma_chn );
    }

    if ( tx_context && tx_context->audiodma_pp_chn ) {
        audiodmafuncs.channel_release( tx_context->audiodma_pp_chn );
    }
    if ( rx_context && rx_context->audiodma_pp_chn ) {
        audiodmafuncs.channel_release( rx_context->audiodma_pp_chn );
    }
}

void audio_dmac_deinit(audio_dmac_context_t* tx_context, audio_dmac_context_t* rx_context)
{
    if( !tx_context && !rx_context ) {
        return;
    }

    if ( tx_context && tx_context->audiodma_chn ) {
        audio_dmac_stop( tx_context->audiodma_chn );
    }
    if ( rx_context && rx_context->audiodma_chn ) {
        audio_dmac_stop( rx_context->audiodma_chn );
    }

    if ( tx_context && tx_context->audiodma_pp_chn ) {
        audio_dmac_pp_stop( tx_context->audiodma_pp_chn );
    }
    if ( rx_context && rx_context->audiodma_pp_chn ) {
        audio_dmac_pp_stop( rx_context->audiodma_pp_chn );
    }

    audio_dmac_init_cleanup( tx_context, rx_context );
}

int audio_dmac_count_register_get(void *audiodma_chn, uint32_t* tc_val)
{
    if( !audiodma_chn ) {
        return EINVAL;
    }

    *tc_val = audiodmafuncs.bytes_left(audiodma_chn);

    return EOK;
}

int audio_dmac_mp_setup(void *audiodma_chn, audio_peripheral_t dst,  off64_t mem_addr, int len)
{
    dma_transfer_t  tinfo;
    dma_addr_t      saddr[2], daddr[2];
    audio_dmac_config_t audio_dmac_config;

    if (!audiodma_chn) {
        return EINVAL;
    }

    if( audio_dmac_mp_get_config(dst, &audio_dmac_config) != EOK ) {
        return EINVAL;
    }

    memset(&tinfo, 0, sizeof(tinfo));

    tinfo.mode_flags     = DMA_MODE_FLAG_REPEAT;
    tinfo.xfer_unit_size = 4;
    tinfo.xfer_bytes     = len;
    tinfo.src_flags      = DMA_ADDR_FLAG_SEGMENTED;
    tinfo.src_fragments  = 2;
    tinfo.dst_flags      = DMA_ADDR_FLAG_NO_INCREMENT | DMA_ADDR_FLAG_DEVICE;

    saddr[0].paddr       = mem_addr;
    saddr[1].paddr       = mem_addr + len / 2;
    saddr[0].len         = len / 2;
    saddr[1].len         = len / 2;
    tinfo.src_addrs      = &saddr[0];

    daddr[0].paddr       = daddr[1].paddr = audio_dmac_config.addr;
    tinfo.dst_addrs      = &daddr[0];

    tinfo.req_id         = audio_dmac_config.mid_rid;

    audiodmafuncs.setup_xfer(audiodma_chn, &tinfo);

    return EOK;
}

/*  peripheral to memory  */
int audio_dmac_pm_setup(void *audiodma_chn, audio_peripheral_t src, off64_t mem_addr, int len)
{
    dma_transfer_t  tinfo;
    dma_addr_t      saddr[2], daddr[2];
    audio_dmac_config_t audio_dmac_config;

    if (!audiodma_chn) {
        return EINVAL;
    }

    if( audio_dmac_pm_get_config(src, &audio_dmac_config) != EOK ) {
        return EINVAL;
    }

    memset(&tinfo, 0, sizeof(tinfo));

    tinfo.mode_flags     = DMA_MODE_FLAG_REPEAT;
    tinfo.xfer_unit_size = 4;
    tinfo.xfer_bytes     = len;
    tinfo.dst_flags      = DMA_ADDR_FLAG_SEGMENTED;
    tinfo.dst_fragments  = 2;
    tinfo.src_flags      = DMA_ADDR_FLAG_NO_INCREMENT | DMA_ADDR_FLAG_DEVICE;

    daddr[0].paddr       = mem_addr;
    daddr[1].paddr       = mem_addr + len / 2;
    daddr[0].len         = len / 2;
    daddr[1].len         = len / 2;
    tinfo.dst_addrs      = &daddr[0];

    saddr[0].paddr       = saddr[1].paddr = audio_dmac_config.addr;
    tinfo.src_addrs      = &saddr[0];

    tinfo.req_id         = audio_dmac_config.mid_rid;

    audiodmafuncs.setup_xfer(audiodma_chn, &tinfo);

    return EOK;
}

inline int audio_dmac_cleanup(void * audiodma_chn)
{
    if( !audiodma_chn ) {
        return EINVAL;
    }

    audiodmafuncs.xfer_complete(audiodma_chn);

    return EOK;
}

inline int audio_dmac_start(void *audiodma_chn)
{
    if( !audiodma_chn ) {
        return EINVAL;
    }

    /* Start Audio-DMAC */
    audiodmafuncs.xfer_start (audiodma_chn);

    return EOK;
}

inline int audio_dmac_stop(void *audiodma_chn)
{
    if( !audiodma_chn ) {
        return EINVAL;
    }

    /* Stop Audio-DMAC */
    audiodmafuncs.xfer_abort(audiodma_chn);

    return EOK;
}

int audio_dmac_pp_setup(void *audiodma_pp_chn, audio_peripheral_t src, audio_peripheral_t dst)
{
    dma_transfer_t  tinfo;
    dma_addr_t      saddr, daddr;
    audio_dmac_pp_config_t dmac_pp_config;

    if( !audiodma_pp_chn ) {
        return EINVAL;
    }

    if( audio_dmac_pp_get_config( src, dst, &dmac_pp_config) != EOK ) {
        ado_error( "%s: failed getting the DMAC PP config for src %d dst %d", __func__, src, dst );
        return EINVAL;
    }

    memset(&tinfo, 0, sizeof(tinfo));

    /* Setup peripheral-peripheral DMA transfer from SSI1-0 to SCU-SRC1 */
    saddr.paddr     = dmac_pp_config.src_addr;
    tinfo.src_addrs = &saddr;
    daddr.paddr     = dmac_pp_config.dst_addr;
    tinfo.dst_addrs = &daddr;
    tinfo.req_id    = dmac_pp_config.chcr;

    audiodmappfuncs.setup_xfer (audiodma_pp_chn, &tinfo);

    return EOK;
}

inline int audio_dmac_pp_cleanup(void * audiodma_pp_chn)
{
    if( !audiodma_pp_chn ) {
        return EINVAL;
    }

    audiodmappfuncs.xfer_complete(audiodma_pp_chn);

    return EOK;
}

inline int audio_dmac_pp_start(void * audiodma_pp_chn)
{
    if( !audiodma_pp_chn ) {
        return EINVAL;
    }

    /* Start Peripheral-Peripheral DMAC */
    audiodmappfuncs.xfer_start(audiodma_pp_chn);

    return 0;
}

inline int audio_dmac_pp_stop(void * audiodma_pp_chn)
{
    if( !audiodma_pp_chn ) {
        return EINVAL;
    }

    /* Stop Peripheral-Peripheral DMAC */
    audiodmappfuncs.xfer_abort(audiodma_pp_chn);

    return 0;
}

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/hardware/deva/ctrl/rcar/dmac.c $ $Rev: 812827 $")
#endif

