/*
 * $QNXLicenseC:
 * Copyright 2015, QNX Software Systems.
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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <atomic.h>
#include <sys/rsrcdbmgr.h>
#include <sys/rsrcdbmsg.h>
#include <hw/dma.h>
#include <arm/inout.h>
#include <arm/r-car-m3.h>
#include "sysdmac.h"

/* these definitions should be modified in r-car.h, but this affects hardware/startup/boards/rcar_h2/init_board.c */
/* add these definitions for now here */
#define RCAR_SYSDMAC0_H2_BASE RCAR_SYSDMAC0_BASE
#define RCAR_SYSDMAC1_H2_BASE RCAR_SYSDMAC1_BASE

#define RCAR_SYSDMAC0_M3_BASE RCAR_SYSDMAC0_BASE
#define RCAR_SYSDMAC1_M3_BASE RCAR_SYSDMAC1_BASE
#define RCAR_SYSDMAC2_M3_BASE RCAR_SYSDMAC2_BASE

#define SYSDMAC_REG_SIZE            0x3000

/* Contents of the CHCR register */
#define SYSDMAC_CHCR_CAE            0x80000000
#define SYSDMAC_CHCR_CAIE           0x40000000
#define SYSDMAC_CHCR_DPM            0x30000000
#define SYSDMAC_CHCR_DPM_DISABLE    0x00000000
#define SYSDMAC_CHCR_DPM_NORMAL     0x10000000
#define SYSDMAC_CHCR_DPM_RPT        0x20000000
#define SYSDMAC_CHCR_DPM_RPT_INF    0x30000000
#define SYSDMAC_CHCR_RPT            0x0E000000
#define SYSDMAC_CHCR_RPT_S          0x08000000
#define SYSDMAC_CHCR_RPT_D          0x04000000
#define SYSDMAC_CHCR_RPT_C          0x02000000
#define SYSDMAC_CHCR_DPB            0x00400000
#define SYSDMAC_CHCR_TS2            0x00100000
#define SYSDMAC_CHCR_DSE            0x00080000
#define SYSDMAC_CHCR_DSIE           0x00040000
#define SYSDMAC_CHCR_DM             0x0000C000
#define SYSDMAC_CHCR_DM_FIXED       0x00000000
#define SYSDMAC_CHCR_DM_INCR        0x00004000
#define SYSDMAC_CHCR_DM_DECR        0x00008000
#define SYSDMAC_CHCR_SM             0x00003000
#define SYSDMAC_CHCR_SM_FIXED       0x00000000
#define SYSDMAC_CHCR_SM_INCR        0x00001000
#define SYSDMAC_CHCR_SM_DECR        0x00002000
#define SYSDMAC_CHCR_RS             0x00000F00
#define SYSDMAC_CHCR_RS_AUTO        0x00000400
#define SYSDMAC_CHCR_RS_ONCHIP      0x00000800
#define SYSDMAC_CHCR_TS             0x00000018
#define SYSDMAC_CHCR_TS_BYTE        0x00000000
#define SYSDMAC_CHCR_TS_WORD        0x00000008
#define SYSDMAC_CHCR_TS_LONG        0x00000010
#define SYSDMAC_CHCR_TS_8           0x00100018
#define SYSDMAC_CHCR_TS_16          0x00000018
#define SYSDMAC_CHCR_TS_32          0x00100000
#define SYSDMAC_CHCR_TS_64          0x00100008
#define SYSDMAC_CHCR_IE             0x00000004
#define SYSDMAC_CHCR_TE             0x00000002
#define SYSDMAC_CHCR_DE             0x00000001

/* Contents of the CHCRB register */
#define SYSDMAC_CHCRB_DRST          0x00008000

/* Memory descriptors related definitions */
#define SYSDMAC_DESCRIPTORS_PER_GROUP     128

/* Channel and channel group number related definitions */
#define RCAR_SYSDMAC_GROUPS_H2         2   /* H2/M2/E2/V2 variants have two SYSDMAC groups */
#define RCAR_SYSDMAC_GROUPS_H3         3   /* H3/M3 variants have three SYSDMAC groups */
#define RCAR_MAX_SYSDMAC_GROUPS  RCAR_SYSDMAC_GROUPS_H3

#define RCAR_SYSDMAC_CHANNELS_H2         30   /* H2/M2/E2/V2 variants have 30 SYSDMAC channels */
#define RCAR_SYSDMAC_CHANNELS_H3         48   /* H3/M3 variants have 48 SYSDMAC channels */
#define RCAR_MAX_SYSDMAC_CHANNELS  RCAR_SYSDMAC_CHANNELS_H3

#define RCAR_AUDIODMAC_GROUPS_H2         2   /* H2/M2 variants have two AUDIODMAC groups */
#define RCAR_AUDIODMAC_GROUPS_E2         1   /* E2 variants have one AUDIODMAC group */
#define RCAR_AUDIODMAC_GROUPS_V2         1   /* V2 variants have one AUDIODMAC group */
#define RCAR_AUDIODMAC_GROUPS_H3         2   /* H3/M3 variants have three AUDIODMAC groups */
#define RCAR_MAX_AUDIODMAC_GROUPS  RCAR_AUDIODMAC_GROUPS_H3

