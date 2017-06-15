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

#include <malloc.h>
#include "rcar_otg.h"


////////////////////////////////////////////////////////////////////////////////
//                        PRIVATE FUNCTIONS                                   //
////////////////////////////////////////////////////////////////////////////////


#define RCAR_MAX_SAMPLING   10

#ifdef CONFIG_USB_RCAR_TYPE_BULK_PIPES_12
#define RCAR_MAX_NUM_PIPE   16
#define RCAR_MAX_NUM_BULK   10
#define RCAR_MAX_NUM_ISOC   2
#define RCAR_MAX_NUM_INT    3
#else
#define RCAR_MAX_NUM_PIPE   10
#define RCAR_MAX_NUM_BULK   3
#define RCAR_MAX_NUM_ISOC   2
#define RCAR_MAX_NUM_INT    4
#endif

#define RCAR_BASE_PIPENUM_BULK  3
#define RCAR_BASE_PIPENUM_ISOC  1
#define RCAR_BASE_PIPENUM_INT   6

#define RCAR_BASE_BUFNUM    6
#define RCAR_MAX_BUFNUM 0x4F
#define RCAR_MAX_DMA_CHANNELS   2

#define DIV_ROUND_UP(n,d) (((n) + (d) - 1) / (d))

static void out32_rep(uintptr_t addr, const void *buffer, int count)
{
    volatile uint32_t *buf = (volatile uint32_t *)buffer;
    if (count) {
        do {
            out32(addr, (uint32_t)(*buf++));
        } while (--count);
    }
}

static void in32_rep(uintptr_t addr, void *buffer, int count)
{
    uint32_t x;
    volatile uint32_t *buf = buffer;
    if (count) {
        do {
            x = in32(addr);
            *buf++ = x;
        } while (--count);
    }
}

static uint32_t rcar_read(rcar_dctrl_t *rcar, unsigned long offset)
{
    return in16((uintptr_t )(rcar->reg + offset));
}

static void rcar_read_fifo(rcar_dctrl_t *rcar,
                      unsigned long offset,
                      unsigned char *buf,
                      int len)
{
    uintptr_t fifoaddr = (uintptr_t)(rcar->reg + offset);
    unsigned int data = 0;
    int i;

    /* aligned buf case */
    if (len >= 4 && !((unsigned long)buf & 0x03)) {
        in32_rep(fifoaddr, buf, len / 4);
        buf += len & ~0x03;
        len &= 0x03;
    }

    /* unaligned buf case */
    for (i = 0; i < len; i++) {
        if (!(i & 0x03))
            data = in32(fifoaddr);

        buf[i] = (data >> ((i & 0x03) * 8)) & 0xff;
    }
}

static void rcar_write(rcar_dctrl_t *rcar, uint16_t val,
                  unsigned long offset)
{
    out16((uintptr_t)(rcar->reg + offset), val);
}

static void rcar_mdfy(rcar_dctrl_t *rcar,
                 uint16_t val, uint16_t pat, unsigned long offset)
{
    uint16_t tmp;
    tmp = rcar_read(rcar, offset);
    tmp = tmp & (~pat);
    tmp = tmp | val;
    rcar_write(rcar, tmp, offset);
}

/* USBHS-DMAC read/write */
static uint32_t rcar_dma_read(rcar_dctrl_t *rcar,
                unsigned long offset)
{
    return in32((uintptr_t)(rcar->dmac_reg + offset));
}

static void rcar_dma_write(rcar_dctrl_t *rcar, uint32_t val,
                unsigned long offset)
{
    out32((uintptr_t)(rcar->dmac_reg + offset), val);
}

static void rcar_dma_mdfy(rcar_dctrl_t *rcar,
                 uint32_t val, uint32_t pat, unsigned long offset)
{
    uint32_t tmp;
    tmp = rcar_dma_read(rcar, offset);
    tmp = tmp & (~pat);
    tmp = tmp | val;
    rcar_dma_write(rcar, tmp, offset);
}

#define rcar_bclr(rcar, val, offset)    \
            rcar_mdfy(rcar, 0, val, offset)
#define rcar_bset(rcar, val, offset)    \
            rcar_mdfy(rcar, val, 0, offset)

#define rcar_dma_bclr(rcar, val, offset)    \
            rcar_dma_mdfy(rcar, 0, val, offset)
#define rcar_dma_bset(rcar, val, offset)    \
            rcar_dma_mdfy(rcar, val, 0, offset)

static void rcar_write_fifo(rcar_dctrl_t *rcar,
                       rcar_ep_t *ep,
                       unsigned char *buf,
                       int len)
{
    uintptr_t fifoaddr = (uintptr_t)(rcar->reg + ep->fifoaddr);
    int adj = 0;
    int i;

    /* 32-bit access only if buf is 32-bit aligned */
    if (len >= 4 && !((unsigned long)buf & 0x03)) {
        out32_rep(fifoaddr, buf, len / 4);
        buf += len & ~0x03;
        len &= 0x03;
    }
    /* adjust fifo address in the little endian case */
    if (!(rcar_read(rcar, CFIFOSEL) & BIGEND)) {
        adj = 0x03; /* 32-bit wide */
    }
    for (i = 0; i < len; i++)
        out8(fifoaddr + adj - (i & adj), buf[i]);
}

#define enable_irq_ready(rcar, pipenum) \
    enable_pipe_irq(rcar, pipenum, BRDYENB)
#define disable_irq_ready(rcar, pipenum)    \
    disable_pipe_irq(rcar, pipenum, BRDYENB)
#define enable_irq_empty(rcar, pipenum) \
    enable_pipe_irq(rcar, pipenum, BEMPENB)
#define disable_irq_empty(rcar, pipenum)    \
    disable_pipe_irq(rcar, pipenum, BEMPENB)
#define enable_irq_nrdy(rcar, pipenum)  \
    enable_pipe_irq(rcar, pipenum, NRDYENB)
#define disable_irq_nrdy(rcar, pipenum) \
    disable_pipe_irq(rcar, pipenum, NRDYENB)

#define     UGCTRL_OFF      0x0180
#define     UGCTRL2_OFF     0x0184
#define     UGSTS_OFF       0x0190
#define     LPSTS_OFF       0x0102

void rcar_usb_func_start(rcar_dctrl_t *rcar)
{
    uintptr_t hsusb = (uintptr_t)rcar->reg;
    uint32_t status;
    /*
     * TODO: we should give a software reset to the HS-USB module from
     * HS-USB operation point of view.  The module reset, however, blows
     * away UGCTRL2 register content, that means USB2.0 Ch 0/2 selection
     * settings get lost.
     */

    out32(hsusb + UGCTRL2_OFF, in32(hsusb + UGCTRL2_OFF) | (3 << 4));
    RCAR_SLOGF_DBG(rcar, _SLOG_INFO, "%s: UGCTRL 0x%08x UGCTRL2 0x%08x UGSTS 0x%08x",
                    __func__,
                    in32(hsusb + UGCTRL_OFF), in32(hsusb + UGCTRL2_OFF),
                    in32(hsusb + UGSTS_OFF));

    /* PLL reset release */
    out32(hsusb + UGCTRL_OFF, 0); /* clear PLLRESET */

    /* UTMI normal mode */
    out16(hsusb + LPSTS_OFF, 1 << 14); /* SUSPM */

    /* check embedded USB PHY lock status */
    status = in32(hsusb + UGSTS_OFF);
    if (status != 0x3) {
        RCAR_SLOGF_DBG(rcar, _SLOG_INFO, "%s: Embedded USB PHY PLL Lock status 0x%04x",
                    __func__,
                    status);
    /* FIXME: LOCK[1:0] can never be 0x3, why? */
    }

    out32(hsusb + UGCTRL_OFF, 1 << 2); /* CONNECT */

}

