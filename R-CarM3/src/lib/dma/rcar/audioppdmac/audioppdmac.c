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
#include "audioppdmac.h"

/* the following def, applicable to 3rd gen R-Car (H3/M3) only, should move to r-car.h */
#define RCAR_AUDIODMACPP_EXT_BASE  0xEC760000

/* Register offsets */
#define AUDIOPPDMAC_SAR         0x00
#define AUDIOPPDMAC_DAR         0x04
#define AUDIOPPDMAC_CHCR        0x0C

/* Register contents for AUDIOPPDMAC_CHCR register */
#define AUDIOPPDMAC_CHCR_DE     (1 << 0)    /* DMA enable */

/* Number of AUDIO PP DMAC channel groups, each with its own memory address base */
#define RCAR_PP_DMAC_GROUPS_H2         1   /* H2/M2/E2 variants have 1 group */
#define RCAR_PP_DMAC_GROUPS_V2         0   /* V2 variants don't support this functionality */
#define RCAR_PP_DMAC_GROUPS_H3         2   /* H3/M3 variants have two groups */
#define RCAR_MAX_PP_DMAC_GROUPS  RCAR_PP_DMAC_GROUPS_H3

#define RCAR_PP_DMAC_CHANNELS_H2         29   /* H2/M2/E2 variants have 29 channels */
#define RCAR_PP_DMAC_CHANNELS_V2         0   /* V2 variants don't support this functionality */
#define RCAR_PP_DMAC_CHANNELS_H3         58   /* H3/M3 variants have 58 channels */
#define RCAR_MAX_PP_DMAC_CHANNELS  RCAR_PP_DMAC_CHANNELS_H3

static audioppdmac_ctrl_t  audioppdmac[RCAR_MAX_PP_DMAC_GROUPS] = { {0, 0}, };

static char *dmac_opts[] = {
    "ver",
    NULL
};

static int
audioppdmac_parse_options(dma_channel_t *chan, char *options)
{
    char   *value;
    int     opt;

    while (options && *options != '\0') {
        if ((opt = getsubopt(&options, dmac_opts, &value)) == -1) {
            return EINVAL;
        }
        switch (opt) {
            case 0:
                if( !strcmp( value, "h2" ) ||
                    !strcmp( value, "m2" ) ||
                    !strcmp( value, "e2" ) ) {
                    chan->channels = RCAR_PP_DMAC_CHANNELS_H2;
                    chan->channel_groups = RCAR_PP_DMAC_GROUPS_H2;
                } else if( !strcmp( value, "v2" ) ) {
                    chan->channels = RCAR_PP_DMAC_CHANNELS_V2;
                    chan->channel_groups = RCAR_PP_DMAC_GROUPS_V2;
                } else if ( !strcmp( value, "h3" ) ||
                            !strcmp( value, "m3" ) ) {
                    chan->channels = RCAR_PP_DMAC_CHANNELS_H3;
                    chan->channel_groups = RCAR_PP_DMAC_GROUPS_H3;
                } else { /* use H2/M2 defaults */
                    chan->channels = RCAR_PP_DMAC_CHANNELS_H2;
                    chan->channel_groups = RCAR_PP_DMAC_GROUPS_H2;
                }
                break;
            default:
                return EINVAL;
        }
    }

    return EOK;
}

static int
audioppdma_init(const char *options)
{
    return EOK;
}

static void
audioppdma_fini()
{
}

static void
audioppdma_query_channel(void *handle, dma_channel_query_t *chinfo)
{
    dma_channel_t   *chan = handle;

    chinfo->chan_idx = chan->chan_idx;
}

static int
audioppdma_driver_info(dma_driver_info_t *info)
{
    info->dma_version_major = DMALIB_VERSION_MAJOR;
    info->dma_version_minor = DMALIB_VERSION_MINOR;
    info->dma_rev           = DMALIB_REVISION;
    info->lib_rev           = 0;
    info->description       = "RCAR AUDIO PP DMAC Controller";
    info->num_channels      = RCAR_MAX_PP_DMAC_CHANNELS;  // R-Car H3/M3 has 58 channels, R-Car H2/M2/E2 has 29 channels

    return 0;
}

