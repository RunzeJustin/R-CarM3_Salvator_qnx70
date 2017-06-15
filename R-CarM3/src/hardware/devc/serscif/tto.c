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

/**************************************************************************************
*    Copyright (C) 2010-2011 Renesas Electronics Corporation. All rights reserved.    *
***************************************************************************************/


#include "externs.h"
#include <math.h>

static uint16_t scfdr_tx(DEV_SCIF *dev)
{
    uint16_t count = in16(dev->port + dev->tx_fifo_count_reg);

    switch (dev->scif)
    {
        case UART_TYPE_SCIF:
            count = SCIF_FIFO_LEN - SCIF_SCFDR_TX(count);
            break;
        case UART_TYPE_HSCIF:
            count = HSCIF_FIFO_LEN - HSCIF_HSFDR_TX(count);
            break;
        default:
            break;
    }
    return (count);
}

/*
 * tto()
 *  Transmission
 */
int tto(TTYDEV *ttydev, int action, int arg1)
{
    TTYBUF         *bup = &ttydev->obuf;
    DEV_SCIF       *dev = (DEV_SCIF *)ttydev;
    uintptr_t      port = dev->port;
    unsigned char  c;
    int            status = 0, nbytes = 0;

    switch(action)
    {
        case TTO_STTY:
            ser_stty_scif(dev);
            return(0);

        case TTO_DATA:
        case TTO_EVENT:
        break;

        //Something should be done w/ these messages
        case TTO_CTRL:
            if (arg1 & _SERCTL_BRK_CHG)
            {
                // BREAK for SCIF
                if(arg1 & _SERCTL_BRK)
                {
                    // Disable the transmitter to start the break signal
                    set_port16(dev->port + dev->sc_reg, SCIF_SCSCR_TE, 0);
                }
                else
                {
                    // Re-enable the transmitter
                    set_port16(dev->port + dev->sc_reg, SCIF_SCSCR_TE, SCIF_SCSCR_TE);
                }
            }

            if( (arg1 & _SERCTL_RTS_CHG) && dev->scif )
            {
                if(arg1 & _SERCTL_RTS)
                {
                    // automatic hw flow control
                    // io-char input buffer has room, so restart reading the rx buffer
                    dev->rts_flag = 0;
                    set_port16(dev->port + dev->sc_reg, dev->rx_intr_mask, dev->rx_intr_mask);
                }
                else
                {
                    // automatic hw flow control
                    // io-char input buffer is almost full, so stop reading chars.
                    // This causes the rx fifo to fill, and the automatic hw flow control to start
                    set_port16(dev->port + dev->sc_reg, dev->rx_intr_mask, 0);
                    dev->rts_flag = 1;
                }
            }
            return(0);

        case TTO_LINESTATUS:
                return status;
        default:
            return(0);
    }

    if(dev->tty.flags & (OHW_PAGED|OSW_PAGED) && !(dev->tty.xflags & OSW_PAGED_OVERRIDE))
        return(0);

    nbytes = scfdr_tx(dev);
    while (nbytes && bup->cnt)
    {
        dev_lock(&dev->tty);
        c = tto_getchar(&dev->tty);
        dev_unlock(&dev->tty);

        /* Write character to UART */
        out8(port + dev->tx_reg, c);
        nbytes = scfdr_tx(dev);

        /* Clear the OSW_PAGED_OVERRIDE flag as we only want
         * one character to be transmitted in this case.
         */
        if (dev->tty.xflags & OSW_PAGED_OVERRIDE)
        {
            atomic_clr(&dev->tty.xflags, OSW_PAGED_OVERRIDE);
            break;
        }
    }
    /* Clear End of Transmission and Transmit FIFO Data empty interrupt status */
    set_port16(dev->port + dev->status_reg, SCIF_SCSSR_TEND | SCIF_SCSSR_TDFE, 0);

    /* If there is still data in the obuf and we are not in a flow
     * controlled state then turn TX interrupts back on to notify us
     * when the hardware is ready for more characters.
     */
    if (bup->cnt > 0 && !(dev->tty.flags & (OHW_PAGED|OSW_PAGED)))
    {
        dev->tty.un.s.tx_tmr = 3; /* Timeout */
        set_port16(dev->port + dev->sc_reg, SCIF_SCSCR_TIE, SCIF_SCSCR_TIE);
    }

    /* Check the client lists for notify conditions */
    return( tto_checkclients(&dev->tty) );
}