void rcar_usb_func_stop(rcar_dctrl_t *rcar)
{
    uintptr_t hsusb = (uintptr_t)rcar->reg ;
    uint32_t tmp = in32(hsusb + UGCTRL2_OFF);
    tmp &= ~(3 << 4);
    out32(hsusb + UGCTRL2_OFF, tmp | (1 << 4));

    out32(hsusb + UGCTRL_OFF, 0); /* PHY receiver halted */
    out32(hsusb + UGCTRL_OFF, 1 << 0); /* PLL reset assert */
}
#define RCAR_PIPE(_ep_name, _maxpacket, _bufnum) \
    { .ep_name = _ep_name, .maxpacket = _maxpacket, .bufnum = _bufnum }

const struct rcar_pipe_config rcar_pipe[RCAR_MAX_NUM_PIPE] = {
    RCAR_PIPE("ep0", 64, 0),
    RCAR_PIPE("ep1-iso", 1024, 0x08),
    RCAR_PIPE("ep2-iso", 1024, 0x28),
    RCAR_PIPE("ep3-bulk", 512, 0x48),
    RCAR_PIPE("ep4-bulk", 512, 0x58),
    RCAR_PIPE("ep5-bulk", 512, 0x68),
    RCAR_PIPE("ep6-int", 64, 0x04),
    RCAR_PIPE("ep7-int", 64, 0x05),
    RCAR_PIPE("ep8-int", 64, 0x06),
#ifdef CONFIG_USB_RCAR_TYPE_BULK_PIPES_12
    RCAR_PIPE("ep9-bulk", 512, 0x78),
    RCAR_PIPE("ep10-bulk", 512, 0x88),
    RCAR_PIPE("ep11-bulk", 512, 0x98),
    RCAR_PIPE("ep12-bulk", 512, 0xa8),
    RCAR_PIPE("ep13-bulk", 512, 0xb8),
    RCAR_PIPE("ep14-bulk", 512, 0xc8),
    RCAR_PIPE("ep15-bulk", 512, 0xd8),
#else
    RCAR_PIPE("ep9-int", 64, 0x07),
#endif
};

static void disable_controller(rcar_dctrl_t *rcar);
static void irq_ep0_write(rcar_ep_t *ep);
static void irq_packet_write(rcar_ep_t *ep);

static void transfer_complete(rcar_ep_t *ep, int status);

static uint16_t control_reg_get(rcar_dctrl_t *rcar, uint16_t pipenum);

/*-------------------------------------------------------------------------*/
static uint16_t get_usb_speed(rcar_dctrl_t *rcar)
{
    return rcar_read(rcar, DVSTCTR0) & RHST;
}

static void enable_pipe_irq(rcar_dctrl_t *rcar, uint16_t pipenum,
        unsigned long reg)
{
    uint16_t tmp;

    tmp = rcar_read(rcar, INTENB0);
    rcar_bclr(rcar, BEMPE | NRDYE | BRDYE,
            INTENB0);
    rcar_bset(rcar, (1 << pipenum), reg);
    rcar_write(rcar, tmp, INTENB0);
}

static void disable_pipe_irq(rcar_dctrl_t *rcar, uint16_t pipenum,
        unsigned long reg)
{
    uint16_t tmp;

    tmp = rcar_read(rcar, INTENB0);
    rcar_bclr(rcar, BEMPE | NRDYE | BRDYE,
            INTENB0);
    rcar_bclr(rcar, (1 << pipenum), reg);
    rcar_write(rcar, tmp, INTENB0);
}

static void rcar_dma_reset(rcar_dctrl_t *rcar)
{
    rcar_dma_bclr(rcar, IE | SP | DE | TE, USBHS_DMAC_CHCR(0));
    rcar_dma_bclr(rcar, IE | SP | DE | TE, USBHS_DMAC_CHCR(1));
    rcar_dma_bclr(rcar, DME, DMAOR);
    rcar_bset(rcar, BCLR, D0FIFOCTR);
    rcar_bset(rcar, BCLR, D1FIFOCTR);
    rcar_dma_bset(rcar, SWR_RST, SWR);
    usleep(100);
    rcar_dma_bclr(rcar, SWR_RST, SWR);
}

static void rcar_set_pullup(rcar_dctrl_t *rcar)
{
    rcar_bset(rcar, DPRPU, SYSCFG0);
}

static void rcar_usb_connect(rcar_dctrl_t *rcar)
{
    rcar_bset(rcar, CTRE, INTENB0);
    rcar_bset(rcar, BEMPE | BRDYE, INTENB0);
    rcar_bset(rcar, RESM | DVSE, INTENB0);
    rcar_bset(rcar, VBSE, INTENB0);

    rcar_set_pullup(rcar);
    rcar_dma_reset(rcar);
}

static void rcar_usb_disconnect(rcar_dctrl_t *rcar)
{
    rcar_bclr(rcar, CTRE, INTENB0);
    rcar_bclr(rcar, BEMPE | BRDYE, INTENB0);
    rcar_bclr(rcar, RESM, INTENB0);
    rcar_bclr(rcar, DPRPU, SYSCFG0);

    rcar_dma_reset(rcar);

    disable_controller(rcar);

    rcar->flags |= DC_FLAG_UNKNOW_SPEED;
    rcar->flags &= ~DC_FLAG_CONNECTED;
}

static void control_reg_set_pid(rcar_dctrl_t *rcar, uint16_t pipenum,
        uint16_t pid)
{
    unsigned long offset;
    if (pipenum == 0) {
        rcar_mdfy(rcar, pid, PID, DCPCTR);
    } else if (pipenum < RCAR_MAX_NUM_PIPE) {
        offset = get_pipectr_addr(pipenum);
        rcar_mdfy(rcar, pid, PID, offset);
    } else {
        printf("unexpect pipe num (%d)\n", pipenum);
    }
}

static void rcar_wait_pbusy(rcar_dctrl_t *rcar, uint16_t pipenum)
{
    uint16_t tmp;
    int i = 0;
    do {
        tmp = control_reg_get(rcar, pipenum);
        if (i++ > 1000000) {    /* 1 msec */
            printf("%s: pipenum = %d, timeout \n",
                __func__, pipenum);
            break;
        }
        nanospin_ns(1000);
        //usleep (1);
    } while ((tmp & PBUSY) != 0);
}

void pipe_start(rcar_dctrl_t *rcar, uint16_t pipenum)
{
    control_reg_set_pid(rcar, pipenum, PID_BUF);
}

void pipe_stop(rcar_dctrl_t *rcar, uint16_t pipenum)
{
    control_reg_set_pid(rcar, pipenum, PID_NAK);
    rcar_wait_pbusy(rcar, pipenum);
}

void pipe_stall(rcar_dctrl_t *rcar, uint16_t pipenum)
{
    control_reg_set_pid(rcar, pipenum, PID_STALL);
}

static uint16_t control_reg_get(rcar_dctrl_t *rcar, uint16_t pipenum)
{
    uint16_t ret = 0;
    unsigned long offset;

    if (pipenum == 0) {
        ret = rcar_read(rcar, DCPCTR);
    } else if (pipenum < RCAR_MAX_NUM_PIPE) {
        offset = get_pipectr_addr(pipenum);
        ret = rcar_read(rcar, offset);
    } else {
        printf("unexpect pipe num (%d)\n",
            pipenum);
    }

    return ret;
}

void control_reg_sqclr(rcar_dctrl_t *rcar, uint16_t pipenum)
{
    unsigned long offset;

    pipe_stop(rcar, pipenum);

    if (pipenum == 0) {
        rcar_bset(rcar, SQCLR, DCPCTR);
    } else if (pipenum < RCAR_MAX_NUM_PIPE) {
        offset = get_pipectr_addr(pipenum);
        rcar_bset(rcar, SQCLR, offset);
    } else {
        printf("unexpect pipe num (%d)\n",
            pipenum);
    }
}

static void control_reg_sqset(rcar_dctrl_t *rcar, uint16_t pipenum)
{
    unsigned long offset;

    pipe_stop(rcar, pipenum);

    if (pipenum == 0) {
        rcar_bset(rcar, SQSET, DCPCTR);
    } else if (pipenum < RCAR_MAX_NUM_PIPE) {
        offset = get_pipectr_addr(pipenum);
        rcar_bset(rcar, SQSET, offset);
    } else {
        printf("unexpect pipe num(%d)\n", pipenum);
    }
}

