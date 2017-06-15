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

#include "adv7482.h"

int fd = -1;
const char*	adv7482_i2c_dev = "/dev/i2c4";	
int adv7482_i2c_speed = 50000;	

/****************************************/
/* ADV7482 structure definition         */
/****************************************/

/* Structure for register values */
struct adv7482_reg_info {
    uint8_t addr;				/* i2c slave address */
    uint8_t sub_addr;			/* sub (register) address */
    uint8_t value;				/* register value */
};

const struct adv7482_reg_info Adv7482_Power_Up_Hdmi_Rx[] = {
    { 0xE0, 0x00, 0x40 },  
    { 0xFF, 0xFE, 0x00 }  
};

const struct adv7482_reg_info Adv7482_Enable_Csi4_Csi1[] = {
    { 0xE0, 0x10, 0xE0 },  
    { 0xFF, 0xFE, 0x00 }  
};

const struct adv7482_reg_info Adv7482_Sw_Reset[] = {
    { 0xE0, 0xFF, 0xFF },  // SW reset
    { 0xFF, 0x01, 0x05 },  // ** delay 5 **
    { 0xE0, 0x01, 0x76 },  // ADI Required Write
    { 0xE0, 0x05, 0x53 },
    { 0xE0, 0xF2, 0x01 },  // Enable I2C Read Auto-Increment
    { 0xFF, 0xFE, 0x00 }   // TBL END
};

// Supported Formats For Script Below - 
// 720x480p60, 1280x720p60, 1920x1080i60, 720(1440)x480i60, 
// 720x576p50, 1280x720p50, 1920x1080i50, 720(1440)x576i50, 
// 800x600(SVGA)@60, 640x480(VGA)@60, 800x480(WVGA)@60, 1024x768(XGA)@60
// 01-29 HDMI to MIPI TxA CSI 4-Lane - RGB888, Up to 600Mbps
const struct adv7482_reg_info Adv7482_Hdmi_to_Mipi_Txa_Csi4[] = {
    
    { 0xE0, 0xF3, 0x4C },  // DPLL Map Address Set to 0x4C
    { 0xE0, 0xF4, 0x44 },  // CP Map Address Set to 0x44
    { 0xE0, 0xF5, 0x68 },  // HDMI RX Map Address Set to 0x68
    { 0xE0, 0xF6, 0x6C },  // EDID Map Address Set to 0x6C
    { 0xE0, 0xF7, 0x64 },  // HDMI RX Repeater Map Address Set to 0x64
    { 0xE0, 0xF8, 0x62 },  // HDMI RX Infoframe Map Address Set to 0x62
    { 0xE0, 0xF9, 0xF0 },  // CBUS Map Address Set to 0xF0
    { 0xE0, 0xFA, 0x82 },  // CEC Map Address Set to 0x82
    { 0xE0, 0xFB, 0xF2 },  // SDP Main Map Address Set to 0xF2
    { 0xE0, 0xFC, 0x90 },  // CSI-TXB Map Address Set to 0x90
    { 0xE0, 0xFD, 0x94 },  // CSI-TXA Map Address Set to 0x94
    { 0xE0, 0x00, 0x40 },  // Disable chip powerdown & Enable HDMI Rx block
    { 0x64, 0x40, 0x83 },  // Enable HDCP 1.1  
    { 0x68, 0x00, 0x08 },  // Foreground Channel = A
    { 0x68, 0x98, 0xFF },  // ADI Required Write
    { 0x68, 0x99, 0xA3 },  // ADI Required Write
    { 0x68, 0x9A, 0x00 },  // ADI Required Write
    { 0x68, 0x9B, 0x0A },  // ADI Required Write
    { 0x68, 0x9D, 0x40 },  // ADI Required Write
    { 0x68, 0xCB, 0x09 },  // ADI Required Write
    { 0x68, 0x3D, 0x10 },  // ADI Required Write
    { 0x68, 0x3E, 0x7B },  // ADI Required Write
    { 0x68, 0x3F, 0x5E },  // ADI Required Write
    { 0x68, 0x4E, 0xFE },  // ADI Required Write
    { 0x68, 0x4F, 0x18 },  // ADI Required Write
    { 0x68, 0x57, 0xA3 },  // ADI Required Write
    { 0x68, 0x58, 0x04 },  // ADI Required Write
    { 0x68, 0x85, 0x10 },  // ADI Required Write
    { 0x68, 0x83, 0x00 },  // Enable All Terminations
    { 0x68, 0xA3, 0x01 },  // ADI Required Write
    { 0x68, 0xBE, 0x00 },  // ADI Required Write
    { 0x68, 0x6C, 0x01 },  // HPA Manual Enable
    { 0x68, 0xF8, 0x01 },  // HPA Asserted
    { 0x68, 0x0F, 0x00 },  // Audio Mute Speed Set to Fastest (Smallest Step Size)
    { 0xE0, 0x04, 0x02 },  // RGB Out of CP
    { 0xE0, 0x12, 0xF0 },  // CSC Depends on ip Packets - SDR 444
    { 0xE0, 0x17, 0x80 },  // Luma & Chroma Values Can Reach 254d
    { 0xE0, 0x03, 0x86 },  // CP-Insert_AV_Code
    { 0x44, 0x7C, 0x00 },  // ADI Required Write
    { 0xE0, 0x0C, 0xE0 },  // Enable LLC_DLL & Double LLC Timing
    { 0xE0, 0x0E, 0xDD },  // LLC/PIX/SPI PINS TRISTATED AUD Outputs Enabled
    { 0xE0, 0x10, 0xA0 },  // Enable 4-lane CSI Tx & Pixel Port
    { 0x94, 0x00, 0x84 },  // Enable 4-lane MIPI
    { 0x94, 0x00, 0xA4 },  // Set Auto DPHY Timing
    { 0x94, 0xDB, 0x10 },  // ADI Required Write
    { 0x94, 0xD6, 0x07 },  // ADI Required Write
    { 0x94, 0xC4, 0x0A },  // ADI Required Write
    { 0x94, 0x71, 0x33 },  // ADI Required Write
    { 0x94, 0x72, 0x11 },  // ADI Required Write
    { 0x94, 0xF0, 0x00 },  // i2c_dphy_pwdn - 1'b0
    { 0x94, 0x31, 0x82 },  // ADI Required Write
    { 0x94, 0x1E, 0x40 },  // ADI Required Write
    { 0x94, 0xDA, 0x01 },  // i2c_mipi_pll_en - 1'b1
    { 0xFF, 0x01, 0x02 },  // ** delay 2 **
    { 0x94, 0x00, 0x24 },  // Power-up CSI-TX
    { 0xFF, 0x01, 0x01 },  // ** delay 1 **
    { 0x94, 0xC1, 0x2B },  // ADI Required Write
    { 0xFF, 0x01, 0x01 },  // ** delay 1 **
    { 0x94, 0x31, 0x80 },  // ADI Required Write
    { 0xFF, 0xFE, 0x00 }   // TBL END
};

