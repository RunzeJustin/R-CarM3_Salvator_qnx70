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

#ifndef _R_Car_RSRC_H
#define _R_Car_RSRC_H

#include <stdint.h>
#include <stdbool.h>
#include <errno.h>

int rcar_create_resources(void);

/* reserve specified serial sound interface (SSI) channel range */
int rcar_reserve_ssi( uint32_t min_ssi_channel, uint32_t max_ssi_channel );

/* release specified serial sound interface (SSI) channel range */
int rcar_release_ssi( uint32_t min_ssi_channel, uint32_t max_ssi_channel );

/* reserve a sample rate convertor (SRC) with specified features */
int rcar_reserve_src( uint32_t multichannel, uint32_t highsound, uint32_t is_inline, uint32_t* src_channel );

/* release specified sample rate convertor (SRC) */
int rcar_release_src( uint32_t src_channel );

/* reserve a CTU-MIX-DVC (CMD) instance (CTU=Channel Transfer Unit, MIX=Mixer, DVC=Digital Volume Control) */
int rcar_reserve_cmd( uint32_t* cmd_channel );

/* release specified CTU-MIX-DVC (CMD) instance */
int rcar_release_cmd( uint32_t cmd_channel );

/* reserve a MediaLB+ Local Memory (MLM) instance */
int rcar_reserve_mlm( uint32_t* mlm_channel );

/* release specified MediaLB+ Local Memory (MLM) instance */
int rcar_release_mlm( uint32_t mlm_channel );

/* reserve a Digital Transmission Content Protection (DTCP) instance */
int rcar_reserve_dtcp( uint32_t* dtcp );

/* release specified Digital Transmission Content Protection (DTCP) instance */
int rcar_release_dtcp( uint32_t dtcp );

/* DMAC channel reservation is covered in lib/dma/rcar, doesn't need to be covered here */

#endif /* _R_Car_RSRC_H */

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/hardware/deva/ctrl/rcar/rcar_rsrc.h $ $Rev: 804482 $")
#endif