static uint16_t control_reg_sqmon(rcar_dctrl_t *rcar, uint16_t pipenum)
{
    unsigned long offset;

    if (pipenum == 0) {
        return rcar_read(rcar, DCPCTR) & SQMON;
    } else if (pipenum < RCAR_MAX_NUM_PIPE) {
        offset = get_pipectr_addr(pipenum);
        return rcar_read(rcar, offset) & SQMON;
    } else {
        printf(
            "unexpect pipe num(%d)\n", pipenum);
    }

    return 0;
}

static uint16_t save_usb_toggle(rcar_dctrl_t *rcar, uint16_t pipenum)
{
    return control_reg_sqmon(rcar, pipenum);
}

static void restore_usb_toggle(rcar_dctrl_t *rcar, uint16_t pipenum,
                   uint16_t toggle)
{
    if (toggle)
        control_reg_sqset(rcar, pipenum);
    else
        control_reg_sqclr(rcar, pipenum);
}

static int get_buffer_size(rcar_dctrl_t *rcar, uint16_t pipenum)
{
    uint16_t tmp;
    int size;

    if (pipenum == 0) {
        tmp = rcar_read(rcar, DCPCFG);
        if ((tmp & RCAR_CNTMD) != 0)
            size = 256;
        else {
            tmp = rcar_read(rcar, DCPMAXP);
            size = tmp & MAXP;
        }
    } else {
        rcar_write(rcar, pipenum, PIPESEL);
        tmp = rcar_read(rcar, PIPECFG);
        if ((tmp & RCAR_CNTMD) != 0) {
            tmp = rcar_read(rcar, PIPEBUF);
            size = ((tmp >> 10) + 1) * 64;
        } else {
            tmp = rcar_read(rcar, PIPEMAXP);
            size = tmp & MXPS;
        }
    }

    return size;
}

static void rcar_change_curpipe(rcar_dctrl_t *rcar, uint16_t pipenum,
                    uint16_t isel, uint16_t fifosel)
{
    uint16_t tmp, mask, loop;
    int i = 0;

    if (!pipenum) {
        mask = ISEL | CURPIPE;
        loop = isel;
    } else {
        mask = CURPIPE;
        loop = pipenum;
    }
    rcar_mdfy(rcar, loop, mask, fifosel);
    do {
        tmp = rcar_read(rcar, fifosel);
        if (i++ > 1000000) {
            printf("ERROR: rcar: register%x, loop %x is timeout\n",
                    fifosel, loop);
            break;
        }
        nanospin_ns(1000);
        //usleep(1);
    } while ((tmp & mask) != loop);
}

static void pipe_change(rcar_dctrl_t *rcar, uint16_t pipenum)
{
    rcar_ep_t *ep = rcar->pipenum2ep[pipenum];

    if (ep->use_dma)
        rcar_bclr(rcar, DREQE, ep->fifosel);

    rcar_change_curpipe(rcar, pipenum, 0, ep->fifosel);

    rcar_bset(rcar, MBW_32, ep->fifosel);

    if (ep->use_dma)
        rcar_bset(rcar, DREQE, ep->fifosel);
}

static int pipe_buffer_setting(rcar_ep_t *ep, rcar_pipe_info_t *info)
{
    rcar_dctrl_t *rcar = ep->dc;
    uint16_t bufnum = 0, buf_bsize = 0;
    uint16_t pipecfg = 0;

    if ((info->epnum <= 0) || (info->epnum >= RCAR_MAX_NUM_PIPE))
        return -EINVAL;

    rcar_write(rcar, info->pipe, PIPESEL);

    if (info->dir_in)
        pipecfg |= RCAR_DIR;

    pipecfg |= info->type;
    pipecfg |= info->epnum;
    switch (info->type) {
    case RCAR_INT:
        buf_bsize = 0;
        break;
    case RCAR_BULK:
        buf_bsize = 7;
        pipecfg |= RCAR_DBLB;
        if (!info->dir_in)
            pipecfg |= (RCAR_SHTNAK | RCAR_BFRE);
        break;
    case RCAR_ISO:
        buf_bsize = 15;
        break;
    }

    bufnum = rcar_pipe[info->pipe].bufnum;
    if (buf_bsize && ((bufnum + 16) >= 0xFF)) {
        printf("rcar pipe memory is insufficient (%d,0x%x,0x%x)\n",
            info->pipe, bufnum, 0xFF);
        return -ENOMEM;
    }

    rcar_write(rcar, pipecfg, PIPECFG);
    rcar_write(rcar, (buf_bsize << 10) | (bufnum), PIPEBUF);
    rcar_write(rcar, info->maxpacket, PIPEMAXP);
    if (info->interval)
        info->interval--;
    rcar_write(rcar, info->interval, PIPEPERI);

    return 0;
}

static void pipe_initialize(rcar_ep_t *ep)
{
    rcar_dctrl_t *rcar = ep->dc;

    rcar_change_curpipe(rcar, 0, 0, ep->fifosel);

    rcar_write(rcar, ACLRM, ep->pipectr);
    rcar_write(rcar, 0, ep->pipectr);
    rcar_write(rcar, SQCLR, ep->pipectr);
    if (ep->use_dma) {
        rcar_change_curpipe(rcar, ep->pipenum, 0, ep->fifosel);
        rcar_bset(rcar, MBW_32, ep->fifosel);
    }
}

static void rcar_ep_setting(rcar_ep_t *ep, int dma)
{
    rcar_dctrl_t *rcar = ep->dc;
    ep->use_dma = 0;
    ep->fifoaddr = CFIFO;
    ep->fifosel = CFIFOSEL;
    ep->fifoctr = CFIFOCTR;

    ep->pipectr = get_pipectr_addr(ep->pipenum);
    if ((ep->type == USB_ATTRIB_BULK) || (ep->type == USB_ATTRIB_ISOCHRONOUS)) {
        ep->pipetre = get_pipetre_addr(ep->pipenum);
        ep->pipetrn = get_pipetrn_addr(ep->pipenum);
    } else {
        ep->pipetre = 0;
        ep->pipetrn = 0;
    }
    rcar->pipenum2ep[ep->pipenum] = ep;
    rcar->epaddr2ep[ep->pipenum] = ep;
}

static void rcar_ep_release(rcar_ep_t *ep)
{
    /* reset .maxpacket in preparation for next ep_matches */
    ep->mps = rcar_pipe[ep->pipenum].maxpacket;

    if (ep->pipenum == 0)
        return;
    ep->pipenum = 0;
    ep->busy = 0;
}

static int alloc_pipe_config(rcar_ep_t *ep)
{
    rcar_pipe_info_t info;
	rcar_dctrl_t *rcar = ep->dc;
    int dma = 0;
    int ret = 0;

    info.pipe = ep->pipenum;

    rcar_pthread_mutex_lock( &rcar->usb_mutex );
    switch (ep->type) {
    case USB_ATTRIB_BULK:
        info.type = RCAR_BULK;
        dma = 1;
        break;
    case USB_ATTRIB_INTERRUPT:
        info.type = RCAR_INT;
        break;
    case USB_ATTRIB_ISOCHRONOUS:
        info.type = RCAR_ISO;
        break;
    default:
        printf("unexpect xfer type\n");
        ret = -EINVAL;
        goto out;
    }
    //ep->type = info.type;

    info.epnum = ep->pipenum;
    info.maxpacket = ep->mps;
    info.interval = ep->intval;
    info.dir_in = ep->dir ? 1: 0;

    ret = pipe_buffer_setting(ep, &info);
    if (ret < 0) {
        printf("pipe_buffer_setting fail\n");
        goto out;
    }

    rcar_ep_setting(ep, dma);
    pipe_initialize(ep);

out:
	rcar_pthread_mutex_unlock( &rcar->usb_mutex );
    return ret;
}

static int free_pipe_config(rcar_ep_t *ep)
{
    rcar_ep_release(ep);

    return 0;
}

