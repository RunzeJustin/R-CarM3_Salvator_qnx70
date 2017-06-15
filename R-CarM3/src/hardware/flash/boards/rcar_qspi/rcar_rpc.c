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

#include "rcar_rpc.h"

static rcar_rpc_t  rcar_rpc = {0,};

static void rpc_init_ext_mode(rpc_dev_t *dev)
{
    rcar_rpc_t *rpc = dev->rpc;

    out32(rpc->vbase + RCAR_RPC_PHYCNT, 0x80040263);
    out32(rpc->vbase + RCAR_RPC_CMNCR, 0x01FFF301);
    out32(rpc->vbase + RCAR_RPC_DRCR, 0x00000000);
    out32(rpc->vbase + RCAR_RPC_DRCMR, 0x00A00000);
    out32(rpc->vbase + RCAR_RPC_DRENR, 0xA222D400);
    out32(rpc->vbase + RCAR_RPC_DRDMCR, 0x0000000E);
    out32(rpc->vbase + RCAR_RPC_DRDRENR, 0x00005101);
    out32(rpc->vbase + RCAR_RPC_OFFSET1, 0x21511144);
    out32(rpc->vbase + RCAR_RPC_PHYINT, 0x07070002);
}

static void rpc_config(rpc_dev_t *dev)
{
    rcar_rpc_t *rpc = dev->rpc;

    out32(rpc->vbase + RCAR_RPC_PHYCNT, 0x80000263);
    //bit31  CAL         =  1 : PHY calibration
    //bit1-0 PHYMEM[1:0] = 11 : HyperFlash
    out32(rpc->vbase + RCAR_RPC_CMNCR, 0x81FFF301);
    //bit31  MD       =  0 : Manual mode
    //bit1-0 BSZ[1:0] = 01 : QSPI Flash x 2 or HyperFlash
    out32(rpc->vbase + RCAR_RPC_SMOPR, 0x00000000);
    out32(rpc->vbase + RCAR_RPC_SMDRENR, 0x00005111);
    out32(rpc->vbase + RCAR_RPC_SSLDR, 0x00010101);
}

static void rpc_xfer_init(rpc_dev_t *dev, int dummy)
{
    rcar_rpc_t *rpc = dev->rpc;

    // Disable fucntion
    out32(rpc->vbase + RCAR_RPC_SMCR, 0x00000000);
    // configure interface
    rpc_config(dev);
    // data dummy control
    if (dummy != 0)
    {
        out32(rpc->vbase + RCAR_RPC_SMDMCR, dummy - 1);
    }
}

/*
 * setup DMA transfer
 */
static int rpc_setup_dma(rpc_dev_t *dev, int len, int dir, int offset)
{
    rcar_rpc_t     *rpc = dev->rpc;
    dma_transfer_t  tinfo;
    dma_addr_t      saddr, daddr;

    memset(&tinfo, 0, sizeof (tinfo));

    tinfo.xfer_unit_size = 4;
    tinfo.xfer_bytes     = len;

    if (dir == RCAR_RPC_WRITE) {
        saddr.paddr     = rpc->dbuf.paddr + offset;
        tinfo.src_addrs = &saddr;
        daddr.paddr     = RCAR_RPC_BUFFER_BASE;
        tinfo.dst_addrs = &daddr;

        rpc->dmafuncs.setup_xfer(rpc->txdma, &tinfo);
        rpc->dmafuncs.xfer_start(rpc->txdma);
    } else {
        saddr.paddr     = RCAR_RPC_ADDR_MAP + offset;
        tinfo.src_addrs = &saddr;
        daddr.paddr     = rpc->dbuf.paddr;
        tinfo.dst_addrs = &daddr;

        rpc->dmafuncs.setup_xfer(rpc->rxdma, &tinfo);
        rpc->dmafuncs.xfer_start(rpc->rxdma);
    }

    return (len);
}

