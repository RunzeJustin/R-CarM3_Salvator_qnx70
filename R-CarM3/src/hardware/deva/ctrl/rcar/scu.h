/*
 * $QNXLicenseC:
 * Copyright 2014, 2016 QNX Software Systems.
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

#ifndef _R_Car_SCU_H
#define _R_Car_SCU_H

#include <sys/types.h>
#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/neutrino.h>
#include <hw/inout.h>
#include <arm/r-car-m3.h>

#define SCU_SRC_CHANNEL_NUM     9
#define SCU_CMD_CHANNEL_NUM     2

/* SCU level functionality */
int scu_init();
void scu_deinit();

/* SRC level functionality */
int scu_src_reset( uint32_t src_channel );
int scu_src_setup( uint32_t src_channel,
                   uint32_t in_sync,
                   uint32_t out_sync,
                   uint32_t bitnum,
                   uint32_t voicenum,
                   uint32_t freq_in,
                   uint32_t freq_out);
int scu_src_cleanup(uint32_t src_channel);
int scu_src_start(uint32_t src_channel);
int scu_src_stop(uint32_t src_channel);

/* DVC level functionality */
int scu_dvc_reset( uint32_t dvc_channel );
int scu_dvc_setup( uint32_t dvc_channel,
                   uint32_t bitnum,
                   uint32_t voicenum,
                   uint32_t *vol );
int scu_dvc_cleanup( uint32_t dvc_channel );
int scu_dvc_get_vol( uint32_t dvc_channel, uint32_t voice_channel, uint32_t * vol );
int scu_dvc_set_vol( uint32_t dvc_channel, uint32_t voice_channel, uint32_t vol );

/* CMD level functionality */
int scu_cmd_setup( uint32_t cmd_channel, uint32_t src_channel );
int scu_cmd_start( uint32_t cmd_channel );
int scu_cmd_stop( uint32_t cmd_channel );

/* register dump functionality */
void scu_src_register_dump( uint32_t src_channel );
void scu_cmd_register_dump( uint32_t cmd_channel );
void scu_dvc_register_dump( uint32_t dvc_channel );

#endif /* _R_Car_SCU_H */

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/hardware/deva/ctrl/rcar/scu.h $ $Rev: 812827 $")
#endif