const struct adv7482_reg_info Adv7482_Program_EDID[] = {
    {ADV7482_I2C_REPEATER, 0x74, 0x00},	/* Disable the Internal EDID for all ports */
    /* EDID */
    /* Header information(0-19th byte) */
    {ADV7482_I2C_EDID, 0x00, 0x00},	/* Fixed header pattern */
    {ADV7482_I2C_EDID, 0x01, 0xFF},
    {ADV7482_I2C_EDID, 0x02, 0xFF},
    {ADV7482_I2C_EDID, 0x03, 0xFF},
    {ADV7482_I2C_EDID, 0x04, 0xFF},
    {ADV7482_I2C_EDID, 0x05, 0xFF},
    {ADV7482_I2C_EDID, 0x06, 0xFF},
    {ADV7482_I2C_EDID, 0x07, 0x00},
    {ADV7482_I2C_EDID, 0x08, 0x00},	/* Manufacturer ID */
    {ADV7482_I2C_EDID, 0x09, 0x00},
    {ADV7482_I2C_EDID, 0x0A, 0x00},	/* Manufacturer product code */
    {ADV7482_I2C_EDID, 0x0B, 0x00},
    {ADV7482_I2C_EDID, 0x0C, 0x00},	/* Serial number */
    {ADV7482_I2C_EDID, 0x0D, 0x00},
    {ADV7482_I2C_EDID, 0x0E, 0x00},
    {ADV7482_I2C_EDID, 0x0F, 0x00},
    {ADV7482_I2C_EDID, 0x10, 0x01},	/* Week of manufacture */
    {ADV7482_I2C_EDID, 0x11, 0x0C},	/* Year of manufacture */
    {ADV7482_I2C_EDID, 0x12, 0x01},	/* EDID version */
    {ADV7482_I2C_EDID, 0x13, 0x03},
    /* Basic display parameters(20-24th byte) */
    {ADV7482_I2C_EDID, 0x14, 0x80},	/* Video input parameters bitmap */
    {ADV7482_I2C_EDID, 0x15, 0x50},	/* Maximum horizontal image size */
    {ADV7482_I2C_EDID, 0x16, 0x2D},	/* Maximum vertical image size */
    {ADV7482_I2C_EDID, 0x17, 0x78},	/* Display gamma */
    {ADV7482_I2C_EDID, 0x18, 0x0A},	/* Supported features bitmap */
    /* Chromaticity coordinates(25-34th byte) */
    {ADV7482_I2C_EDID, 0x19, 0x0D},
    {ADV7482_I2C_EDID, 0x1A, 0xC9},
    {ADV7482_I2C_EDID, 0x1B, 0xA0},
    {ADV7482_I2C_EDID, 0x1C, 0x57},
    {ADV7482_I2C_EDID, 0x1D, 0x47},
    {ADV7482_I2C_EDID, 0x1E, 0x98},
    {ADV7482_I2C_EDID, 0x1F, 0x27},
    {ADV7482_I2C_EDID, 0x20, 0x12},
    {ADV7482_I2C_EDID, 0x21, 0x48},
    {ADV7482_I2C_EDID, 0x22, 0x4C},
    /* Established timing bitmap(35-37th byte) */
    {ADV7482_I2C_EDID, 0x23, 0x20},
    {ADV7482_I2C_EDID, 0x24, 0x00},
    {ADV7482_I2C_EDID, 0x25, 0x00},
    /* Standard timing information(38-53th byte) */
    /* Because they are unused, in this field, all values are 0101h. */
    {ADV7482_I2C_EDID, 0x26, 0x01},
    {ADV7482_I2C_EDID, 0x27, 0x01},
    {ADV7482_I2C_EDID, 0x28, 0x01},
    {ADV7482_I2C_EDID, 0x29, 0x01},
    {ADV7482_I2C_EDID, 0x2A, 0x01},
    {ADV7482_I2C_EDID, 0x2B, 0x01},
    {ADV7482_I2C_EDID, 0x2C, 0x01},
    {ADV7482_I2C_EDID, 0x2D, 0x01},
    {ADV7482_I2C_EDID, 0x2E, 0x01},
    {ADV7482_I2C_EDID, 0x2F, 0x01},
    {ADV7482_I2C_EDID, 0x30, 0x01},
    {ADV7482_I2C_EDID, 0x31, 0x01},
    {ADV7482_I2C_EDID, 0x32, 0x01},
    {ADV7482_I2C_EDID, 0x33, 0x01},
    {ADV7482_I2C_EDID, 0x34, 0x01},
    {ADV7482_I2C_EDID, 0x35, 0x01},
    /* Descriptor blocks of Descriptor 1(54-71th byte) */
    {ADV7482_I2C_EDID, 0x36, 0x01},
    {ADV7482_I2C_EDID, 0x37, 0x1D},
    {ADV7482_I2C_EDID, 0x38, 0x80},
    {ADV7482_I2C_EDID, 0x39, 0x18},
    {ADV7482_I2C_EDID, 0x3A, 0x71},
    {ADV7482_I2C_EDID, 0x3B, 0x1C},
    {ADV7482_I2C_EDID, 0x3C, 0x16},
    {ADV7482_I2C_EDID, 0x3D, 0x20},
    {ADV7482_I2C_EDID, 0x3E, 0x58},
    {ADV7482_I2C_EDID, 0x3F, 0x2C},
    {ADV7482_I2C_EDID, 0x40, 0x25},
    {ADV7482_I2C_EDID, 0x41, 0x00},
    {ADV7482_I2C_EDID, 0x42, 0x20},
    {ADV7482_I2C_EDID, 0x43, 0xC2},
    {ADV7482_I2C_EDID, 0x44, 0x31},
    {ADV7482_I2C_EDID, 0x45, 0x00},
    {ADV7482_I2C_EDID, 0x46, 0x00},
    {ADV7482_I2C_EDID, 0x47, 0x98},
    /* Descriptor blocks of Descriptor 2(72-89th byte) */
    {ADV7482_I2C_EDID, 0x48, 0x8C},
    {ADV7482_I2C_EDID, 0x49, 0x0A},
    {ADV7482_I2C_EDID, 0x4A, 0xD0},
    {ADV7482_I2C_EDID, 0x4B, 0x8A},
    {ADV7482_I2C_EDID, 0x4C, 0x20},
    {ADV7482_I2C_EDID, 0x4D, 0xE0},
    {ADV7482_I2C_EDID, 0x4E, 0x2D},
    {ADV7482_I2C_EDID, 0x4F, 0x10},
    {ADV7482_I2C_EDID, 0x50, 0x10},
    {ADV7482_I2C_EDID, 0x51, 0x3E},
    {ADV7482_I2C_EDID, 0x52, 0x96},
    {ADV7482_I2C_EDID, 0x53, 0x00},
    {ADV7482_I2C_EDID, 0x54, 0x58},
    {ADV7482_I2C_EDID, 0x55, 0xC2},
    {ADV7482_I2C_EDID, 0x56, 0x21},
    {ADV7482_I2C_EDID, 0x57, 0x00},
    {ADV7482_I2C_EDID, 0x58, 0x00},
    {ADV7482_I2C_EDID, 0x59, 0x18},
    /* Descriptor blocks of Descriptor 3(90-107th byte) */
    {ADV7482_I2C_EDID, 0x5A, 0x00},
    {ADV7482_I2C_EDID, 0x5B, 0x00},
    {ADV7482_I2C_EDID, 0x5C, 0x00},
    {ADV7482_I2C_EDID, 0x5D, 0xFC},
    {ADV7482_I2C_EDID, 0x5E, 0x00},
    {ADV7482_I2C_EDID, 0x5F, 0x4D},
    {ADV7482_I2C_EDID, 0x60, 0x59},
    {ADV7482_I2C_EDID, 0x61, 0x20},
    {ADV7482_I2C_EDID, 0x62, 0x48},
    {ADV7482_I2C_EDID, 0x63, 0x44},
    {ADV7482_I2C_EDID, 0x64, 0x54},
    {ADV7482_I2C_EDID, 0x65, 0x56},
    {ADV7482_I2C_EDID, 0x66, 0x0A},
    {ADV7482_I2C_EDID, 0x67, 0x20},
    {ADV7482_I2C_EDID, 0x68, 0x20},
    {ADV7482_I2C_EDID, 0x69, 0x20},
    {ADV7482_I2C_EDID, 0x6A, 0x20},
    {ADV7482_I2C_EDID, 0x6B, 0x20},
    /* Descriptor blocks of Descriptor 4(108-125th byte) */
    {ADV7482_I2C_EDID, 0x6C, 0x00},
    {ADV7482_I2C_EDID, 0x6D, 0x00},
    {ADV7482_I2C_EDID, 0x6E, 0x00},
    {ADV7482_I2C_EDID, 0x6F, 0xFD},
    {ADV7482_I2C_EDID, 0x70, 0x00},
    {ADV7482_I2C_EDID, 0x71, 0x3B},
    {ADV7482_I2C_EDID, 0x72, 0x3D},
    {ADV7482_I2C_EDID, 0x73, 0x0F},
    {ADV7482_I2C_EDID, 0x74, 0x2E},
    {ADV7482_I2C_EDID, 0x75, 0x08},
    {ADV7482_I2C_EDID, 0x76, 0x00},
    {ADV7482_I2C_EDID, 0x77, 0x0A},
    {ADV7482_I2C_EDID, 0x78, 0x20},
    {ADV7482_I2C_EDID, 0x79, 0x20},
    {ADV7482_I2C_EDID, 0x7A, 0x20},
    {ADV7482_I2C_EDID, 0x7B, 0x20},
    {ADV7482_I2C_EDID, 0x7C, 0x20},
    {ADV7482_I2C_EDID, 0x7D, 0x20},
    /* Number of extensions to follow(126th byte) */
    {ADV7482_I2C_EDID, 0x7E, 0x01},	/* Extension enable */
    /* Checksum(127th byte) */
    {ADV7482_I2C_EDID, 0x7F, 0x75},

    /* CEA EDID Timing Extension Version 3 */
    {ADV7482_I2C_EDID, 0x80, 0x02},	/* CEA 861 Externsion Block */
    {ADV7482_I2C_EDID, 0x81, 0x03},
    {ADV7482_I2C_EDID, 0x82, 0x26},
    {ADV7482_I2C_EDID, 0x83, 0x41},
    {ADV7482_I2C_EDID, 0x84, 0x41},
    {ADV7482_I2C_EDID, 0x85, 0x85},
    {ADV7482_I2C_EDID, 0x86, 0x35},
    {ADV7482_I2C_EDID, 0x87, 0x0F},
    {ADV7482_I2C_EDID, 0x88, 0x06},
    {ADV7482_I2C_EDID, 0x89, 0x07},
    {ADV7482_I2C_EDID, 0x8A, 0x17},
    {ADV7482_I2C_EDID, 0x8B, 0x1F},
    {ADV7482_I2C_EDID, 0x8C, 0x38},
    {ADV7482_I2C_EDID, 0x8D, 0x1F},
    {ADV7482_I2C_EDID, 0x8E, 0x07},
    {ADV7482_I2C_EDID, 0x8F, 0x30},
    {ADV7482_I2C_EDID, 0x90, 0x2F},
    {ADV7482_I2C_EDID, 0x91, 0x07},
    {ADV7482_I2C_EDID, 0x92, 0x72},
    {ADV7482_I2C_EDID, 0x93, 0x3F},
    {ADV7482_I2C_EDID, 0x94, 0x7F},
    {ADV7482_I2C_EDID, 0x95, 0x72},
    {ADV7482_I2C_EDID, 0x96, 0x57},
    {ADV7482_I2C_EDID, 0x97, 0x7F},
    {ADV7482_I2C_EDID, 0x98, 0x00},
    {ADV7482_I2C_EDID, 0x99, 0x37},
    {ADV7482_I2C_EDID, 0x9A, 0x7F},
    {ADV7482_I2C_EDID, 0x9B, 0x72},
    {ADV7482_I2C_EDID, 0x9C, 0x83},
    {ADV7482_I2C_EDID, 0x9D, 0x7F},
    {ADV7482_I2C_EDID, 0x9E, 0x00},
    {ADV7482_I2C_EDID, 0x9F, 0x00},
    {ADV7482_I2C_EDID, 0xA0, 0x65},
    {ADV7482_I2C_EDID, 0xA1, 0x03},
    {ADV7482_I2C_EDID, 0xA2, 0x0C},
    {ADV7482_I2C_EDID, 0xA3, 0x00},
    {ADV7482_I2C_EDID, 0xA4, 0x00},
    {ADV7482_I2C_EDID, 0xA5, 0x00},
    {ADV7482_I2C_EDID, 0xA6, 0x01},
    {ADV7482_I2C_EDID, 0xA7, 0x1D},
    {ADV7482_I2C_EDID, 0xA8, 0x80},
    {ADV7482_I2C_EDID, 0xA9, 0x18},
    {ADV7482_I2C_EDID, 0xAA, 0x71},
    {ADV7482_I2C_EDID, 0xAB, 0x1C},
    {ADV7482_I2C_EDID, 0xAC, 0x16},
    {ADV7482_I2C_EDID, 0xAD, 0x20},
    {ADV7482_I2C_EDID, 0xAE, 0x58},
    {ADV7482_I2C_EDID, 0xAF, 0x2C},
    {ADV7482_I2C_EDID, 0xB0, 0x25},
    {ADV7482_I2C_EDID, 0xB1, 0x00},
    {ADV7482_I2C_EDID, 0xB2, 0x20},
    {ADV7482_I2C_EDID, 0xB3, 0xC2},
    {ADV7482_I2C_EDID, 0xB4, 0x31},
    {ADV7482_I2C_EDID, 0xB5, 0x00},
    {ADV7482_I2C_EDID, 0xB6, 0x00},
    {ADV7482_I2C_EDID, 0xB7, 0x98},
    {ADV7482_I2C_EDID, 0xB8, 0X20},
    {ADV7482_I2C_EDID, 0xB9, 0X20},
    {ADV7482_I2C_EDID, 0xBA, 0X20},
    {ADV7482_I2C_EDID, 0xBB, 0X20},
    {ADV7482_I2C_EDID, 0xBC, 0X20},
    {ADV7482_I2C_EDID, 0xBD, 0X20},
    {ADV7482_I2C_EDID, 0xBE, 0X20},
    {ADV7482_I2C_EDID, 0xBF, 0X20},
    {ADV7482_I2C_EDID, 0xC0, 0X20},
    {ADV7482_I2C_EDID, 0xC1, 0X20},
    {ADV7482_I2C_EDID, 0xC2, 0X20},
    {ADV7482_I2C_EDID, 0xC3, 0X20},
    {ADV7482_I2C_EDID, 0xC4, 0X20},
    {ADV7482_I2C_EDID, 0xC5, 0X20},
    {ADV7482_I2C_EDID, 0xC6, 0X20},
    {ADV7482_I2C_EDID, 0xC7, 0X20},
    {ADV7482_I2C_EDID, 0xC8, 0X20},
    {ADV7482_I2C_EDID, 0xC9, 0X20},
    {ADV7482_I2C_EDID, 0xCA, 0X20},
    {ADV7482_I2C_EDID, 0xCB, 0X20},
    {ADV7482_I2C_EDID, 0xCC, 0X20},
    {ADV7482_I2C_EDID, 0xCD, 0X20},
    {ADV7482_I2C_EDID, 0xCE, 0X20},
    {ADV7482_I2C_EDID, 0xCF, 0X20},
    {ADV7482_I2C_EDID, 0xD0, 0X20},
    {ADV7482_I2C_EDID, 0xD1, 0X20},
    {ADV7482_I2C_EDID, 0xD2, 0X20},
    {ADV7482_I2C_EDID, 0xD3, 0X20},
    {ADV7482_I2C_EDID, 0xD4, 0X20},
    {ADV7482_I2C_EDID, 0xD5, 0X20},
    {ADV7482_I2C_EDID, 0xD6, 0X20},
    {ADV7482_I2C_EDID, 0xD7, 0X20},
    {ADV7482_I2C_EDID, 0xD8, 0X20},
    {ADV7482_I2C_EDID, 0xD9, 0X20},
    {ADV7482_I2C_EDID, 0xDA, 0X20},
    {ADV7482_I2C_EDID, 0xDB, 0X20},
    {ADV7482_I2C_EDID, 0xDC, 0X20},
    {ADV7482_I2C_EDID, 0xDD, 0X20},
    {ADV7482_I2C_EDID, 0xDE, 0X20},
    {ADV7482_I2C_EDID, 0xDF, 0X20},
    {ADV7482_I2C_EDID, 0xE0, 0X20},
    {ADV7482_I2C_EDID, 0xE1, 0X20},
    {ADV7482_I2C_EDID, 0xE2, 0X20},
    {ADV7482_I2C_EDID, 0xE3, 0X20},
    {ADV7482_I2C_EDID, 0xE4, 0X20},
    {ADV7482_I2C_EDID, 0xE5, 0X20},
    {ADV7482_I2C_EDID, 0xE6, 0X20},
    {ADV7482_I2C_EDID, 0xE7, 0X20},
    {ADV7482_I2C_EDID, 0xE8, 0X20},
    {ADV7482_I2C_EDID, 0xE9, 0X20},
    {ADV7482_I2C_EDID, 0xEA, 0X20},
    {ADV7482_I2C_EDID, 0xEB, 0X20},
    {ADV7482_I2C_EDID, 0xEC, 0X20},
    {ADV7482_I2C_EDID, 0xED, 0X20},
    {ADV7482_I2C_EDID, 0xEE, 0x00},
    {ADV7482_I2C_EDID, 0xEF, 0x00},
    {ADV7482_I2C_EDID, 0xF0, 0x00},
    {ADV7482_I2C_EDID, 0xF1, 0x00},
    {ADV7482_I2C_EDID, 0xF2, 0x00},
    {ADV7482_I2C_EDID, 0xF3, 0x00},
    {ADV7482_I2C_EDID, 0xF4, 0x00},
    {ADV7482_I2C_EDID, 0xF5, 0x00},
    {ADV7482_I2C_EDID, 0xF6, 0x00},
    {ADV7482_I2C_EDID, 0xF7, 0x00},
    {ADV7482_I2C_EDID, 0xF8, 0x00},
    {ADV7482_I2C_EDID, 0xF9, 0x00},
    {ADV7482_I2C_EDID, 0xFA, 0x00},
    {ADV7482_I2C_EDID, 0xFB, 0x00},
    {ADV7482_I2C_EDID, 0xFC, 0x00},
    {ADV7482_I2C_EDID, 0xFD, 0x00},
    {ADV7482_I2C_EDID, 0xFE, 0x00},
    {ADV7482_I2C_EDID, 0xFF, 0xE0},
    {ADV7482_I2C_REPEATER, 0x74, 0x01},	/* Enable the Internal EDID for ports */
    
    { 0xFF, 0xFE, 0x00 }   // TBL END
};