#define RCAR_AUDIODMAC_CHANNELS_H2         26   /* H2/M2 variants have 26 AUDIODMAC channels */
#define RCAR_AUDIODMAC_CHANNELS_E2         13   /* E2 variants have 13 AUDIODMAC channels */
#define RCAR_AUDIODMAC_CHANNELS_V2         2    /* V2 variants have 2 AUDIODMAC channels */
#define RCAR_AUDIODMAC_CHANNELS_H3         32   /* H3/M3 variants have 32 AUDIODMAC channels */
#define RCAR_MAX_AUDIODMAC_CHANNELS  RCAR_AUDIODMAC_CHANNELS_H3

/* Number of SYSDMAC + AUDIODMAC channel groups, each with its own memory address base */
#define RCAR_MAX_DMAC_GROUPS              5   /* support groups SYSDMAC 0/1/2 and AUDIODMAC 0/1 for H3/M3 */

static char*        sysdmac_version  = "h2";

static const uint16_t SYSDMAC0_TE_IRQS[] =
{
    200 + 32,   /* SYSDMAC0 - CH0 */
    201 + 32,   /* SYSDMAC0 - CH1 */
    202 + 32,   /* SYSDMAC0 - CH2 */
    203 + 32,   /* SYSDMAC0 - CH3 */
    204 + 32,   /* SYSDMAC0 - CH4 */
    205 + 32,   /* SYSDMAC0 - CH5 */
    206 + 32,   /* SYSDMAC0 - CH6 */
    207 + 32,   /* SYSDMAC0 - CH7 */
    208 + 32,   /* SYSDMAC0 - CH8 */
    209 + 32,   /* SYSDMAC0 - CH9 */
    210 + 32,   /* SYSDMAC0 - CH10 */
    211 + 32,   /* SYSDMAC0 - CH11 */
    212 + 32,   /* SYSDMAC0 - CH12 */
    213 + 32,   /* SYSDMAC0 - CH13 */
    214 + 32,   /* SYSDMAC0 - CH14 */
    215 + 32,   /* SYSDMAC0 - CH15 - applicable only to H3/M3 */
};

static const uint16_t SYSDMAC1_TE_IRQS[] =
{
    216 + 32,   /* SYSDMAC1 - CH0 */
    217 + 32,   /* SYSDMAC1 - CH1 */
    218 + 32,   /* SYSDMAC1 - CH2 */
    219 + 32,   /* SYSDMAC1 - CH3 */
    308 + 32,   /* SYSDMAC1 - CH4 */
    309 + 32,   /* SYSDMAC1 - CH5 */
    310 + 32,   /* SYSDMAC1 - CH6 */
    311 + 32,   /* SYSDMAC1 - CH7 */
    312 + 32,   /* SYSDMAC1 - CH8 */
    313 + 32,   /* SYSDMAC1 - CH9 */
    314 + 32,   /* SYSDMAC1 - CH10 */
    315 + 32,   /* SYSDMAC1 - CH11 */
    316 + 32,   /* SYSDMAC1 - CH12 */
    317 + 32,   /* SYSDMAC1 - CH13 */
    318 + 32,   /* SYSDMAC1 - CH14 */
    319 + 32,   /* SYSDMAC1 - CH15 - only applicable to H3/M3 */
};

static const uint16_t SYSDMAC2_TE_IRQS[] =
{
    /* all entries are only applicable to H3/M3 */
    417 + 32,   /* SYSDMAC2 - CH0 */
    418 + 32,   /* SYSDMAC2 - CH1 */
    419 + 32,   /* SYSDMAC2 - CH2 */
    420 + 32,   /* SYSDMAC2 - CH3 */
    421 + 32,   /* SYSDMAC2 - CH4 */
    422 + 32,   /* SYSDMAC2 - CH5 */
    423 + 32,   /* SYSDMAC2 - CH6 */
    424 + 32,   /* SYSDMAC2 - CH7 */
    425 + 32,   /* SYSDMAC2 - CH8 */
    426 + 32,   /* SYSDMAC2 - CH9 */
    427 + 32,   /* SYSDMAC2 - CH10 */
    428 + 32,   /* SYSDMAC2 - CH11 */
    429 + 32,   /* SYSDMAC2 - CH12 */
    430 + 32,   /* SYSDMAC2 - CH13 */
    431 + 32,   /* SYSDMAC2 - CH14 */
    397 + 32,   /* SYSDMAC2 - CH15 */
};