/*-------------------------------------------------------------------------*/
static void pipe_irq_enable(rcar_dctrl_t *rcar, uint16_t pipenum)
{
    enable_irq_ready(rcar, pipenum);
    enable_irq_nrdy(rcar, pipenum);
}

static void pipe_irq_disable(rcar_dctrl_t *rcar, uint16_t pipenum)
{
    disable_irq_ready(rcar, pipenum);
    disable_irq_nrdy(rcar, pipenum);
}

/* if complete is true, gadget driver complete function is not call */
void control_end(rcar_dctrl_t *rcar, unsigned ccpl)
{
    rcar->ep_arr[0].internal_ccpl = ccpl;
    pipe_start(rcar, 0);
    rcar_bset(rcar, CCPL, DCPCTR);
}

static void start_ep0_write(rcar_ep_t *ep)
{
    rcar_dctrl_t *dc = ep->dc;
    rcar_change_curpipe(dc, 0, ISEL, CFIFOSEL);
    rcar_write(dc, BCLR, ep->fifoctr);
    if (ep->req_xfer_len == 0) {
        rcar_bset(dc, BVAL, ep->fifoctr);
        pipe_start(dc, 0);
        transfer_complete(ep, 0);
    } else {
        rcar_write(dc, ~BEMP0, BEMPSTS);
        irq_ep0_write(ep);
    }
}

static void disable_fifosel(rcar_dctrl_t *rcar, uint16_t pipenum,
                uint16_t fifosel)
{
    uint16_t tmp;

    tmp = rcar_read(rcar, fifosel) & CURPIPE;
    if (tmp == pipenum)
        rcar_change_curpipe(rcar, 0, 0, fifosel);
}

static void change_bfre_mode(rcar_dctrl_t *rcar, uint16_t pipenum,
                 int enable)
{
    rcar_ep_t *ep = rcar->pipenum2ep[pipenum];
    uint16_t tmp, toggle;

    /* check current BFRE bit */
    rcar_write(rcar, pipenum, PIPESEL);
    tmp = rcar_read(rcar, PIPECFG) & RCAR_BFRE;
    if ((enable && tmp) || (!enable && !tmp))
        return;

    /* change BFRE bit */
    pipe_stop(rcar, pipenum);
    disable_fifosel(rcar, pipenum, CFIFOSEL);
    disable_fifosel(rcar, pipenum, D0FIFOSEL);
    disable_fifosel(rcar, pipenum, D1FIFOSEL);

    toggle = save_usb_toggle(rcar, pipenum);

    rcar_write(rcar, pipenum, PIPESEL);
    if (enable)
        rcar_bset(rcar, RCAR_BFRE, PIPECFG);
    else
        rcar_bclr(rcar, RCAR_BFRE, PIPECFG);

    /* initialize for internal BFRE flag */
    rcar_bset(rcar, ACLRM, ep->pipectr);
    rcar_bclr(rcar, ACLRM, ep->pipectr);

    restore_usb_toggle(rcar, pipenum, toggle);
}

static int usb_dma_check_alignment(void *buf, int size)
{
    return !((unsigned long)buf & (size - 1));
}

static int dmac_alloc_channel(rcar_dctrl_t *rcar,
                rcar_ep_t *ep)
{
    rcar_dma_t *dma;
    int ch;

    //return -1;

    if (!rcar->dmac)
        return -ENODEV;
    /* Check transfer length */
    if (!ep->req_xfer_len)
        return -EINVAL;
    /* Check transfer type */
    if (ep->type != USB_ATTRIB_BULK)
        return -EIO;
    /* Check buffer alignment */
    if (!usb_dma_check_alignment(ep->xfer_padd_buffer, 8))
        return -EINVAL;
    /* Find available DMA channels */
    if (ep->iousb_ep->edesc.bEndpointAddress & USB_ENDPOINT_IN)
        ch = USBHS_DMAC_IN_CHANNEL;
    else
        ch = USBHS_DMAC_OUT_CHANNEL;


    if (rcar->dma[ch].used)
        return -EBUSY;
    dma = &rcar->dma[ch];
    /* set USBHS-DMAC parameters */
    dma->channel = ch;
    dma->ep = ep;
    dma->used = 1;
    if (usb_dma_check_alignment(ep->xfer_padd_buffer, 32)) {
        dma->tx_size = 32;
        dma->chcr_ts = TS_32;
    } else if (usb_dma_check_alignment(ep->xfer_padd_buffer, 16)) {
        dma->tx_size = 16;
        dma->chcr_ts = TS_16;
    } else {
        dma->tx_size = 8;
        dma->chcr_ts = TS_8;
    }

    if (ep->iousb_ep->edesc.bEndpointAddress & USB_ENDPOINT_IN) {
        dma->dir = 1;
        dma->expect_dmicr = USBHS_DMAC_DMICR_TE(ch);
    } else {
        dma->dir = 0;
        dma->expect_dmicr = USBHS_DMAC_DMICR_TE(ch) |
                    USBHS_DMAC_DMICR_SP(ch) |
                    USBHS_DMAC_DMICR_NULL(ch);
        change_bfre_mode(rcar, ep->pipenum, 1);
    }

    /* set rcar_ep paramters */
    ep->use_dma = 1;
    ep->dma = dma;
    if (dma->channel == 0) {
        ep->fifoaddr = D0FIFO;
        ep->fifosel = D0FIFOSEL;
        ep->fifoctr = D0FIFOCTR;
    } else {
        ep->fifoaddr = D1FIFO;
        ep->fifosel = D1FIFOSEL;
        ep->fifoctr = D1FIFOCTR;
    }
    /* dma mapping */
    /* Initialize pipe, if needed */
    if (!dma->initialized) {
        pipe_initialize(ep);
        dma->initialized = 1;
    }

    return EOK;
}

static void dmac_free_channel(rcar_dctrl_t *rcar,
                            rcar_ep_t *ep)
{
    if (!rcar->dmac)
        return;

    rcar_bclr(rcar, DREQE, ep->fifosel);
    rcar_change_curpipe(rcar, 0, 0, ep->fifosel);

    ep->dma->used = 0;
    ep->use_dma = 0;
    ep->fifoaddr = CFIFO;
    ep->fifosel = CFIFOSEL;
    ep->fifoctr = CFIFOCTR;
}

static void dmac_start(rcar_dctrl_t *rcar, rcar_ep_t *ep)
{
    int ch = ep->dma->channel;

    if (ep->req_xfer_len == 0)
        return;
    rcar_dma_bclr(rcar, DE, USBHS_DMAC_CHCR(ch));

    /* Warning: In case of 64-bit QNX kernel, xfer_padd_buffer can contain a 64-bit value, whereas SAR & DAR
     * are 32-bit registers. But the value of xfer_padd_buffer is got from usb stack, the stack should always
     * give a below4G pointer, otherwise the below writes to SAR & DAR will not work as expected.
     */
    rcar_dma_write(rcar, (uintptr_t)ep->xfer_padd_buffer, USBHS_DMAC_SAR(ch));
    rcar_dma_write(rcar, (uintptr_t)ep->xfer_padd_buffer, USBHS_DMAC_DAR(ch));

    rcar_dma_write(rcar,
            DIV_ROUND_UP(ep->req_xfer_len, ep->dma->tx_size),
            USBHS_DMAC_TCR(ch));
    rcar_dma_write(rcar, 0, USBHS_DMAC_CHCR(ch));
    rcar_dma_write(rcar, 0x0027AC40, USBHS_DMAC_TOCSTR(ch));

    if (ep->dma->dir) {
        if ((ep->req_xfer_len % ep->dma->tx_size) == 0)
            rcar_dma_write(rcar, 0xFFFFFFFF,
                        USBHS_DMAC_TEND(ch));
        else
            rcar_dma_write(rcar,
                    ~(0xFFFFFFFF >>
                    (ep->req_xfer_len &
                     (ep->dma->tx_size - 1))),
                    USBHS_DMAC_TEND(ch));
    } else {
        rcar_dma_write(rcar, 0,  USBHS_DMAC_TEND(ch));
    }

    rcar_dma_bset(rcar, DME, DMAOR);

    if (!ep->dma->dir)
        rcar_dma_bset(rcar, NULLE, USBHS_DMAC_CHCR(ch));

    rcar_dma_bset(rcar, IE | ep->dma->chcr_ts, USBHS_DMAC_CHCR(ch));
    rcar_dma_bset(rcar, DE, USBHS_DMAC_CHCR(ch));
}

