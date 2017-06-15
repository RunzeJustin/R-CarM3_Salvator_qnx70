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

struct ak4613_context;
#define MIXER_CONTEXT_T struct ak4613_context

#include <audio_driver.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <devctl.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/neutrino.h>
#include <hw/inout.h>
#include <sys/mman.h>
#include "scu.h"
#include "ak4613.h"
#include "i2c2.h"

/* ak4613 registers */
#define AK4613_POWER_MANAGEMENT1        0x00
#define AK4613_POWER_MANAGEMENT2        0x01
#define AK4613_POWER_MANAGEMENT3        0x02
#define AK4613_CONTROL1                 0x03
 #define AK4613_CONTROL1_TDM512         (1 << 6)
 #define AK4613_CONTROL1_TDM256         (2 << 6)
 #define AK4613_CONTROL1_TDM128         (3 << 6)
#define AK4613_CONTROL2                 0x04
#define AK4613_DE_EMPHASIS1             0x05
#define AK4613_DE_EMPHASIS2             0x06
#define AK4613_OVERFLOW_DETECT          0x07
#define AK4613_ZERO_DETECT              0x08
#define AK4613_INPUT_CONTROL            0x09
#define AK4613_OUTPUT_CONTROL           0x0A
#define AK4613_LOUT1_VOLUME_CONTROL     0x0B
#define AK4613_ROUT1_VOLUME_CONTROL     0x0C
#define AK4613_LOUT2_VOLUME_CONTROL     0x0D
#define AK4613_ROUT2_VOLUME_CONTROL     0x0E
#define AK4613_LOUT3_VOLUME_CONTROL     0x0F
#define AK4613_ROUT3_VOLUME_CONTROL     0x10
#define AK4613_LOUT4_VOLUME_CONTROL     0x11
#define AK4613_ROUT4_VOLUME_CONTROL     0x12
#define AK4613_LOUT5_VOLUME_CONTROL     0x13
#define AK4613_ROUT5_VOLUME_CONTROL     0x14
#define AK4613_LOUT6_VOLUME_CONTROL     0x15
#define AK4613_ROUT6_VOLUME_CONTROL     0x16

#define AK4613_I2C_SLAVE_ADDR           0x10

int ak4613_open_i2c_fd(void)
{
    int status = open_i2c_fd();

    if(status != EOK)
        return status;

    return EOK;
}

int ak4613_close_i2c_fd(void)
{
    return close_i2c_fd();
}

