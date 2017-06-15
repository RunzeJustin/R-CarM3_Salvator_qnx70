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

#ifndef _PROTO_H_INCLUDED
#define _PROTO_H_INCLUDED

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <stdint.h>
#include <sys/neutrino.h>
#include <hw/inout.h>
#include <hw/i2c.h>
#include <sys/slog.h>
#include <sys/slogcodes.h>
#include <arm/r-car-m3.h>
#include <unistd.h>

#define	CMODE_SEND          0x00000001
#define	CMODE_RECV          0x00000002
#define	CMODE_DOSTOP        0x00000010

#define	VERBOSE_QUIET       0
#define	VERBOSE_LEVEL1      1
#define	VERBOSE_LEVEL2      2
#define	VERBOSE_LEVEL4      4
#define	VERBOSE_LEVEL8      8

#define I2C_DEFAULT_PCLK    50000000
#define I2C_DEFAULT_BAUD    50000

extern int verbose;

#define rcar_i2c_slogf(dev, vl, msg, ...) \
            if (vl == VERBOSE_QUIET) \
                slogf(_SLOG_SETCODE(_SLOGC_CHAR, 0), _SLOG_ERROR, msg, ##__VA_ARGS__); \
            else if (vl <= dev->verbose) \
                slogf(_SLOG_SETCODE(_SLOGC_CHAR, 0), _SLOG_INFO, msg, ##__VA_ARGS__)

typedef struct {
    uintptr_t           regbase;
    unsigned            physbase;

    uint32_t            busSpeed;
    unsigned            pck;		/* peripheral clock */
    unsigned            scl_freq;
    unsigned long       scl_period;	/* in ns */

    unsigned            slave_addr;
    i2c_addrfmt_t       slave_addr_fmt;

    int                 irq;
    int                 intr;

    uint8_t             *buf;
    int                 len;
    unsigned            mode;
    unsigned            status;
    int                 verbose;

    struct sigevent     intrEvent;
} rcar_i2c_dev_t;

const struct sigevent* rcar_i2c_intr(void* arg);
int rcar_i2c_wait_complete(rcar_i2c_dev_t *dev);
int rcar_i2c_parse_options(rcar_i2c_dev_t *dev, int argc, char *argv[]);
void *rcar_i2c_init(int argc, char *argv[]);
void rcar_i2c_fini(void *hdl);
void rcar_i2c_reset(rcar_i2c_dev_t *dev);
int rcar_i2c_set_slave_addr(void *hdl, unsigned int addr, i2c_addrfmt_t fmt);
int rcar_i2c_set_bus_speed(void *hdl, unsigned int speed, unsigned int *ospeed);
int rcar_i2c_version_info(i2c_libversion_t *version);
int rcar_i2c_driver_info(void *hdl, i2c_driver_info_t *info);
i2c_status_t rcar_i2c_recv(void *hdl, void *buf, unsigned int len, unsigned int stop);
i2c_status_t rcar_i2c_send(void *hdl, void *buf, unsigned int len, unsigned int stop);

#endif /* _PROTO_H_INCLUDED */

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/hardware/i2c/rcar-A/proto.h $ $Rev: 805407 $")
#endif