static const uint16_t AUDIODMAC_TE_IRQS[] =
{
    320 + 32,   /* AUDIODMAC CH0 */
    321 + 32,   /* AUDIODMAC CH1 */
    322 + 32,   /* AUDIODMAC CH2 */
    323 + 32,   /* AUDIODMAC CH3 */
    324 + 32,   /* AUDIODMAC CH4 */
    325 + 32,   /* AUDIODMAC CH5 */
    326 + 32,   /* AUDIODMAC CH6 */
    327 + 32,   /* AUDIODMAC CH7 */
    328 + 32,   /* AUDIODMAC CH8 */
    329 + 32,   /* AUDIODMAC CH9 */
    330 + 32,   /* AUDIODMAC CH10 */
    331 + 32,   /* AUDIODMAC CH11 */
    332 + 32,   /* AUDIODMAC CH12 */
    333 + 32,   /* AUDIODMAC CH13 */
    334 + 32,   /* AUDIODMAC CH14 */
    335 + 32,   /* AUDIODMAC CH15 */
    336 + 32,   /* AUDIODMAC CH16 */
    337 + 32,   /* AUDIODMAC CH17 */
    338 + 32,   /* AUDIODMAC CH18 */
    339 + 32,   /* AUDIODMAC CH19 */
    340 + 32,   /* AUDIODMAC CH20 */
    341 + 32,   /* AUDIODMAC CH11 */
    342 + 32,   /* AUDIODMAC CH22 */
    343 + 32,   /* AUDIODMAC CH23 */
    344 + 32,   /* AUDIODMAC CH24 */
    345 + 32,   /* AUDIODMAC CH25 */
    346 + 32,   /* AUDIODMAC CH26 - only applicable to H3/M3*/
    347 + 32,   /* AUDIODMAC CH27 - only applicable to H3/M3*/
    348 + 32,   /* AUDIODMAC CH28 - only applicable to H3/M3*/
    349 + 32,   /* AUDIODMAC CH29 - only applicable to H3/M3*/
    382 + 32,   /* AUDIODMAC CH30 - only applicable to H3/M3 */
    383 + 32,   /* AUDIODMAC CH31 - only applicable to H3/M3 */
};

static sysdmac_ctrl_t  sysdmac[RCAR_MAX_DMAC_GROUPS] = { {0, 0}, };

static int dma_alloc_buffer(void *handle, dma_addr_t *addr, unsigned size, unsigned flags);
static void dma_free_buffer(void *handle, dma_addr_t *addr);

static int
dma_init(const char* options)
{
    return 0;
}

static void
dma_fini()
{
}

static void
dma_query_channel(void *handle, dma_channel_query_t *chinfo)
{
    dma_channel_t   *chan = handle;

    chinfo->chan_idx = chan->chan_idx;
    chinfo->irq      = chan->irq;
}

static int
dma_driver_info(dma_driver_info_t *info)
{
    info->dma_version_major = DMALIB_VERSION_MAJOR;
    info->dma_version_minor = DMALIB_VERSION_MINOR;
    info->dma_rev           = DMALIB_REVISION;
    info->lib_rev           = 0;
    info->description       = "RCAR SYS/AUDIO DMAC Controller";
    info->num_channels      = RCAR_MAX_SYSDMAC_CHANNELS;
    // 48/30 channels for SYS DMAC on RCAR H3/H2
    // 32/26 channels for Audio DMAC on RCAR H3/H2

    return 0;
}

static int
dma_channel_info(unsigned channel, dma_channel_info_t *info)
{
    if (channel > RCAR_MAX_SYSDMAC_CHANNELS) {
        errno = ECHRNG;
        return -1;
    }

    info->max_xfer_size         = 0xffffffff;
    info->xfer_unit_sizes       = 0x1; // 64, 32, 16, 8, 4, 2, 1 byte unit transfers
    info->max_src_fragments     = SYSDMAC_DESCRIPTORS_PER_GROUP;
    info->max_dst_fragments     = SYSDMAC_DESCRIPTORS_PER_GROUP;
    info->max_src_segments      = SYSDMAC_DESCRIPTORS_PER_GROUP;
    info->max_dst_segments      = SYSDMAC_DESCRIPTORS_PER_GROUP;
    info->caps                  = DMA_CAP_SRC_INCREMENT | DMA_CAP_SRC_DECREMENT | DMA_CAP_SRC_SEGMENTED | DMA_CAP_SRC_NO_INCREMENT |
                                  DMA_CAP_DST_INCREMENT | DMA_CAP_DST_DECREMENT | DMA_CAP_DST_SEGMENTED | DMA_CAP_DST_NO_INCREMENT |
                                  DMA_CAP_DEVICE_TO_MEMORY | DMA_CAP_MEMORY_TO_DEVICE | DMA_CAP_EVENT_ON_COMPLETE;
    info->mem_lower_limit       = 0;
    info->mem_upper_limit       = 0xffffffff;
    info->mem_nocross_boundary  = 0;

    return 0;
}

static char *dma_opts[] = {
    "ver",          // optional, R-car version, e.g. "h2", "m2", "e2", "v2", "h3", "m3"
    "dma",          // dmac type, "sys" or "audio", default "sys"
    "desc",         // number of descriptors required
    NULL
};

