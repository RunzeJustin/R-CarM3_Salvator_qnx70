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


#ifndef __CSI2_H__
#define __CSI2_H__

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
#include <time.h>
#include <pthread.h>
#include <sys/siginfo.h>
#include <hw/i2c.h> 
#include <graphics/display.h>
#include <graphics/disputil.h>
#include <sys/rsrcdbmgr.h>
#include <arm/r-car.h>

/* Bits defined */
#define RCAR_CSI2_PHYCNT_SHUTDOWNZ			(1 << 17)
#define RCAR_CSI2_PHYCNT_RSTZ				(1 << 16)
#define RCAR_CSI2_PHYCNT_ENABLECLK			(1 << 4)
#define RCAR_CSI2_PHYCNT_ENABLE_3			(1 << 3)
#define RCAR_CSI2_PHYCNT_ENABLE_2			(1 << 2)
#define RCAR_CSI2_PHYCNT_ENABLE_1			(1 << 1)
#define RCAR_CSI2_PHYCNT_ENABLE_0			(1 << 0)
#define RCAR_CSI2_VCDT_VCDTN_EN				(1 << 15)
#define RCAR_CSI2_VCDT_SEL_VCN				(1 << 8)
#define RCAR_CSI2_VCDT_SEL_DTN_ON			(1 << 6)
#define RCAR_CSI2_VCDT_SEL_DTN				(1 << 0)
#define RCAR_CSI2_LINKCNT_MONITOR_EN		(1 << 31)
#define RCAR_CSI2_LINKCNT_REG_MONI_PACT_EN	(1 << 25)
#define RCAR_CSI2_LSWAP_L3SEL_PLANE0		(0 << 6)
#define RCAR_CSI2_LSWAP_L3SEL_PLANE1		(1 << 6)
#define RCAR_CSI2_LSWAP_L3SEL_PLANE2		(2 << 6)
#define RCAR_CSI2_LSWAP_L3SEL_PLANE3		(3 << 6)
#define RCAR_CSI2_LSWAP_L2SEL_PLANE0		(0 << 4)
#define RCAR_CSI2_LSWAP_L2SEL_PLANE1		(1 << 4)
#define RCAR_CSI2_LSWAP_L2SEL_PLANE2		(2 << 4)
#define RCAR_CSI2_LSWAP_L2SEL_PLANE3		(3 << 4)
#define RCAR_CSI2_LSWAP_L1SEL_PLANE0		(0 << 2)
#define RCAR_CSI2_LSWAP_L1SEL_PLANE1		(1 << 2)
#define RCAR_CSI2_LSWAP_L1SEL_PLANE2		(2 << 2)
#define RCAR_CSI2_LSWAP_L1SEL_PLANE3		(3 << 2)
#define RCAR_CSI2_LSWAP_L0SEL_PLANE0		(0 << 0)
#define RCAR_CSI2_LSWAP_L0SEL_PLANE1		(1 << 0)
#define RCAR_CSI2_LSWAP_L0SEL_PLANE2		(2 << 0)
#define RCAR_CSI2_LSWAP_L0SEL_PLANE3		(3 << 0)
#define RCAR_CSI2_PHTC_TESTCLR				(1 << 0)

/* Interrupt status registers */
#define RCAR_CSI2_INTSTATE_EBD_CH1			(1 << 29)
#define RCAR_CSI2_INTSTATE_LESS_THAN_WC		(1 << 28)
#define RCAR_CSI2_INTSTATE_AFIFO_OF			(1 << 27)
#define RCAR_CSI2_INTSTATE_VD4_START		(1 << 26)
#define RCAR_CSI2_INTSTATE_VD4_END			(1 << 25)
#define RCAR_CSI2_INTSTATE_VD3_START		(1 << 24)
#define RCAR_CSI2_INTSTATE_VD3_END			(1 << 23)
#define RCAR_CSI2_INTSTATE_VD2_START		(1 << 22)
#define RCAR_CSI2_INTSTATE_VD2_END			(1 << 21)
#define RCAR_CSI2_INTSTATE_VD1_START		(1 << 20)
#define RCAR_CSI2_INTSTATE_VD1_END			(1 << 19)
#define RCAR_CSI2_INTSTATE_SHP				(1 << 18)
#define RCAR_CSI2_INTSTATE_FSFE				(1 << 17)
#define RCAR_CSI2_INTSTATE_LNP				(1 << 16)
#define RCAR_CSI2_INTSTATE_CRC_ERR			(1 << 15)
#define RCAR_CSI2_INTSTATE_HD_WC_ZERO		(1 << 14)
#define RCAR_CSI2_INTSTATE_FRM_SEQ_ERR1		(1 << 13)
#define RCAR_CSI2_INTSTATE_FRM_SEQ_ERR2		(1 << 12)
#define RCAR_CSI2_INTSTATE_ECC_ERR			(1 << 11)
#define RCAR_CSI2_INTSTATE_ECC_CRCT_ERR		(1 << 10)
#define RCAR_CSI2_INTSTATE_LPDT_START		(1 << 9)
#define RCAR_CSI2_INTSTATE_LPDT_END			(1 << 8)
#define RCAR_CSI2_INTSTATE_ULPS_START		(1 << 7)
#define RCAR_CSI2_INTSTATE_ULPS_END			(1 << 6)
#define RCAR_CSI2_INTSTATE_RESERVED			(1 << 5)
#define RCAR_CSI2_INTSTATE_ERRSOTHS			(1 << 4)
#define RCAR_CSI2_INTSTATE_ERRSOTSYNCCHS	(1 << 3)
#define RCAR_CSI2_INTSTATE_ERRESC			(1 << 2)
#define RCAR_CSI2_INTSTATE_ERRSYNCESC		(1 << 1)
#define RCAR_CSI2_INTSTATE_ERRCONTROL		(1 << 0)

