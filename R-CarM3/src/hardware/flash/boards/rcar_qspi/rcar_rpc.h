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

#ifndef _RCAR_RPC_H
#define _RCAR_RPC_H

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <errno.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/neutrino.h>
#include <hw/inout.h>
#include <hw/dma.h>
#include <arm/r-car-m3.h>

#include <f3s_spi.h>

#define RCAR_MAX_DEVICE         4

#define RCAR_RPC_WRITE          0
#define RCAR_RPC_READ           1

#define RCAR_RPC_ADDR_MAP       0x08000000

#define RCAR_RPC_EVENT          0x55
#define RCAR_RPC_RDMA_EVENT     0x56
#define RCAR_RPC_TDMA_EVENT     0x57
#define RCAR_RPC_PRIORITY       21

#define RCAR_TxDMA_THRESHOLD    16
#define RCAR_ALIGN_RxBUF(clen)  (4 - (clen & 3))

/* SPI controller specific */
typedef struct {
    unsigned        pbase;
    uintptr_t       vbase;
    int             chid, coid;
    int             irq, iid;
    uint32_t        ctrl;
    uint32_t        clock;
    uint32_t        drate;
    int             dtime;
    struct sigevent spievent;

    dma_functions_t dmafuncs;
    void            *txdma;
    dma_addr_t      dbuf;
    void            *rxdma;
#define RCAR_DMABUF_SIZE        ((8) * 1024)   // DMA buffer size
#define RCAR_TXDMA_OFF          (4 * 1024)     // Tx DMA buffer offset

    pthread_mutex_t mutex;
    int             ndev;
} rcar_rpc_t;

/* device specific structure */
typedef struct {
    spi_flash_t     spic;
    uint32_t        mode;
    uint32_t        drate;
    uint32_t        csel;
    uint32_t        bus;
    uint32_t        flags;
#define RCAR_FLAG_MUTEX         (1 << 0)
    rcar_rpc_t      *rpc;
} rpc_dev_t;

#endif

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/hardware/flash/boards/rcar_qspi/salvatorx/rcar_rpc.h $ $Rev: 811059 $")
#endif