int rpc_wait(rpc_dev_t *dev, int len, int dir)
{
    rcar_rpc_t     *rpc = dev->rpc;
    struct _pulse   pulse;
    uint64_t        to = len;

    to *= 2048 * 2048;

    while (1) {
       TimerTimeout(CLOCK_REALTIME, _NTO_TIMEOUT_RECEIVE, NULL, &to, NULL);

        if (MsgReceivePulse(rpc->chid, &pulse, sizeof(pulse), NULL) == -1) {
            fprintf(stderr, "RCAR_RPC: XFER error! sts = %#x\n", in32(rpc->vbase + RCAR_RPC_CMNSR));
            return (errno = EIO);
        }

        switch (pulse.code) {
            case RCAR_RPC_RDMA_EVENT:
                if (rpc->dmafuncs.bytes_left(rpc->rxdma) != 0) {
                    fprintf(stderr, "RCAR_RPC: Rx DMA is not completed properly\n");
                    return (EIO);
                }
                rpc->dmafuncs.xfer_complete(rpc->rxdma);
                if (dir == RCAR_RPC_READ)
                    return (EOK);
                break;
            case RCAR_RPC_TDMA_EVENT:
                if (rpc->dmafuncs.bytes_left(rpc->txdma) != 0) {
                    fprintf(stderr, "RCAR_RPC: Tx DMA is not completed properly\n");
                    return (EIO);
                }
                rpc->dmafuncs.xfer_complete(rpc->txdma);
                if (dir == RCAR_RPC_WRITE)
                    return (EOK);
                break;
        }
    }
}

/*
 * DMA transfer
 */
static int rpc_dma_xfer(rpc_dev_t *dev, int len, int dir)
{
    rcar_rpc_t *rpc = dev->rpc;
    int         rc;

    if ((rc = rpc_wait(dev, len, dir)) != EOK)
        rpc->dmafuncs.xfer_abort(rpc->rxdma);

    return (rc);
}


/* SPI flash interface calls */
static void rcar_write_protection_disable(rpc_dev_t *dev)
{
    rcar_rpc_t *rpc = dev->rpc;
    uint32_t dataL = 0;

    dataL = in32(rpc->vbase + RCAR_RPC_PHYINT);

    if(dataL & RCAR_RPC_PHYINT_WPVAL)
    {   //bit1:  WPVAL(0:RPC_WP#=H(Protect Disable), 1:RPC_WP#=L(Protect Enable))
        dataL &= ~RCAR_RPC_PHYINT_WPVAL;
        out32(rpc->vbase + RCAR_RPC_PHYINT, dataL);
    }
}

static void rpc_wait_tx_end(rpc_dev_t *dev)
{
    rcar_rpc_t *rpc = dev->rpc;
    uint32_t status = 0;

    while(1)
    {
        status = in32(rpc->vbase + RCAR_RPC_CMNSR);
        if(status & RCAR_RPC_CMNSR_TEND)
            break;
    }
}

static void rpc_write_4bytes(rpc_dev_t *dev, uint32_t addr, uint32_t data)
{
    rcar_rpc_t *rpc = dev->rpc;

    out32(rpc->vbase + RCAR_RPC_SMCMR, 0x00000000);
    out32(rpc->vbase + RCAR_RPC_SMADR, addr>>1);
    out32(rpc->vbase + RCAR_RPC_SMENR, 0xA222540C);
    out32(rpc->vbase + RCAR_RPC_SMWDR0, data);
    out32(rpc->vbase + RCAR_RPC_SMCR, 0x00000003);
    rpc_wait_tx_end(dev);
}

static uint32_t rpc_read_4bytes_reg(rpc_dev_t *dev, uint32_t addr)
{
    rcar_rpc_t *rpc = dev->rpc;
    uint32_t rddata;

    rpc_xfer_init(dev, 16);
    out32(rpc->vbase + RCAR_RPC_SMCMR, 0x00800000);
    out32(rpc->vbase + RCAR_RPC_SMADR, addr>>1);
    out32(rpc->vbase + RCAR_RPC_SMENR, 0xA222D40C);
    out32(rpc->vbase + RCAR_RPC_SMCR, 0x00000005);
    rpc_wait_tx_end(dev);
    rddata = in32(rpc->vbase + RCAR_RPC_SMRDR0);

    return rddata;
}

static uint32_t rpc_read_4bytes(rpc_dev_t *dev, uint32_t addr)
{
    rcar_rpc_t *rpc = dev->rpc;
    uint32_t rddata;

    out32(rpc->vbase + RCAR_RPC_PHYCNT, 0x80000263);
    out32(rpc->vbase + RCAR_RPC_SMCMR, 0x00800000);
    out32(rpc->vbase + RCAR_RPC_SMADR, addr>>1);
    out32(rpc->vbase + RCAR_RPC_SMENR, 0xA222D40C);
    out32(rpc->vbase + RCAR_RPC_SMCR, 0x00000005);
    rpc_wait_tx_end(dev);
    rddata = in32(rpc->vbase + RCAR_RPC_SMRDR0);

    return rddata;
}

