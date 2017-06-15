/*
 * $QNXLicenseC:
 * Copyright 2014-2016, QNX Software Systems. 
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
 
#ifndef _VSP_DRV_H_
#define _VSP_DRV_H_
#include <sys/neutrino.h>
#include <stdint.h>
#include "vsp_reg.h"

#define VSPD_INPUT_IMAGE_NUM 5

#define VI6_WPFn_IRQ_FRE		(1 << 0)
#define VI6_WPFn_IRQ_DFE		(1 << 1)
#define VI6_DISP_IRQ_STA_DST	(1 << 8)

/* Display List Control */
#define VI6_DL_CTRL 0x0100
#define VI6_DL_CTRL_AR_WAIT (256 << 16)
#define VI6_DL_CTRL_DL_ENABLE (1 << 12 | 1 << 8 | 1 << 4 | 1 << 0)
#define VI6_DL_CTRL_CFM0 (1 << 2)
#define VI6_DL_CTRL_NH0 (1 << 1)
#define VI6_DL_SWAP 0x0114
#define VI6_DL_SWAP_LWS (1 << 2) /* LWORD swap */
#define VI6_DL_SWAP_WDS (1 << 1) /* WORD swap */
#define VI6_DL_SWAP_BTS (1 << 0) /* BYTE swap */
#define VI6_DL_BODY_SIZE 0x0120
#define VI6_DL_BODY_SIZE_UPD0 (1 << 24)

/* Read Pixel Formatter */
#define VI6_RPFn_INFMT_CSC				(1 << 8)
#define VI6_RPFn_INFMT_BT601			(0 << 9)
#define VI6_RPFn_INFMT_BT709			(2 << 9)

/* Up Down Scaler */
#define VI6_UDSn_CTRL_AON				(1 << 25)
#define VI6_UDSn_CTRL_TDIPC				(1 << 1)
#define VSP_UDSn_CTRL_AMDSLH            (1 << 2)
#define VI6_UDSn_IPC_FIELD_TOP			(0 << 27)
#define VI6_UDSn_IPC_FIELD_BOTTOM		(1 << 27)


#define VSP_INT_EVENT		            (0x69)

#define PIPE_VALIDATE(pipe, ret) {\
	if (pipe >= VSPD_INPUT_IMAGE_NUM) \
		ret; \
}


/* VSP Manager APIs return codes */
#define R_VSP_OK			(0)
#define	R_VSP_NG			(-1)	/* abnormal termination */
#define	R_VSP_PARAERR		(-2)	/* illegal parameter */
#define	R_VSP_SEQERR		(-3)	/* sequence error */
#define R_VSP_QUE_FULL		(-4)	/* request queue full */
#define R_VSP_CANCEL		(-5)	/* processing was canceled */
#define R_VSP_ALREADY_USED	(-6)	/* already used all channel */
#define R_VSP_OCCUPY_CH	    (-7)	/* occupy channel */
#define R_VSP_DRIVER_ERR	(-10)	/* IP error(in driver) */

#define VSP_MASTER_LAYER_DEFAULT 0

#define INPUT_WPF_HSIZE_MAX     (0x100)

#define VSP_ROUND_UP(base, div) \
	(((base) + ((div) - 1)) / (div))
#define VSP_ROUND_DOWN(base, div) \
	((base) / (div))
#define VSP_CLIP0(base, sub) \
	(((base) > (sub)) ? ((base) - (sub)) : 0)

#define get_reg_off(reg) ((uintptr_t)reg - (uintptr_t)dev->reg_base_ptr)

#define VSP_VP_TO_INT(addr)	\
	((unsigned int)((uint32_t)(addr)))

#define VSP_DL_HARD_TO_VIRT(addr) \
	(dev->pvdata->dlmemory->vaddr + \
	((uint32_t)(addr) - (uint32_t)(dev->pvdata->dlmemory->paddr)))
#define	VSP_COMPOSITION_PIPELINE_MAX	4

enum VSP_IDX_LIST {
	VSPD0	=	0,
	VSPD1	=	1,
	VSPD2	=	2,
	VSPD3	=	3,
	VSPI0	=	4,
	VSPI1	=	5,
	VSPI2	=	6,
	DEVICE_NONE	=	-1,
};