static int
sysdmac_parse_options(dma_channel_t *chan, char *options)
{
    char   *value;
    int     opt;

    /* for now initialize these variables corresponding to the H2 variant;
       after all users of this module are updated to pass the ver parameter,
       update the code to initialize these variables to 0 */
    uint32_t sys_channels = RCAR_SYSDMAC_CHANNELS_H2;
    uint32_t audio_channels = RCAR_AUDIODMAC_CHANNELS_H2;
    uint32_t sys_channel_groups = RCAR_SYSDMAC_GROUPS_H2;
    uint32_t audio_channel_groups = RCAR_SYSDMAC_CHANNELS_H2;
    sysdmac_version = "h2";

    while (options && *options != '\0') {
        if ((opt = getsubopt(&options, dma_opts, &value)) == -1) {
            return EINVAL;
        }
        switch (opt) {
            case 0:
                if( !strcmp( value, "h2" ) ||
                    !strcmp( value, "m2" ) ) {
                    sys_channels = RCAR_SYSDMAC_CHANNELS_H2;
                    sys_channel_groups = RCAR_SYSDMAC_GROUPS_H2;
                    audio_channels = RCAR_AUDIODMAC_CHANNELS_H2;
                    audio_channel_groups = RCAR_AUDIODMAC_GROUPS_H2;
                } else if( !strcmp( value, "e2" ) ) {
                    sys_channels = RCAR_SYSDMAC_CHANNELS_H2;
                    sys_channel_groups = RCAR_SYSDMAC_GROUPS_H2;
                    audio_channels = RCAR_AUDIODMAC_CHANNELS_E2;
                    audio_channel_groups = RCAR_AUDIODMAC_GROUPS_E2;
                } else if( !strcmp( value, "v2" ) ) {
                    sys_channels = RCAR_SYSDMAC_CHANNELS_H2;
                    sys_channel_groups = RCAR_SYSDMAC_GROUPS_H2;
                    audio_channels = RCAR_AUDIODMAC_CHANNELS_V2;
                    audio_channel_groups = RCAR_AUDIODMAC_GROUPS_V2;
                } else if ( !strcmp( value, "h3" ) ||
                            !strcmp( value, "m3" ) ) {
                    sys_channels = RCAR_SYSDMAC_CHANNELS_H3;
                    sys_channel_groups = RCAR_SYSDMAC_GROUPS_H3;
                    audio_channels = RCAR_AUDIODMAC_CHANNELS_H3;
                    audio_channel_groups = RCAR_AUDIODMAC_GROUPS_H3;
                    sysdmac_version = "h3";
                } else {
                    return EINVAL;
                }
                break;
            case 1:
                strcpy(chan->name, value);
                break;
            case 2:
                chan->desc_num = strtoul(value, 0, 0);
                break;
            default:
                return EINVAL;
        }
    }

    if( sys_channels == 0 || audio_channels == 0 ) {
        return EINVAL;
    }

    /* a number of descriptors larger than 128 is invalid */
    if( chan->desc_num > 128 ) {
        chan->desc_num = 0;
    }

    if( strcmp(chan->name, "sys") == 0 ) {
        chan->channels = sys_channels;
        chan->channel_groups = sys_channel_groups;
    } else if( strcmp(chan->name, "audio") == 0 ) {
        chan->channels = audio_channels;
        chan->channel_groups = audio_channel_groups;
    } else {
        return EINVAL;
    }

    return EOK;
}

static uintptr_t sysdmac_map_registers(dma_channel_t *chan)
{
    uint32_t mem_map_idx;
    uint32_t chan_per_grp = chan->channels / chan->channel_groups;
    uint32_t chan_in_grp_idx = chan->chan_idx % chan_per_grp;

    chan->pbase = 0;

    if( strcmp(chan->name, "sys") == 0 ) {
        if( !strcmp( sysdmac_version, "h3" ) ||
            !strcmp( sysdmac_version, "m3" ) ) { /* Rcar H3/M3 */
            switch( chan->grp_idx ) {
                case 0: chan->pbase = RCAR_SYSDMAC0_M3_BASE; break;
                case 1: chan->pbase = RCAR_SYSDMAC1_M3_BASE; break;
                case 2: chan->pbase = RCAR_SYSDMAC2_M3_BASE; break;
            }
        } else { /* Rcar H2/M2/E2 */
            switch( chan->grp_idx ) {
                case 0: chan->pbase = RCAR_SYSDMAC0_H2_BASE; break;
                case 1: chan->pbase = RCAR_SYSDMAC1_H2_BASE; break;
            }
        }
    } else if( strcmp(chan->name, "audio") == 0 ) {
        switch( chan->grp_idx ) {
            case 0: chan->pbase = RCAR_AUDIODMAC0_BASE; break;
            case 1: chan->pbase = RCAR_AUDIODMAC1_BASE; break;
        }
    }

    if( chan->pbase == 0 ) {
        return (uintptr_t)MAP_FAILED;
    }

    chan->pbase += RCAR_SYSDMAC_REGS;

    // check whether the memory for the DMA channel group is already mapped in.
    for (mem_map_idx = 0; mem_map_idx < RCAR_MAX_DMAC_GROUPS; mem_map_idx++) {
        if (sysdmac[mem_map_idx].paddr == chan->pbase) {
            atomic_add(&sysdmac[mem_map_idx].num, 1);
            chan->vbase = sysdmac[mem_map_idx].vaddr;
            break;
        }
    }

    if (chan->vbase == 0) {
        chan->vbase = mmap_device_io(SYSDMAC_REG_SIZE, chan->pbase);
        if (chan->vbase == (uintptr_t)MAP_FAILED)
            return (uintptr_t)MAP_FAILED;

        for (mem_map_idx = 0; mem_map_idx < RCAR_MAX_DMAC_GROUPS; mem_map_idx++) {
            if (sysdmac[mem_map_idx].num == 0) {
                sysdmac[mem_map_idx].paddr = chan->pbase;
                sysdmac[mem_map_idx].vaddr = chan->vbase;
                sysdmac[mem_map_idx].num   = 1;
                break;
            }
        }
    }

    chan->regs = chan->vbase + chan_in_grp_idx * RCAR_SYSDMAC_REGSIZE;

    return (chan->vbase);
}

