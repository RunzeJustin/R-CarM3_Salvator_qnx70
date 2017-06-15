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

/*************************************************************************************
*   Copyright (C) 2010-2011 Renesas Electronics Corporation. All rights reserved.    *
*************************************************************************************/


#include "externs.h"
#include <sys/mman.h>

//This is the maximum of the SCI and SCIF register sizes
#define SCI_MEM_SIZE 36*8

void set_port8(unsigned port, uint8_t mask, uint8_t data)
{
    out8(port, (in8(port) & ~mask) | (data & mask));
}

void set_port16(unsigned port, uint16_t mask, uint16_t data)
{
    out16(port, (in16(port) & ~mask) | (data & mask));
}

void set_port32(unsigned port, uint32_t mask, uint32_t data)
{
    out32(port, (in32(port) & ~mask) | (data & mask));
}

void ser_attach_intr(DEV_SCIF *dev)
{
    struct dev_list **owner;
    struct dev_list *curr;

    owner = &devices;
    for (;;)
    {
        curr = *owner;
        if(curr == NULL)
        {
            curr = malloc(sizeof(*curr));
            if(curr == NULL)
            {
                fprintf(stderr, "Init allocation failed\n");
                exit(EXIT_FAILURE);
            }
            *owner = curr;
            curr->next = NULL;
            curr->device = NULL;
            break;
        }
        if (curr->device->sh_intr == dev->sh_intr)
            break;
        owner = &curr->next;
    }

    dev->next = curr->device;
    curr->device = dev;

    if (curr->device->next == NULL)
    {
        if (dev->sh_intr != 0)
            curr->iid = InterruptAttach(dev->sh_intr, (void*) ser_intr, curr, 0, 0);
    }
}

/*
 * create_device()
 * Set up data structures for the console serial device
 */