int ak4613_reset( void )
{
    /* For both playback and capture */

    /*
     * (0x03) Control 1 (default=0x20)
     *  [7:6] TDM1-0 =00    TDM Format Select                  (00:Stereo, 01:TDM512, 10:TDM256, 11:TDM128)
     *  [5:3] DIF2-0 =100   Audio Data Interface Modes         (100: 24bit I2S)
     *  [2:1] ATS1-0 =00    Digital attenuator transition time (00:4096/fs, 01:2048/fs, 10:512/fs, 11:256/fs)
     *  [0]   SMUTE  =0     Soft Mute Enable                   (0: Normal operation, 1: All DAC outputs soft-muted)
     */
    i2c_write(AK4613_I2C_SLAVE_ADDR, AK4613_CONTROL1, 0x20);

    /* wait 4096/fs --> 4096 * 1/32kHz = 128ms */
    delay(200);  // wait 200ms

    /* (0x00) Power Management 1 (default=0x0F)
     *  [7:4] fixed =0
     *  [3]   PMVR  =1      Power management of reference voltage (1: Normal operation, 0: Power-down)
     *  [2]   PMADC =1      Power management of ADC               (1: Normal operation, 0: Power-down)
     *  [1]   PMDAC =1      Power management of DAC               (1: Normal operation, 0: Power-down)
     *  [0]   RSTN  =1      Internal timing reset                 (1: Normal operation, 0: Reset, but registers are not initialized)
     */
    i2c_write(AK4613_I2C_SLAVE_ADDR, AK4613_POWER_MANAGEMENT1, 0x0F );

    /* (0x01) Power Management 2 (default=0x07)
     *  [7:3] fixed =0
     *  [2]   fixed =1
     *  [1]   PMAD2 =0      Power management of ADC2 (0: Power-down, 1: Normal operation)
     *  [0]   PMAD1 =1      Power management of ADC1 (0: Power-down, 1: Normal operation)
     */
    i2c_write(AK4613_I2C_SLAVE_ADDR, AK4613_POWER_MANAGEMENT2, 0x01 );

    /* (0x02) Power Management 3 (default=0x3F)
     *  [7:6] fixed =0
     *  [5]   PMDA6 =0      Power management of DAC6 (0: Power-down, 1: Normal operation)
     *  [4]   PMDA5 =0      Power management of DAC5 (0: Power-down, 1: Normal operation)
     *  [3]   PMDA4 =0      Power management of DAC4 (0: Power-down, 1: Normal operation)
     *  [2]   PMDA3 =0      Power management of DAC3 (0: Power-down, 1: Normal operation)
     *  [1]   PMDA2 =0      Power management of DAC2 (0: Power-down, 1: Normal operation)
     *  [0]   PMDA1 =0      Power management of DAC1 (0: Power-down, 1: Normal operation)
     */
    i2c_write(AK4613_I2C_SLAVE_ADDR, AK4613_POWER_MANAGEMENT3, 0x01);

    /* (0x04) Control 2 (default=0x20)
     *  [7]   fixed  =0
     *  [6]   MCKO   =0     Master clock output enable          (0:Output"L", 1:Output "MCKO")
     *  [5:4] CKS1-0 =00    Master Clock Input Frequency Select (00:Normal Speed=256fs, Double Speed=256fs, Quad Speed=128fs)
     *  [3:2] DFS1-0 =00/01 Sampling speed mode                 (00:Normal Speed, 01:Double Speed, 10:Quad Speed, 11:N/A)
     *                      DFS is ignored at ACKS bit=1.        Normal Speed : 32kHz~48kHz,
     *                                                           Double Speed : 64kHz~96kHz
     *                                                           Quad Speed   : 128kHz~192kHz
     *  [1]   ACKS   =0     Master Clock Frequency Auto Setting (0:Disable, 1:Enable)
     *  [0]   DIV    =0     Output of Master clock frequency    (0:x1, 1:x1/2) --> do not use MCKO, then set default
     */
    i2c_write(AK4613_I2C_SLAVE_ADDR, AK4613_CONTROL2, 0x00 );  // Normal Speed Mode=32kHz~48kHz, Sampling speed=256fs

    /* (0x05) De-emphasis1 (default=0x55)
     *  [7:6] DEM41-0 =01   De-emphasis response control for DAC4 data on SDTI1 (01:OFF)
     *  [5:4] DEM31-0 =01   De-emphasis response control for DAC3 data on SDTI1 (01:OFF)
     *  [3:2] DEM21-0 =01   De-emphasis response control for DAC2 data on SDTI1 (01:OFF)
     *  [1:0] DEM11-0 =01   De-emphasis response control for DAC1 data on SDTI1 (01:OFF)
     */
    i2c_write(AK4613_I2C_SLAVE_ADDR, AK4613_DE_EMPHASIS1, 0x55 );

    /* (0x06) De-emphasis2 (default=0x05)
     *  [7:4] fixed   =0
     *  [3:2] DEM61-0 =01   De-emphasis response control for DAC6 data on SDTI1 (01:OFF)
     *  [1:0] DEM51-0 =01   De-emphasis response control for DAC5 data on SDTI1 (01:OFF)
     */
    i2c_write(AK4613_I2C_SLAVE_ADDR, AK4613_DE_EMPHASIS2, 0x05 );

    /* (0x07) Overflow Detect (default=0x07)
     *  [7:4] fixed   =0
     *  [3]   OVFE    =0    Overflow detection enable   (0:Disable, 1:Enable)
     *  [2:0] OVFM2-0 =111  Overflow detect mode select (111:Disable)
     */
    i2c_write(AK4613_I2C_SLAVE_ADDR, AK4613_OVERFLOW_DETECT, 0x07 );

    /* (0x08) Zero Detect (default=0x0F)
     *  [7:6] LOOP1-0 =00   Loopback mode enable (00:Normal, No loop back)
     *  [5:4] fixed   =0
     *  [3:0] DZFM3-0 =1111 Zero detect mode select (1111:Disable)
     */
    i2c_write(AK4613_I2C_SLAVE_ADDR, AK4613_ZERO_DETECT, 0x0F );

    /* (0x09) Output Control (default=0x07)
     *  [7:3] fixed =0
     *  [2]   fixed =1
     *  [1]   DIE2  =0      ADC2 Differential Input Enable (0:Single-End Input, 1:Differential Input)
     *  [0]   DIE1  =0      ADC1 Differential Input Enable (0:Single-End Input, 1:Differential Input)
     */
    i2c_write(AK4613_I2C_SLAVE_ADDR, AK4613_INPUT_CONTROL, 0x04 );

    /* (0x0A) Output Control (default=0x3F)
     *  [7:6] fixed =0
     *  [5]   DOE6  =0      DAC6 Differential Output Enable (0:Single-End Input, 1:Differential Input)
     *  [4]   DOE5  =0      DAC5 Differential Output Enable (0:Single-End Input, 1:Differential Input)
     *  [3]   DOE4  =0      DAC4 Differential Output Enable (0:Single-End Input, 1:Differential Input)
     *  [2]   DOE3  =0      DAC3 Differential Output Enable (0:Single-End Input, 1:Differential Input)
     *  [1]   DOE2  =0      DAC2 Differential Output Enable (0:Single-End Input, 1:Differential Input)
     *  [0]   DOE1  =1      DAC1 Differential Output Enable (0:Single-End Input, 1:Differential Input)
     */
    i2c_write(AK4613_I2C_SLAVE_ADDR, AK4613_OUTPUT_CONTROL, 0x01 );

    /* (0x0B-0x16) Volume Control (default=0x00)
     *  [7:0] ATT7-0        Attenuation Level
     *       ==> 0x00:0.0dB, 0x01:-0.5dB, 0x02:-1.0dB, ..., 0xFD:-126.5dB, 0xFE:-127.0dB, 0xFF:MUTE
     */
    i2c_write(AK4613_I2C_SLAVE_ADDR, AK4613_LOUT1_VOLUME_CONTROL, 0x00 );  // LOUT1 Volume Control (0x00:0.0dB)
    i2c_write(AK4613_I2C_SLAVE_ADDR, AK4613_ROUT1_VOLUME_CONTROL, 0x00 );  // ROUT1 Volume Control (0x00:0.0dB)
    i2c_write(AK4613_I2C_SLAVE_ADDR, AK4613_LOUT2_VOLUME_CONTROL, 0xFF );  // LOUT2 Volume Control (0xFF:MUTE)
    i2c_write(AK4613_I2C_SLAVE_ADDR, AK4613_ROUT2_VOLUME_CONTROL, 0xFF );  // ROUT2 Volume Control (0xFF:MUTE)
    i2c_write(AK4613_I2C_SLAVE_ADDR, AK4613_LOUT3_VOLUME_CONTROL, 0xFF );  // LOUT3 Volume Control (0xFF:MUTE)
    i2c_write(AK4613_I2C_SLAVE_ADDR, AK4613_ROUT3_VOLUME_CONTROL, 0xFF );  // ROUT3 Volume Control (0xFF:MUTE)
    i2c_write(AK4613_I2C_SLAVE_ADDR, AK4613_LOUT4_VOLUME_CONTROL, 0xFF );  // LOUT4 Volume Control (0xFF:MUTE)
    i2c_write(AK4613_I2C_SLAVE_ADDR, AK4613_ROUT4_VOLUME_CONTROL, 0xFF );  // ROUT4 Volume Control (0xFF:MUTE)
    i2c_write(AK4613_I2C_SLAVE_ADDR, AK4613_LOUT5_VOLUME_CONTROL, 0xFF );  // LOUT5 Volume Control (0xFF:MUTE)
    i2c_write(AK4613_I2C_SLAVE_ADDR, AK4613_ROUT5_VOLUME_CONTROL, 0xFF );  // ROUT5 Volume Control (0xFF:MUTE)
    i2c_write(AK4613_I2C_SLAVE_ADDR, AK4613_LOUT6_VOLUME_CONTROL, 0xFF );  // LOUT6 Volume Control (0xFF:MUTE)
    i2c_write(AK4613_I2C_SLAVE_ADDR, AK4613_ROUT6_VOLUME_CONTROL, 0xFF );  // ROUT6 Volume Control (0xFF:MUTE)

    return 0;
}