static int find_min(float err[200], int count)
{
    float min = err[0];
    int i, k = 0;

    for (i = 0; i < count; i++) {
        if (min > err[i]) {
            min = err[i];
            k = i;
        }
    }
    return (k);
}

static float calc_err(DEV_SCIF *dev, float N, float clk, int baud, int sr, int n)
{
    float err = 0.0f;

    switch (dev->scif)
    {
        case UART_TYPE_SCIF:
            err = clk / baud;
            err = err / ((1 << 2 * n) * sr * (1 << (2 * n)) / 2);
            err = fabs(((err / N) - 1) * 100);
            break;

        case UART_TYPE_HSCIF:
            err = clk / baud;
            err = err / ((1 << 2 * n) * sr * (1 << (2 * n + 1)));
            err = fabs(((err / N) - 1) * 100);
            break;
    }

    return (err);
}

static float calc_N(DEV_SCIF *dev, float clk, int baud, int sr, int n)
{
    float N = 0.0f;

    switch (dev->scif)
    {
        case UART_TYPE_SCIF:
            N = clk / baud;
            N = N / ((1 << 2 * n) * sr * (1 << (2 * n)) / 2);
            N = (int)(N + 0.5);
            break;

        case UART_TYPE_HSCIF:
            N = clk / baud;
            N = N / ((1 << 2 * n) * sr * (1 << (2 * n + 1)));
            N = (int)(N + 0.5);
            break;
    }

    return (N);
}

static float find_arg(DEV_SCIF *dev, float clk, int baud, int *sample_rate, float *scbrr, int *cks)
{
    int SR[200], CKS[200];
    int n, K, k = 0;
    float err[200], N[200];
    int sr_scif = 64;
    int sr_hscif;

    switch (dev->scif)
    {
        case UART_TYPE_SCIF:
            for (n = 0; n <= 3; n++) {
                N[k] = calc_N(dev, clk, baud, sr_scif, n);

                if ((N[k] >= 1) && (N[k] < 256)) {
                    err[k] = calc_err(dev, N[k], clk, baud, sr_scif, n);
                }
                else {
                    err[k] = 0xffffffff;
                }

                CKS[k] = n;
                k++;
            }
            break;

        case UART_TYPE_HSCIF:
            for (n = 0; n <= 3; n++) {
                for (sr_hscif = 8; sr_hscif <= 32; sr_hscif++) {
                    N[k] = calc_N(dev, clk, baud, sr_hscif, n);

                    if ((N[k] >= 1) && (N[k] < 256)) {
                        err[k] = calc_err(dev, N[k], clk, baud, sr_hscif, n);
                    }
                    else {
                        err[k] = 0xffffffff;
                    }

                    SR[k] = sr_hscif;
                    CKS[k] = n;
                    k++;
                }
            }
            break;
    }

    K = find_min(err, k);
    *sample_rate = SR[K];
    *scbrr = N[K] - 1;
    *cks = CKS[K];

    return 0;
}