static void dmac_cancel(rcar_ep_t *ep)
{
    rcar_dctrl_t *rcar = ep->dc;
    uint32_t chcr0, chcr1;

    if (!ep->use_dma)
        return;

    rcar_dma_bclr(rcar, DE | IE, USBHS_DMAC_CHCR(ep->dma->channel));
    if (!ep->dma->dir)
        rcar_bset(rcar, BCLR, ep->fifoctr);

    chcr0 = rcar_dma_read(rcar, USBHS_DMAC_CHCR(0));
    chcr1 = rcar_dma_read(rcar, USBHS_DMAC_CHCR(1));
    if (!(chcr0 & DE) && !(chcr1 & DE))
        rcar_dma_reset(rcar);
}

static void start_packet_write(rcar_ep_t *ep)
{
    rcar_dctrl_t *rcar = ep->dc;
    uint16_t tmp;

    if (!ep->xfer_buffer)
        printf("%s: buffer pointer is NULL\n", __func__);

    rcar_write(rcar, ~(1 << ep->pipenum), BRDYSTS);
    if (dmac_alloc_channel(rcar, ep) < 0) {
        /* PIO mode */
        pipe_change(rcar, ep->pipenum);
        disable_irq_empty(rcar, ep->pipenum);
        pipe_start(rcar, ep->pipenum);
        tmp = rcar_read(rcar, ep->fifoctr);
        if ((tmp & FRDY) == 0)
            pipe_irq_enable(rcar, ep->pipenum);
        else
            irq_packet_write(ep);
    } else {
        /* DMA mode */
        pipe_change(rcar, ep->pipenum);
        disable_irq_nrdy(rcar, ep->pipenum);
        pipe_start(rcar, ep->pipenum);
        enable_irq_nrdy(rcar, ep->pipenum);
        dmac_start(rcar, ep);
    }
}

static void start_packet_read(rcar_ep_t *ep)
{
    rcar_dctrl_t *rcar = ep->dc;
    uint16_t pipenum = ep->pipenum;
    if (ep->pipenum == 0) {
        rcar_change_curpipe(rcar, 0, 0, CFIFOSEL);
        rcar_write(rcar, BCLR, ep->fifoctr);
        pipe_start(rcar, pipenum);
        pipe_irq_enable(rcar, pipenum);
    } else {
        pipe_stop(rcar, pipenum);
        if (ep->pipetre) {
            enable_irq_nrdy(rcar, pipenum);
            rcar_write(rcar, TRCLR, ep->pipetre);
            rcar_write(rcar,
                DIV_ROUND_UP(ep->req_xfer_len, ep->mps),
                ep->pipetrn);
            rcar_bset(rcar, TRENB, ep->pipetre);
        }

        rcar_write(rcar, ~(1 << pipenum), BRDYSTS);
        if (dmac_alloc_channel(rcar, ep) < 0) {
            /* PIO mode */
            change_bfre_mode(rcar, ep->pipenum, 0);
            pipe_start(rcar, pipenum);  /* trigger once */
            pipe_irq_enable(rcar, pipenum);
        } else {
            pipe_change(rcar, pipenum);
            dmac_start(rcar, ep);
            pipe_start(rcar, pipenum);  /* trigger once */
        }
    }
}

void start_packet(rcar_ep_t *ep)
{
    if (ep->dir & USB_ENDPOINT_IN)
        start_packet_write(ep);
    else
        start_packet_read(ep);
}

void start_ep0(rcar_ep_t *ep, int idx)
{
    uint16_t ctsq;
    ctsq = rcar_read(ep->dc, INTSTS0) & CTSQ;
    switch (ctsq) {
    case CS_RDDS:
        start_ep0_write(ep);
        break;
    case CS_WRDS:
        start_packet_read(ep);
        break;

    case CS_WRND:
        control_end(ep->dc, 0);
        break;
    default:
        rcar_complete_urb(ep->dc, ep, 0);
        break;
    }
}

static void init_controller(rcar_dctrl_t *rcar)
{
    uint16_t bwait = rcar->bwait ? : 0xf;

    rcar_write(rcar, bwait, SYSCFG1);
    rcar_bset(rcar, HSE, SYSCFG0);

    rcar_bclr(rcar, USBE, SYSCFG0);
    rcar_bclr(rcar, DPRPU, SYSCFG0);
    rcar_bset(rcar, USBE, SYSCFG0);

    rcar_bset(rcar, SCKE, SYSCFG0);

    rcar_bset(rcar, 0, INTENB1);
    rcar_write(rcar, BURST | CPU_ADR_RD_WR, DMA0CFG);
}

static void disable_controller(rcar_dctrl_t *rcar)
{
    rcar_bset(rcar, SCKE, SYSCFG0);
    rcar_bclr(rcar, UTST, TESTMODE);

    /* disable interrupts */
    rcar_write(rcar, 0, INTENB0);
    rcar_write(rcar, 0, INTENB1);
    rcar_write(rcar, 0, BRDYENB);
    rcar_write(rcar, 0, BEMPENB);
    rcar_write(rcar, 0, NRDYENB);

    /* clear status */
    rcar_write(rcar, 0, BRDYSTS);
    rcar_write(rcar, 0, NRDYSTS);
    rcar_write(rcar, 0, BEMPSTS);

    rcar_bclr(rcar, USBE, SYSCFG0);
    rcar_bclr(rcar, SCKE, SYSCFG0);
}

static void rcar_start_xclock(rcar_dctrl_t *rcar)
{
    uint16_t tmp;

    tmp = rcar_read(rcar, SYSCFG0);
    if (!(tmp & XCKE))
        rcar_bset(rcar, XCKE, SYSCFG0);
}

/*-------------------------------------------------------------------------*/
static void transfer_complete(rcar_ep_t *ep, int status)
{
    if (ep->pipenum == 0) {
        if (ep->internal_ccpl) {
            ep->internal_ccpl = 0;
            return;
        }
    }
    if (ep->use_dma)
        dmac_free_channel(ep->dc, ep);
    rcar_complete_urb(ep->dc, ep, status);
}

static void irq_ep0_write(rcar_ep_t *ep)
{
    int i;
    uint16_t tmp;
    unsigned bufsize;
    size_t size;
    void *buf;
    uint16_t pipenum = ep->pipenum;
    rcar_dctrl_t *rcar = ep->dc;

    pipe_change(rcar, pipenum);
    rcar_bset(rcar, ISEL, ep->fifosel);
    i = 0;
    do {
        tmp = rcar_read(rcar, ep->fifoctr);
        if (i++ > 100000) {
            printf("pipe0 is busy. maybe cpu i/o bus "
                "conflict. please power off this controller.\n");
            return;
        }
        //nanospin_ns(1);
        usleep (1);
    } while ((tmp & FRDY) == 0);

    /* prepare parameters */
    bufsize = get_buffer_size(rcar, pipenum);
    buf = ep->xfer_buffer + ep->bytes_xfered;
    size = min(bufsize, ep->req_xfer_len - ep->bytes_xfered);
    /* write fifo */
    if (ep->xfer_buffer) {
        if (size > 0)
        {
            rcar_write_fifo(rcar, ep, buf, size);
        }
        if ((size == 0) || ((size % ep->mps) != 0))
        {
            rcar_bset(rcar, BVAL, ep->fifoctr);
        }
    }
    /* update parameters */
    ep->bytes_xfered += size;

    /* check transfer finish */
    if ((ep->bytes_xfered == ep->req_xfer_len)
            || (size % ep->mps)
            || (size == 0)) {
        disable_irq_ready(rcar, pipenum);
        disable_irq_empty(rcar, pipenum);
    } else {
        disable_irq_ready(rcar, pipenum);
        enable_irq_empty(rcar, pipenum);
    }
    pipe_start(rcar, pipenum);
}