int ak4613_output_vol_get( uint8_t* left_vol, uint8_t* right_vol )
{
    int status = EOK;
    if( left_vol ) {
        status = i2c_read(AK4613_I2C_SLAVE_ADDR, AK4613_LOUT1_VOLUME_CONTROL, left_vol);
        if( status != EOK ) {
            return status;
        }
        *left_vol = AK4613_MAX_DIGITAL_VOL - *left_vol;
    }
    if( right_vol ) {
        status = i2c_read(AK4613_I2C_SLAVE_ADDR, AK4613_ROUT1_VOLUME_CONTROL, right_vol);
        if( status != EOK ) {
            return status;
        }
        *right_vol = AK4613_MAX_DIGITAL_VOL - *right_vol;
    }
    return EOK;
}

int ak4613_output_vol_set( uint8_t left_vol, uint8_t right_vol )
{
    int status = EOK;
    status = i2c_write(AK4613_I2C_SLAVE_ADDR,  AK4613_LOUT1_VOLUME_CONTROL,
                               AK4613_MAX_DIGITAL_VOL - left_vol );
    if( status != EOK ) {
        return status;
    }
    status = i2c_write(AK4613_I2C_SLAVE_ADDR,  AK4613_ROUT1_VOLUME_CONTROL,
                               AK4613_MAX_DIGITAL_VOL - right_vol );
    if( status != EOK ) {
        return status;
    }

    return EOK;
}

