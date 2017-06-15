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

 
#ifndef __VIN_H__
#define __VIN_H__

#include <stdint.h>
#include <errno.h>
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <vcapture/capture.h>
#include "csi2.h"
#include "hdmi_audio.h"
#include "imrlx4.h"

#define SOC_SCALING_UPDATE				0x01
#define SOC_CROPPING_UPDATE				0x02
#define RCAR_VIN_MAX_WIDTH				4096
#define RCAR_VIN_MAX_HEIGHT				4096
#define RCAR_VIN_MAX_BUFFER 			3
#define RCAR_VIN_PULSE 					55
#define RCAR_VIN_END 					56
#define RCAR_VIN_IMR_PULSE 				57

/* Video n Main Control Register */
#define RCAR_VIN_MC_DPINE				(1 << 27)
#define RCAR_VIN_MC_SCLE				(1 << 26)
#define RCAR_VIN_MC_FOC					(1 << 21)
#define RCAR_VIN_MC_YCAL				(1 << 19)
#define RCAR_VIN_MC_INF_YUV8_BT656		(0 << 16)
#define RCAR_VIN_MC_INF_YUV8_BT601		(1 << 16)
#define RCAR_VIN_MC_INF_YUV10_BT656		(2 << 16)
#define RCAR_VIN_MC_INF_YUV10_BT601		(3 << 16)
#define RCAR_VIN_MC_INF_YUV16			(5 << 16)
#define RCAR_VIN_MC_INF_RGB888			(6 << 16)
#define RCAR_VIN_MC_INF_MASK			(7 << 16)
#define RCAR_VIN_MC_VUP					(1 << 10)
#define RCAR_VIN_MC_IM_ODD				(0 << 3)
#define RCAR_VIN_MC_IM_ODD_EVEN			(1 << 3)
#define RCAR_VIN_MC_IM_EVEN				(2 << 3)
#define RCAR_VIN_MC_IM_FULL				(3 << 3)
#define RCAR_VIN_MC_BPS					(1 << 1)
#define RCAR_VIN_MC_ME					(1 << 0)

/* Video n Module Status Register */
#define RCAR_VIN_MS_FBS_MASK			(3 << 3)
#define RCAR_VIN_MS_FBS_SHIFT			3
#define RCAR_VIN_MS_AV					(1 << 1)
#define RCAR_VIN_MS_CA					(1 << 0)

/* Video n Frame Capture Register */
#define RCAR_VIN_FC_C_FRAME				(1 << 1)
#define RCAR_VIN_FC_S_FRAME				(1 << 0)

/* Video n Interrupt Enable Register */
#define RCAR_VIN_IE_FIE					(1 << 4)
#define RCAR_VIN_IE_EFE					(1 << 1)
#define RCAR_VIN_IE_FOE					(1 << 0)

/* Video n Interrupt Status Register */
#define RCAR_VIN_INTS_FIS				(1 << 4)
#define RCAR_VIN_INTS_EFS				(1 << 1)
#define RCAR_VIN_INTS_FOS				(1 << 0)

/* Video n Data Mode Register */
#define RCAR_VIN_DMR_EXRGB					(1 << 8)
#define RCAR_VIN_DMR_BPSM					(1 << 4)
#define RCAR_VIN_DMR_DTMD_YCSEP				(1 << 1)
#define RCAR_VIN_DMR_DTMD_ARGB				(1 << 0)
#define RCAR_VIN_DMR_DTMD_YCSEP_YCBCR420	(3 << 0)

/* Video n Data Mode Register 2 */
#define RCAR_VIN_DMR2_VPS				(1 << 30)
#define RCAR_VIN_DMR2_HPS				(1 << 29)
#define RCAR_VIN_DMR2_FTEV				(1 << 17)
#define RCAR_VIN_DMR2_VLV(n)			((n & 0xf) << 12)

/* Setting CSI2 on R-Car Gen3*/	
#define RCAR_VIN_CSI_IFMD_DES2			(1 << 27) 
#define RCAR_VIN_CSI_IFMD_DES1			(1 << 26) 
#define RCAR_VIN_CSI_IFMD_DES0			(1 << 25) 
#define RCAR_VIN_CSI_IFMD_CSI_CHSEL(n)	(n << 0)

/* UDS */
#define RCAR_VIN_UDS_CTRL_AMD			(1 << 30)
#define RCAR_VIN_UDS_CTRL_BC			(1 << 20)
#define RCAR_VIN_UDS_CTRL_TDIPC			(1 << 1)

typedef struct _cam_info {
	char interlace;
	uint32_t sw;
	uint32_t sh;
	uint32_t dw;
	uint32_t dh;
	char sfmt[64];
	uint32_t dfmt;
	uint32_t dstride;
	uint32_t cx;
	uint32_t cy;
	uint32_t cw;
	uint32_t ch;
	uint32_t update;
} cam_info_t;

typedef struct _cam_buf {
	paddr_t	addr[RCAR_VIN_MAX_BUFFER];
	int done[RCAR_VIN_MAX_BUFFER];
	int latest;
	int release;
} cam_buf_t;

typedef struct _rcar_vin {
	int iid;	
	int tid;
	int	chid;
	int	coid;
	int irq;
	pthread_cond_t cond;
	pthread_mutex_t mutex;
	pthread_attr_t attr;
	struct sched_param param;
	struct sigevent event;
	unsigned frm_end;
	void **frm_bufs;
	int frm_nbufs;
	int nbufs;
	cam_buf_t buf;
	cam_info_t cam;
	uintptr_t pbase;
	uintptr_t vbase;
} rcar_vin_t;

typedef struct  _capture_context {
	int enable;
	int is_runing;
	int active_dev;
	int screen_idx;
	rcar_vin_t vin;
	rcar_csi2_t csi;
} rcar_context_t;

rcar_context_t* rcar_context;

#endif // __VIN_H__