/* Monitoring registers of interrupt error status */
#define RCAR_CSI2_INTSTATE_ECC_ERR			(1 << 11)
#define RCAR_CSI2_INTSTATE_ECC_CRCT_ERR		(1 << 10)
#define RCAR_CSI2_INTSTATE_LPDT_START		(1 << 9)
#define RCAR_CSI2_INTSTATE_LPDT_END			(1 << 8)
#define RCAR_CSI2_INTSTATE_ULPS_START		(1 << 7)
#define RCAR_CSI2_INTSTATE_ULPS_END			(1 << 6)
#define RCAR_CSI2_INTSTATE_RESERVED			(1 << 5)
#define RCAR_CSI2_INTSTATE_ERRSOTHS			(1 << 4)
#define RCAR_CSI2_INTSTATE_ERRSOTSYNCCHS	(1 << 3)
#define RCAR_CSI2_INTSTATE_ERRESC			(1 << 2)
#define RCAR_CSI2_INTSTATE_ERRSYNCESC		(1 << 1)
#define RCAR_CSI2_INTSTATE_ERRCONTROL		(1 << 0)

/* CSI2 bandwidth */
#define RCAR_CSI_80MBPS		0
#define RCAR_CSI_90MBPS		1
#define RCAR_CSI_100MBPS	2
#define RCAR_CSI_110MBPS	3
#define RCAR_CSI_120MBPS	4
#define RCAR_CSI_130MBPS	5
#define RCAR_CSI_140MBPS	6
#define RCAR_CSI_150MBPS	7
#define RCAR_CSI_160MBPS	8
#define RCAR_CSI_170MBPS	9
#define RCAR_CSI_180MBPS	10
#define RCAR_CSI_190MBPS	11
#define RCAR_CSI_205MBPS	12
#define RCAR_CSI_220MBPS	13
#define RCAR_CSI_235MBPS	14
#define RCAR_CSI_250MBPS	15
#define RCAR_CSI_275MBPS	16
#define RCAR_CSI_300MBPS	17
#define RCAR_CSI_325MBPS	18
#define RCAR_CSI_350MBPS	19
#define RCAR_CSI_400MBPS	20
#define RCAR_CSI_450MBPS	21
#define RCAR_CSI_500MBPS	22
#define RCAR_CSI_550MBPS	23
#define RCAR_CSI_600MBPS	24
#define RCAR_CSI_650MBPS	25
#define RCAR_CSI_700MBPS	26
#define RCAR_CSI_750MBPS	27
#define RCAR_CSI_800MBPS	28
#define RCAR_CSI_850MBPS	29
#define RCAR_CSI_900MBPS	30
#define RCAR_CSI_950MBPS	31
#define RCAR_CSI_1000MBPS	32
#define RCAR_CSI_1050MBPS	33
#define RCAR_CSI_1100MBPS	34
#define RCAR_CSI_1150MBPS	35
#define RCAR_CSI_1200MBPS	36
#define RCAR_CSI_1250MBPS	37
#define RCAR_CSI_1300MBPS	38
#define RCAR_CSI_1350MBPS	39
#define RCAR_CSI_1400MBPS	40
#define RCAR_CSI_1450MBPS	41
#define RCAR_CSI_1500MBPS	42

typedef struct _csi2_info {
	char interlace;
	uint32_t width;
	uint32_t height;
} csi2_info_t;

typedef struct _rcar_csi2 {
	uintptr_t vbase;
	uintptr_t pbase;
	uint32_t lanes;
	csi2_info_t* info;
} rcar_csi2_t;


int rcar_csi2_init(int channel, rcar_csi2_t* csi);
int rcar_csi2_fini(int channel, rcar_csi2_t* csi);

#endif // __CSI2_H__