static int rpc_write_cmd(void *hdl, uint32_t addr, uint32_t data)
{
    rpc_dev_t  *dev = hdl;
    rcar_rpc_t *rpc = dev->rpc;

    rpc_xfer_init(dev, 0);
    out32(rpc->vbase + RCAR_RPC_SMCMR, 0x00000000);
    out32(rpc->vbase + RCAR_RPC_SMADR, addr);
    out32(rpc->vbase + RCAR_RPC_SMENR, 0xA2225408);
    out32(rpc->vbase + RCAR_RPC_SMWDR0, data);
    out32(rpc->vbase + RCAR_RPC_SMCR, 0x00000003);
    rpc_wait_tx_end(dev);

    return EOK;
}

static int rpc_read_reg(void *hdl, uint32_t addr, uint8_t *dbuf, int dlen)
{
    rpc_dev_t   *dev = hdl;
    uint32_t rddata;
    uint32_t add = addr;
    int i;

    if (dlen == 1)
    {
        rddata = rpc_read_4bytes_reg(dev, add<<1);
        *dbuf = (rddata >> 8) & 0xFF;
    }
    else
    {
        for (i = 0; i < dlen; i+=2)
        {
            rddata = rpc_read_4bytes_reg(dev, add<<1);
            dbuf[i] = (rddata >> 8) & 0xFF;
            dbuf[i + 1] = (rddata >> 24) & 0xFF;
            add += 0x2;
        }
    }

    return dlen;
}

static void rpc_enable_write(void *hdl)
{
    rpc_write_cmd(hdl, HYPER_UNLOCK1_ADDR, HYPER_UNLOCK1_DATA);
    rpc_write_cmd(hdl, HYPER_UNLOCK2_ADDR, HYPER_UNLOCK2_DATA);
}

static void rpc_read_status(void *hdl, uint16_t *status)
{
    rpc_dev_t  *dev = hdl;
    uint32_t rdstatus;

    rpc_write_cmd(hdl, HYPER_READ_STATUS_ADDR, HYPER_READ_STATUS_DATA);
    rdstatus = rpc_read_4bytes_reg(dev, 0x0);
    *status = ((rdstatus & 0xFF00) >> 8) | ((rdstatus & 0x00FF) << 8);
}

static int rpc_write_buffer(void *hdl, uint32_t addr, uint8_t *data, int dlen)
{
    rpc_dev_t   *dev = hdl;
    rcar_rpc_t *rpc = dev->rpc;
    uint16_t status;
    int rc = -1;

    rpc_enable_write(hdl);
    rpc_write_cmd(hdl, HYPER_ENTRY_ADDR, HYPER_WORD_PROGRAM_DATA);
    out32(rpc->vbase + RCAR_RPC_DRCR, 0x01FF0301);
    out32(rpc->vbase + RCAR_RPC_PHYCNT, 0x80000277);
    out32(rpc->vbase + RCAR_RPC_SMADR, addr>>1);
    out32(rpc->vbase + RCAR_RPC_SMENR, 0xA222540F);

    memcpy((void *)(rpc->dbuf.vaddr + RCAR_TXDMA_OFF), data, dlen);
    rpc_setup_dma(dev, dlen, RCAR_RPC_WRITE, RCAR_TXDMA_OFF);

    if (rpc_dma_xfer(dev, dlen, RCAR_RPC_WRITE) == EOK)
        rc = dlen;

    out32(rpc->vbase + RCAR_RPC_SMCR, 0x00000003);
    rpc_wait_tx_end(dev);
    out32(rpc->vbase + RCAR_RPC_DRCR, 0x01FF0301);
    out32(rpc->vbase + RCAR_RPC_PHYCNT, 0x80000273);

    while (1)
    {
        rpc_read_status(dev, &status);

        if (status & HYPER_DEVICE_READY)
            break;
    }

    return rc;
}