// 02-01 Analog CVBS to MIPI TX-B CSI 1-Lane - Autodetect CVBS Single Ended In Ain 1 - MIPI Out
const struct adv7482_reg_info Adv7482_Cvbs_to_Mipi_Txb_Csi1[] = {
    
    { 0xE0, 0x00, 0x30 },  // Disable chip powerdown - powerdown Rx
    { 0xE0, 0xF2, 0x01 },  // Enable I2C Read Auto-Increment
    { 0xE0, 0xF3, 0x4C },  // DPLL Map Address Set to 0x4C
    { 0xE0, 0xF4, 0x44 },  // CP Map Address Set to 0x44
    { 0xE0, 0xF5, 0x68 },  // HDMI RX Map Address Set to 0x68
    { 0xE0, 0xF6, 0x6C },  // EDID Map Address Set to 0x6C
    { 0xE0, 0xF7, 0x64 },  // HDMI RX Repeater Map Address Set to 0x64
    { 0xE0, 0xF8, 0x62 },  // HDMI RX Infoframe Map Address Set to 0x62
    { 0xE0, 0xF9, 0xF0 },  // CBUS Map Address Set to 0xF0
    { 0xE0, 0xFA, 0x82 },  // CEC Map Address Set to 0x82
    { 0xE0, 0xFB, 0xF2 },  // SDP Main Map Address Set to 0xF2
    { 0xE0, 0xFC, 0x90 },  // CSI-TXB Map Address Set to 0x90
    { 0xE0, 0xFD, 0x94 },  // CSI-TXA Map Address Set to 0x94
    { 0xE0, 0x0E, 0xFF },  // LLC/PIX/AUD/SPI PINS TRISTATED
    { 0xF2, 0x0F, 0x00 },  // Exit Power Down Mode
    { 0xF2, 0x52, 0xCD },  // ADI Required Write
    { 0xF2, 0x00, 0x07 },  // INSEL = CVBS in on Ain 8 *** changed from recommended value ***
    { 0xF2, 0x0E, 0x80 },  // ADI Required Write
    { 0xF2, 0x9C, 0x00 },  // ADI Required Write
    { 0xF2, 0x9C, 0xFF },  // ADI Required Write
    { 0xF2, 0x0E, 0x00 },  // ADI Required Write
    { 0xF2, 0x80, 0x51 },  // ADI Required Write
    { 0xF2, 0x81, 0x51 },  // ADI Required Write
    { 0xF2, 0x82, 0x68 },  // ADI Required Write
    { 0xF2, 0x03, 0x42 },  // Tri-S Output Drivers, PwrDwn 656 pads
    { 0xF2, 0x04, 0xB5 },  // ITU-R BT.656-4 compatible
    { 0xF2, 0x13, 0x00 },  // ADI Required Write
    { 0xF2, 0x17, 0x41 },  // Select SH1
    { 0xF2, 0x31, 0x12 },  // ADI Required Write
    { 0xF2, 0xE6, 0x4F },  // Set V bit end position manually in NTSC mode
    { 0xE0, 0x10, 0x70 },  // Enable 1-Lane MIPI Tx, enable pixel output and route SD through Pixel port
    { 0x90, 0x00, 0x81 },  // Enable 1-lane MIPI
    { 0x90, 0x00, 0xA1 },  // Set Auto DPHY Timing
#if 0
    { 0x94, 0xF0, 0x00 },  // ADI Required Write
    { 0x94, 0xD6, 0x07 },  // ADI Required Write
    { 0x94, 0xC0, 0x3C },  // ADI Required Write
    { 0x94, 0xC3, 0x3C },  // ADI Required Write
    { 0x94, 0xC6, 0x3C },  // ADI Required Write
    { 0x94, 0xC9, 0x3C },  // ADI Required Write
    { 0x94, 0xCC, 0x3C },  // ADI Required Write
    { 0x94, 0xD5, 0x03 },  // ADI Required Write	
#endif
    { 0x90, 0xD2, 0x40 },  // ADI Required Write
    { 0x90, 0xC4, 0x0A },  // ADI Required Write
    { 0x90, 0x71, 0x33 },  // ADI Required Write
    { 0x90, 0x72, 0x11 },  // ADI Required Write
    { 0x90, 0xF0, 0x00 },  // i2c_dphy_pwdn - 1'b0
    { 0x90, 0x31, 0x82 },  // ADI Required Write
    { 0x90, 0x1E, 0x40 },  // ADI Required Write
    { 0x90, 0xDA, 0x01 },  // i2c_mipi_pll_en - 1'b1
    { 0xFF, 0x01, 0x02 },  // ** delay 2 **
    { 0x90, 0x00, 0x21 },  // Power-up CSI-TX
    { 0xFF, 0x01, 0x01 },  // ** delay 1 **
    { 0x90, 0xC1, 0x2B },  // ADI Required Write
    { 0xFF, 0x01, 0x01 },  // ** delay 1 **
    { 0x90, 0x31, 0x80 },  // ADI Required Write
    { 0xFF, 0xFE, 0x00 }   // TBL END
};

