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


#ifndef __ADV7482_H__
#define __ADV7482_H__

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <atomic.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <hw/inout.h>
#include <sys/neutrino.h>
#include <string.h>
#include <vcapture/capture.h>
#include <sys/siginfo.h>
#include <hw/i2c.h> 
#include <graphics/display.h>
#include <graphics/disputil.h>
#include <sys/rsrcdbmgr.h>
#include <sys/rsrcdbmsg.h>
#include <arm/r-car.h>
#include <time.h>
#include <pthread.h>

/****************************************/
/* ADV7482 I2C slave address definition */
/****************************************/
#define ADV7482_I2C_IO					0xE0
#define ADV7482_I2C_DPLL				0x4C
#define ADV7482_I2C_CP					0x44	
#define ADV7482_I2C_HDMI				0x68	
#define ADV7482_I2C_REPEATER			0x64	
#define ADV7482_I2C_INFORFRAME			0x62	
#define ADV7482_I2C_EDID				0x6C
#define ADV7482_I2C_CBUS				0xF0
#define ADV7482_I2C_CEC					0x82	
#define ADV7482_I2C_SDP					0xF2
#define ADV7482_I2C_TXB					0x90
#define ADV7482_I2C_TXA					0x94

#define ADV7482_I2C_WAIT				0x01
#define ADV7482_I2C_EOR					0xFE	
#define ADV7482_I2C_NOT_ADDR			0xFF	

/****************************************/
/* ADV7482 IO register definition       */
/****************************************/
#define ADV7482_IO_RD_INFO1_REG			0xDF	/* chip version register */
#define ADV7482_IO_RD_INFO2_REG			0xE0	/* chip version register */
#define ADV7482_IO_CP_DATAPATH_REG		0x03	/* datapath ctrl */
#define ADV7482_IO_CP_COLORSPACE_REG	0x04
#define ADV7482_IO_CP_VID_STD_REG		0x05	/* Video Standard */

#define ADV7482_IO_PWR_MAN_REG			0x0C	/* Power management register */
#define ADV7482_IO_PWR_ON				0xE0	/* Power on */
#define ADV7482_IO_PWR_OFF				0x00	/* Power down */

#define ADV7482_HDMI_DDC_PWRDN			0x73	/* Power DDC pads control register */
#define ADV7482_HDMI_DDC_PWR_ON			0x00	/* Power on */
#define ADV7482_HDMI_DDC_PWR_OFF		0x01	/* Power down */

#define ADV7482_IO_CP_VID_STD_480I		0x40
#define ADV7482_IO_CP_VID_STD_576I		0x41
#define ADV7482_IO_CP_VID_STD_480P		0x4A
#define ADV7482_IO_CP_VID_STD_576P		0x4B
#define ADV7482_IO_CP_VID_STD_720P		0x53
#define ADV7482_IO_CP_VID_STD_1080I		0x54
#define ADV7482_IO_CP_VID_STD_1080P		0x5E
#define ADV7482_IO_CP_VID_STD_SVGA56	0x80
#define ADV7482_IO_CP_VID_STD_SVGA60	0x81
#define ADV7482_IO_CP_VID_STD_SVGA72	0x82
#define ADV7482_IO_CP_VID_STD_SVGA75	0x83
#define ADV7482_IO_CP_VID_STD_SVGA85	0x84
#define ADV7482_IO_CP_VID_STD_SXGA60	0x85
#define ADV7482_IO_CP_VID_STD_SXGA75	0x86
#define ADV7482_IO_CP_VID_STD_VGA60		0x88
#define ADV7482_IO_CP_VID_STD_VGA72		0x89
#define ADV7482_IO_CP_VID_STD_VGA75		0x8A
#define ADV7482_IO_CP_VID_STD_VGA85		0x8B
#define ADV7482_IO_CP_VID_STD_XGA60		0x8C
#define ADV7482_IO_CP_VID_STD_XGA70		0x8D
#define ADV7482_IO_CP_VID_STD_XGA75		0x8E
#define ADV7482_IO_CP_VID_STD_XGA85		0x8F
#define ADV7482_IO_CP_VID_STD_UXGA60	0x96
#define ADV7482_IO_CSI4_EN_ENABLE       0x80
#define ADV7482_IO_CSI4_EN_DISABLE      0x00
#define ADV7482_IO_CSI1_EN_ENABLE       0x40
#define ADV7482_IO_CSI1_EN_DISABLE      0x00

/****************************************/
/* ADV7482 CP register definition       */
/****************************************/
#define ADV7482_CP_CON_REG				0x3a	/* Contrast (unsigned) */
#define ADV7482_CP_CON_MIN				0		/* Minimum contrast */
#define ADV7482_CP_CON_DEF				128		/* Default */
#define ADV7482_CP_CON_MAX				255		/* Maximum contrast */

#define ADV7482_CP_SAT_REG				0x3b	/* Saturation (unsigned) */
#define ADV7482_CP_SAT_MIN				0		/* Minimum saturation */
#define ADV7482_CP_SAT_DEF				128		/* Default */
#define ADV7482_CP_SAT_MAX				255		/* Maximum saturation */