static void irq_packet_write(rcar_ep_t *ep)
{
    uint16_t tmp;
    unsigned bufsize;
    size_t size;
    void *buf;
    uint16_t pipenum = ep->pipenum;
    rcar_dctrl_t *rcar = ep->dc;

    if (ep->req_xfer_len == 0) {
        rcar_write(rcar, BCLR, ep->fifoctr);
        pipe_change(rcar, ep->pipenum);
        rcar_bset(rcar, BVAL, ep->fifoctr);
        pipe_start(rcar, ep->pipenum);
        transfer_complete(ep, 0);
        return;
    }

    pipe_change(rcar, pipenum);
    tmp = rcar_read(rcar, ep->fifoctr);
    if ((tmp & FRDY) == 0) {
        pipe_stop(rcar, pipenum);
        pipe_irq_disable(rcar, pipenum);
        printf("write fifo not ready. pipenum=%d\n", pipenum);
        return;
    }

    /* prepare parameters */
    bufsize = get_buffer_size(rcar, pipenum);
    buf = ep->xfer_buffer + ep->bytes_xfered;
    size = min(bufsize, ep->req_xfer_len - ep->bytes_xfered);

    /* write fifo */
    if (ep->xfer_buffer) {
        rcar_write(rcar, ~(1 << pipenum), BEMPSTS);
        rcar_write_fifo(rcar, ep, buf, size);
        if ((size == 0)
                || ((size % ep->mps) != 0)
                || ((bufsize != ep->mps)
                    && (bufsize > size)))
        {
            rcar_bset(rcar, BVAL, ep->fifoctr);
        }
    }

    /* update parameters */
    ep->bytes_xfered += size;
    /* check transfer finish */
    if ((ep->bytes_xfered == ep->req_xfer_len)
            || (size % ep->mps)
            || (size == 0)) {
        disable_irq_ready(rcar, pipenum);
        enable_irq_empty(rcar, pipenum);
    } else {
        disable_irq_empty(rcar, pipenum);
        pipe_irq_enable(rcar, pipenum);
    }
}

static void irq_packet_read(rcar_ep_t *ep)
{
    uint16_t tmp;
    int rcv_len, bufsize, req_len;
    int size;
    void *buf;
    uint16_t pipenum = ep->pipenum;
    rcar_dctrl_t *dc = ep->dc;
    iousb_transfer_t        *urb = ep->urb;
    int finish = 0;
    pipe_change(dc, pipenum);
    tmp = rcar_read(dc, ep->fifoctr);
    if (!(tmp & FRDY)) {
        urb->status = -EPIPE;
        pipe_stop(dc, pipenum);
        pipe_irq_disable(dc, pipenum);
        printf("read fifo not ready (%d)\n", pipenum);
        return;
    }
    /* prepare parameters */
    rcv_len = tmp & DTLN;
    bufsize = ep->req_xfer_len;

    buf = ep->xfer_buffer + ep->bytes_xfered;
    req_len = ep->req_xfer_len - ep->bytes_xfered;
    if (rcv_len < bufsize)
        size = min(rcv_len, req_len);
    else
        size = min(bufsize, req_len);

    /* update parameters */
    ep->bytes_xfered += size;

    /* check transfer finish */
    if ((ep->bytes_xfered == ep->req_xfer_len)
            || (size % ep->mps)
            || (size == 0)) {
        pipe_stop(dc, pipenum);
        pipe_irq_disable(dc, pipenum);
        finish = 1;
    }
    /* read fifo */
    if (ep->xfer_buffer) {
        if (size == 0)
            rcar_write(dc, BCLR, ep->fifoctr);
        else {
            rcar_read_fifo(dc, ep->fifoaddr, buf, size);
        }
    }

    if ((ep->pipenum != 0) && finish) {
        transfer_complete(ep, 0);
    }
}

static void irq_pipe_ready(rcar_dctrl_t *rcar, uint16_t status, uint16_t enb)
{
    uint16_t check;
    uint16_t pipenum;
    rcar_ep_t *ep;
    if ((status & BRDY0) && (enb & BRDY0)) {
        rcar_write(rcar, ~BRDY0, BRDYSTS);

        ep = &rcar->ep_arr[0];
        irq_packet_read(ep);
    } else {
        for (pipenum = 1; pipenum < RCAR_MAX_NUM_PIPE; pipenum++) {
            check = 1 << pipenum;
            if ((status & check) && (enb & check)) {
                rcar_write(rcar, ~check, BRDYSTS);
                ep = rcar->pipenum2ep[pipenum];
                if (ep->iousb_ep->edesc.bEndpointAddress & USB_ENDPOINT_IN)
                    irq_packet_write(ep);
                else
                    irq_packet_read(ep);
            }
        }
    }
}

static void irq_pipe_empty(rcar_dctrl_t *rcar, uint16_t status, uint16_t enb)
{
    uint16_t tmp;
    uint16_t check;
    uint16_t pipenum;
    rcar_ep_t *ep;
    if ((status & BEMP0) && (enb & BEMP0)) {
        rcar_write(rcar, ~BEMP0, BEMPSTS);

        ep = &rcar->ep_arr[0];
        irq_ep0_write(ep);
    } else {
        for (pipenum = 1; pipenum < RCAR_MAX_NUM_PIPE; pipenum++) {
            check = 1 << pipenum;
            if ((status & check) && (enb & check)) {
                rcar_write(rcar, ~check, BEMPSTS);
                tmp = control_reg_get(rcar, pipenum);
                if ((tmp & INBUFM) == 0) {
                    disable_irq_empty(rcar, pipenum);
                    pipe_irq_disable(rcar, pipenum);
                    pipe_stop(rcar, pipenum);
                    ep = rcar->pipenum2ep[pipenum];
                    transfer_complete(ep, 0);
                }
            }
        }
    }
}

