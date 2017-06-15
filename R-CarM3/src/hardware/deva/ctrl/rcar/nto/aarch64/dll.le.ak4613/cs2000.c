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

#include <audio_driver.h>

#include <stdint.h>
#include <errno.h>
#include "i2c2.h"

#define CS2000_I2C_SLAVE_ADDR           0x4F
#define CS2000_DEVICE_ID                0x01
#define CS2000_DEVICE_CONTROL           0x02
#define CS2000_DEVICE_CFG1              0x03
#define CS2000_DEVICE_CFG2              0x04
#define CS2000_GLOBAL_CFG               0x05
#define CS2000_FUNC_CFG1                0x16
#define CS2000_RATIO_0_0                0x06
#define CS2000_RATIO_0_1                0x07
#define CS2000_RATIO_0_2                0x08
#define CS2000_RATIO_0_3                0x09

void cs2000_dump()
{
    uint8_t data;

    if(EOK == i2c_read(CS2000_I2C_SLAVE_ADDR, CS2000_DEVICE_CONTROL, &data)) {
        ado_debug (DB_LVL_DRIVER, "cs2000: CS2000_DEVICE_CONTROL 0x%x", data);
    }
    if(EOK == i2c_read(CS2000_I2C_SLAVE_ADDR, CS2000_DEVICE_CFG1, &data)) {
        ado_debug (DB_LVL_DRIVER, "cs2000: CS2000_DEVICE_CFG1 0x%x", data);
    }
    if(EOK == i2c_read(CS2000_I2C_SLAVE_ADDR, CS2000_DEVICE_CFG2, &data)) {
        ado_debug (DB_LVL_DRIVER, "cs2000: CS2000_DEVICE_CFG2 0x%x", data);
    }
    if(EOK == i2c_read(CS2000_I2C_SLAVE_ADDR, CS2000_GLOBAL_CFG, &data)) {
        ado_debug (DB_LVL_DRIVER, "cs2000: CS2000_GLOBAL_CFG 0x%x", data);
    }
    if(EOK == i2c_read(CS2000_I2C_SLAVE_ADDR, CS2000_FUNC_CFG1, &data)) {
        ado_debug (DB_LVL_DRIVER, "cs2000: CS2000_FUNC_CFG1 0x%x", data);
    }
    if(EOK == i2c_read(CS2000_I2C_SLAVE_ADDR, CS2000_RATIO_0_0, &data)) {
        ado_debug (DB_LVL_DRIVER, "cs2000: CS2000_RATIO_0_0 0x%x", data);
    }
    if(EOK == i2c_read(CS2000_I2C_SLAVE_ADDR, CS2000_RATIO_0_1, &data)) {
        ado_debug (DB_LVL_DRIVER, "cs2000: CS2000_RATIO_0_1 0x%x", data);
    }
    if(EOK == i2c_read(CS2000_I2C_SLAVE_ADDR, CS2000_RATIO_0_2, &data)) {
        ado_debug (DB_LVL_DRIVER, "cs2000: CS2000_RATIO_0_2 0x%x", data);
    }
    if(EOK == i2c_read(CS2000_I2C_SLAVE_ADDR, CS2000_RATIO_0_3, &data)) {
        ado_debug (DB_LVL_DRIVER, "cs2000: CS2000_RATIO_0_3 0x%x", data);
    }
}

void cs2000_set(uint8_t enable)
{
    uint8_t data;

    if(EOK != i2c_devrdy())
        return;

    // Device configuration freeze
    if(EOK == i2c_read(CS2000_I2C_SLAVE_ADDR, CS2000_GLOBAL_CFG, &data)) {
        data &= ~0x8;
        data |= 0x8;
        i2c_write(CS2000_I2C_SLAVE_ADDR, CS2000_GLOBAL_CFG, data);
    }

    // EnDevCfg1: enable device configuration
    if(EOK == i2c_read(CS2000_I2C_SLAVE_ADDR, CS2000_DEVICE_CFG1, &data)) {
        data &= ~0x1;
        if(enable) data |= 0x1;
        i2c_write(CS2000_I2C_SLAVE_ADDR, CS2000_DEVICE_CFG1, data);
    }

    // EnDevCfg2: enable device configuration
    if(EOK == i2c_read(CS2000_I2C_SLAVE_ADDR, CS2000_GLOBAL_CFG, &data)) {
        data &= ~0x1;
        if(enable) data |= 0x1;
        i2c_write(CS2000_I2C_SLAVE_ADDR, CS2000_GLOBAL_CFG, data);
    }

    if(enable) {
        // RefClkDiv: set divider to 2 for REF_CLK 16 MHz to 28 MHz
        if(EOK == i2c_read(CS2000_I2C_SLAVE_ADDR, CS2000_FUNC_CFG1, &data)) {
            data &= ~0x8;
            data |= 0x8;
            i2c_write(CS2000_I2C_SLAVE_ADDR, CS2000_FUNC_CFG1, data);
        }

        // FracNSrc: Use static Fractional-N Ratio
        if(EOK == i2c_read(CS2000_I2C_SLAVE_ADDR, CS2000_DEVICE_CFG2, &data)) {
            data &= ~0x1;
            i2c_write(CS2000_I2C_SLAVE_ADDR, CS2000_DEVICE_CFG2, data);
        }

        // Ratio 0: Set Ratio to 1
        if(EOK == i2c_read(CS2000_I2C_SLAVE_ADDR, CS2000_RATIO_0_1, &data)) {
            data &= ~0x10;
            data |= 0x10;
            i2c_write(CS2000_I2C_SLAVE_ADDR, CS2000_RATIO_0_1, data);
        }
    }

    // Device configuration unfreeze
    if(EOK == i2c_read(CS2000_I2C_SLAVE_ADDR, CS2000_GLOBAL_CFG, &data)) {
        data &= ~0x8;
        i2c_write(CS2000_I2C_SLAVE_ADDR, CS2000_GLOBAL_CFG, data);
    }
}

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/hardware/deva/ctrl/rcar/nto/aarch64/dll.le.ak4613/cs2000.c $ $Rev: 812929 $")
#endif
