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

#include "externs.h"

static uint16_t scfdr_rx( DEV_SCIF *dev )
{
    uint16_t count = in16(dev->port + dev->rx_fifo_count_reg);

    switch (dev->scif)
    {
        case UART_TYPE_SCIF:
            count = SCIF_SCFDR_RX(count);
            break;
        case UART_TYPE_HSCIF:
            count = HSCIF_HSFDR_RX(count);
            break;
        default:
            break;
    }

    return (count);
}

/*
 * error_handling()
 * Check for Overrun, Parity, and Framing errors
 */
static unsigned error_handling(DEV_SCIF *dev)
{
#define STATUS_MASK (SCIF_SCSSR_PER | SCIF_SCSSR_FER | SCIF_SCSSR_BRK | SCIF_SCSSR_ER)
    unsigned err = 0;
    uint16_t lsr = 0;
    uint16_t status_mask = STATUS_MASK;

    /* Read error status */
    lsr = in16(dev->port + dev->status_reg);

    /* The Overrun Error status is in different registers on the different SCIF devices types */
    switch (dev->scif)
    {
        case UART_TYPE_SCIF:
            if (in16(dev->port + SCIF_SCLSR_OFF) & SCIF_SCLSR_ORER)
            {
                err |= TTI_OVERRUN;
                dev->tty.oband_data |= _OBAND_SER_OE;
                /* Clear LSR status */
                set_port16(dev->port + SCIF_SCLSR_OFF, SCIF_SCLSR_ORER, 0);
            }
            break;
        case UART_TYPE_HSCIF:
            break;
        default:
            break;
    }
    /* Clear status */
    set_port16(dev->port + dev->status_reg, status_mask, 0);

    if (lsr & SCIF_SCSSR_PER)
    {
        err |= TTI_PARITY;
        dev->tty.oband_data |= _OBAND_SER_PE;
    }
    if (lsr & SCIF_SCSSR_FER)
    {
        err |= TTI_FRAME;
        dev->tty.oband_data |= _OBAND_SER_FE;
    }
    if (lsr & SCIF_SCSSR_BRK )
    {
        err |= TTI_BREAK;
        dev->tty.oband_data |= _OBAND_SER_BI;
        /* break could cause FE/PE */
        err &= ~(TTI_PARITY|TTI_FRAME);
        dev->tty.oband_data &= ~(_OBAND_SER_FE | _OBAND_SER_PE);
    }

    /* Save the error as out-of-band data which can be retrieved via devctl(). */
    atomic_set (&dev->tty.flags, OBAND_DATA);

    return (err);
}

static unsigned char read_char(DEV_SCIF *dev)
{
    unsigned char c = 0;

    /* Read the character from the buffer */
    c = in8(dev->port + dev->rx_reg);
    /* Clear interrupt status */
    set_port16(dev->port + dev->status_reg, SCIF_SCSSR_RDF | SCIF_SCSSR_DR, 0);

    return (c);
}

static int process_tx(DEV_SCIF *dev)
{
    int status = EOK;
    uint16_t tx_status = 0;

    tx_status = in16 (dev->port + dev->status_reg) & SCIF_SCSSR_TDFE;
    if(tx_status & SCIF_SCSSR_TDFE)
    {
        /* Disable TX interrupt, don't bother trying to clear the interrupt
         * status until we write more data to the TX FIFO in tto().
         */
        set_port16(dev->port + dev->sc_reg, SCIF_SCSCR_TIE, 0);
        /* Clear timer */
        dev->tty.un.s.tx_tmr = 0;
        atomic_set(&dev->tty.flags, EVENT_TTO);
        status |= 1;
    }

    return (status);
}

/*
 * This routine will handle all of our interrupts and dispatch them as required
 */
const struct sigevent * ser_intr(void *arg, int iid) {
    DEV_SCIF        *dev;
    int             status = EOK;
    unsigned        c;
    struct sigevent *event = NULL;

    dev = ((struct dev_list*)arg)->device;

    for (dev = ((struct dev_list*)arg)->device; dev != NULL; dev = dev->next)
    {
        /* Process RX and error interrupts */
        if (scfdr_rx(dev))
        {
            // If we are hw flow controlled, don't read the char's.
            // This causes the fifo to fill up, and the the scif chip
            // automatically handles hw flow control.
            if (!dev->rts_flag)
            {
                do
                {
                    c = error_handling(dev);
                    c |= read_char(dev);
                    status |= tti(&dev->tty, c);
                } while (!status && scfdr_rx(dev) > 0);
            }
            else
            {
                // Make sure that RX and RX error interrupts get disabled if we are
                // HW flow controlled or we could lock up the board, stuck in the ISR.
                set_port16(dev->port + dev->sc_reg, dev->rx_intr_mask, 0);
            }
        }

        status |= process_tx(dev);

        // Return the status if any to the io-char library
        if (status && !(dev->tty.flags & EVENT_QUEUED))
        {
            event = &ttyctrl.event;
            dev_lock(&ttyctrl);
            ttyctrl.event_queue[ttyctrl.num_events++] = &dev->tty;
            atomic_set(&dev->tty.flags, EVENT_QUEUED);
            dev_unlock(&ttyctrl);
        }
    }

    return (event);
}

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/hardware/devc/serscif/intr.c $ $Rev: 805958 $")
#endif
