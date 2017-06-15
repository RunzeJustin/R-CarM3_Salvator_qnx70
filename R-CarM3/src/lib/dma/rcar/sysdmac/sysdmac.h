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


#ifndef __RCAR_SYSDMAC_H__
#define __RCAR_SYSDMAC_H__

#include <inttypes.h>
#include <sys/neutrino.h>
#include <sys/mman.h>

typedef struct
{
    paddr_t         paddr;
    uintptr_t       vaddr;
    unsigned        num;    // attach count
} sysdmac_ctrl_t;


typedef struct
{
    uint32_t        sar;
    uint32_t        dar;
    uint32_t        tcr;
    uint32_t        reserved;
} sysdmac_desc_t;

typedef struct
{
    uint32_t        chan_idx;
    uint32_t        grp_idx;
    paddr_t         pbase;
    paddr_t         vbase;
    uintptr_t       regs;
    uint32_t        channels;
    uint32_t        channel_groups;

    uint32_t        xfer_count;
    uint32_t        xfer_unit_size;
    int             irq;
    int             iid;

    char            name[16];   // "sys" or "audio"

    dma_mode_flags  mflags;     // mode flag of current transfer
    dma_attach_flags aflags;    // attach flag of client

    // For build in descriptor memory
    int             desc_num;   // number of descriptor sets
    int             desc_idx;   // descriptor index

    // external descriptor memory
    dma_addr_t      desc;
} dma_channel_t;


#endif /* #ifndef __RCAR_SYSDMAC_H__ */

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/lib/dma/rcar/sysdmac/sysdmac.h $ $Rev: 804693 $")
#endif