/* if return value is true, call class driver's setup() */
static int setup_packet(rcar_dctrl_t *rcar, usb100_setup_packet_t *ctrl)
{
    uint16_t *p = (uint16_t *)ctrl;
    unsigned long offset = USBREQ;
    int i;
    /* read fifo */
    rcar_write(rcar, ~VALID, INTSTS0);

    for (i = 0; i < 4; i++)
        p[i] = rcar_read(rcar, offset + i*2);
    return 1;
}
static void rcar_update_usb_speed(rcar_dctrl_t *rcar)
{
    uint16_t speed = get_usb_speed(rcar);
    switch (speed) {
    case HSMODE:
        rcar->udc->usbdc_self->usbdc_set_device_speed( rcar->udc, IOUSB_DEVICE_HIGH_SPEED );
        rcar->flags &= ~DC_FLAG_UNKNOW_SPEED;
        break;
    case FSMODE:
        rcar->udc->usbdc_self->usbdc_set_device_speed( rcar->udc, IOUSB_DEVICE_FULL_SPEED );
        rcar->flags &= ~DC_FLAG_UNKNOW_SPEED;
        break;
    default:
        rcar->udc->usbdc_self->usbdc_set_device_speed( rcar->udc, IOUSB_DEVICE_FULL_SPEED );
        printf("USB speed unknown\n");
        rcar->flags |= DC_FLAG_UNKNOW_SPEED;
        break;
    }
    /*
     * delay first setup packet detected after insertion to give the extract
     * code a chance to run and cleanup
     */
    rcar->ep_arr[0].setup_packet_delay = 1;
    rcar->flags |= DC_FLAG_CONNECTED;
    rcar->flags &= ~DC_FLAG_UNKNOW_SPEED;
}
static void irq_device_state(rcar_dctrl_t *rcar)
{
    uint16_t dvsq;
    dvsq = rcar_read(rcar, INTSTS0) & DVSQ;
    rcar_write(rcar, ~DVST, INTSTS0);

    if (dvsq == DS_DFLT) {
        /* bus reset */
        if((rcar->flags & DC_FLAG_UNKNOW_SPEED) == DC_FLAG_UNKNOW_SPEED)
        {
            rcar_update_usb_speed(rcar);
            rcar->udc->usbdc_self->usbdc_device_state_change( rcar->udc, IOUSB_DEVICE_STATE_RESET);
        }
    }
    if (rcar->old_dvsq == DS_CNFG && dvsq != DS_CNFG)
    {
        rcar_update_usb_speed(rcar);
    }
    if ((dvsq == DS_CNFG || dvsq == DS_ADDS) && ((rcar->flags & DC_FLAG_UNKNOW_SPEED) == DC_FLAG_UNKNOW_SPEED))
    {
        rcar_update_usb_speed(rcar);
    }
    if (dvsq & DS_SUSP)
    {
        rcar->flags |= DC_FLAG_PHY_SUSPEND;
        if (rcar->udc->usbdc_self->usbdc_device_state_change( rcar->udc, IOUSB_DEVICE_SUSPEND_REQUEST) == EOK) {
            RCAR_SLOGF_DBG( rcar, _SLOG_INFO, "USB device suspend ...." );
            rcar->udc->usbdc_self->usbdc_device_state_change( rcar->udc, IOUSB_DEVICE_STATE_SUSPENDED);
        }
    }

    rcar->old_dvsq = dvsq;
}
static void irq_control_stage(rcar_dctrl_t *rcar)
{
    uint16_t ctsq;
    rcar_ep_t *ep;
    ep = &rcar->ep_arr[0];

    ctsq = rcar_read(rcar, INTSTS0) & CTSQ;
    rcar_write(rcar, ~CTRT, INTSTS0);

    switch (ctsq) {
    case CS_IDST: {
        ep = &rcar->ep_arr[0];
        transfer_complete(ep, 0);
        break;
    }

    case CS_RDDS:
    case CS_WRDS:
    case CS_WRND:
        if(ep->control_phase == CONTROL_PHASE_SETUP){
            if (setup_packet(rcar, rcar->setup_packet)) {
                if(ep->setup_packet_delay){
                    ep->setup_packet_delay = 0;
                    MsgSendPulse ( rcar->coid, 10, PULSE_DELAY_SETUP_PKT, 0 );
                }else {
                    if (rcar->udc->usbdc_self->usbdc_setup_packet_process(rcar->udc, (uint8_t *)rcar->setup_packet) < 0)
                        pipe_stall(rcar, 0);
                }
            }
        }
        break;
    case CS_RDSS:
    case CS_WRSS:
        control_end(rcar, 0);
        break;
    default:
        RCAR_SLOGF_DBG(ep->dc, _SLOG_ERROR, "ctrl_stage: unexpect ctsq(%x)\n", ctsq);
        break;
    }
}
void rcar_irq_process(rcar_dctrl_t *rcar)
{
    uint16_t intsts0;
    uint16_t intenb0;
    uint16_t brdysts, bempsts;
    uint16_t brdyenb, bempenb;
    uint16_t mask0;

    intsts0 = rcar_read(rcar, INTSTS0);
    intenb0 = rcar_read(rcar, INTENB0);

    mask0 = intsts0 & intenb0;
    if (mask0) {
        brdysts = rcar_read(rcar, BRDYSTS);
        bempsts = rcar_read(rcar, BEMPSTS);
        brdyenb = rcar_read(rcar, BRDYENB);
        bempenb = rcar_read(rcar, BEMPENB);

        if (mask0 & VBINT) {
            rcar_write(rcar,  0xffff & ~VBINT,
                    INTSTS0);
            rcar_start_xclock(rcar);

            /* start vbus sampling */
            rcar->old_vbus = rcar_read(rcar, INTSTS0)
                    & VBSTS;
            rcar->scount = RCAR_MAX_SAMPLING;
            if(rcar->old_vbus) {
                RCAR_SLOGF_DBG( rcar, _SLOG_INFO, "USB device connect ...." );
                rcar->udc->usbdc_self->usbdc_device_state_change( rcar->udc, IOUSB_DEVICE_STATE_INSERTED);
                rcar_vbus_session(rcar, 1);
            }
            else {
                RCAR_SLOGF_DBG( rcar, _SLOG_INFO, "USB device disconnect ...." );
                rcar->udc->usbdc_self->usbdc_device_state_change( rcar->udc, IOUSB_DEVICE_STATE_REMOVED);
                rcar_vbus_session(rcar, 0);

                /* Keep VBUS interrupt enabled */
                rcar_bset(rcar, VBSE, INTENB0);
                rcar->softconnect = 0;
            }
        }
        if (intsts0 & DVST)
            irq_device_state(rcar);

        if ((intsts0 & BRDY) && (intenb0 & BRDYE)
                && (brdysts & brdyenb))
            irq_pipe_ready(rcar, brdysts, brdyenb);
        if ((intsts0 & BEMP) && (intenb0 & BEMPE)
                && (bempsts & bempenb))
            irq_pipe_empty(rcar, bempsts, bempenb);

        if (intsts0 & CTRT)
            irq_control_stage(rcar);
        if (intsts0 & RESM) {
            rcar->udc->usbdc_self->usbdc_device_state_change(rcar->udc, IOUSB_DEVICE_STATE_RESUMED);
            rcar_bclr(rcar,  RESM, INTSTS0);
            rcar_dma_reset(rcar);
        }
    }

}

static void dma_write_complete(rcar_dctrl_t *rcar,
                   rcar_dma_t *dma)
{
    rcar_ep_t *ep = dma->ep;

    int ch = dma->channel;
    uint16_t tmp;
    rcar_dma_bclr(rcar, DE | IE | TOE, USBHS_DMAC_CHCR(ch));
    ep->bytes_xfered += ep->req_xfer_len;


    /* Clear interrupt flag for next transfer. */
    rcar_write(rcar, ~(1 << ep->pipenum), BRDYSTS);

    if (!(ep->bytes_xfered % (ep->mps + 1))) {
        /* Send zero-packet by irq_packet_write(). */
        tmp = control_reg_get(rcar, ep->pipenum);
        if (tmp & BSTS)
            irq_packet_write(ep);
        else
            enable_irq_ready(rcar, ep->pipenum);
    } else {
        /* To confirm the end of transmit */
        enable_irq_empty(rcar, ep->pipenum);
    }
    rcar_dma_bclr(rcar, TE | DE, USBHS_DMAC_CHCR(ch));
}

static unsigned long rcar_dma_received_size(rcar_dctrl_t *rcar,
                        rcar_dma_t *dma,
                        uint16_t size)
{
    rcar_ep_t *ep = dma->ep;
    int ch = dma->channel;
    unsigned long received_size;
    /*
     * DAR will increment the value every transfer-unit-size,
     * but the "size" (DTLN) will be set within MaxPacketSize.
     *
     * The calucuation would be:
     *   (((DAR-SAR) - TransferUnitSize) & ~MaxPacketSize) + DTLN.
     *
     * Be careful that if the "size" is zero, no correction is needed.
     * Just return (DAR-SAR) as-is.
     */
    received_size = rcar_dma_read(rcar, USBHS_DMAC_DAR(ch)) -
            rcar_dma_read(rcar, USBHS_DMAC_SAR(ch));
    if (size) {
        received_size -= dma->tx_size;
        received_size &= ~(ep->mps - 1);
        received_size += size; /* DTLN */
    }

    return received_size;
}