int get_num_instance()
{
    int shmfd;
    int* addr;
    int first_run = 0;
    
    if(access("/dev/shmem/vin", F_OK) == -1) {
        first_run = 1;
    }
    
    shmfd = shm_open("/vin", O_RDWR | O_CREAT, 0777);
    if(fd == -1) {
        fprintf(stderr, "%s: shm_open failed\n", __FUNCTION__);
        return -1;
    }
    
    if(first_run) {
        if(ftruncate(shmfd, sizeof(int)) == -1) {
            fprintf(stderr, "%s: ftruncate failed\n", __FUNCTION__);
            return -1;
        }
    }
    
    addr = mmap(0, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, shmfd, 0);
    if(addr == MAP_FAILED) {
        fprintf(stderr, "%s: mmap failed\n", __FUNCTION__);
        return -1;
    }

    /* Set value */
    if(first_run) {
        *addr = 1;
    }
    else {
        (*addr)++;
    }
    
    close(shmfd);
    
    return *addr;
}

static int adv7482_write_table(const struct adv7482_reg_info *regs)
{
    int ret = 0;
    int	status = EOK;
    i2c_addr_t slave;

    struct {
        i2c_send_t hdr;		
        unsigned char bytes[8];	
    } omsg;

    while (1) {
        if(regs->addr == ADV7482_I2C_NOT_ADDR) {
            if(regs->sub_addr == ADV7482_I2C_EOR)
                break;		// End of script
            if(regs->sub_addr == ADV7482_I2C_WAIT)
                delay(5);	// Wait a moment
        }
        else {
            
            slave.addr = regs->addr >> 1;
            slave.fmt = I2C_ADDRFMT_7BIT;

            status = devctl(fd, DCMD_I2C_SET_SLAVE_ADDR, &slave, sizeof(slave), NULL);

            if (status != EOK) {
                fprintf(stderr, "%s: Set slave addr failed\n", __FUNCTION__);
                return -1;
            }
            
            omsg.hdr.slave = slave;
            omsg.hdr.slave.fmt = I2C_ADDRFMT_7BIT;
            omsg.hdr.len = 2;
            omsg.hdr.stop = 1;
            omsg.bytes[0] = regs->sub_addr;
            omsg.bytes[1] = regs->value;

            status = devctl(fd, DCMD_I2C_SEND, &omsg, sizeof(omsg.hdr) + omsg.hdr.len, NULL);

            if(status != EOK) {
                fprintf(stderr, "%s: Send failed, addr=%x, reg=%x, val=%x\n", __FUNCTION__, 
                                        regs->addr, regs->sub_addr, regs->value);
                return -1;
            }	
        }
    
        regs++;
    }
    return ret;
}

