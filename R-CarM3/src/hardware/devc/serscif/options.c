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
    Copyright (C) 2010-2011 Renesas Electronics Corporation. All rights reserved.    *
*************************************************************************************/

/*
#ifdef __USAGE
%C - Serial driver for SCIF/HSCIF

%C [options] [scifX | hscifX] &
Options:
 -b number    Define initial baud rate (default )
 -c clk[/div] Set the input clock rate and divisor
 -C number    Size of canonical input buffer (default 256)
 -e           Set options to "edit" mode
 -E           Set options to "raw" mode (default)
 -h           Enable RS-232 use of RTS and CTS lines (default)
 -H           Permanently disable RS-232 use of RTS and CTS lines (for use as GPIOs)
 -I number    Size of raw input buffer (default 2048)
 -x           Use external clock
 -f           Enable hardware flow control (default)
 -F           Disable hardware flow control
 -O number    Size of output buffer (default 2048)
 -r num       Set RTS trigger level (SCIF default 15)
 -s           Enable software flow control
 -S           Disable software flow control (default)
 -t num       Set receive FIFO trigger level (SCIF default 14)
 -T num       Set transmit FIFO data trigger level (SCIF default 1)
 -u unit      Set serial unit number (default 1)
 -o options   string options:
                  disable=rx (Disable receiver on startup)
#endif
*/

#include "externs.h"
#include <string.h>
#include <stdint.h>
#include <sys/mman.h>
#include <arm/r-car-m3.h>
#include <arm/scif.h>

static uint16_t get_rstrg(unsigned rstrg, int scif, unsigned fifo_rx)
{
    uint16_t rstrg_val = 0;

    switch (scif)
    {
        case UART_TYPE_SCIF:
            if (!rstrg)
            {
                // If rstrg has not been set yet, set it to the default
                rstrg = 15;
            }

            // Need to make sure that the RTS trigger level > RX FIFO trigger
            if (rstrg <= fifo_rx)
            {
                fprintf(stderr,"RTS trigger must be larger than RX FIFO trigger.\n");
                fprintf(stderr,"RTS trigger will be set to 15.\n");
                rstrg = 15; // 15 is larger than the largest RX FIFO trigger
            }

            switch (rstrg)
            {
                default:
                    fprintf(stderr, "RTS Trigger number must be 1, 4, 6, 8, 10, 12, 14, 15.\nRTS trigger will be set to 15.\n");
                    /* Fall through */
                case 15:
                    rstrg_val = SCIF_SCFCR_RSTRG_15;
                    break;
                case 14:
                    rstrg_val = SCIF_SCFCR_RSTRG_14;
                    break;
                case 12:
                    rstrg_val = SCIF_SCFCR_RSTRG_12;
                    break;
                case 10:
                    rstrg_val = SCIF_SCFCR_RSTRG_10;
                    break;
                case 8:
                    rstrg_val = SCIF_SCFCR_RSTRG_8;
                    break;
                case 6:
                    rstrg_val = SCIF_SCFCR_RSTRG_6;
                    break;
                case 4:
                    rstrg_val = SCIF_SCFCR_RSTRG_4;
                    break;
                case 1:
                    rstrg_val = SCIF_SCFCR_RSTRG_1;
                    break;
            }
            break;
        case UART_TYPE_HSCIF:
            break;
        default:
            rstrg_val = 0;
            break;
    }
    return (rstrg_val);
}

static int get_fifo(unsigned fifo_tx, unsigned fifo_rx, int scif)
{
    int fifo = 0;

    switch (scif)
    {
        case UART_TYPE_SCIF:
            switch (fifo_rx)
            {
                case 14:
                    fifo = SCIF_SCFCR_RTRG_E;
                    break;
                case 8:
                    fifo = SCIF_SCFCR_RTRG_8;
                    break;
                case 4:
                    fifo = SCIF_SCFCR_RTRG_4;
                    break;
                default:
                    fprintf(stderr,"RX FIFO Trigger number must be 1, 4, 8 or 14. \nRx trigger will be set to 1.\n");
                    /* Fall through */
                case 1:
                    fifo = SCIF_SCFCR_RTRG_1;
                    break;
            }

            switch (fifo_tx)
            {
                case 8:
                    fifo |= SCIF_SCFCR_TTRG_8;
                    break;
                case 4:
                    fifo |= SCIF_SCFCR_TTRG_4;
                    break;
                case 2:
                    fifo |= SCIF_SCFCR_TTRG_2;
                    break;
                default:
                    fprintf(stderr,"TX FIFO Trigger number must be 0, 2, 4, or 8. \nTx trigger will be set to 0.\n");
                    /* Fall through */
                case 0:
                    fifo |= SCIF_SCFCR_TTRG_0;
                    break;
            }
            break;
        case UART_TYPE_HSCIF:
            break;
        default:
            break;
    }
    return(fifo);
}