static void sysdmac_unmap_registers(dma_channel_t *chan)
{
    uint32_t mem_map_idx;

    // unmap memory if not in use for other DMA channels in same group
    for (mem_map_idx = 0; mem_map_idx < RCAR_MAX_DMAC_GROUPS; mem_map_idx++) {
        if (sysdmac[mem_map_idx].paddr == chan->pbase) {
            atomic_sub(&sysdmac[mem_map_idx].num, 1);
            if (sysdmac[mem_map_idx].num == 0) {
                munmap_device_io(sysdmac[mem_map_idx].vaddr, SYSDMAC_REG_SIZE);
                sysdmac[mem_map_idx].num = sysdmac[mem_map_idx].paddr = sysdmac[mem_map_idx].vaddr = 0;
            }
            chan->vbase = 0;
        }
    }
}

static void *
dma_channel_attach(const char *options, const struct sigevent *event, unsigned *channel, int priority, dma_attach_flags flags)
{
    dma_channel_t   *chan;
    char            *optstr = NULL;
    rsrc_request_t  req = { 0 };
    uint32_t chans_per_group;

    if (options) {
        optstr = strdup(options);
    } else {
        optstr = strdup("dma=sys");
    }

    if (optstr == NULL) {
        fprintf(stderr,  "%s: strdup failed\n", __FUNCTION__);
        return NULL;
    }

    chan = calloc(1, sizeof(*chan));
    if (chan == NULL) {
        fprintf(stderr, "%s: calloc failed\n", __FUNCTION__);
        goto fail0;
    }

    if ((errno = sysdmac_parse_options(chan, optstr)) != EOK) {
        fprintf(stderr, "%s: parse options failed\n", __FUNCTION__);
        goto fail1;
    }

    req.length = 1;
    req.start  = 0;
    req.end    = chan->channels - 1;
    req.flags  = RSRCDBMGR_DMA_CHANNEL | RSRCDBMGR_FLAG_RANGE | RSRCDBMGR_FLAG_NAME;
    req.name   = chan->name;

    // The lower DMA channel numbers have highest priority
    if (flags & DMA_ATTACH_PRIORITY_STRICT) {
        req.start = req.end = priority;
    } else if (flags & DMA_ATTACH_PRIORITY_ATLEAST) {
        req.start = 0;
        req.end = chan->channels - 1 - priority;
        req.flags |= RSRCDBMGR_FLAG_TOPDOWN;
    } else if (!(flags & DMA_ATTACH_PRIORITY_HIGHEST)) {
        req.flags |= RSRCDBMGR_FLAG_TOPDOWN;
    }

    if (rsrcdbmgr_attach(&req, 1) == -1) {
        fprintf(stderr, "%s: DMA channel rsrcdbmgr_attach failed: %s\n", __FUNCTION__, strerror(errno));
        goto fail1;
    }

    chans_per_group = chan->channels / chan->channel_groups;
    chan->chan_idx = req.start;
    chan->grp_idx = req.start / chans_per_group;

    // Request descriptor memory if necessary
    if (chan->desc_num > 0 && chan->desc_num <= 128) {
        req.length = chan->desc_num;
        req.start  = chan->grp_idx * 128;
        req.end    = req.start + 128 - 1;
        req.flags  = RSRCDBMGR_MEMORY | RSRCDBMGR_FLAG_RANGE | RSRCDBMGR_FLAG_NAME;
        req.name   = chan->name;

        // If resource request fails, the library will try to allocate DMA descriptor from system memory later
        if (rsrcdbmgr_attach(&req, 1) == -1) {
            fprintf(stderr, "%s: DMA descriptor rsrcdbmgr_attach failed: %s\n", __FUNCTION__, strerror(errno));
            chan->desc_num = 0;
        } else {
            chan->desc_idx = req.start;
        }
    }

    // map in DMA registers for DMA channel group, including descriptor memory
    if (sysdmac_map_registers(chan) == (uintptr_t)MAP_FAILED) {
        goto fail2;
    }

    // Disable channel
    out32(chan->regs + RCAR_SYSDMAC_DMACHCR, 0);

    chan->iid = -1;

    if (strcmp(chan->name, "sys") == 0) {
        switch( chan->grp_idx ) {
            case 0:
                chan->irq = SYSDMAC0_TE_IRQS[chan->chan_idx];
                break;
            case 1:
                chan->irq = SYSDMAC1_TE_IRQS[chan->chan_idx % chans_per_group];
                break;
            case 2:
                chan->irq = SYSDMAC2_TE_IRQS[chan->chan_idx % chans_per_group];
                break;
        }
    } else {
        chan->irq = AUDIODMAC_TE_IRQS[chan->chan_idx];
    }

    if (flags & (DMA_ATTACH_EVENT_ON_COMPLETE | DMA_ATTACH_EVENT_PER_SEGMENT) && event != NULL) {
        chan->iid = InterruptAttachEvent(chan->irq, event, _NTO_INTR_FLAGS_TRK_MSK);

        if (chan->iid == -1) {
            goto fail3;
        }
    }

    chan->aflags = flags;

    return chan;

fail3:
    sysdmac_unmap_registers(chan);
fail2:
    req.length = 1;
    req.start  = req.end = chan->chan_idx;
    req.flags  = RSRCDBMGR_DMA_CHANNEL | RSRCDBMGR_FLAG_RANGE | RSRCDBMGR_FLAG_NAME;
    req.name   = chan->name;
    rsrcdbmgr_detach(&req, 1);

    if (chan->desc_num > 0) {
        req.length = chan->desc_num;
        req.start  = chan->desc_idx;
        req.end    = req.start + chan->desc_num - 1;
        req.flags  = RSRCDBMGR_MEMORY | RSRCDBMGR_FLAG_RANGE | RSRCDBMGR_FLAG_NAME;
        req.name   = chan->name;
        rsrcdbmgr_detach(&req, 1);
    }

fail1:
    free(chan);

fail0:
    if (optstr) {
        free(optstr);
    }
    return (NULL);
}