static int adv7482_read(uint8_t addr, uint8_t sub_addr, uint8_t *value)
{
    int ret = 0;
    int status = EOK;
    i2c_addr_t slave;

    struct  {
        i2c_send_t hdr;		
        unsigned char bytes[8];	
    } omsg;
    
    struct {
        i2c_recv_t hdr;	
        unsigned char bytes[16];
    } imsg;

    slave.addr = addr >> 1;
    slave.fmt = I2C_ADDRFMT_7BIT;

    status = devctl(fd, DCMD_I2C_SET_SLAVE_ADDR, &slave, sizeof(slave), NULL);

    if (status != EOK) {
        fprintf(stderr, "%s: Set slave addr failed\n", __FUNCTION__);
        return -1;
    }

    omsg.hdr.slave = slave;
    omsg.hdr.len = 1;
    omsg.hdr.stop = 0; 
    omsg.bytes[0] = sub_addr;

    status = devctl(fd, DCMD_I2C_SEND, &omsg, sizeof(omsg.hdr) + omsg.hdr.len, NULL);

    if (status != EOK) {
        fprintf(stderr, "%s: Send failed\n", __FUNCTION__);
        return -1;
    }
    
    imsg.hdr.slave = slave;
    imsg.hdr.len = 1;
    imsg.hdr.stop = 1;

    status = devctl(fd, DCMD_I2C_RECV, &imsg, sizeof(imsg.hdr) + imsg.hdr.len, NULL);

    if (status != EOK) {
        fprintf(stderr, "%s: Read failed\n", __FUNCTION__);
        return -1;
    }
    
    *value = imsg.bytes[0];
    
    return ret;
}