#define ADV7482_CP_BRI_REG				0x3c	/* Brightness (signed) */
#define ADV7482_CP_BRI_MIN				-128	/* Luma is -512d */
#define ADV7482_CP_BRI_DEF				0		/* Luma is 0 */
#define ADV7482_CP_BRI_MAX				127		/* Luma is 508d */

#define ADV7482_CP_HUE_REG				0x3d	/* Hue (unsigned) */
#define ADV7482_CP_HUE_MIN				0		/* -90 degree */
#define ADV7482_CP_HUE_DEF				0		/* -90 degree */
#define ADV7482_CP_HUE_MAX				255		/* +90 degree */

#define ADV7482_CP_VID_ADJ_REG			0x3e	/* Video adjustment register */
#define ADV7482_CP_VID_ADJ_MASK			0x7F	/* Video adjustment mask */
#define ADV7482_CP_VID_ADJ_ENABLE		0x80	/* Enable color controls */

/****************************************/
/* ADV7482 HDMI register definition     */
/****************************************/
#define ADV7482_HDMI_I2SOUTMODE_REG		0x03	/* Configure I2S output */
#define ADV7482_HDMI_STATUS1_REG		0x07	/* VERT_FILTER_LOCKED flag */
#define ADV7482_HDMI_VF_LOCKED_FLG		0x80	/* DE_REGEN_FILTER_LOCKED flag */
#define ADV7482_HDMI_DERF_LOCKED_FLG	0x20	/* LINE_WIDTH[12:8] mask */
#define ADV7482_HDMI_LWIDTH_MSBS_MASK	0x1F	/* LINE_WIDTH[7:0] register */
#define ADV7482_HDMI_LWIDTH_REG			0x08	/* FIELD0_HEIGHT[12:8] register */
#define ADV7482_HDMI_F0HEIGHT_MSBS_REG	0x09	/* FIELD0_HEIGHT[12:8] mask */
#define ADV7482_HDMI_F0HEIGHT_MSBS_MASK	0x1F	/* FIELD0_HEIGHT[7:0] register */
#define ADV7482_HDMI_F0HEIGHT_LSBS_REG	0x0A	/* HDMI status register */
#define ADV7482_HDMI_STATUS2_REG		0x0B	/* DEEP_COLOR_MODE[1:0] mask */
#define ADV7482_HDMI_DCM_MASK			0xC0	/* HDMI_INTERLACED flag */
#define ADV7482_HDMI_IP_FLAG			0x20	/* FIELD1_HEIGHT[12:8] mask */
#define ADV7482_HDMI_F1HEIGHT_MSBS_MASK	0x1F	/* FIELD1_HEIGHT[7:0] register */
#define ADV7482_HDMI_F1HEIGHT_REG		0x0C

#define ADV7482_HDMI_I2S_MODE			0x20	/* Right-justified */
#define ADV7482_HDMI_I2S_BITWITH		0x10	/* 16 bit data */

/****************************************/
/* ADV7482 SDP register definition      */
/****************************************/
#define ADV7482_SDP_MAIN_MAP			0x00
#define ADV7482_SDP_SUB_MAP1			0x20
#define ADV7482_SDP_SUB_MAP2			0x40
#define ADV7482_SDP_NO_RO_MAIN_MAP		0x00
#define ADV7482_SDP_RO_MAIN_MAP			0x01
#define ADV7482_SDP_RO_SUB_MAP1			0x02
#define ADV7482_SDP_RO_SUB_MAP2			0x03
#define ADV7482_SDP_MAIN_MAP_RW 		(ADV7482_SDP_MAIN_MAP | ADV7482_SDP_NO_RO_MAIN_MAP)

#define ADV7482_SDP_STD_AD_PAL_BG_NTSC_J_SECAM		0x0
#define ADV7482_SDP_STD_AD_PAL_BG_NTSC_J_SECAM_PED	0x1
#define ADV7482_SDP_STD_AD_PAL_N_NTSC_J_SECAM		0x2
#define ADV7482_SDP_STD_AD_PAL_N_NTSC_M_SECAM		0x3