static void
dma_channel_release(void *handle)
{
    dma_channel_t   *chan = handle;
    rsrc_request_t  req = { 0 };

    // Disable the channel
    out32(chan->regs + RCAR_SYSDMAC_DMACHCR, 0);

    // release descriptor memory
    if (chan->desc.len > 0) {
        dma_free_buffer(handle, &chan->desc);
    }

    // Detach interrupt
    if (chan->iid != -1) {
        InterruptDetach(chan->iid);
    }

    // release DMA resource
    req.length = 1;
    req.start  = req.end = chan->chan_idx;
    req.flags  = RSRCDBMGR_DMA_CHANNEL | RSRCDBMGR_FLAG_RANGE | RSRCDBMGR_FLAG_NAME;
    req.name   = chan->name;
    rsrcdbmgr_detach(&req, 1);

    if (chan->desc_num > 0) {
        req.length = chan->desc_num;
        req.start  = chan->desc_idx;
        req.end    = req.start + chan->desc_num - 1;
        req.flags  = RSRCDBMGR_MEMORY | RSRCDBMGR_FLAG_RANGE | RSRCDBMGR_FLAG_NAME;
        req.name   = chan->name;
        rsrcdbmgr_detach(&req, 1);
    }

    // unmap registers
    sysdmac_unmap_registers(chan);

    free(handle);
}