static int adv7482_write(uint8_t addr, uint8_t sub_addr, uint8_t value)
{
    struct adv7482_reg_info regs[2];

    regs[0].addr = addr;
    regs[0].sub_addr = sub_addr;
    regs[0].value = value;
    
    regs[1].addr = ADV7482_I2C_NOT_ADDR;
    regs[1].sub_addr = ADV7482_I2C_EOR;

    return adv7482_write_table(regs);
}

int adv7482_powerup(int channel)
{
    int ret = 0;
    uint32_t lane, addr;
    
    if(channel == 0) {
        lane = 4;
        addr = ADV7482_I2C_TXA; // 0x94
    }
    else {
        lane = 1;
        addr = ADV7482_I2C_TXB;
    }
    
    ret += adv7482_write(addr, 0x00, 0x80|lane);
    ret += adv7482_write(addr, 0x00, 0xA0|lane);
    ret += adv7482_write(addr, 0x31, 0x82);
    ret += adv7482_write(addr, 0x1E, 0x40);
    ret += adv7482_write(addr, 0xDA, 0x01);
    delay(2);
    ret += adv7482_write(addr, 0x00, 0x20|lane);
    delay(1);
    ret += adv7482_write(addr, 0xC1, 0x2B);
    delay(1);
    ret += adv7482_write(addr, 0x31, 0x80);
    
    if(ret < 0) {
        fprintf(stderr, "%s: failed\n", __FUNCTION__);
    }
    
    return ret;
}

int adv7482_powerdown(int channel)
{
    int ret = 0;
    uint32_t lane, addr;
    
    if(channel == 0) {
        lane = 4;
        addr = ADV7482_I2C_TXA;
    }
    else {
        lane = 1;
        addr = ADV7482_I2C_TXB;
    }
    
    ret += adv7482_write(addr, 0x31, 0x82);
    ret += adv7482_write(addr, 0x1E, 0x00);
    ret += adv7482_write(addr, 0x00, 0x80|lane);
    ret += adv7482_write(addr, 0xDA, 0x00);
    ret += adv7482_write(addr, 0xC1, 0x3B);
    
    if(ret < 0) {
        fprintf(stderr, "%s: failed\n", __FUNCTION__);
    }
    
    return ret;
}

int adv7482_signal(int channel)
{
	uint8_t tmp;
	
	if(channel == 0) {
		if(adv7482_read(ADV7482_I2C_HDMI, ADV7482_HDMI_STATUS1_REG, &tmp)) {
			return 0;
		}
		if ((tmp & ADV7482_HDMI_VF_LOCKED_FLG) ||
			(tmp & ADV7482_HDMI_DERF_LOCKED_FLG)) {
			return 1;
		}
		else {
			return 0;
		}
	}
	else if(channel == 1) {
		if(adv7482_write(ADV7482_I2C_SDP, ADV7482_SDP_REG_CTRL, 
                                    ADV7482_SDP_RO_MAIN_MAP)) {
			return 0;
		}
		if(adv7482_read(ADV7482_I2C_SDP, ADV7482_SDP_R_REG_10, &tmp)) {
			return 0;
		}
		if (tmp & ADV7482_SDP_R_REG_10_IN_LOCK) {
			return 1;
		}
		else {
			return 0;
		}
	}
    return 0;
}

static int adv7482_hdmi_get_vid_info(video_info_t* video)
{
    int ret = 0;
    uint8_t msb;
    uint8_t lsb;
    uint8_t hdmi_int;
    
    video->signal = 0;
	
	/* Check cable pluged or not */
	if(adv7482_read(0xE0, 0x71, &msb)) {
		return -1;
	}
	
	/* Exit when cable is not detected */
	if(!(msb & 0x40)) {
		return -1;
	}
	
	while(1) {
        /* Detect input signal */
		ret = adv7482_read(ADV7482_I2C_HDMI, ADV7482_HDMI_STATUS1_REG, &msb);

		if (ret < 0)
            return -1;

        if ((msb & ADV7482_HDMI_VF_LOCKED_FLG) ||
            (msb & ADV7482_HDMI_DERF_LOCKED_FLG)) {
            break;
        }

        usleep(1000);
    }
	
	delay(1000);	// Fix me: wait for signal is stable
	
	video->signal = 1;

    /* Decide interlaced or progressive */
    ret = adv7482_read(ADV7482_I2C_HDMI, ADV7482_HDMI_STATUS2_REG, &hdmi_int);
            
    if (ret < 0)
        return -1;
    
    video->interlace = 0;
    
    if ((hdmi_int & ADV7482_HDMI_IP_FLAG) != 0)
        video->interlace = 1;
    
    ret = adv7482_read(ADV7482_I2C_HDMI, ADV7482_HDMI_LWIDTH_REG, &lsb);
            
    if (ret < 0)
        return -1;
    
    video->width = (uint32_t)(ADV7482_HDMI_LWIDTH_MSBS_MASK & msb);
    video->width = (lsb | (video->width << 8));
    
    /* Decide lines per frame */
    ret = adv7482_read(ADV7482_I2C_HDMI, ADV7482_HDMI_F0HEIGHT_MSBS_REG, &msb);
            
    if (ret < 0)
        return -1;
    
    ret = adv7482_read(ADV7482_I2C_HDMI, ADV7482_HDMI_F0HEIGHT_LSBS_REG, &lsb);
            
    if (ret < 0)
        return -1;
    
    video->height = (uint32_t)(ADV7482_HDMI_F0HEIGHT_MSBS_MASK & msb);
    video->height = (lsb | (video->height << 8));
    
    if (video->interlace)
        video->height *= 2;
    
    
    if (video->width == 0 || video->height == 0) {
        fprintf(stderr, "Got invalid resolution(%dx%d).\n", video->width, video->height);
        return -1;
    }
    
    return ret;
}

