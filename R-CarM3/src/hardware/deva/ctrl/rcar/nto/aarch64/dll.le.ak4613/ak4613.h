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

#ifndef _R_Car_AK4613_H
#define _R_Car_AK4613_H

#include <stdint.h>

#define AK4613_MAX_DIGITAL_VOL          254

int ak4613_open_i2c_fd( void );
int ak4613_close_i2c_fd( void );
int ak4613_reset( void );
int ak4613_output_vol_get( uint8_t* left_vol, uint8_t* right_vol );
int ak4613_output_vol_set( uint8_t left_vol, uint8_t right_vol );
int ak4613_rate_setting( uint32_t rate );
void ak4613_register_dump( void );

#endif /* _R_Car_AK4613_H */

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/hardware/deva/ctrl/rcar/nto/aarch64/dll.le.ak4613/ak4613.h $ $Rev: 812929 $")
#endif