enum{
	VSPCORE_RGB332=0,
	VSPCORE_RGBX444,
	VSPCORE_RGB444X,
	VSPCORE_RGBx555=4,
	VSPCORE_RGB555x,
	VSPCORE_RGB565,	
	VSPCORE_RGBPX666,
	VSPCORE_RGB666XP,
	VSPCORE_RGBX666P,
	VSPCORE_RGBP666X,
	VSPCORE_RGBPx666,
	VSPCORE_RGBx666P,
	VSPCORE_RGBP666,
	VSPCORE_RGB666P,
	VSPCORE_RGBX666,
	VSPCORE_RGB666X,
	VSPCORE_RGBx666,
	VSPCORE_RGB666x,
	VSPCORE_RGBP888,
	VSPCORE_RGB888P,
	VSPCORE_RGB888,	
	VSPCORE_RGBx666x,
	VSPCORE_RGBXX666,
	VSPCORE_BGR888,	
	VSPCORE_RGBP444,
	VSPCORE_RGB444P,
	VSPCORE_RGBP555,
	VSPCORE_RGB555P,
	VSPCORE_BGRP444,
	VSPCORE_BGR444P,
	VSPCORE_BGRP555,
	VSPCORE_BGR555P,
	VSPCORE_BGRx666,
	VSPCORE_BGRX888,
	VSPCORE_RGBX565,
	VSPCORE_YUV444SP=64,
	VSPCORE_YUV422SP,
	VSPCORE_YUV420SP,
	VSPCORE_YUV444I=70,
	VSPCORE_YUV422Itype0,
	VSPCORE_YUV422Itype1,
	VSPCORE_YUV420I,
	VSPCORE_YUV444P,
	VSPCORE_YUV422P,
	VSPCORE_YUV420P,
	VSPCORE_RGBCLUT=0x3F,
	VSPCORE_YUVCLUT=0x7F,
};

#define OPACITY_FULL 	(1<<28)
#define ORDER_YUY2		(1<<24)	
#define ORDER_UYVY		(2<<24)
#define ORDER_YVYU		(4<<24)

typedef struct {
	int 		Id;				/* VSP ID number */
	int 		mWidth; 		/* Master layer width */
	int 		mHeight;		/* Master layer height */
	int 		mFormat; 		/* Master layer color format */
	uint32_t 	mColor;			/* Master layer initial color */
	int			mode;			/* VSP usage mode */
	uint32_t 	composited_pbuffer;
}vsp_info_t;

typedef struct {
	uint32_t 	y_rgb;
	uint32_t 	c0;
	uint32_t 	c1;
} img_add_t;

typedef struct {
	int 	width;
	int 	height;
	int		hcoord;
	int		vcoord;
	int 	fmt;
	img_add_t	addr;
}image_t;

typedef struct {
	int			vsp_pipe_id;
	image_t		src;
	int	 		src_rect[4];
	image_t 	dst;
} vsp_pipe_t;
   
enum VSP_STATE {
	VSP_STOPPING,
	VSP_RUNNING,
	VSP_INITIALIZED,
};

typedef struct {
	volatile int active;
	int connected_uds;
	uint32_t rpf_id;
	uint32_t src_bsize;
	uint32_t src_esize;
	uint32_t infmt;
	uint32_t dswap;
	uint32_t vrtcol_set;
	uint32_t loc;
	uint32_t alph_sel;
	uint32_t srcm_pstride;
	uint32_t srcm_addr_y;
	uint32_t srcm_addr_c0;
	uint32_t srcm_addr_c1;
	uint32_t srcm_addr_ai;
} rpf_par_t;

typedef struct {
	uint32_t in_use;
	uint32_t connected_rpf;
	uint32_t uds_id;
	uint32_t need_update_param;
	uint32_t need_remove_route;
	uint32_t need_add_route;
	uint32_t ctrl;
	uint32_t scale;
	uint32_t alpth;
	uint32_t alpval;
	uint32_t pass_bwidt;
	uint32_t hphase;
	uint32_t ipc;
	uint32_t hszclip;
	uint32_t clip_size;
	uint32_t fill_color;
} uds_par_t;

typedef struct {
	uint32_t ctrl;
	uint32_t csbth;
} lif_par_t;

typedef struct {
	int		wpf_id;
	uint32_t src_rpf;
	uint32_t hszclip;
	uint32_t vszclip;
	uint32_t outfmt;
	uint32_t dswap;
	uint32_t rndctrl;
	uint32_t dstm_stride_y;
	uint32_t dstm_stride_c;
	uint32_t dstm_addr_y;
	uint32_t dstm_addr_c0;
	uint32_t dstm_addr_c1;
} wpf_par_t;

typedef struct {
	uint32_t rpf_route;
	uint32_t sru_route;
	uint32_t uds_route;
	uint32_t lut_route;
	uint32_t clu_route;
	uint32_t hst_route;
	uint32_t hsi_route;
	uint32_t bru_route;
	uint32_t hgo_smppt;
} dpr_par_t;

typedef struct {
	rpf_par_t rpf_par[VSPD_INPUT_IMAGE_NUM];
	uds_par_t uds_par[VI6_UDS_NUM];
	dpr_par_t dpr_par;
	wpf_par_t wpf_par[VI6_WPF_NUM];
	lif_par_t lif_par;	
} vsp_par_t;