static int
dma_setup_descriptor(void *handle, const dma_transfer_t *tinfo, uint32_t chcr)
{
    dma_channel_t   *chan = handle;
    sysdmac_desc_t  *desc;
    off64_t         dpbase;     // descriptor physical address
    int             segs, sgi, desc_idx_in_grp;

    // We only support source and destination have same amount of segments
    if (tinfo->src_fragments <= 0 && tinfo->dst_fragments <= 0) {
        return -1;
    }
    if (chan->desc_num == 0) {  // no internal descriptor memory
        if (chan->desc.len == 0) {
            if( dma_alloc_buffer( handle,
                                  &chan->desc,
                                  SYSDMAC_DESCRIPTORS_PER_GROUP * sizeof(sysdmac_desc_t),
                                  DMA_BUF_FLAG_NOCACHE) != 0 ) {
                fprintf(stderr, "SYSDMAC: Unable to allocate DMA descriptor buffer");
                return (-1);
            }
        }
        dpbase = chan->desc.paddr | 1;  // use external memory
        desc   = (sysdmac_desc_t *)chan->desc.vaddr;
    } else {
        desc_idx_in_grp = chan->desc_idx % 128;
        dpbase = chan->pbase + 0x2000 + desc_idx_in_grp * sizeof(sysdmac_desc_t);
        desc   = (sysdmac_desc_t *)(chan->vbase + 0x2000 + desc_idx_in_grp * sizeof(sysdmac_desc_t));
    }

    segs = tinfo->src_flags & DMA_ADDR_FLAG_SEGMENTED ? tinfo->src_fragments : tinfo->dst_fragments;

    out32(chan->regs + RCAR_SYSDMAC_DMADPBASE, dpbase);
    out32(chan->regs + RCAR_SYSDMAC_DMAFIXDPBASE, dpbase >> 32);
    out32(chan->regs + RCAR_SYSDMAC_DMACHCRB, SYSDMAC_CHCRB_DRST);  // Reset descriptor
    in32(chan->regs + RCAR_SYSDMAC_DMACHCRB);
    out32(chan->regs + RCAR_SYSDMAC_DMACHCRB, (segs - 1) << 24);

    out32(chan->regs + RCAR_SYSDMAC_DMAFIXSAR, tinfo->src_addrs[0].paddr >> 32);
    out32(chan->regs + RCAR_SYSDMAC_DMAFIXDAR, tinfo->dst_addrs[0].paddr >> 32);

    for (sgi = 0; sgi < segs; sgi++, desc++) {
        desc->sar  = tinfo->src_flags & DMA_ADDR_FLAG_NO_INCREMENT ? tinfo->src_addrs[0].paddr : tinfo->src_addrs[sgi].paddr;
        desc->dar  = tinfo->dst_flags & DMA_ADDR_FLAG_NO_INCREMENT ? tinfo->dst_addrs[0].paddr : tinfo->dst_addrs[sgi].paddr;
	    desc->tcr  = tinfo->src_flags & DMA_ADDR_FLAG_NO_INCREMENT ? tinfo->dst_addrs[sgi].len : tinfo->src_addrs[sgi].len;
        desc->tcr /= tinfo->xfer_unit_size;
    }

    chcr |= SYSDMAC_CHCR_RPT | SYSDMAC_CHCR_DPB;

    if (chan->aflags & DMA_ATTACH_EVENT_PER_SEGMENT) {
        chcr |= SYSDMAC_CHCR_DSIE;
    }
    if (chan->aflags & DMA_ATTACH_EVENT_ON_COMPLETE) {
        chcr |= SYSDMAC_CHCR_IE;
    }

    out32(chan->regs + RCAR_SYSDMAC_DMACHCR, chcr);

    return 0;
}

static int
dma_setup_xfer(void *handle, const dma_transfer_t *tinfo)
{
    dma_channel_t   *chan = handle;
    uint32_t        chcr = 0;

    switch (tinfo->xfer_unit_size) {
        case 2:
            chcr |= SYSDMAC_CHCR_TS_WORD;
            break;
        case 4:
            chcr |= SYSDMAC_CHCR_TS_LONG;
            break;
        case 8:
            chcr |= SYSDMAC_CHCR_TS_8;
            break;
        case 16:
            chcr |= SYSDMAC_CHCR_TS_16;
            break;
        case 32:
            chcr |= SYSDMAC_CHCR_TS_32;
            break;
        case 64:
            chcr |= SYSDMAC_CHCR_TS_32;
            break;
        default:
            chcr |= SYSDMAC_CHCR_TS_BYTE;
            break;
    }

    if (tinfo->src_flags & DMA_ADDR_FLAG_DECREMENT) {
        chcr |= SYSDMAC_CHCR_SM_DECR;
    } else if (!(tinfo->src_flags & DMA_ADDR_FLAG_NO_INCREMENT)) {
        chcr |= SYSDMAC_CHCR_SM_INCR;
    }

    if (tinfo->dst_flags & DMA_ADDR_FLAG_DECREMENT) {
        chcr |= SYSDMAC_CHCR_DM_DECR;
    }
    if (!(tinfo->dst_flags & DMA_ADDR_FLAG_NO_INCREMENT)) {
        chcr |= SYSDMAC_CHCR_DM_INCR;
    }

    // Hardware triggered DMA transfer
    if ((tinfo->src_flags | tinfo->dst_flags) & DMA_ADDR_FLAG_DEVICE) {
        out16(chan->regs + RCAR_SYSDMAC_DMARS, tinfo->req_id);
        chcr |= SYSDMAC_CHCR_RS_ONCHIP;
    } else {
        chcr |= SYSDMAC_CHCR_RS_AUTO;
    }

    // xfer_complete needs this
    chan->mflags = tinfo->mode_flags;

    if ((tinfo->src_flags | tinfo->dst_flags) & DMA_ADDR_FLAG_SEGMENTED) {
        if (tinfo->mode_flags & DMA_MODE_FLAG_REPEAT) {
            chcr |= SYSDMAC_CHCR_DPM_RPT;
        } else {
            chcr |= SYSDMAC_CHCR_DPM_NORMAL;
        }

        return dma_setup_descriptor(handle, tinfo, chcr);
    }

    if (chan->aflags & DMA_ATTACH_EVENT_ON_COMPLETE) {
        chcr |= SYSDMAC_CHCR_IE;
    }

    out32(chan->regs + RCAR_SYSDMAC_DMACHCR, chcr);

    chan->xfer_count = tinfo->xfer_bytes / tinfo->xfer_unit_size;
    chan->xfer_unit_size = tinfo->xfer_unit_size;

    // Single DMA buffer
    out32(chan->regs + RCAR_SYSDMAC_DMASAR, tinfo->src_addrs[0].paddr);
    out32(chan->regs + RCAR_SYSDMAC_DMAFIXSAR, tinfo->src_addrs[0].paddr >> 32);
    out32(chan->regs + RCAR_SYSDMAC_DMADAR, tinfo->dst_addrs[0].paddr);
    out32(chan->regs + RCAR_SYSDMAC_DMAFIXDAR, tinfo->dst_addrs[0].paddr >> 32);
    out32(chan->regs + RCAR_SYSDMAC_DMATCR, chan->xfer_count);

    return 0;
}

