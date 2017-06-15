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
 
#ifndef _VSP1_REG_H_
#define _VSP1_REG_H_

#define VSPD0_REG_BASE 		0xFEA20000
#define VSPD1_REG_BASE 		0xFEA28000
#define VSPD2_REG_BASE 		0xFEA30000
#define VSPI0_REG_BASE		0xFE9A0000
#define VSP_REG_SIZE 		0x8000

#define VSPD0_IRQ 		(466 + 32)
#define VSPD1_IRQ 		(467 + 32)
#define VSPD2_IRQ 		(468 + 32)
#define VSPI0_IRQ 		(444 + 32)

#define VI6_RPF_NUM 5
#define VI6_WPF_NUM 4
#define VI6_CMD_NUM	VI6_WPF_NUM
#define VI6_UDS_NUM 3
#define VI6_BRU_CHN_NUM 5

#define VI6_CTRL_REG_OFF	0
#define VI6_DL_CTRL			0x0100
#define VI6_RPF_OFF			0x0300
#define VI6_WPF_OFF			0x1000
#define VI6_DPR_OFF			0x2000
#define VI6_UDS_OFF			0x2300
#define VI6_BRU_OFF			0x2c00
#define VI6_LIF_OFF			0x3b00

/* Other definition */
#define DPR_SET_SRU				(16)
#define DPR_SET_UDS0			(17)
#define DPR_SET_UDS1			(18)
#define DPR_SET_UDS2			(19)
#define DPR_SET_LUT				(22)
#define DPR_SET_CLU				(29)
#define DPR_SET_HST				(30)
#define DPR_SET_HSI				(31)
#define DPR_SET_BRUin0			(23)
#define DPR_SET_BRUin1			(24)
#define DPR_SET_BRUin2			(25)
#define DPR_SET_BRUin3			(26)
#define DPR_SET_BRUin4			(49)
#define DPR_SET_WPF0			(56)
#define DPR_SET_WPF1			(57)
#define DPR_SET_WPF2			(58)
#define DPR_SET_WPF3			(59)
#define DPR_SET_NOT_USED		(63)
#define DPR_SET_WPF_FPORCH		(0x00000500)

#define SRC_RPF_MASTER_LAYER_SET 	(2)
#define SRC_RPF_SUB_LAYER_SET 		(1)
#define SRC_RPF_MASK 			    (3)

#define VSP_UDS_SCALE_1_8		(0x8000)
#define VSP_UDS_SCALE_1_4		(0x4000)	/* quarter */
#define VSP_UDS_SCALE_1_2		(0x2000)	/* half */
#define VSP_UDS_SCALE_1_1		(0x1000)
#define VSP_UDS_SCALE_2_1		(0x0800)	/* double */
#define VSP_UDS_SCALE_4_1		(0x0400)	/* quadruple */
#define VSP_UDS_SCALE_8_1		(0x0200)
#define VSP_UDS_SCALE_16_1		(0x0100)	/* maximum scale */
#define VSP_UDS_SCALE_MANT		(0xF000)
#define VSP_UDS_SCALE_FRAC		(0x0FFF)

#define VSP_UDS_HSZCLIP_HCEN    (0x10000000)
#define VSP_WPF_HSZCLIP_HCEN    (0x10000000)

#define FCPV_OFFSET             (0xF000)
#define FCPV_CFG0			    (0x0004)
#define FCPV_RST				(0x0010)
#define FCPV_STA				(0x0018)
#define FCPV_CFG0_FCPVSEL       (1<<1)

#define CMD_START				(0x00000001)
/*	Struct for general control registers 	*/
typedef volatile struct {
	uint32_t irq_enb;
	uint32_t irq_sta;
	uint32_t reserved[1];
} ctrl_wpf_irq_t;

typedef volatile struct {
	uint32_t dsp_irq_enb;
	uint32_t dsp_irq_sta;
	uint32_t reserved[1];
} ctrl_dsp_irq_t;

