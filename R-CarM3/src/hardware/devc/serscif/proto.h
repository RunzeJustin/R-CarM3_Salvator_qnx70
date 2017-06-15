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

#ifndef PROTO_H
#define PROTO_H

/* init.c */
void set_port8(unsigned port, uint8_t mask, uint8_t data);
void set_port16(unsigned port, uint16_t mask, uint16_t data);
void set_port32(unsigned port, uint32_t mask, uint32_t data);
void create_device(TTYINIT_SCIF *dip, unsigned unit);
void disable_uart(void);

/* options.c */
unsigned options(int argc, char *argv[]);

/* intr.c */
const struct sigevent * ser_intr(void *arg, int iid);

/* tto.c */
void ser_stty_scif(DEV_SCIF *dev);

#endif /* PROTO_H */

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/hardware/devc/serscif/proto.h $ $Rev: 805958 $")
#endif