int ak4613_input_vol_get( uint8_t* left_vol, uint8_t* right_vol )
{
    int status = EOK;
    if( left_vol ) {
        status = i2c_read(AK4613_I2C_SLAVE_ADDR, AK4613_LOUT1_VOLUME_CONTROL, left_vol);
    }
    if( status != EOK ) {
        return status;
    }
    if( right_vol ) {
        status = i2c_read(AK4613_I2C_SLAVE_ADDR, AK4613_ROUT1_VOLUME_CONTROL, right_vol);
    }
    return status;
}

int ak4613_rate_setting (uint32_t rate)
{
    switch (rate) {
        case 32000:
        case 44100:
        case 48000:
            ado_debug (DB_LVL_DRIVER, "rcar : ak4613_rate_setting 32kHz-48kHz");
            i2c_write(AK4613_I2C_SLAVE_ADDR, AK4613_CONTROL2, 0x00);
            break;
        case 64000:
        case 88200:
        case 960000:
            ado_debug (DB_LVL_DRIVER, "rcar : ak4613_rate_setting 64kHz-96kHz");
            i2c_write(AK4613_I2C_SLAVE_ADDR, AK4613_CONTROL2, 0x04);
            break;
        case 128000:
        case 176400:
        case 192000:
            ado_debug (DB_LVL_DRIVER, "rcar : ak4613_rate_setting 128kHz-192kHz");
            i2c_write(AK4613_I2C_SLAVE_ADDR, AK4613_CONTROL2, 0x08);
            break;
        case 0:
            ado_debug (DB_LVL_DRIVER, "rcar : ak4613_rate_setting set auto rate");
            i2c_write(AK4613_I2C_SLAVE_ADDR, AK4613_CONTROL2, 0x02);
        default:
            return EINVAL;
    }
    return EOK;
}