static void config_scif_baud (DEV_SCIF *dev)
{
    int sample_rate, cks;
    unsigned baud, div, clk;
    float scbrr;

    baud = dev->tty.baud;
    div = dev->div;
    clk = (float)dev->clk;

    /*
     * Set the speed of the serial line
     */

    if (baud != dev->baud)
    {
        /* Put FIFOs into reset */
        set_port16(dev->port + dev->fc_reg, SCIF_SCFCR_TFRST | SCIF_SCFCR_RFRST,
                     SCIF_SCFCR_TFRST | SCIF_SCFCR_RFRST);

        if (dev->clock_source == EXTERNAL_CLOCK) {
            if (div != 16) {
                out16(dev->port + SCIF_CKS_OFF, 0x8000);
                div = 1;
            }
            else {
                out16(dev->port + SCIF_CKS_OFF, 0);
            }

            out16(dev->port + SCIF_DL_OFF, clk / (div * baud));
            set_port16(dev->port + dev->sc_reg, SCIF_SCSCR_CKE_M, SCIF_SCSCR_CKE_1);
        }
        else {
            find_arg(dev, clk, baud, &sample_rate, &scbrr, &cks);
            switch (cks)
            {
                case 0:
                    cks = SCIF_SCSMR_CKS_1;
                    break;
                case 1:
                    cks = SCIF_SCSMR_CKS_4;
                    break;
                case 2:
                    cks = SCIF_SCSMR_CKS_16;
                    break;
                case 3:
                    cks = SCIF_SCSMR_CKS_64;
                    break;
                default:
                    fprintf(stderr, "%s: Invalid SCIF CLKS %d", __FUNCTION__, cks);
                    break;
            }
            /* Set N, cks*/
            out8(dev->port + SCIF_SCBRR_OFF, scbrr);
            set_port16(dev->port + SCIF_SCSMR_OFF, SCIF_SCSMR_CKS_M, cks);
            set_port16(dev->port + dev->sc_reg, SCIF_SCSCR_CKE_M, 0);
        }

        /* Wait for one bit interval, then turn on send/receive */
        for (clk /= 10000; clk; clk--)
            ;

        /* Disable the reset state of the fifo */
        set_port16(dev->port + dev->fc_reg, SCIF_SCFCR_TFRST | SCIF_SCFCR_RFRST, 0);

        dev->baud = baud;
    }
}

static void config_hscif_baud (DEV_SCIF *dev)
{
    int sample_rate, cks;
    unsigned baud, clk;
    float hsbrr;

    baud = dev->tty.baud;
    clk = (float)dev->clk;

    /*
     * Set the speed of the serial line
     */

    if (baud != dev->baud)
    {
        /* Put FIFOs into reset */
        set_port16(dev->port + dev->fc_reg, SCIF_SCFCR_TFRST | SCIF_SCFCR_RFRST,
                     SCIF_SCFCR_TFRST | SCIF_SCFCR_RFRST);

        if (dev->clock_source == EXTERNAL_CLOCK) {
            int dl, sr_hscif, K, k = 0, SR[200];
            float err[200], DL[200];
            for (sr_hscif = 8; sr_hscif <= 32; sr_hscif++) {
                DL[k] = clk / baud;
                DL[k] = DL[k] / sr_hscif;
                DL[k] = (int)(DL[k] + 0.5);

                if (DL[k] > 0) {
                    err[k] = clk / baud;
                    err[k] = err[k] / sr_hscif;
                    err[k] = fabs(((err[k] / DL[k]) - 1) * 100);
                } else {
                    err[k] = 0xffffffff;
                }
                SR[k] = sr_hscif;
                k++;
            }

            K = find_min(err, k);
            dl = DL[K];
            sample_rate = SR[K];

            set_port16(dev->port + dev->sc_reg, SCIF_SCSCR_CKE_M | SCIF_SCSCR_TOIE, SCIF_SCSCR_CKE_1 | SCIF_SCSCR_TOIE);
            out16(dev->port + HSCIF_CKS_OFF, 0);
            out16(dev->port + HSCIF_DL_OFF, dl);
            set_port16(dev->port + HSCIF_HSSRR_OFF, HSCIF_HSSRR_SRE, HSCIF_HSSRR_SRE);
            set_port16(dev->port + HSCIF_HSSRR_OFF, HSCIF_HSSRR_SRCYC, sample_rate - 1);
        }
        else {
            find_arg(dev, clk, baud, &sample_rate, &hsbrr, &cks);
            switch (cks)
            {
                case 0:
                    cks = SCIF_SCSMR_CKS_1;
                    break;
                case 1:
                    cks = SCIF_SCSMR_CKS_4;
                    break;
                case 2:
                    cks = SCIF_SCSMR_CKS_16;
                    break;
                case 3:
                    cks = SCIF_SCSMR_CKS_64;
                    break;
                default:
                    fprintf(stderr, "%s: Invalid HSCIF CLKS %d", __FUNCTION__, cks);
                    break;
            }
            /* Set N, cks*/
            out8(dev->port + HSCIF_HSBRR_OFF, hsbrr);
            set_port16(dev->port + SCIF_SCSMR_OFF, SCIF_SCSMR_CKS_M, cks);
            set_port16(dev->port + dev->sc_reg, SCIF_SCSCR_CKE_M, 0);
            set_port16(dev->port + HSCIF_HSSRR_OFF, HSCIF_HSSRR_SRCYC, sample_rate - 1);
            set_port16(dev->port + HSCIF_HSSRR_OFF, HSCIF_HSSRR_SRE, HSCIF_HSSRR_SRE);
        }

        /* Wait for one bit interval, then turn on send/receive */
        for (clk /= 10000; clk; clk--)
            ;

        /* Disable the reset state of the fifo */
        set_port16(dev->port + dev->fc_reg, SCIF_SCFCR_TFRST | SCIF_SCFCR_RFRST, 0);

        dev->baud = baud;
    }
}