static int rpc_write_word(void *hdl, uint32_t addr, uint8_t *data, int dlen)
{
    rpc_dev_t   *dev = hdl;
    uint32_t    writeData;
    uint32_t    add = addr;
    uint16_t    status;
    int         rc = 0;
    int         i;

    for (i = 0; i < dlen; i+=4)
    {
        rpc_enable_write(hdl);
        rpc_write_cmd(hdl, HYPER_ENTRY_ADDR, HYPER_WORD_PROGRAM_DATA);
        writeData = (data[i] | (data[i + 1]<<8) | (data[i + 2]<<16) | (data[i + 3]<<24));
        rpc_write_4bytes(dev, add, writeData);

        while (1)
        {
            rpc_read_status(dev, &status);
            if (status & HYPER_DEVICE_READY){
                rc = dlen;
                break;
            }
        }

        add += 0x4;
    }

    return rc;
}

static int rpc_write(void *hdl, uint32_t addr, uint8_t *data, int dlen)
{
    rpc_dev_t   *dev = hdl;
    int         rc = 0;
    int         i;
    uint32_t    add = addr;
    int         length;
    uint8_t     buf[256];
    int length1, length2;

    rpc_xfer_init(dev, 0);
   // rcar_write_protection_disable(dev);

    length1 = dlen % 8;
    length2 = dlen - length1;

    if (dlen >= 8)
    {
        for (i = 0; i < length2; i+=256)
        {
            if((length2 - i) >= 256)
                length = 256;
            else
                length = length2 - i;

            memcpy(buf, data + i, length);
            rpc_write_buffer(hdl, add, buf, length);

            if((length2 - i) >= 256)
                add+=256;
            else
                add+= (length2 - i);
        }
        rc = dlen;
    }

    if (length1 != 0){
        memcpy(buf, data + length2, length1);
        rpc_write_word(hdl, add, buf, length1);
        rc = dlen;
    }

    return (rc);
}

static int rpc_read(void *hdl, uint32_t addr, uint8_t *data, int dlen)
{
    rpc_dev_t  *dev = hdl;
    rcar_rpc_t *rpc = dev->rpc;
    uint8_t     *pbuf;
    int         len;
    int         rc = 0;

    /* This calculation is to make 'dlen' divisible by 4 */
    if ((dlen%4) != 0)
    {
        len = dlen + (4 - (dlen%4));
    }
    else
    {
        len = dlen;
    }

    rpc_init_ext_mode(dev);
    rpc_setup_dma(dev, len, RCAR_RPC_READ, addr);

    if (rpc_dma_xfer(dev, len, RCAR_RPC_READ) == EOK)
    {
        pbuf = (uint8_t *)rpc->dbuf.vaddr;
        memcpy(data, pbuf, dlen);
        rc = dlen;
    }

    return rc;
}

static int rpc_dinit(void *hdl)
{
    rpc_dev_t  *dev = hdl;
    rcar_rpc_t *rpc = dev->rpc;

    if (--rpc->ndev)
        goto fini;

    // Disable fucntion
    out32(rpc->vbase + RCAR_RPC_SMCR, 0x00000000);

    ConnectDetach(rpc->coid);
    ChannelDestroy(rpc->chid);
    InterruptDetach(rpc->iid);

    rpc->dmafuncs.free_buffer(rpc->txdma, &rpc->dbuf);
    rpc->dmafuncs.channel_release(rpc->txdma);
    rpc->dmafuncs.channel_release(rpc->rxdma);
    rpc->dmafuncs.fini();

    munmap_device_io(rpc->vbase, RCAR_RPC_SIZE);

    rpc->pbase = 0;

fini:
    free(dev);

    return EOK;
}