static int adv7482_hdmi_set_vid_info(video_info_t* video)
{
    int ret = 0;
    uint8_t val = ADV7482_IO_CP_VID_STD_480P;

    /* Get video information */
    ret = adv7482_hdmi_get_vid_info(video);
    
    if (ret < 0) {
        return ret;
    } 
    else {
        if ((video->width == 640) &&
            (video->height == 480) && (video->interlace == 0)) {
            val = ADV7482_IO_CP_VID_STD_VGA60;
            strcpy(video->format, "VGA60");
        } else if ((video->width == 720) &&
            (video->height == 480) && (video->interlace == 0)) {
            val = ADV7482_IO_CP_VID_STD_480P;
            strcpy(video->format, "480P");
        } else if ((video->width == 720) &&
            (video->height == 576) && (video->interlace == 0)) {
            val = ADV7482_IO_CP_VID_STD_576P;
            strcpy(video->format, "576P");
        } else if ((video->width == 1280) &&
            (video->height == 720) && (video->interlace == 0)) {
            val = ADV7482_IO_CP_VID_STD_720P;
            strcpy(video->format, "720P");
        } else if ((video->width == 1920) &&
            (video->height == 1080) && (video->interlace == 0)) {
            val = ADV7482_IO_CP_VID_STD_1080P;
            strcpy(video->format, "1080P");
        } else if ((video->width == 1920) &&
            (video->height == 1080) && (video->interlace == 1)) {
            val = ADV7482_IO_CP_VID_STD_1080I;
            strcpy(video->format, "1080I");
        } else {
            fprintf(stderr, "Not support resolution %dx%d%c\n",
                  video->width, video->height, (video->interlace) ? 'i' : 'p');
            return -1;
        }
    }
    
    /* Freerun setting */
    adv7482_write(0x44, 0xBA, 0x03);
    adv7482_write(0x44, 0xC9, 0x2D);
    adv7482_write(0x44, 0xF3, 0xDE);
    adv7482_write(0xE0, 0x05, val);
    
    /* Enable free run */
    adv7482_write(0xE0, 0x03, 0x86);
                
    return ret;
}

static int adv7482_cvbs_get_vid_info(video_info_t* video)
{
    int ret = 0;
    uint8_t value;
	
	video->signal = 0;
    
    ret = adv7482_write(ADV7482_I2C_SDP, ADV7482_SDP_REG_CTRL, 
                                    ADV7482_SDP_RO_MAIN_MAP);
    
    if (ret < 0)
        return ret;
    
	delay(500);	// Fix me: wait for signal is stable
	
	/* Detect input signal */
    if(adv7482_read(ADV7482_I2C_SDP, ADV7482_SDP_R_REG_10, &value)) {
		return -1;
	}
    
	if (!(value & ADV7482_SDP_R_REG_10_IN_LOCK)) {
		return -1;
	}
	
	video->signal = 1;
    
    switch (value & ADV7482_SDP_R_REG_10_AUTOD_MASK) 
    {
        case ADV7482_SDP_R_REG_10_AUTOD_NTSM_M_J:
            strcpy(video->format, CAPTURE_NORM_NTSC_M_J);
            break;
        case ADV7482_SDP_R_REG_10_AUTOD_NTSC_4_43:
            strcpy(video->format, CAPTURE_NORM_NTSC_4_43);
            break;
        case ADV7482_SDP_R_REG_10_AUTOD_PAL_M:
            strcpy(video->format, CAPTURE_NORM_PAL_M);
            break;
        case ADV7482_SDP_R_REG_10_AUTOD_PAL_B_G:
            strcpy(video->format, CAPTURE_NORM_PAL_B_G_H_I_D);
            break;
        case ADV7482_SDP_R_REG_10_AUTOD_PAL_COMB:
            strcpy(video->format, CAPTURE_NORM_PAL_COMBINATION_N);
            break;
        case ADV7482_SDP_R_REG_10_AUTOD_PAL_60:
            strcpy(video->format, CAPTURE_NORM_PAL_60);
            break;
        case ADV7482_SDP_R_REG_10_AUTOD_SECAM:
            strcpy(video->format, CAPTURE_NORM_SECAM);
            break;
        default:
            fprintf(stderr, "Not support video standard, val=%x\n", 
                            value & ADV7482_SDP_R_REG_10_AUTOD_MASK);
            return -1;
    }
    
    switch (value & ADV7482_SDP_R_REG_10_AUTOD_MASK) 
    {
        case ADV7482_SDP_R_REG_10_AUTOD_NTSM_M_J:
        case ADV7482_SDP_R_REG_10_AUTOD_NTSC_4_43:
            video->width = 720;
            video->height = 480;
            break;
        case ADV7482_SDP_R_REG_10_AUTOD_PAL_M:
        case ADV7482_SDP_R_REG_10_AUTOD_PAL_B_G:
        case ADV7482_SDP_R_REG_10_AUTOD_PAL_COMB:
        case ADV7482_SDP_R_REG_10_AUTOD_PAL_60:
        case ADV7482_SDP_R_REG_10_AUTOD_SECAM:
            video->width = 720;
            video->height = 576;
            break;
    }
    
    video->interlace = 1;
    
    return ret;
}

static int adv7482_cp_s_ctrl(video_info_t* video)
{
    uint8_t val;
    int ret;
    
    /* Enable video adjustment first */
    ret = adv7482_read(ADV7482_I2C_CP, ADV7482_CP_VID_ADJ_REG, &val);
            
    if (ret < 0)
        return ret;
    
    val |= ADV7482_CP_VID_ADJ_ENABLE;
    
    ret = adv7482_write(ADV7482_I2C_CP, ADV7482_CP_VID_ADJ_REG, val);
            
    if (ret < 0)
        return ret;
    
    /* Brightness */
    if(video->update & DECODER_COLOR_BRI_UPDATE) {
        if ((video->bri < ADV7482_CP_BRI_MIN) || (video->bri > ADV7482_CP_BRI_MAX))
            ret = -ERANGE;
        else 
            ret = adv7482_write(ADV7482_I2C_CP, ADV7482_CP_BRI_REG, video->bri);
    }
    
    if(ret < 0)
        return ret;
    
    video->update &= ~DECODER_COLOR_BRI_UPDATE;
    
    /* Hue */
    if(video->update & DECODER_COLOR_HUE_UPDATE) {
        if ((video->hue < ADV7482_CP_HUE_MIN) || (video->hue > ADV7482_CP_HUE_MAX))
            ret = -ERANGE;
        else
            ret = adv7482_write(ADV7482_I2C_CP, ADV7482_CP_HUE_REG, video->hue);
    }
    
    if(ret < 0)
        return ret;
    
    video->update &= ~DECODER_COLOR_HUE_UPDATE;
    
    /* Contrast */
    if(video->update & DECODER_COLOR_CON_UPDATE) {
        if ((video->con < ADV7482_CP_CON_MIN) || (video->con > ADV7482_CP_CON_MAX))
            ret = -ERANGE;
        else 
            ret = adv7482_write(ADV7482_I2C_CP, ADV7482_CP_CON_REG, video->con);
    }

    if(ret < 0)
        return ret;
    
    video->update &= ~DECODER_COLOR_CON_UPDATE;
    
    /* Saturation */
    if(video->update & DECODER_COLOR_SAT_UPDATE) {
        if ((video->sat < ADV7482_CP_SAT_MIN) || (video->sat > ADV7482_CP_SAT_MAX))
            ret = -ERANGE;
        else 
            ret = adv7482_write(ADV7482_I2C_CP, ADV7482_CP_SAT_REG, video->sat);
    }
    
    if(ret < 0)
        return ret;
    
    video->update &= ~DECODER_COLOR_SAT_UPDATE;
    
    return ret;
}