void ser_stty_scif(DEV_SCIF *dev)
{
    uint16_t scsmr, scscr;

    if ((dev->baud == dev->tty.baud) && (dev->c_cflag == dev->tty.c_cflag))
    {
        /* nothing change, so do nothing */
        return;
    }

    /* Mode or baud rate was changed, reset the transceiver then configure mode/baud rate */

    /* Must disable Transmitter and receiver before changing UART format */
    set_port16(dev->port + dev->sc_reg, SCIF_SCSCR_RE | SCIF_SCSCR_TE, 0);
    /* Put FIFOs into Reset */
    set_port16(dev->port + dev->fc_reg, SCIF_SCFCR_RFRST | SCIF_SCFCR_TFRST,
                SCIF_SCFCR_RFRST | SCIF_SCFCR_TFRST);

    /* Reset format bits */
    scsmr = in16(dev->port + SCIF_SCSMR_OFF);
    scsmr &= ~(SCIF_SCSMR_CHR | SCIF_SCSMR_STOP | SCIF_SCSMR_PE | SCIF_SCSMR_OE);

    /* Set the bit length (8 bit default, 7 bit if requested) */
    if ((dev->tty.c_cflag & CSIZE) == CS7)
    {
        //7bits data
        scsmr |= SCIF_SCSMR_CHR;
    }
    else
    {
        //8bits data
        dev->tty.c_cflag &= ~CSIZE;
        dev->tty.c_cflag |=  CS8;
    }

    /* Check parity and stop bit requirements */
    if (dev->tty.c_cflag & CSTOPB)
    {
        //2 stop bits
        scsmr |= SCIF_SCSMR_STOP;
    }

    /* Parity bits (default none, or even if we have it)  */
    if (dev->tty.c_cflag & PARENB)
    {
        scsmr |= SCIF_SCSMR_PE;
        if ((dev->tty.c_cflag & PARODD) != 0)
        {
            //odd parity
            scsmr |= SCIF_SCSMR_OE;
        }
    }

    /* Commit format changes */
    out16 (dev->port + SCIF_SCSMR_OFF, scsmr);

    switch (dev->scif)
    {
        case UART_TYPE_SCIF:
            config_scif_baud(dev);
            /* Configure RTS as an output pin and CTS as an input pin */
            out16(dev->port + SCIF_SCSPTR_OFF, SCIF_SCSPTR_RTSIO);
            break;
        case UART_TYPE_HSCIF:
            config_hscif_baud(dev);
            break;
        default:
            break;
    }

    if (dev->tty.verbose > 2)
    {
        slogf(_SLOG_SETCODE(_SLOGC_CHAR, 0), _SLOG_INFO, "Port (%s) settings:", dev->tty.name);
        slogf(_SLOG_SETCODE(_SLOGC_CHAR, 0), _SLOG_INFO, "Baud .......................... %d", dev->baud);
        slogf(_SLOG_SETCODE(_SLOGC_CHAR, 0), _SLOG_INFO, "Data bits ..................... %d", (scsmr & SCIF_SCSMR_CHR) ? 7 : 8);
        slogf(_SLOG_SETCODE(_SLOGC_CHAR, 0), _SLOG_INFO, "Stop bits ..................... %d", (scsmr & SCIF_SCSMR_STOP) ? 2 : 1);
        slogf(_SLOG_SETCODE(_SLOGC_CHAR, 0), _SLOG_INFO, "Parity ........................ %s", (scsmr & SCIF_SCSMR_PE) ? ((scsmr & SCIF_SCSMR_OE) ? "Odd" : "Even") : "Disabled");
    }

    if (dev->tty.c_cflag & (IHFLOW|OHFLOW))
    {
        /* Apply RTS Trigger level and enable hardware flow control */
        set_port16(dev->port + dev->fc_reg, SCIFB_SCBFCR_RSTRG_M | SCIF_SCFCR_MCE,
                        dev->rstrg | SCIF_SCFCR_MCE);
        /* TODO: Remove this?? */
        set_port16(dev->port + SCIFB_SCBPDR_OFF, SCIFB_SCBPDR_RTSD, 0);
    }
    else
    {
        /* Disable hardware flow control */
        set_port16(dev->port + dev->fc_reg, SCIF_SCFCR_MCE, 0);
    }

    scscr = in16(dev->port + dev->sc_reg);
    if ((dev->tty.c_cflag & CREAD) && ( !(scscr & SCIF_SCSCR_RE) || !(scscr & SCIF_SCSCR_RIE)))
    {
        /* Take Rx Fifo out of reset */
        set_port16(dev->port + dev->fc_reg, SCIF_SCFCR_RFRST, 0);

        dev->rts_flag = 0; /* Make sure we are not still flow controlled */

        /* Must re-enable rx interrupts incase we were flow controlled when receiver was disabled */
        if (dev->tty.verbose > 2)
            slogf(_SLOG_SETCODE(_SLOGC_CHAR, 0), _SLOG_INFO, "%s: Enabling Receiver\n", __FUNCTION__);

        //enable receive interrupts
        set_port16(dev->port + dev->sc_reg, SCIF_SCSCR_RE | dev->rx_intr_mask, SCIF_SCSCR_RE | dev->rx_intr_mask);
    }
    else if (!(dev->tty.c_cflag & CREAD) && (scscr & SCIF_SCSCR_RE))
    {
        /* Disable Receiver */
        if (dev->tty.verbose > 2)
            slogf(_SLOG_SETCODE(_SLOGC_CHAR, 0), _SLOG_INFO, "%s: Disabling Receiver\n", __FUNCTION__);

        set_port16(dev->port + dev->sc_reg, SCIF_SCSCR_RE, 0);
    }

    /* Update last c_cflag */
    dev->c_cflag = dev->tty.c_cflag;

    /* Take TX Fifo out of reset and re-enable transmitter */
    set_port16(dev->port + dev->fc_reg, SCIF_SCFCR_TFRST, 0);
    set_port16(dev->port + dev->sc_reg, SCIF_SCSCR_TE, SCIF_SCSCR_TE);
}

int drain_check(TTYDEV *ttydev, uintptr_t *count)
{
    DEV_SCIF *dev = (DEV_SCIF *)ttydev;
    TTYBUF   *bup = &ttydev->obuf;
    int      value;

    if (bup->cnt == 0)
    {
        value = in16(dev->port + dev->status_reg);
        if (value & SCIF_SCSSR_TEND)
            return 1;
    }

    /* TX FIFO is not empty - specify a wait time equal to the time it takes to xmit one char */
    if (count != NULL)
        *count = (ttydev->baud == 0) ? 0 : ((IO_CHAR_DEFAULT_BITSIZE * 20) / ttydev->baud) + 1;
    return 0;
}

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/hardware/devc/serscif/tto.c $ $Rev: 808962 $")
#endif