static int
audioppdma_channel_info(unsigned channel, dma_channel_info_t *info)
{
    if (channel >= RCAR_MAX_PP_DMAC_CHANNELS) {
        errno = ECHRNG;
        return -1;
    }

    info->max_xfer_size         = 0xffffffff;
    info->xfer_unit_sizes       = 0x4; /* 4 byte unit transfers, fixed */
    info->max_src_fragments     = 1;
    info->max_dst_fragments     = 1;
    info->max_src_segments      = 1;
    info->max_dst_segments      = 1;
    info->caps                  = DMA_CAP_DEVICE_TO_DEVICE;
    info->mem_lower_limit       = 0;
    info->mem_upper_limit       = 0xffffffff;
    info->mem_nocross_boundary  = 0;

    return 0;
}

static uintptr_t audioppdma_map_registers(dma_channel_t *chan)
{
    uint32_t mem_map_idx;
    uint32_t chan_per_grp = chan->channels / chan->channel_groups;
    uint32_t chan_in_grp_idx = chan->chan_idx % chan_per_grp;

    chan->pbase = 0;

    switch( chan->grp_idx ) {
        case 0: chan->pbase = RCAR_AUDIODMACPP_BASE; break;
        case 1: chan->pbase = RCAR_AUDIODMACPP_EXT_BASE; break;
    }

    if( chan->pbase == 0 ) {
        return (uintptr_t)MAP_FAILED;
    }

    // check whether the memory for the DMA channel group is already mapped in.
    for (mem_map_idx = 0; mem_map_idx < RCAR_MAX_PP_DMAC_GROUPS; mem_map_idx++) {
        if (audioppdmac[mem_map_idx].paddr == chan->pbase) {
            atomic_add(&audioppdmac[mem_map_idx].num, 1);
            chan->vbase = audioppdmac[mem_map_idx].vaddr;
            break;
        }
    }

    if (chan->vbase == 0) {
        chan->vbase = mmap_device_io(0x1000, chan->pbase);
        if (chan->vbase == (uintptr_t)MAP_FAILED) {
            return (uintptr_t)MAP_FAILED;
        }
        for (mem_map_idx = 0; mem_map_idx < RCAR_MAX_PP_DMAC_GROUPS; mem_map_idx++) {
            if (audioppdmac[mem_map_idx].num == 0) {
                audioppdmac[mem_map_idx].paddr = chan->pbase;
                audioppdmac[mem_map_idx].vaddr = chan->vbase;
                audioppdmac[mem_map_idx].num   = 1;
                break;
            }
        }
    }

    chan->regs = chan->vbase + 0x20 + chan_in_grp_idx * 0x10;

    return (chan->vbase);
}

static void audioppdmac_unmap_registers(dma_channel_t *chan)
{
    uint32_t mem_map_idx;

    // unmap memory if not in use for other DMA channels in same group
    for (mem_map_idx = 0; mem_map_idx < RCAR_MAX_PP_DMAC_GROUPS; mem_map_idx++) {
        if (audioppdmac[mem_map_idx].paddr == chan->pbase) {
            atomic_sub(&audioppdmac[mem_map_idx].num, 1);
            if (audioppdmac[mem_map_idx].num == 0) {
                munmap_device_io(audioppdmac[mem_map_idx].vaddr, 0x1000);
                audioppdmac[mem_map_idx].num = audioppdmac[mem_map_idx].paddr = audioppdmac[mem_map_idx].vaddr = 0;
            }
            chan->vbase = 0;
        }
    }
}

