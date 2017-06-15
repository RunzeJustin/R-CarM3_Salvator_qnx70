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

#include "proto.h"

int
i2c_master_getfuncs(i2c_master_funcs_t *funcs, int tabsize)

{
    I2C_ADD_FUNC(i2c_master_funcs_t, funcs, init, rcar_i2c_init, tabsize);
    I2C_ADD_FUNC(i2c_master_funcs_t, funcs, fini, rcar_i2c_fini, tabsize);
    I2C_ADD_FUNC(i2c_master_funcs_t, funcs, send, rcar_i2c_send, tabsize);
    I2C_ADD_FUNC(i2c_master_funcs_t, funcs, recv, rcar_i2c_recv, tabsize);
    I2C_ADD_FUNC(i2c_master_funcs_t, funcs, set_slave_addr, rcar_i2c_set_slave_addr, tabsize);
    I2C_ADD_FUNC(i2c_master_funcs_t, funcs, set_bus_speed, rcar_i2c_set_bus_speed, tabsize);
    I2C_ADD_FUNC(i2c_master_funcs_t, funcs, version_info, rcar_i2c_version_info, tabsize);
    I2C_ADD_FUNC(i2c_master_funcs_t, funcs, driver_info, rcar_i2c_driver_info, tabsize);

    return 0;
}

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/hardware/i2c/rcar-B/lib.c $ $Rev: 805407 $")
#endif