void ak4613_register_dump( void )
{
    int status = EOK;
    uint8_t reg_val = 0;

    status = i2c_read(AK4613_I2C_SLAVE_ADDR, AK4613_POWER_MANAGEMENT1, &reg_val);
    if( status == EOK ) {
        ado_debug (DB_LVL_DRIVER, "rcar : AK4613_POWER_MANAGEMENT1 - 0x%02x", reg_val);
    }

    status = i2c_read(AK4613_I2C_SLAVE_ADDR, AK4613_POWER_MANAGEMENT2, &reg_val );
    if( status == EOK ) {
        ado_debug (DB_LVL_DRIVER, "rcar : AK4613_POWER_MANAGEMENT2 - 0x%02x", reg_val);
    }

    status = i2c_read(AK4613_I2C_SLAVE_ADDR, AK4613_POWER_MANAGEMENT3, &reg_val);
    if( status == EOK ) {
        ado_debug (DB_LVL_DRIVER, "rcar : AK4613_POWER_MANAGEMENT3 - 0x%02x", reg_val);
    }

    status = i2c_read(AK4613_I2C_SLAVE_ADDR, AK4613_CONTROL1, &reg_val);
    if( status == EOK ) {
        ado_debug (DB_LVL_DRIVER, "rcar : AK4613_CONTROL1 - 0x%02x", reg_val);
    }

    status = i2c_read(AK4613_I2C_SLAVE_ADDR, AK4613_CONTROL2, &reg_val);
    if( status == EOK ) {
        ado_debug (DB_LVL_DRIVER, "rcar : AK4613_CONTROL2 - 0x%02x", reg_val);
    }

    status = i2c_read(AK4613_I2C_SLAVE_ADDR, AK4613_DE_EMPHASIS1, &reg_val);
    if( status == EOK ) {
        ado_debug (DB_LVL_DRIVER, "rcar : AK4613_DE_EMPHASIS1 - 0x%02x", reg_val);
    }

    status = i2c_read(AK4613_I2C_SLAVE_ADDR, AK4613_DE_EMPHASIS2, &reg_val);
    if( status == EOK ) {
        ado_debug (DB_LVL_DRIVER, "rcar : AK4613_DE_EMPHASIS2 - 0x%02x", reg_val);
    }

    status = i2c_read(AK4613_I2C_SLAVE_ADDR, AK4613_OVERFLOW_DETECT, &reg_val);
    if( status == EOK ) {
        ado_debug (DB_LVL_DRIVER, "rcar : AK4613_OVERFLOW_DETECT - 0x%02x", reg_val);
    }

    status = i2c_read(AK4613_I2C_SLAVE_ADDR, AK4613_ZERO_DETECT, &reg_val);
    if( status == EOK ) {
        ado_debug (DB_LVL_DRIVER, "rcar : AK4613_ZERO_DETECT - 0x%02x", reg_val);
    }

    status = i2c_read(AK4613_I2C_SLAVE_ADDR, AK4613_INPUT_CONTROL, &reg_val);
    if( status == EOK ) {
        ado_debug (DB_LVL_DRIVER, "rcar : AK4613_INPUT_CONTROL - 0x%08x", reg_val);
    }

    status = i2c_read(AK4613_I2C_SLAVE_ADDR, AK4613_OUTPUT_CONTROL, &reg_val);
    if( status == EOK ) {
        ado_debug (DB_LVL_DRIVER, "rcar : AK4613_OUTPUT_CONTROL - 0x%08x", reg_val);
    }

    status = i2c_read(AK4613_I2C_SLAVE_ADDR, AK4613_LOUT1_VOLUME_CONTROL, &reg_val);
    if( status == EOK ) {
        ado_debug (DB_LVL_DRIVER, "rcar : AK4613_LOUT1_VOLUME_CONTROL - 0x%08x", reg_val);
    }
    status = i2c_read(AK4613_I2C_SLAVE_ADDR, AK4613_ROUT1_VOLUME_CONTROL, &reg_val);
    if( status == EOK ) {
        ado_debug (DB_LVL_DRIVER, "rcar : AK4613_ROUT1_VOLUME_CONTROL - 0x%08x", reg_val);
    }
}

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/hardware/deva/ctrl/rcar/nto/aarch64/dll.le.ak4613/ak4613.c $ $Rev: 812929 $")
#endif