void log_settings(DEV_SCIF *dev, TTYINIT devinit)
{
    slogf(_SLOG_SETCODE(_SLOGC_CHAR, 0), _SLOG_INFO, "Port .......................... %s (0x%x)", dev->tty.name, (uint32_t)devinit.port);
    slogf(_SLOG_SETCODE(_SLOGC_CHAR, 0), _SLOG_INFO, "IRQ ........................... 0x%x", devinit.intr);
    slogf(_SLOG_SETCODE(_SLOGC_CHAR, 0), _SLOG_INFO, "Input Clock ................... %d", dev->clk);
    slogf(_SLOG_SETCODE(_SLOGC_CHAR, 0), _SLOG_INFO, "Clock Divisor ................. %d", dev->div);
    slogf(_SLOG_SETCODE(_SLOGC_CHAR, 0), _SLOG_INFO, "Tx fifo size .................. %d", (dev->tty.fifo >> 4));
    slogf(_SLOG_SETCODE(_SLOGC_CHAR, 0), _SLOG_INFO, "Rx fifo trigger ............... %d", (dev->tty.fifo & 0x0f));
    slogf(_SLOG_SETCODE(_SLOGC_CHAR, 0), _SLOG_INFO, "Input buffer size ............. %d", dev->tty.ibuf.size);
    slogf(_SLOG_SETCODE(_SLOGC_CHAR, 0), _SLOG_INFO, "Input flow control highwater .. %d", dev->tty.highwater);
    slogf(_SLOG_SETCODE(_SLOGC_CHAR, 0), _SLOG_INFO, "Output buffer size ............ %d", dev->tty.obuf.size);
    slogf(_SLOG_SETCODE(_SLOGC_CHAR, 0), _SLOG_INFO, "Canonical buffer size ......... %d\n", dev->tty.cbuf.size);
}

