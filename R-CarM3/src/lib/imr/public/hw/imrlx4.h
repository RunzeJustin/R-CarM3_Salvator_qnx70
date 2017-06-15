/*
* $QNXLicenseC:
* Copyright 2014, QNX Software Systems.
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

#ifndef	_IMRLX4_H_INCLUDED
#define	_IMRLX4_H_INCLUDED

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/neutrino.h>
#include <hw/inout.h>
#include <sys/resmgr.h>
#include <sys/iofunc.h>
#include <sys/dispatch.h>
#include <errno.h>
#include <string.h>
#include <atomic.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <inttypes.h>

#define RCAR_IMRLX4_PULSE 			58
#define RCAR_IMRLX4_END				59

typedef struct _correct_conf {
	int min;
	int max;
	int scal;
	int offs;
} correct_conf_t;

typedef struct _img_conf {
	int squareW;
	int squareH;
	correct_conf_t Y;
	correct_conf_t U;
	correct_conf_t V;
	uint32_t *coords;
	int def_set;
} img_conf_t;

typedef struct _img_info {
	int hcoid;
	int pulse;
	int channel;
	uint32_t cx;
	uint32_t cy;
	uint32_t dw;
	uint32_t dh;
	uint32_t bpp;
} img_info_t;

typedef struct _rcar_imr {
	int iid;
	int tid;
	int	chid;
	int	coid;
	int irq;
	pthread_attr_t attr;
	struct sched_param param;
	struct sigevent event;
	img_info_t img;
	img_conf_t img_conf;
	uintptr_t pbase;
	uintptr_t vbase;
} rcar_imr_t;

paddr_t rcar_imrlx4_mphys(void *addr);
void rcar_imrlx4_setup();
void rcar_imrlx4_enable_clock();
void rcar_imrlx4_configure();
void rcar_imrlx4_DL_init();
void rcar_imrlx4_update_frame(paddr_t source, paddr_t dest);
void *rcar_imrlx4_event_handler(void *data);
int rcar_imrlx4_create_thread();
int rcar_imrlx4_init(img_info_t img);
int rcar_imrlx4_fini();
int parse_device_config(rcar_imr_t *imr);
int get_config_data(rcar_imr_t *imr, const char *filename);
int parse_config (rcar_imr_t *imr, char *opt, int line);

#endif