static void dma_read_complete(rcar_dctrl_t *rcar,
                  rcar_dma_t *dma)
{
    rcar_ep_t *ep = dma->ep;
    int ch = dma->channel;
    unsigned short tmp, size;
    /* Clear interrupt flag for next transfer. */

    rcar_dma_bclr(rcar, TE, USBHS_DMAC_CHCR(ch));

    rcar_write(rcar, ~(1 << ep->pipenum), BRDYSTS);

    tmp = rcar_read(rcar, ep->fifoctr);
    size = tmp & DTLN;
    rcar_bset(rcar, BCLR, ep->fifoctr);

    ep->bytes_xfered += rcar_dma_received_size(rcar, dma, size);

    if (rcar_dma_read(rcar, USBHS_DMAC_CHCR(ch)) & NULLF) {
        /*
         * When a NULL packet is received during a DMA transfer,
         * the DMA transfer can be suspended and resumed on each
         * channel independently in the following sequence.
         */
        uint32_t chcr;

        rcar_bclr(rcar, DREQE, ep->fifosel);
        /* wait for the internal bus to be stabilized (20clk@ZS) */
        usleep(1);
        chcr = rcar_dma_read(rcar, USBHS_DMAC_CHCR(ch));
        chcr = (chcr & ~(NULLF | DE)) | FTE;
        rcar_dma_write(rcar, chcr, USBHS_DMAC_CHCR(ch));
        rcar_dma_bclr(rcar, IE | SP, USBHS_DMAC_CHCR(ch));
    } else {
        rcar_dma_bclr(rcar, IE | SP | DE, USBHS_DMAC_CHCR(ch));
    }

    rcar_dma_bclr(rcar, TE, USBHS_DMAC_CHCR(ch));
    pipe_stop(rcar, ep->pipenum);
    transfer_complete(ep, 0);
}

void rcar_dma_irq(rcar_dctrl_t *rcar)
{
    uint32_t dmicrsts;
    int ch;

    rcar_pthread_mutex_lock(&rcar->usb_mutex);
    dmicrsts = rcar_dma_read(rcar, DMICR);

    for (ch = 0; ch < RCAR_MAX_DMA_CHANNELS; ch++) {
        if (!(dmicrsts & rcar->dma[ch].expect_dmicr))
            continue;

        if (rcar->dma[ch].dir)
            dma_write_complete(rcar, &rcar->dma[ch]);
        else
            dma_read_complete(rcar, &rcar->dma[ch]);
    }

    rcar_pthread_mutex_unlock(&rcar->usb_mutex);

}

/*-------------------------------------------------------------------------*/
int rcar_ep_enable(rcar_ep_t *ep)
{
    return alloc_pipe_config(ep);
}

int rcar_ep_disable(rcar_ep_t *ep)
{
    rcar_dctrl_t *rcar = ep->dc;

        pipe_stop(rcar, ep->pipenum);
        dmac_cancel(ep);
        pipe_irq_disable(rcar, ep->pipenum);
        rcar_write(rcar, ~(1 << ep->pipenum), BRDYSTS);
        rcar_write(rcar, ~(1 << ep->pipenum), BEMPSTS);
        transfer_complete(ep, -ECONNRESET);

    return free_pipe_config(ep);
}

void rcar_udc_stop(rcar_dctrl_t *rcar)
{
    rcar_bclr(rcar, VBSE, INTENB0);
    disable_controller(rcar);
}

int rcar_vbus_session(rcar_dctrl_t *rcar, int is_active)
{
    uint16_t bwait = rcar->bwait ? : 0xf;

    RCAR_SLOGF_DBG( rcar, _SLOG_INFO, "VBUS %s => %s",
        rcar->vbus_active ? "on" : "off", is_active ? "on" : "off");

    if ((is_active ^ rcar->vbus_active) == 0)
        return 0;

    if (is_active) {
        rcar_usb_func_start(rcar);
        init_controller(rcar);
        /* start clock */
        rcar_write(rcar, bwait, SYSCFG1);
        rcar_bset(rcar, HSE, SYSCFG0);
        rcar_bset(rcar, USBE, SYSCFG0);
        rcar_bset(rcar, SCKE, SYSCFG0);

        rcar_usb_connect(rcar);
    } else {

        disable_controller(rcar);
        rcar_usb_disconnect(rcar);

        /* stop clock */
        rcar_bclr(rcar, HSE, SYSCFG0);
        rcar_bclr(rcar, SCKE, SYSCFG0);
        rcar_bclr(rcar, USBE, SYSCFG0);

        rcar_usb_func_stop(rcar);
    }

    rcar->vbus_active = is_active;
    return 0;
}


int rcar_map_regs(rcar_dctrl_t *rcar)
{
    rcar->reg =  mmap_device_memory( NULL,
                        0x194,
                        PROT_READ | PROT_WRITE | PROT_NOCACHE,
                        MAP_SHARED | MAP_PHYS,
                        RCAR_USBF_BASE);
    if ( rcar->reg == MAP_FAILED ) {
        printf("%s mmap usb failed",__func__);
        return ENOMEM;
    }

    if(rcar->dmac)
    {
        rcar->dmac_reg =  mmap_device_memory( NULL,
                        0x64,
                        PROT_READ | PROT_WRITE | PROT_NOCACHE,
                        MAP_SHARED | MAP_PHYS,
                        USBDMA_BASE1);
        if ( rcar->dmac_reg == MAP_FAILED ) {
            printf("%s mmap dmac failed",__func__);
            return ENOMEM;
        }
    }
    return EOK;
}
void rcar_unmap_reg(rcar_dctrl_t *rcar)
{
    munmap_device_memory(rcar->reg, 0x194);
    if(rcar->dmac)
        munmap_device_memory(rcar->dmac_reg, 0x64);
}

void rcar_feature_test_mode_wait_complete(rcar_dctrl_t *rcar, uint16_t wIndex)
{
    uint16_t tmp;
    int timeout = 3000;
    do {
        tmp = rcar_read(rcar, INTSTS0) & CTSQ;
        delay(1);
    } while (tmp != CS_IDST || timeout-- > 0);

    if (tmp == CS_IDST)
        rcar_bset(rcar, wIndex >> 8, TESTMODE);
}

static struct timespec start, stop;
/*
 *  rcar_setupPulseAndTimer
 *
 *  This routine is responsible for setting up a pulse so it
 *  sends a message with code MT_TIMER.  It then sets up a
 *  periodic timer that fires once per second.
 *
 */
void rcar_setupPulseAndTimer (int chid)
{
    timer_t             timerid;    // timer ID for timer
    struct sigevent     event;      // event to deliver
    struct itimerspec   timer;      // the timer data structure
    int                 coid;       // connection back to ourselves

    // create a connection back to ourselves
    coid = ConnectAttach (0, 0, chid, 0, 0);
    if (coid == -1) {
        fprintf (stderr, "  couldn't ConnectAttach to self!\n"
                 );
        perror (NULL);
        exit (EXIT_FAILURE);
    }

    // set up the kind of event that we want to deliver -- a pulse
    SIGEV_PULSE_INIT (&event, coid,
                      SIGEV_PULSE_PRIO_INHERIT, RCAR_CODE_TIMER, 0);

    // create the timer, binding it to the event
    if (timer_create (CLOCK_REALTIME, &event, &timerid) == -1) {
        fprintf (stderr, "%s:  couldn't create a timer, errno %d\n",
                 __FUNCTION__, errno);
        perror (NULL);
        return;
    }
    // setup the timer (0.5s delay, 0.5s reload)
    timer.it_value.tv_sec = 0;
    timer.it_value.tv_nsec = 100000000UL;

    timer.it_interval.tv_sec = 0;
    timer.it_interval.tv_nsec = 100000000UL;
    if( clock_gettime( CLOCK_REALTIME, &start) == -1 ) {
          perror( "clock gettime" );

     }
    // and start it!
   timer_settime (timerid, 0, &timer, NULL);
    if( clock_gettime( CLOCK_REALTIME, &stop) == -1 ) {
          perror( "clock gettime" );
    }

}
int rcar_dmac_attach_intr(rcar_dctrl_t *rcar)
{
    struct sigevent     event;      // event to deliver
    int                 coid;       // connection back to ourselves
    if ((coid = ConnectAttach(0, 0, rcar->chid, _NTO_SIDE_CHANNEL, 0)) == -1)
        return -1;

    SIGEV_PULSE_INIT(&event, coid, 21, RCAR_DMAC_IRQ_EVENT, NULL);
    rcar->iid_dma = InterruptAttachEvent(rcar->irq_dma, &event, _NTO_INTR_FLAGS_TRK_MSK);
    if (rcar->iid_dma == -1)
        return -1;

    return 0;
}

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/hardware/devu/dc/rcar/hsusb/rcar_udc.c $ $Rev: 810496 $")
#endif