unsigned options(int argc, char *argv[])
{
    int opt, numports = 0;
    char *cp;
    unsigned unit = 1;  // dev name prefix
    unsigned fifo_tx = 0;
    unsigned fifo_rx = 0;
    unsigned rstrg = 0;

    unit = 1;
    static TTYINIT_SCIF devinit = {
        {
            R_CAR_M3_SCIF_BASE2,       // Port
            0,                         // Port shift
            RCAR_INTCSYS_SCIF2,        // Interrupt address
            38400, 2048, 2048, 256,    // Baud, isize, osize, csize
            0, 0, 0, 0,                // cflag, iflag, lflag, oflag
            0, 0, 0,                   // fifo, clk, div
            "/dev/ser",                // Name
            NULL,                      // reserved1
            0,                         // reserved2
            0                          // verbose
        },
        0,                           // rstrg
        0,                           // scif
        RTS_CTS_HW_ENABLE,           // rts_hw_disable
        INTERNAL_CLOCK,              // Internal clock
    };

    // Initialize the devinit to raw mode
    ttc(TTC_INIT_RAW, &devinit, 0);

    while (optind < argc) {
        // Process dash options.
        while ((opt = getopt(argc, argv, IO_CHAR_SERIAL_OPTIONS "u:c:T:t:r:hHx")) != -1) {
            switch(ttc(TTC_SET_OPTION, &devinit, opt)) {
                case 't':
                    // (Wait to process until we know if we are SCIF, HSCIF)
                    fifo_rx = atoi(optarg);
                    break;
                case 'T':
                    // (Wait to process until we know if we are SCIF, HSCIF)
                    fifo_tx = atoi(optarg);
                    break;
                case 'h':
                    // Enable the RTS/CTS lines to be used as RS-232 lines
                    devinit.rts_hw_disable = RTS_CTS_HW_ENABLE;
                    break;
                case 'H':
                    // Disable the RTS/CTS lines to be used as RS-232 lines - use as GPIO's instead
                    devinit.rts_hw_disable = RTS_CTS_HW_DISABLE;
                    break;
                case 'x':
                    // Use external clock
                    devinit.clock_source = EXTERNAL_CLOCK;
                    break;
                case 'r':
                    // Read in the RTS trigger value
                    rstrg = atoi(optarg);
                    break;
                case 'u':       // Where to start device name at (def 1)
                    unit = atoi(optarg);
                    break;
                case 'c':       //clk/div value for baud rate
                    devinit.tty.clk = atol(optarg);
                    if ((cp = strchr(optarg, '/')))
                        devinit.tty.div = atoi(cp + 1);
                    break;
            }
        }

        // Don't allow the RTS/CTS lines to be disabled if HW Flow control option is selected
        if (devinit.tty.c_cflag & IHFLOW || devinit.tty.c_cflag & OHFLOW)
            devinit.rts_hw_disable = RTS_CTS_HW_ENABLE;

        // Custom ports and interrupts
        while (optind < argc && *(optarg = argv[optind]) != '-')
        {
            char *p;

            if (strlen(optarg) < 3)
            {
                fprintf(stderr, "Illegal option, port must scif# or hscif# \n\r");
                exit(0);
            }

            if ((p = strstr(optarg, "hscif")) != NULL)
            {
                /* Initialize based on device type */
                devinit.tty.port_shift = 0;
                if (devinit.tty.clk == 0)
                {
                    devinit.tty.clk = 133333333;
                }

                switch( p[5] )
                {
                    case '0':
                        devinit.tty.port = R_CAR_M3_HSCIF_BASE0;
                        devinit.tty.intr = RCAR_INTCSYS_HSCIF0;
                        devinit.scif = UART_TYPE_HSCIF;
                        break;
                    case '1':
                        devinit.tty.port = R_CAR_M3_HSCIF_BASE1;
                        devinit.tty.intr = RCAR_INTCSYS_HSCIF1;
                        devinit.scif = UART_TYPE_HSCIF;
                        break;
                    case '2':
                        devinit.tty.port = R_CAR_M3_HSCIF_BASE2;
                        devinit.tty.intr = RCAR_INTCSYS_HSCIF2;
                        devinit.scif = UART_TYPE_HSCIF;
                        break;
                    default:
                        fprintf(stderr, "Unsupported HSCIF channel.\n");
                        exit(0);
                        break;
                }
            }
            else if ((p = strstr(optarg, "scif")) != NULL)
            {
                /* Initialize based on device type */
                devinit.tty.port_shift = 0;
                if (devinit.tty.clk == 0)
                {
                    devinit.tty.clk = 33333333;
                    devinit.tty.div = 16;
                }
                if (fifo_rx == 0)
                    fifo_rx = 14;

                switch( p[4] )
                {
                    case '0':
                        devinit.tty.port = R_CAR_M3_SCIF_BASE0;
                        devinit.tty.intr = RCAR_INTCSYS_SCIF0;
                        devinit.scif = UART_TYPE_SCIF;
                        break;
                    case '1':
                        devinit.tty.port = R_CAR_M3_SCIF_BASE1;
                        devinit.tty.intr = RCAR_INTCSYS_SCIF1;
                        devinit.scif = UART_TYPE_SCIF;
                        break;
                    case '2':
                        devinit.tty.port = R_CAR_M3_SCIF_BASE2;
                        devinit.tty.intr = RCAR_INTCSYS_SCIF2;
                        devinit.scif = UART_TYPE_SCIF;
                        break;
                    default:
                        fprintf(stderr, "Unsupported SCIF channel.\n");
                        exit(0);
                        break;
                }
            }
            else
            {
                fprintf(stderr, "Illegal option, port must be scif or hscif\n\r");
                exit(0);
            }

            // Setup the values to program the FIFO
            devinit.tty.fifo = get_fifo(fifo_tx, fifo_rx, devinit.scif);
            devinit.rstrg = get_rstrg(rstrg, devinit.scif, fifo_rx);
            create_device(&devinit, unit++);
            ++numports;
            ++optind;
        }
    }

    if (numports == 0)
    {
        if (devinit.tty.verbose)
            fprintf(stderr, "Creating device %s%d\n", devinit.tty.name, unit);

        // Setup the values to program the FIFO
        devinit.tty.fifo = get_fifo(fifo_tx, fifo_rx, devinit.scif);
        devinit.rstrg = get_rstrg(rstrg, devinit.scif, fifo_rx);
        create_device(&devinit, unit++);
        ++numports;
        ++optind;
    }
    return numports;
}

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/hardware/devc/serscif/options.c $ $Rev: 808962 $")
#endif