typedef volatile struct {
	uint32_t cmd[VI6_CMD_NUM];
	uint32_t reserved1[2];
	uint32_t clk_dcswt;
	uint32_t reserved2[3];
	uint32_t sreset;
	uint32_t reserved3[3];
	uint32_t status;
	uint32_t reserved4[3];
	ctrl_wpf_irq_t wpf_irq[VI6_WPF_NUM];
	ctrl_dsp_irq_t dsp_irq;
	uint32_t line_count[VI6_WPF_NUM];
} vi6_ctrl_t;

/*	Struct for RPF, 5 channel 	*/
typedef volatile struct {
	uint32_t src_bsize;
	uint32_t src_esize;
	uint32_t infmt;
	uint32_t dswap;
	uint32_t loc;
	uint32_t alph_sel;
	uint32_t vrtcol_set;
	uint32_t mskctrl;
	uint32_t mskset0;
	uint32_t mskset1;
	uint32_t ckey_ctrl;
	uint32_t ckey_set0;
	uint32_t ckey_set1;
	uint32_t srcm_pstride;
	uint32_t srcm_astride;
	uint32_t srcm_addr_y;
	uint32_t srcm_addr_c0;
	uint32_t srcm_addr_c1;
	uint32_t srcm_addr_ai;
	uint32_t reserved[45];
} rpf_t;

/*	Struct for WPF, 4 channel 	*/
typedef volatile struct {
	uint32_t src_rpf;
	uint32_t hszclip;
	uint32_t vszclip;
	uint32_t outfmt;
	uint32_t dswap;
	uint32_t rndctrl;
	uint32_t reserved1[1];
	uint32_t dstm_stride_y;
	uint32_t dstm_stride_c;
	uint32_t dstm_addr_y;
	uint32_t dstm_addr_c0;
	uint32_t dstm_addr_c1;
	uint32_t reserved2[52];
} wpf_t;

/*	Struct for UDS, 3 channel 	*/
typedef volatile struct {
	uint32_t ctrl;
	uint32_t scale;
	uint32_t alpth;
	uint32_t alpval;
	uint32_t pass_bwidt;
	uint32_t hphase;
	uint32_t ipc;
	uint32_t hszclip;
	uint32_t reserved1[1];
	uint32_t clip_size;
	uint32_t fill_color;
	uint32_t reserved2[53];
} uds_t;

/*	Struct for LIF 	*/
typedef volatile struct {
	uint32_t ctrl;
	uint32_t csbth;
} lif_t;

/*	Struct for DPR 	*/
typedef volatile struct {
	uint32_t rpf_route[5];
	uint32_t wpf_fporch[4];
	uint32_t sru_route;
	uint32_t uds_route[3];
	uint32_t reserved1[2];
	uint32_t lut_route;
	uint32_t clu_route;
	uint32_t hst_route;
	uint32_t hsi_route;
	uint32_t bru_route;
	uint32_t reserved2[1];
	uint32_t hgo_smppt;
	uint32_t hgt_smppt;
} dpr_t;

/*	Struct for BRU 	*/
typedef volatile struct {
	uint32_t ctrl;
	uint32_t bld;
} bru_chan_t;

typedef volatile struct {
	uint32_t inctrl;
	uint32_t virrpf_size;
	uint32_t virrpf_loc;
	uint32_t virrpf_col;
	bru_chan_t bru_chan[4];	//BRU channel for A, B, C, D
	uint32_t rop;
	bru_chan_t bru_chan_5; //BRU channel for E
} bru_t;

typedef volatile struct {
	uint32_t ctrl;
	uint32_t hdr_addr0;
	uint32_t hdr_addr1;
	uint32_t hdr_addr2;
	uint32_t hdr_addr3;
	uint32_t swap;
	uint32_t reserve;
	uint32_t ext_ctrl;
	uint32_t body_size0;
} dl_t;

typedef struct {
	uint32_t	reg_base;
	uint32_t	reg_size;
	int 		irq;
	int			cpg_stp_bit;	//Module stop bit number of VSP in MSTPSR1
	int			num_uds;
} vsp_private_t;

typedef struct {
	vi6_ctrl_t	*vi6_ctrl;
	rpf_t		*rpf;
	wpf_t		*wpf;
	uds_t		*uds;
	dpr_t		*dpr;
	bru_t		*bru;
	lif_t		*lif;
	dl_t		*dl;
} vsp_reg_t;

#endif