static void *
audioppdma_channel_attach(const char *options, const struct sigevent *event, unsigned *channel, int priority, dma_attach_flags flags)
{
    dma_channel_t   *chan;
    char   *optstr = NULL;
    rsrc_request_t  req = { 0 };
    uint32_t chans_per_group;

    chan = calloc(1, sizeof(*chan));
    if (chan == NULL) {
        fprintf(stderr, "%s: calloc failed\n", __FUNCTION__);
        return (NULL);
    }

    if (options) {
        optstr = strdup(options);
    } else {
        optstr = strdup("ver=h2");
    }

    if (optstr == NULL) {
        fprintf(stderr,  "%s: strdup failed\n", __FUNCTION__);
        free(chan);
        return NULL;
    }

    if (audioppdmac_parse_options(chan, optstr) != EOK) {
        fprintf(stderr, "%s: dma_parse_options failed\n", __FUNCTION__);
        free(chan);
        free(optstr);
        return NULL;
    }

    req.length = 1;
    req.start  = 0;
    req.end    = chan->channels - 1;
    req.flags  = RSRCDBMGR_DMA_CHANNEL | RSRCDBMGR_FLAG_RANGE | RSRCDBMGR_FLAG_NAME;
    req.name   = "audiopp";

    if (rsrcdbmgr_attach(&req, 1) == -1) {
        fprintf(stderr, "%s: rsrcdbmgr_attach failed: %s\n", __FUNCTION__, strerror(errno));
        free(chan);
        free(optstr);
        return (NULL);
    }

    chans_per_group = chan->channels / chan->channel_groups;
    chan->chan_idx = req.start;
    chan->grp_idx = req.start / chans_per_group;

    if( audioppdma_map_registers( chan ) == (uintptr_t)MAP_FAILED ) {
        fprintf(stderr, "%s: register map failed\n", __FUNCTION__);
        req.end = req.start;
        rsrcdbmgr_detach(&req, 1);
        free(chan);
        free(optstr);
        return (NULL);
    }

    /* Disable the channel */
    out32(chan->regs + AUDIOPPDMAC_CHCR, 0);

    free(optstr);

    return chan;
}

static void
audioppdma_channel_release(void *handle)
{
    dma_channel_t *chan = handle;
    rsrc_request_t req = { 0 };

    /* Disable the channel */
    out32(chan->regs + AUDIOPPDMAC_CHCR, 0);

    /* release DMA resource */
    req.length = 1;
    req.start  = req.end = chan->chan_idx;
    req.flags  = RSRCDBMGR_DMA_CHANNEL | RSRCDBMGR_FLAG_RANGE | RSRCDBMGR_FLAG_NAME;
    req.name   = "audiopp";
    rsrcdbmgr_detach(&req, 1);

    // unmap registers
    audioppdmac_unmap_registers(chan);

    free(handle);
}

static int
audioppdma_setup_xfer(void *handle, const dma_transfer_t *tinfo)
{
    dma_channel_t   *chan = handle;

    out32(chan->regs + AUDIOPPDMAC_SAR,  tinfo->src_addrs[0].paddr);
    out32(chan->regs + AUDIOPPDMAC_DAR,  tinfo->dst_addrs[0].paddr);
    out32(chan->regs + AUDIOPPDMAC_CHCR, tinfo->req_id);

    return 0;
}

static int
audioppdma_start(void *handle)
{
    dma_channel_t   *chan = handle;

    out32(chan->regs + AUDIOPPDMAC_CHCR, in32(chan->regs + AUDIOPPDMAC_CHCR) | AUDIOPPDMAC_CHCR_DE);

    return 0;
}

static int
audioppdma_abort(void *handle)
{
    dma_channel_t   *chan = handle;

    out32(chan->regs + AUDIOPPDMAC_CHCR, 0);

    return 0;
}

int
get_audioppdmafuncs(dma_functions_t *functable, int tabsize)
{
    DMA_ADD_FUNC(functable, init, audioppdma_init, tabsize);
    DMA_ADD_FUNC(functable, fini, audioppdma_fini, tabsize);
    DMA_ADD_FUNC(functable, driver_info, audioppdma_driver_info, tabsize);
    DMA_ADD_FUNC(functable, channel_info, audioppdma_channel_info, tabsize);
    DMA_ADD_FUNC(functable, channel_attach, audioppdma_channel_attach, tabsize);
    DMA_ADD_FUNC(functable, channel_release, audioppdma_channel_release, tabsize);
    DMA_ADD_FUNC(functable, setup_xfer, audioppdma_setup_xfer, tabsize);
    DMA_ADD_FUNC(functable, xfer_start, audioppdma_start, tabsize);
    DMA_ADD_FUNC(functable, xfer_abort, audioppdma_abort, tabsize);
    DMA_ADD_FUNC(functable, xfer_complete, audioppdma_abort, tabsize);
    DMA_ADD_FUNC(functable, query_channel, audioppdma_query_channel, tabsize);

    return 0;
}

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/lib/dma/rcar/audioppdmac/audioppdmac.c $ $Rev: 808492 $")
#endif