typedef struct {
	uint32_t 		reg_base;
	uintptr_t 		reg_base_ptr;
	int				reg_size;
	int				vsp_idx;
    struct sigevent intrevent;
    int             irq;
    int             iid;
    int             irqchan;
    int             irqcoid;
    pthread_mutex_t vsp_mutex;
	int				num_uds;
	vsp_info_t 		info;
	vsp_par_t		param;
	vsp_reg_t		reg;
	struct 			vsp_private_data *pvdata;
	int				state;
} vsp_dev_t;

/* DL mode */
enum {
	DL_MODE_SINGLE = 0,
	DL_MODE_MANUAL_REPEAT,
	DL_MODE_AUTO_REPEAT,
	DL_MODE_HEADER_LESS_MANUAL_REPEAT,
	DL_MODE_HEADER_LESS_AUTO_REPEAT,
};

#define USE_WPF 0
#define VSPD_CHANNEL_NUMBER 4
#define DISPLAY_LIST_NUM 8
#define DISPLAY_LIST_BODY_NUM 8
#define DL_HEADER_SIZE 128
#define DL_BODY_SIZE 512
#define DL_LINKED_BODY_NUM 1
/* use 8 work body for switching a linked body */
#define DL_BODY_NUM_FOR_WORK 1


#define VSP_STATUS_LOOP_CNT		(50)
/* define register set value */
#define VSP_CMD_STRCMD			(0x00000001)
#define VSP_IRQ_FRMEND			(0x00000002)
#define VSP_SRESET_WPF0			(0x00000001)
#define VSP_STATUS_WPF0			(0x00000100)

#define VSP_FCPV_RST_SOFTRST	(0x00000001)
#define VSP_FCPV_STA_ACT        (0x00000001)

/* display list header format */
struct display_header {
	/* zero_bits:29 + num_list_minus1:3 */
	uint32_t num_list_minus1;
	struct {
		/* zero_bits:15 + num_bytes:17 */
		uint32_t num_bytes;
		uint32_t plist;
	} display_list[DISPLAY_LIST_BODY_NUM];
	uint32_t pnext_header;
	/* zero_bits:30 + current_frame_int_enable:1 + */
	/* next_frame_auto_start:1 */
	uint32_t int_auto;

	/* if (VI6_DL_EXT_CTRL.EXT) */
	//uint32_t zero_bits;
	/* zero_bits:6 + pre_ext_dl_exec:1 + */
	/* post_ext_dl_exec:1 + zero_bits:8 + pre_ext_dl_num_cmd:16 */
	//uint32_t pre_post_num;
	//uint32_t pre_ext_dl_plist;
	/* zero_bits:16 + post_ext_dl_num_cmd:16 */
	//uint32_t post_ext_dl_num_cmd;
	//uint32_t post_ext_dl_p_list;
};

/* display list body format */
struct display_list { /* 8byte align */
	uint32_t set_address; /* resistor address */
	uint32_t set_data; /* resistor data */
};

enum {
	DL_LIST_OFFSET_RPF0 = 0,
	DL_LIST_OFFSET_RPF1,
	DL_LIST_OFFSET_RPF2,
	DL_LIST_OFFSET_RPF3,
	DL_LIST_OFFSET_RPF4,
	DL_LIST_OFFSET_CTRL,
	DL_LIST_OFFSET_LUT,
	DL_LIST_OFFSET_CLUT,
};

#define vsp_phy_addr_t uint64_t

struct dl_head;
struct dl_body;

struct dl_body {
	int use;
	int reg_count;
	vsp_phy_addr_t paddr;
	struct display_list *dlist;
	uint32_t dlist_offset;
	int flag;
	struct dl_body *next;
};

struct dl_head {
	int size;
	//int use;
	vsp_phy_addr_t paddr;
	struct display_header *dheader;
	uint32_t dheader_offset;
	struct dl_body *dl_body_list[DL_BODY_NUM_FOR_WORK];
	int flag;
	struct dl_head *next;
};

struct dl_memory {
	int size;
	vsp_phy_addr_t paddr;
	void *vaddr;
	int flag;
	int dl_mode;
	intrspin_t lock;

	/* header mode */
	struct dl_head head[DISPLAY_LIST_NUM];
	struct dl_body body[DL_BODY_NUM_FOR_WORK*DISPLAY_LIST_NUM];
	int active_body_index;
	int start;

	/* header less mode */
	struct dl_body single_body[DISPLAY_LIST_NUM];
	struct dl_body *active_body;
	struct dl_body *active_body_now;
	struct dl_body *active_body_next_set;
	struct dl_body *next_body;
	struct dl_body *pending_body;
};

struct vsp_private_data {
	int active;
	intrspin_t lock;
	struct dl_memory *dlmemory;
};
#endif