void create_device(TTYINIT_SCIF *dip, unsigned unit)
{
    DEV_SCIF *dev;

    // Get buffers and set the name of the device
    if ((dev = calloc(1, sizeof(*dev))) == NULL)
    {
        fprintf(stderr, "Device allocation failed\n");
        exit(EXIT_FAILURE);
    }

    strcpy(dev->tty.name,dip->tty.name);
    dev->scif = dip->scif;
    dev->rstrg = dip->rstrg;
    dev->sh_intr = dip->tty.intr;
    dev->clock_source = dip->clock_source;

    switch (dev->scif)
    {
        case UART_TYPE_SCIF:
            dev->tx_reg = SCIF_SCFTDR_OFF;
            dev->rx_reg = SCIF_SCFRDR_OFF;
            dev->sc_reg = SCIF_SCSCR_OFF;
            dev->fc_reg = SCIF_SCFCR_OFF;
            dev->status_reg = SCIF_SCFSR_OFF;
            dev->tx_fifo_count_reg = SCIF_SCFDR_OFF;
            dev->rx_fifo_count_reg = SCIF_SCFDR_OFF;

            dev->rx_intr_mask = SCIF_SCSCR_RIE | SCIF_SCSCR_REIE;
            break;
        case UART_TYPE_HSCIF:
            dev->tx_reg = HSCIF_HSFTDR_OFF;
            dev->rx_reg = HSCIF_HSFRDR_OFF;
            dev->sc_reg = HSCIF_HSSCR_OFF;
            dev->fc_reg = HSCIF_HSFCR_OFF;
            dev->status_reg = HSCIF_HSFSR_OFF;
            dev->tx_fifo_count_reg = HSCIF_HSFDR_OFF;
            dev->rx_fifo_count_reg = HSCIF_HSFDR_OFF;

            dev->rx_intr_mask = SCIF_SCSCR_RIE | SCIF_SCSCR_REIE;
            break;
        default:
            fprintf(stderr, "Unsupport SCIF type");
            exit(EXIT_FAILURE);
    }

    /* Map the io devices registers into address space - offset 4 bytes */
    if ((dev->port = mmap_device_io(SCI_MEM_SIZE, dip->tty.port)) == (uintptr_t)MAP_FAILED)
    {
        perror("SCI error: MAP FAILED\n");
        exit(EXIT_FAILURE);
    }

    /* Allocate Input buffer */
    if ((dev->tty.ibuf.buff = malloc(dev->tty.ibuf.size = dip->tty.isize)) == NULL)
    {
        fprintf(stderr, "Unable to allocate %d byte input buffer\n", dev->tty.ibuf.size);
        exit(EXIT_FAILURE);
    }
    else
        dev->tty.ibuf.head = dev->tty.ibuf.tail = dev->tty.ibuf.buff;

    /* Allocate Output buffer */
    if ((dev->tty.obuf.buff = malloc(dev->tty.obuf.size = dip->tty.osize)) == NULL)
    {
        fprintf(stderr, "Unable to allocate %d byte output buffer\n", dev->tty.obuf.size);
        exit(EXIT_FAILURE);
    }
    else
        dev->tty.obuf.head = dev->tty.obuf.tail = dev->tty.obuf.buff;

    /* Allocate Canonical buffer */
    if ((dev->tty.cbuf.buff = malloc(dev->tty.cbuf.size = dip->tty.csize)) == NULL)
    {
        fprintf(stderr, "Unable to allocate %d byte canonical buffer\n", dev->tty.cbuf.size);
        exit(EXIT_FAILURE);
    }
    else
        dev->tty.cbuf.head = dev->tty.cbuf.tail = dev->tty.cbuf.buff;

    dev->clk = dip->tty.clk;
    dev->div = dip->tty.div;
    dev->tty.baud = dip->tty.baud;
    dev->tty.fifo = dip->tty.fifo;
    dev->tty.verbose = dip->tty.verbose;
    dev->rts_hw_disable = dip->rts_hw_disable;
    dev->tty.highwater = dev->tty.ibuf.size - (dev->tty.ibuf.size < 128 ? dev->tty.ibuf.size/4 : 32);

    dev->tty.flags = EDIT_INSERT | LOSES_TX_INTR;
    dev->tty.c_cflag = dip->tty.c_cflag;
    dev->tty.c_iflag = dip->tty.c_iflag;
    dev->tty.c_lflag = dip->tty.c_lflag;
    dev->tty.c_oflag = dip->tty.c_oflag;

    /* Initialize termios cc codes to an ANSI terminal. */
    ttc(TTC_INIT_CC, &dev->tty, 0);

    /*
     * Initialize the device's name.
     * Assume that the basename is set in device name.  This will attach
     * to the path assigned by the unit number/minor number combination
     */
    unit = SET_NAME_NUMBER(unit) | NUMBER_DEV_FROM_USER;
    ttc(TTC_INIT_TTYNAME, &dev->tty, unit);

    /* Disable Transmitter, Receiver and all interrupts */
    out16 (dev->port + dev->sc_reg, 0x0);

    /* Attach the interrupt handler */
    ser_attach_intr(dev);

    /* Set FIFO trigger levels
     * SCIF uses SCFCR for RTRG and TTRG
     * HSCIF uses HSRTRGR and HSTTRGR */
    if(dev->scif == UART_TYPE_SCIF) {
        set_port16 (dev->port + dev->fc_reg, SCIF_SCFCR_RTRG_M | SCIF_SCFCR_TTRG_M, dev->tty.fifo);
    }

    ser_stty_scif(dev);

    /* Create resmgr namespace entry */
    ttc(TTC_INIT_ATTACH, &dev->tty, 0);
}

void disable_uart(void)
{
    DEV_SCIF *dev;
    struct dev_list *list;

    list = devices;
    while (list)
    {
        dev = list->device;
        while (dev)
        {
            /* Disable Receiver,  Transmitter and all the Interrupts */
            set_port16(dev->port + dev->sc_reg, SCIF_SCSCR_RE | dev->rx_intr_mask | SCIF_SCSCR_TE | SCIF_SCSCR_TIE, 0);

            /* Clear Rx Fifo */
            set_port16(dev->port + dev->fc_reg, SCIF_SCFCR_RFRST, SCIF_SCFCR_RFRST);
            set_port16(dev->port + dev->fc_reg, SCIF_SCFCR_RFRST, 0);

            dev = dev->next;
        }
        list = list->next;
    }
}

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/hardware/devc/serscif/init.c $ $Rev: 805958 $")
#endif
