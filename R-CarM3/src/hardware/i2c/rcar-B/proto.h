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
#include <unistd.h>
#include <stdint.h>
#include <sys/neutrino.h>
#include <hw/inout.h>
#include <hw/i2c.h>
#include <arm/r-car-m3.h>

#define CMODE_SEND          0x00000001
#define CMODE_RECV          0x00000002
#define CMODE_DOSTOP        0x00000004
#define CMODE_SADDR_L       0x00000008
#define CMODE_SADDR_H       0x00000010
#define CMODE_RECV_ONE      0x00000020
#define CMODE_RECV_TWO      0x00000040
#define CMODE_WAIT          0x00000080
#define CMODE_STOPRECV      0x00000100
#define CMODE_NORMAL_SPEED  0x00000200

#define I2C_BUSY            0x80000000

#define MAX_POOL            0x500

typedef struct {
    uintptr_t       regbase;
    unsigned        physbase;

    uint32_t        pck;        /* peripheral clock */
    uint32_t        slave_addr;
    i2c_addrfmt_t   slave_addr_fmt;
    struct sigevent intrevent;
    int             irq;
    int             iid;

    uint8_t         *buf;
    int             len;
    int             bytes;
    uint32_t        mode;
    uint32_t        iccl;
    uint32_t        icch;
    uint32_t        status;
    int             max_ints;
    int32_t         clockLow;
    int32_t         clockHigh;
    uint32_t        speed;
} rcar_i2c_dev_t;

uint32_t rcar_i2c_poll_busy(rcar_i2c_dev_t *dev, uint8_t mask, uint8_t bit);
uint32_t rcar_i2c_poll_complete(rcar_i2c_dev_t *dev);
int rcar_i2c_options(rcar_i2c_dev_t *dev, int argc, char *argv[]);

void *rcar_i2c_init(int argc, char *argv[]);
void rcar_i2c_fini(void *hdl);

int rcar_i2c_set_slave_addr(void *hdl, unsigned int addr, i2c_addrfmt_t fmt);
int rcar_i2c_set_bus_speed(void *hdl, unsigned int speed, unsigned int *ospeed);

i2c_status_t rcar_i2c_recv(void *hdl, void *buf, unsigned int len, unsigned int stop);
i2c_status_t rcar_i2c_send(void *hdl, void *buf, unsigned int len, unsigned int stop);

const struct sigevent * rcar_i2c_intr(void *area, int id);

int rcar_i2c_version_info(i2c_libversion_t *version);
int rcar_i2c_driver_info(void *hdl, i2c_driver_info_t *info);

#endif

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/hardware/i2c/rcar-B/proto.h $ $Rev: 805407 $")
#endif