static int adv7482_sdp_s_ctrl(video_info_t* video)
{
    int ret;
    
    ret = adv7482_write(ADV7482_I2C_SDP, ADV7482_SDP_REG_CTRL, ADV7482_SDP_MAIN_MAP_RW);
            
    if (ret < 0)
        return ret;
    
    /* Brightness */
    if(video->update & DECODER_COLOR_BRI_UPDATE) {
        if ((video->bri < ADV7482_SDP_BRI_MIN) || (video->bri > ADV7482_SDP_BRI_MAX))
            ret = -1;
        else
            ret = adv7482_write(ADV7482_I2C_SDP, ADV7482_SDP_REG_BRI, video->bri);
    }
    
    if(ret < 0)
        return ret;
    
    video->update &= ~DECODER_COLOR_BRI_UPDATE;
    
    /* Hue */
    if(video->update & DECODER_COLOR_HUE_UPDATE) {
        if ((video->hue < ADV7482_SDP_HUE_MIN) || (video->hue > ADV7482_SDP_HUE_MAX))
            ret = -1;
        else 
            ret = adv7482_write(ADV7482_I2C_SDP, ADV7482_SDP_REG_HUE, video->hue);
    }
    
    if(ret < 0)
        return ret;
    
    video->update &= ~DECODER_COLOR_HUE_UPDATE;
    
    /* Contrast */
    if(video->update & DECODER_COLOR_CON_UPDATE) {
        if ((video->con < ADV7482_SDP_CON_MIN) || (video->con > ADV7482_SDP_CON_MAX))
            ret = -1;
        else
            ret = adv7482_write(ADV7482_I2C_SDP, ADV7482_SDP_REG_CON, video->con);
    }

    if(ret < 0)
        return ret;
    
    video->update &= ~DECODER_COLOR_CON_UPDATE;
    
    /* Saturation */
    if(video->update & DECODER_COLOR_SAT_UPDATE) {
        if ((video->sat < ADV7482_SDP_SAT_MIN) || (video->sat > ADV7482_SDP_SAT_MAX))
            ret = -1;
        else {
            ret = adv7482_write(ADV7482_I2C_SDP, ADV7482_SDP_REG_SD_SAT_CB, video->sat);
            if(ret < 0)
                return ret;
            ret = adv7482_write(ADV7482_I2C_SDP, ADV7482_SDP_REG_SD_SAT_CR, video->sat);
        }
    }
    
    if(ret < 0)
        return ret;
    
    video->update &= ~DECODER_COLOR_SAT_UPDATE;
    
    return ret;
}

int adv7482_update(int channel, video_info_t* video)
{
    int ret = 0;
    
    if(video->update) {
        if(channel == 0) {
            ret = adv7482_cp_s_ctrl(video);
        }
        else {
            ret = adv7482_sdp_s_ctrl(video);
        }
    }
    
    if(ret < 0) {
        fprintf(stderr, "%s: failed, flags=%x\n", __FUNCTION__, video->update);
    }
    
    return ret;
}

int adv7482_get_video_info(int channel, video_info_t* video)
{
    int ret = 0;
    
    if(channel == 0) {
        ret = adv7482_hdmi_set_vid_info(video);
    }
    else {
        ret = adv7482_cvbs_get_vid_info(video);
    }
    
    return ret;
}

int adv7482_init(int channel, video_info_t* video)
{
    int ret = 0;
    int status;
    
    if(fd == -1) {
        fd = open(adv7482_i2c_dev, O_RDWR);
    }
    
    if (fd < 0) {
        fprintf(stderr, "%s: Open device failed\n", __FUNCTION__);
        return -1;
    }
    
    status = devctl(fd, DCMD_I2C_SET_BUS_SPEED, &adv7482_i2c_speed, 
                sizeof(adv7482_i2c_speed), NULL);
                
    if (status != EOK) {
        fprintf(stderr, "%s: Set bus speed failed\n", __FUNCTION__);
        return -1;
    }
    
    /* Init both TXA and TXB at first run */
    if(get_num_instance() == 1) {
        /* Software reset */
        adv7482_write_table(&Adv7482_Sw_Reset[0]);
        /* Init channel 0 and powerdown */
        adv7482_write_table(&Adv7482_Hdmi_to_Mipi_Txa_Csi4[0]);
        adv7482_powerdown(0);
        /* Init channel 1 and powerdown */
        adv7482_write_table(&Adv7482_Cvbs_to_Mipi_Txb_Csi1[0]);
        adv7482_powerdown(1);
        /* Powerup Hdmi Rx and enable both Csi1-Csi4 */
        adv7482_write_table(&Adv7482_Power_Up_Hdmi_Rx[0]);
        adv7482_write_table(&Adv7482_Enable_Csi4_Csi1[0]);
        /* Program EDID */
        adv7482_write_table(&Adv7482_Program_EDID[0]);
    }
    
    /* Enable HDMI audio output, right justified, 16 bits audio format  */
    if(channel == 0) {
        adv7482_write(ADV7482_I2C_IO, 0x0E, 0xDD);
        adv7482_write(ADV7482_I2C_HDMI, 0x03, 0x30);
    }

    /* Power up */
    adv7482_powerup(channel);
	
	/* Get video information */
    adv7482_get_video_info(channel, video);
	
	/* Check video signal */
	if(video->signal == 0) {
		fprintf(stderr, "No cable connected.\n");
		return -1;
	}
    
    /* Decoder will be powered up after CSI had been enabled */
    adv7482_powerdown(channel);
    
    return ret;
}

int	adv7482_fini(int channel, video_info_t* video)
{
    if(video->signal) {
        adv7482_powerdown(channel);
    }
    
    int shmfd = shm_open("/vin", O_RDWR | O_CREAT, 0777);
    int* addr = mmap(0, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, shmfd, 0);
        
    (*addr)--;
    if(0 == *addr) {
        shm_unlink("/vin");
    }
    
    close(shmfd);
    
    return 0;
}