static int
dma_start(void *handle)
{
    dma_channel_t   *chan = handle;
    uint32_t        chcr;

    chcr = in32(chan->regs + RCAR_SYSDMAC_DMACHCR) & ~SYSDMAC_CHCR_TE;

    out32(chan->regs + RCAR_SYSDMAC_DMACHCR, chcr | SYSDMAC_CHCR_DE);

    return 0;
}

static int
dma_alloc_buffer(void *handle, dma_addr_t *addr, unsigned size, unsigned flags)
{
    int     prot_flags = PROT_READ | PROT_WRITE;
    int     map_flags  = MAP_PHYS | MAP_ANON;

    if (flags & DMA_BUF_FLAG_NOCACHE) {
        prot_flags |= PROT_NOCACHE;
    }

    if (flags & DMA_BUF_FLAG_SHARED) {
        map_flags |= MAP_SHARED;
    } else {
        map_flags |= MAP_PRIVATE;
    }

    addr->vaddr = mmap(NULL, size, prot_flags, map_flags, NOFD, 0);

    mem_offset64(addr->vaddr, NOFD, 1, &addr->paddr, 0);
    addr->len = size;

    return 0;
}

static void
dma_free_buffer(void *handle, dma_addr_t *addr)
{
    if (addr->len) {
        munmap(addr->vaddr, addr->len);
    }
    addr->vaddr = NULL;
    addr->len   = 0;
}

static int
dma_abort(void *handle)
{
    dma_channel_t   *chan = handle;

    out32(chan->regs + RCAR_SYSDMAC_DMACHCR,
            in32(chan->regs + RCAR_SYSDMAC_DMACHCR) & ~(SYSDMAC_CHCR_DE | SYSDMAC_CHCR_DSE | SYSDMAC_CHCR_IE | SYSDMAC_CHCR_TE));

    if (chan->iid != -1) {
        InterruptUnmask(chan->irq, chan->iid);
    }

    return 0;
}

static unsigned
dma_bytes_left(void *handle)
{
    dma_channel_t *chan = handle;

    return (in32(chan->regs + RCAR_SYSDMAC_DMATCR) * chan->xfer_unit_size);
}

static int
xfer_complete(void *handle)
{
    dma_channel_t *chan = handle;

    if (!(chan->mflags & DMA_MODE_FLAG_REPEAT)) {
        return dma_abort(handle);
    }

    out32(chan->regs + RCAR_SYSDMAC_DMACHCR,
          in32(chan->regs + RCAR_SYSDMAC_DMACHCR) & ~(SYSDMAC_CHCR_DSE | SYSDMAC_CHCR_TE));

    if (chan->iid != -1) {
        InterruptUnmask(chan->irq, chan->iid);
    }

    return 0;
}

int
get_dmafuncs(dma_functions_t *functable, int tabsize)
{
    DMA_ADD_FUNC(functable, init, dma_init, tabsize);
    DMA_ADD_FUNC(functable, fini, dma_fini, tabsize);
    DMA_ADD_FUNC(functable, driver_info, dma_driver_info, tabsize);
    DMA_ADD_FUNC(functable, channel_info, dma_channel_info, tabsize);
    DMA_ADD_FUNC(functable, channel_attach, dma_channel_attach, tabsize);
    DMA_ADD_FUNC(functable, channel_release, dma_channel_release, tabsize);
    DMA_ADD_FUNC(functable, setup_xfer, dma_setup_xfer, tabsize);
    DMA_ADD_FUNC(functable, xfer_start, dma_start, tabsize);
    DMA_ADD_FUNC(functable, xfer_abort, dma_abort, tabsize);
    DMA_ADD_FUNC(functable, bytes_left, dma_bytes_left, tabsize);
    DMA_ADD_FUNC(functable, xfer_complete, xfer_complete, tabsize);
    DMA_ADD_FUNC(functable, alloc_buffer, dma_alloc_buffer, tabsize);
    DMA_ADD_FUNC(functable, free_buffer, dma_free_buffer, tabsize);
    DMA_ADD_FUNC(functable, query_channel, dma_query_channel, tabsize);

    return 0;
}

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/lib/dma/rcar/sysdmac/sysdmac.c $ $Rev: 812167 $")
#endif