#define ADV7482_SDP_STD_NTSC_J						0x4
#define ADV7482_SDP_STD_NTSC_M						0x5
#define ADV7482_SDP_STD_PAL60						0x6
#define ADV7482_SDP_STD_NTSC_443					0x7
#define ADV7482_SDP_STD_PAL_BG						0x8
#define ADV7482_SDP_STD_PAL_N						0x9
#define ADV7482_SDP_STD_PAL_M						0xa
#define ADV7482_SDP_STD_PAL_M_PED					0xb
#define ADV7482_SDP_STD_PAL_COMB_N					0xc
#define ADV7482_SDP_STD_PAL_COMB_N_PED				0xd
#define ADV7482_SDP_STD_PAL_SECAM					0xe
#define ADV7482_SDP_STD_PAL_SECAM_PED				0xf
#define ADV7482_SDP_REG_INPUT_CONTROL				0x00
#define ADV7482_SDP_INPUT_CONTROL_INSEL_MASK		0x0f
#define ADV7482_SDP_REG_INPUT_VIDSEL				0x02
#define ADV7482_SDP_REG_CTRL						0x0e
#define ADV7482_SDP_REG_PWR_MAN						0x0f
#define ADV7482_SDP_PWR_MAN_ON						0x00
#define ADV7482_SDP_PWR_MAN_OFF						0x20
#define ADV7482_SDP_PWR_MAN_RES						0x80
/* Contrast */
#define ADV7482_SDP_REG_CON							0x08	/*Unsigned */
#define ADV7482_SDP_CON_MIN							0
#define ADV7482_SDP_CON_DEF							128
#define ADV7482_SDP_CON_MAX							255
/* Brightness*/
#define ADV7482_SDP_REG_BRI							0x0a	/*Signed */
#define ADV7482_SDP_BRI_MIN							-128
#define ADV7482_SDP_BRI_DEF							0
#define ADV7482_SDP_BRI_MAX							127
/* Hue */
#define ADV7482_SDP_REG_HUE							0x0b	/*Signed, inverted */
#define ADV7482_SDP_HUE_MIN							-127
#define ADV7482_SDP_HUE_DEF							0
#define ADV7482_SDP_HUE_MAX							128
/* Saturation */
#define ADV7482_SDP_REG_SD_SAT_CB					0xe3
#define ADV7482_SDP_REG_SD_SAT_CR					0xe4
#define ADV7482_SDP_SAT_MIN							0
#define ADV7482_SDP_SAT_DEF							128
#define ADV7482_SDP_SAT_MAX							255
#define ADV7482_SDP_INPUT_CVBS_AIN1					0x00
#define ADV7482_SDP_INPUT_CVBS_AIN2					0x01
#define ADV7482_SDP_INPUT_CVBS_AIN3					0x02
#define ADV7482_SDP_INPUT_CVBS_AIN4					0x03
#define ADV7482_SDP_INPUT_CVBS_AIN5					0x04
#define ADV7482_SDP_INPUT_CVBS_AIN6					0x05
#define ADV7482_SDP_INPUT_CVBS_AIN7					0x06
#define ADV7482_SDP_INPUT_CVBS_AIN8					0x07
#define ADV7482_SDP_INPUT_SVIDEO_AIN1_AIN2			0x08
#define ADV7482_SDP_INPUT_SVIDEO_AIN3_AIN4			0x09
#define ADV7482_SDP_INPUT_SVIDEO_AIN5_AIN6			0x0a
#define ADV7482_SDP_INPUT_SVIDEO_AIN7_AIN8			0x0b
#define ADV7482_SDP_INPUT_YPRPB_AIN1_AIN2_AIN3		0x0c
#define ADV7482_SDP_INPUT_YPRPB_AIN4_AIN5_AIN6		0x0d
#define ADV7482_SDP_INPUT_DIFF_CVBS_AIN1_AIN2		0x0e
#define ADV7482_SDP_INPUT_DIFF_CVBS_AIN3_AIN4		0x0f
#define ADV7482_SDP_INPUT_DIFF_CVBS_AIN5_AIN6		0x10
#define ADV7482_SDP_INPUT_DIFF_CVBS_AIN7_AIN8		0x11
#define ADV7482_SDP_R_REG_10						0x10
#define ADV7482_SDP_R_REG_10_IN_LOCK				0x01
#define ADV7482_SDP_R_REG_10_AUTOD_MASK				0x70
#define ADV7482_SDP_R_REG_10_AUTOD_NTSM_M_J			0x00
#define ADV7482_SDP_R_REG_10_AUTOD_NTSC_4_43		0x10
#define ADV7482_SDP_R_REG_10_AUTOD_PAL_M			0x20
#define ADV7482_SDP_R_REG_10_AUTOD_PAL_60			0x30
#define ADV7482_SDP_R_REG_10_AUTOD_PAL_B_G			0x40
#define ADV7482_SDP_R_REG_10_AUTOD_SECAM			0x50
#define ADV7482_SDP_R_REG_10_AUTOD_PAL_COMB			0x60
#define ADV7482_SDP_R_REG_10_AUTOD_SECAM_525		0x70

#define ADV7482_MAX_WIDTH		1920
#define ADV7482_MAX_HEIGHT		1080

typedef struct _video_info {
	char signal;
	char interlace;
	char format[64];
	uint32_t width;
	uint32_t height;
	int16_t bri;
	uint8_t sat;
	int16_t hue;
	uint8_t con;
	uint32_t update;
} video_info_t;

#define DECODER_COLOR_CON_UPDATE 	0x10
#define DECODER_COLOR_BRI_UPDATE 	0x20
#define DECODER_COLOR_SAT_UPDATE 	0x40
#define DECODER_COLOR_HUE_UPDATE 	0x80

int adv7482_init(int channel, video_info_t* video);
int	adv7482_fini(int channel, video_info_t* video);

int adv7482_powerup(int channel);
int adv7482_powerdown(int channel);

int adv7482_update(int channel, video_info_t* video);
int adv7482_signal(int channel);

#endif // __ADV7482_H__
