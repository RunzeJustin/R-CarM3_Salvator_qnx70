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


#ifdef DEFN
    #define EXT
    #define INIT1(a)            = { a }
#else
    #define EXT extern
    #define INIT1(a)
#endif

#include <time.h>
#include <stdio.h>
#include <ctype.h>
#include <errno.h>
#include <signal.h>
#include <malloc.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/neutrino.h>
#include <termios.h>
#include <devctl.h>
#include <sys/dcmd_chr.h>
#include <sys/iomsg.h>
#include <atomic.h>
#include <hw/inout.h>
#include <sys/io-char.h>
#include <sys/slog.h>
#include <sys/slogcodes.h>
#include <arm/scif.h>

#define UART_TYPE_SCIF          1
#define UART_TYPE_HSCIF         2

#define RTS_CTS_HW_DISABLE      0
#define RTS_CTS_HW_ENABLE       1

typedef struct dev_scif {
    TTYDEV          tty;
    struct dev_scif *next;
    unsigned        sh_intr;
    unsigned        clk;            //Our clock cycle
    unsigned        div;            //Our div value
    uintptr_t       port;           //Our port base value
    int             scif;           // > 0 if we are an scif type (scif - 1 for SH7750, scif = 2 for SH7760)
    uint16_t        rstrg;          //RTS trigger value for SCIF FCR register
    volatile int    rts_flag;       //1 if hw flow control should be on
    uint32_t        baud;           // store the last baud rate. Current baudrate is stored in tty.baud
    uint32_t        c_cflag;        // store the last serial mode. Current c_cflag is stored in tty.c_cflag
    uint8_t         clock_source;   // Clock source
    uint8_t         rts_hw_disable; //Flag to disable the RTS and CTS lines
    uint16_t        rx_intr_mask;
    /* Register offsets */
    uint8_t         tx_reg;
    uint8_t         rx_reg;
    uint8_t         sc_reg;
    uint8_t         fc_reg;
    uint8_t         status_reg;
    uint8_t         tx_fifo_count_reg;
    uint8_t         rx_fifo_count_reg;
} DEV_SCIF;

struct dev_list {
    struct dev_list *next;
    DEV_SCIF        *device;
    int             iid;
};

typedef struct ttyinit_scif {
    TTYINIT     tty;
    int         scif;
    uint16_t    rstrg;              //RTS trigger value for SCIF FCR register
    uint8_t     rts_hw_disable;     //Flag to disable the RTS and CTS lines
#define  INTERNAL_CLOCK 0
#define  EXTERNAL_CLOCK 1
    uint8_t     clock_source;       //Clock Source
} TTYINIT_SCIF;

EXT TTYCTRL          ttyctrl;
EXT struct dev_list  *devices;

#include "proto.h"

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/hardware/devc/serscif/externs.h $ $Rev: 805958 $")
#endif