static int rcar_dma_init(rcar_rpc_t *rpc)
{
    struct sigevent     event;

    if (get_dmafuncs(&rpc->dmafuncs, sizeof(rpc->dmafuncs)) == -1) {
        fprintf(stderr, "rpc: init_dma: failed to get DMA lib functions\n");
        return (-1);
    }

    rpc->dmafuncs.init(NULL);

    event.sigev_notify   = SIGEV_PULSE;
    event.sigev_coid     = rcar_rpc.coid;
    event.sigev_code     = RCAR_RPC_TDMA_EVENT;
    event.sigev_priority = RCAR_RPC_PRIORITY;

    if ((rpc->txdma = rpc->dmafuncs.channel_attach("dma=sys,ver=m3", &event, NULL, 0,
                                    DMA_ATTACH_PRIORITY_STRICT | DMA_ATTACH_ANY_CHANNEL | DMA_ATTACH_EVENT_ON_COMPLETE)) == NULL) {
        fprintf(stderr, "rpc: Unable to attach to DMA Channel");
        return (-1);
    }

    if (rpc->dmafuncs.alloc_buffer(rpc->txdma, &rpc->dbuf, RCAR_DMABUF_SIZE, DMA_BUF_FLAG_NOCACHE) != 0) {
        fprintf(stderr, "rpc: Unable to allocate DMA buffer");
        return (-1);
    }

    event.sigev_notify   = SIGEV_PULSE;
    event.sigev_coid     = rcar_rpc.coid;
    event.sigev_code     = RCAR_RPC_RDMA_EVENT;
    event.sigev_priority = RCAR_RPC_PRIORITY;

    if ((rpc->rxdma = rpc->dmafuncs.channel_attach(NULL, &event, NULL, 1,
                                    DMA_ATTACH_PRIORITY_STRICT | DMA_ATTACH_ANY_CHANNEL | DMA_ATTACH_EVENT_ON_COMPLETE)) == NULL) {
        fprintf(stderr, "rpc: Unable to attach to DMA Channel");
        return (-1);
    }

    return (EOK);
}

void* rpc_init(void *args)
{
    rpc_dev_t           *dev;
    int                 opt;
    char                *value;
    char                *freeptr;
    char                *options;
    int                 clock = 320000000; // 320MHz
    static char         *opts[] = {"cs", "drate", "mode", "clock", NULL};

    if ((dev = calloc(1, sizeof(rpc_dev_t))) == NULL) {
        fprintf(stderr, "RCAR_RPC: Could not allocate memory\n");
        return (NULL);
    }

    dev->bus   = 1;
    dev->csel  = 0;
    dev->drate = 160000000;

    dev->spic.hcap                      = 0;
    dev->spic.dinit                     = rpc_dinit;
    dev->spic.spi_read_flash_registers  = rpc_read_reg;
    dev->spic.spi_read_flash_memory     = rpc_read;
    dev->spic.spi_write_flash_memory    = rpc_write;
    dev->spic.spi_write_flash_cmd       = rpc_write_cmd;
printf("in ham init \n");
    options = freeptr = strdup(args);
    while (options && *options != '\0') {
        opt = getsubopt(&options, opts, &value);
        switch (opt) {
        case 0:     // chip select
            dev->csel  = strtoul(value, 0, 0);
            break;
        case 1:     // data rate
            dev->drate = strtoul(value, 0, 0);
            break;
        case 2:     // mode
            dev->mode  = strtoul(value, 0, 0);
            break;
        case 3:     // clock
            clock      = strtoul(value, 0, 0);
            break;
        }
    }

    free(freeptr);

    // First time called
    if (rcar_rpc.pbase == 0) {
        if ((rcar_rpc.vbase = mmap_device_io(RCAR_RPC_SIZE, RCAR_RPC_BASE)) == (uintptr_t)MAP_FAILED)
            goto fail0;

        /* attach interrupt */
        if ((rcar_rpc.chid = ChannelCreate(_NTO_CHF_DISCONNECT | _NTO_CHF_UNBLOCK)) == -1)
            goto fail1;

        if ((rcar_rpc.coid = ConnectAttach(0, 0, rcar_rpc.chid, _NTO_SIDE_CHANNEL, 0)) == -1)
            goto fail2;

        if (rcar_dma_init(&rcar_rpc) != EOK)
            goto fail3;

        rcar_rpc.pbase = RCAR_RPC_BASE;
        rcar_rpc.clock = clock;
    }

    rcar_rpc.ndev++;
    dev->rpc = &rcar_rpc;

    return (dev);

fail3:
    ConnectDetach(rcar_rpc.coid);
fail2:
    ChannelDestroy(rcar_rpc.chid);
fail1:
    munmap_device_io(rcar_rpc.vbase, RCAR_RPC_SIZE);
fail0:
    free(dev);

    return (NULL);
}


#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/hardware/flash/boards/rcar_qspi/salvatorx/rcar_rpc.c $ $Rev: 811495 $")
#endif
