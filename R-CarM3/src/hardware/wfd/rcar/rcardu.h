/*
 * $QNXLicenseC:
 * Copyright 2014-2015, QNX Software Systems. 
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
 
 
#ifndef __RCARDU_REGS_H__
#define __RCARDU_REGS_H__
#include <time.h>
#include <pthread.h>
#include <sys/siginfo.h>
#include <graphics/display.h>
#include <graphics/disputil.h>
#include <sys/rsrcdbmgr.h>
#include <sys/rsrcdbmsg.h>
#include <errno.h>

#include <wfdqnx/wfdcfg.h>
#include <wfdqnx/wfdcfg_rcardu.h>

#define BIT(x)		(1<<x)

/* Pixel clock source declaration */
#define RCAR_DU_INTERNAL_CLOCK		400000000	/* S2D1φ */
#define RCAR_DU_INTERNAL_CLOCK_WS10	200000000 	/* internal clock workaround of WS1.0 */

#define  R_CAR_DU_VSYNC_PULSE	0x5B

#define DU_GRP_0	0
#define DU_GRP_1	1

#define DU_PLANE_1	1
#define DU_PLANE_2	2
#define DU_PLANE_3	3
#define DU_PLANE_4	4

#define du_priv_reg_ptr(x) 		(uint32_t volatile *)(((unsigned char volatile *)port->du_cfg->priv_vir_base)+x)
#define du_grp_reg_ptr(x) 		(uint32_t volatile *)(((unsigned char volatile *)port->du_cfg->grp_vir_base)+x)
#define du_cmn_reg_ptr(x) 		(uint32_t volatile *)(((unsigned char volatile *)dev->du_vir_reg_base)+x)

/* Product Register */
#define RCAR_PRR                0xFFF00044
#define RCAR_PRR_REGSIZE        0x04
#define RCAR_PRODUCT_SHIFT      8
#define RCAR_PRODUCT_MASK       0x7F
#define RCAR_PRODUCT_H3         0x4F
#define RCAR_PRODUCT_M3W        0x52

#define RCAR_REVISION_SHIFT     0
#define RCAR_REVISION_MASK      0xFF
#define RCAR_REVISION_ES10      0x00
#define RCAR_REVISION_ES11      0x01
#define RCAR_REVISION_ES20      0x10
#define RCAR_REVISION_ES30      0x20

/* base of registers */
#define DU_BASE			0xFEB00000
/* DU group 0 */
#define DU0_OFF			0x00000000
#define DU1_OFF			0x00030000
/* DU group 1 */
#define DU2_OFF			0x00040000
#define DU3_OFF			0x00070000
/* DU group gap */
#define GROUP_GAP		(DU2_OFF - DU0_OFF)

#define DU_REGSIZE		0x80000

#define DU0_IRQ			(256 + 32)
#define DU1_IRQ			(268 + 32)
#define DU2_IRQ			(269 + 32)
#define DU3_IRQ			(270 + 32)

/* Display Unit System Control Register Configuration */
#define DSYSR		du_priv_reg_ptr(0x0000)
#define GRP_DSYSR	du_grp_reg_ptr(0x0000)
#define DSMR		du_priv_reg_ptr(0x0004)
#define DSSR		du_priv_reg_ptr(0x0008)
#define DSRCR		du_priv_reg_ptr(0x000C)
#define DIER		du_priv_reg_ptr(0x0010)
#define CPCR		du_priv_reg_ptr(0x0014)
#define DPPR		du_grp_reg_ptr(0x00018)
#define DEFR		du_priv_reg_ptr(0x0020)
#define DEFR5		du_grp_reg_ptr(0x000E0)
#define DDLTR		du_cmn_reg_ptr(0x000E4)
#define DEFR6		du_grp_reg_ptr(0x000E8)
#define DEFR7		du_grp_reg_ptr(0x000EC)
#define DEFR8		du_grp_reg_ptr(0x20020)
#define DOFLR2		du_grp_reg_ptr(0x20024)
#define DIDSR		du_grp_reg_ptr(0x20028)
#define DEFR10		du_grp_reg_ptr(0x20038)
#define DDTHCR		du_grp_reg_ptr(0x2003C)
#define DPLLCR		du_grp_reg_ptr(0x20044)
#define DPLLC2R		du_grp_reg_ptr(0x20048)

/* Display Timing Generation Register Configuration */
#define HDSR		du_priv_reg_ptr(0x0040)
#define HDER		du_priv_reg_ptr(0x0044)
#define VDSR		du_priv_reg_ptr(0x0048)
#define VDER		du_priv_reg_ptr(0x004C)
#define HCR		    du_priv_reg_ptr(0x0050)
#define HSWR		du_priv_reg_ptr(0x0054)
#define VCR		    du_priv_reg_ptr(0x0058)
#define VSPR		du_priv_reg_ptr(0x005C)
#define EQWR		du_priv_reg_ptr(0x0060)
#define SPWR		du_priv_reg_ptr(0x0064)
#define CLAMPSR		du_priv_reg_ptr(0x0070)
#define CLAMPWR		du_priv_reg_ptr(0x0074)
#define DESR		du_priv_reg_ptr(0x0078)
#define DEWR		du_priv_reg_ptr(0x007C)

/* Display Attribute Register Configuration */
#define DOOR		du_priv_reg_ptr(0x0090)
#define CDER		du_priv_reg_ptr(0x0094)
#define BPOR	 	du_priv_reg_ptr(0x0098)
#define RINTOFSR	du_priv_reg_ptr(0x009C)

/* Display Plane Register Configuration */
#define	PLANE_GAP	(0x100/4)	/* Word count */
#define P1MR 		du_grp_reg_ptr(0x0100)
#define P1DSXR 		du_grp_reg_ptr(0x0110)
#define P1DSYR 		du_grp_reg_ptr(0x0114)
#define P1DPXR 		du_grp_reg_ptr(0x0118)
#define P1DPYR 		du_grp_reg_ptr(0x011C)
#define P1DDCR4     du_grp_reg_ptr(0x0190)

/* External Synchronization Control Register Configuration */
#define ESCR		du_priv_reg_ptr(((port->du_cfg->du_index%2) ? 0x1000 : 0x10000))

/* Dual Display Output Control Register Configuration */
#define DORCR		du_grp_reg_ptr(0x11000)
#define DPTSR		du_grp_reg_ptr(0x11004)
#define DS0PR 		du_grp_reg_ptr(0x11020)
#define DS1PR 		du_grp_reg_ptr(0x11024)

/* YC-RGB Conversion Coefficient Register Configuration */
#define YNCR 		du_grp_reg_ptr(0x14080)
#define YNOR 		du_grp_reg_ptr(0x14084)
#define CRNOR 		du_grp_reg_ptr(0x14088)
#define CBNOR 		du_grp_reg_ptr(0x1408C)
#define RCRCR 		du_grp_reg_ptr(0x14090)
#define GCRCR 		du_grp_reg_ptr(0x14094)
#define GCBCR 		du_grp_reg_ptr(0x14098)
#define BCBCR 		du_grp_reg_ptr(0x1409C)

/* Bit declaration */
/* DSYSR */
#define DSYSR_ILTS		(1 << 29)
#define DSYSR_DSEC		(1 << 20)
#define DSYSR_IUPD		(1 << 16)
#define DSYSR_DRES		(1 << 9)
#define DSYSR_DEN		(1 << 8)
#define DSYSR_TVM_MASTER	(0 << 6)
#define DSYSR_TVM_SWITCH	(1 << 6)
#define DSYSR_TVM_TVSYNC	(2 << 6)
#define DSYSR_TVM_MASK		(3 << 6)
#define DSYSR_SCM_INT_NONE	(0 << 4)
#define DSYSR_SCM_INT_SYNC	(2 << 4)
#define DSYSR_SCM_INT_VIDEO	(3 << 4)
#define DSYSR_SCM_MASK		(3 << 4)

/* DSMR */
#define DSMR_VSPM(n)		(n << 28)
#define DSMR_ODPM(n)		(n << 27)
#define DSMR_DIPM_DISP		(0 << 25)
#define DSMR_DIPM_CSYNC		(1 << 25)
#define DSMR_DIPM_DE		(3 << 25)
#define DSMR_DIPM_MASK		(3 << 25)
#define DSMR_CSPM(n)		(n << 24)
#define DSMR_DIL(n)		(n << 19)
#define DSMR_VSL(n)		(n << 18)
#define DSMR_HSL(n)		(n << 17)
#define DSMR_DDIS(n)		(n << 16)
#define DSMR_CDEL(n)		(n << 15)
#define DSMR_CDEM_CDE		(0 << 13)
#define DSMR_CDEM_LOW		(2 << 13)
#define DSMR_CDEM_HIGH		(3 << 13)
#define DSMR_CDEM_MASK		(3 << 13)
#define DSMR_CDED		(1 << 12)
#define DSMR_ODEV		(1 << 8)
#define DSMR_CSY_VH_OR		(0 << 6)
#define DSMR_CSY_333		(2 << 6)
#define DSMR_CSY_222		(3 << 6)
#define DSMR_CSY_MASK		(3 << 6)

/* DSSR */
#define DSSR_VC1FB_DSA0		(0 << 30)
#define DSSR_VC1FB_DSA1		(1 << 30)
#define DSSR_VC1FB_DSA2		(2 << 30)
#define DSSR_VC1FB_INIT		(3 << 30)
#define DSSR_VC1FB_MASK		(3 << 30)
#define DSSR_VC0FB_DSA0		(0 << 28)
#define DSSR_VC0FB_DSA1		(1 << 28)
#define DSSR_VC0FB_DSA2		(2 << 28)
#define DSSR_VC0FB_INIT		(3 << 28)
#define DSSR_VC0FB_MASK		(3 << 28)
#define DSSR_DFB(n)		(1 << ((n)+15))
#define DSSR_TVR		(1 << 15)
#define DSSR_FRM		(1 << 14)
#define DSSR_VBK		(1 << 11)
#define DSSR_RINT		(1 << 9)
#define DSSR_HBK		(1 << 8)
#define DSSR_ADC(n)		(1 << ((n)-1))

/* DSRCR */
#define DSRCR_TVCL		(1 << 15)
#define DSRCR_FRCL		(1 << 14)
#define DSRCR_VBCL		(1 << 11)
#define DSRCR_RICL		(1 << 9)
#define DSRCR_HBCL		(1 << 8)
#define DSRCR_ADCL(n)		(1 << ((n)-1))
#define DSRCR_MASK		0x0000cbff

/* DIER */
#define DIER_TVE		(1 << 15)
#define DIER_FRE		(1 << 14)
#define DIER_VBE		(1 << 11)
#define DIER_RIE		(1 << 9)
#define DIER_HBE		(1 << 8)
#define DIER_ADCE(n)		(1 << ((n)-1))

/* CPCR */
#define CPCR_CP4CE		(1 << 19)
#define CPCR_CP3CE		(1 << 18)
#define CPCR_CP2CE		(1 << 17)
#define CPCR_CP1CE		(1 << 16)

/* DPPR */
#define DPPR_DPE(n)		(1 << ((n)*4-1))
#define DPPR_DPS(n, p)		(((p)-1) << DPPR_DPS_SHIFT(n))
#define DPPR_DPS_SHIFT(n)	(((n)-1)*4)
#define DPPR_BPP16		(DPPR_DPE(8) | DPPR_DPS(8, 1))	/* plane1 */
#define DPPR_BPP32_P1		(DPPR_DPE(7) | DPPR_DPS(7, 1))
#define DPPR_BPP32_P2		(DPPR_DPE(8) | DPPR_DPS(8, 2))
#define DPPR_BPP32		(DPPR_BPP32_P1 | DPPR_BPP32_P2)	/* plane1 & 2 */

/* DEFR */
#define DEFR_CODE		(0x7773 << 16)
#define DEFR_EXSL		(1 << 12)
#define DEFR_EXVL		(1 << 11)
#define DEFR_EXUP		(1 << 5)
#define DEFR_VCUP		(1 << 4)
#define DEFR_DEFE		(1 << 0)

/* DEFR2 */
#define DEFR2_CODE		(0x7775 << 16)
#define DEFR2_DEFE2G		(1 << 0)

/* DEFR3 */
#define DEFR3_CODE		(0x7776 << 16)
#define DEFR3_EVDA		(1 << 14)
#define DEFR3_EVDM_1		(1 << 12)
#define DEFR3_EVDM_2		(2 << 12)
#define DEFR3_EVDM_3		(3 << 12)
#define DEFR3_VMSM2_EMA		(1 << 6)
#define DEFR3_VMSM1_ENA		(1 << 4)
#define DEFR3_DEFE3		(1 << 0)

/* DEFR4 */
#define DEFR4_CODE		(0x7777 << 16)
#define DEFR4_LRUO		(1 << 5)
#define DEFR4_SPCE		(1 << 4)

/* DEFR5 */
#define DEFR5_CODE		(0x66 << 24)
#define DEFR5_YCRGB2_DIS	(0 << 14)
#define DEFR5_YCRGB2_PRI1	(1 << 14)
#define DEFR5_YCRGB2_PRI2	(2 << 14)
#define DEFR5_YCRGB2_PRI3	(3 << 14)
#define DEFR5_YCRGB2_MASK	(3 << 14)
#define DEFR5_YCRGB1_DIS	(0 << 12)
#define DEFR5_YCRGB1_PRI1	(1 << 12)
#define DEFR5_YCRGB1_PRI2	(2 << 12)
#define DEFR5_YCRGB1_PRI3	(3 << 12)
#define DEFR5_YCRGB1_MASK	(3 << 12)
#define DEFR5_DEFE5		(1 << 0)

/* DDLTR */
#define DDLTR_CODE		(0x7766 << 16)
#define DDLTR_DLAR2		(1 << 6)
#define DDLTR_DLAY2		(1 << 5)
#define DDLTR_DLAY1		(1 << 1)

/* DEFR6 */
#define DEFR6_CODE		(0x7778 << 16)
#define DEFR6_ODPM12_D2SMR	(0 << 10)
#define DEFR6_ODPM12_DISP	(2 << 10)
#define DEFR6_ODPM12_CDE	(3 << 10)
#define DEFR6_ODPM12_MASK	(3 << 10)
#define DEFR6_ODPM02_DSMR	(0 << 8)
#define DEFR6_ODPM02_DISP	(2 << 8)
#define DEFR6_ODPM02_CDE	(3 << 8)
#define DEFR6_ODPM02_MASK	(3 << 8)
#define DEFR6_TCNE2		(1 << 6)
#define DEFR6_MLOS1		(1 << 2)
#define DEFR6_DEFAULT		(DEFR6_CODE | DEFR6_TCNE2)
#define DEFR6_ODPM_D2SMR(du_idx)	(0 << (2*du_idx + 4))
#define DEFR6_ODPM_DISP(du_idx)		(2 << (2*du_idx + 4))
#define DEFR6_ODPM_CDE(du_idx)		(3 << (2*du_idx + 4))

/* DD1SSR */
#define DD1SSR_TVR		(1 << 15)
#define DD1SSR_FRM		(1 << 14)
#define DD1SSR_BUF		(1 << 12)
#define DD1SSR_VBK		(1 << 11)
#define DD1SSR_RINT		(1 << 9)
#define DD1SSR_HBK		(1 << 8)
#define DD1SSR_ADC(n)		(1 << ((n)-1))

/* DD1SRCR */
#define DD1SRCR_TVR		(1 << 15)
#define DD1SRCR_FRM		(1 << 14)
#define DD1SRCR_BUF		(1 << 12)
#define DD1SRCR_VBK		(1 << 11)
#define DD1SRCR_RINT		(1 << 9)
#define DD1SRCR_HBK		(1 << 8)
#define DD1SRCR_ADC(n)		(1 << ((n)-1))

/* DD1IER */
#define DD1IER_TVR		(1 << 15)
#define DD1IER_FRM		(1 << 14)
#define DD1IER_BUF		(1 << 12)
#define DD1IER_VBK		(1 << 11)
#define DD1IER_RINT		(1 << 9)
#define DD1IER_HBK		(1 << 8)
#define DD1IER_ADC(n)		(1 << ((n)-1))

/* DEFR8 */
#define DEFR8_CODE		(0x7790 << 16)
#define DEFR8_VSCS		(1 << 6)
#define DEFR8_DRGBS_DU(n)	((n) << 4)
#define DEFR8_DRGBS_MASK	(3 << 4)
#define DEFR8_DEFE8		(1 << 0)

/* DOFLR */
#define DOFLR_CODE		(0x7790 << 16)
#define DOFLR_HSYCFL1		(1 << 13)
#define DOFLR_VSYCFL1		(1 << 12)
#define DOFLR_ODDFL1		(1 << 11)
#define DOFLR_DISPFL1		(1 << 10)
#define DOFLR_CDEFL1		(1 << 9)
#define DOFLR_RGBFL1		(1 << 8)
#define DOFLR_HSYCFL0		(1 << 5)
#define DOFLR_VSYCFL0		(1 << 4)
#define DOFLR_ODDFL0		(1 << 3)
#define DOFLR_DISPFL0		(1 << 2)
#define DOFLR_CDEFL0		(1 << 1)
#define DOFLR_RGBFL0		(1 << 0)

/* DIDSR */
#define DIDSR_CODE		(0x7790 << 16)
#define DIDSR_LCDS_DCLKIN(n)	(0 << (8 + (n) * 2))
#define DIDSR_LCDS_LVDS0(n)	(2 << (8 + (n) * 2))
#define DIDSR_LCDS_LVDS1(n)	(3 << (8 + (n) * 2))
#define DIDSR_LCDS_MASK(n)	(3 << (8 + (n) * 2))
#define DIDSR_PDCS_CLK(n, clk)	(clk << ((n) * 2))
#define DIDSR_PDCS_MASK(n)	(3 << ((n) * 2))

/* DEF10R */
#define DEF10R_CODE		(0x7795 << 16)
#define DEF10R_VSPF1_RGB	(0 << 14)
#define DEF10R_VSPF1_YC		(1 << 14)
#define DEF10R_DOCF1_RGB	(0 << 12)
#define DEF10R_DOCF1_YC		(1 << 12)
#define DEF10R_VCDF0_YCBCR444	(0 << 11)
#define DEF10R_VCDF0_YCBCR422	(1 << 11)
#define DEF10R_VSPF0_RGB	(0 << 10)
#define DEF10R_VSPF0_YC		(1 << 10)
#define DEF10R_DOCF0_RGB	(0 << 8)
#define DEF10R_DOCF0_YC		(1 << 8)
#define DEF10R_TSEL_H3_TCON1	(0 << 1) /* DEF10R2 register only (DU2/DU3) */
#define DEF10R_DEFE10		(1 << 0)

/* DPLLCR */
#define DPLLCR_CODE		(0x95 << 24)
#define DPLLCR_PLCS1		(1 << 23)
#define DPLLCR_PLCS0		(1 << 20)
#define DPLLCR_CLKE		(1 << 18)
#define DPLLCR_FDPLL(n)		((n) << 12)	/* n=0 Setting prohibited */
/* H'00 to H'26, H'78 to H'7F: Setting prohibited.*/
#define DPLLCR_N(n)		((n) << 5)
#define DPLLCR_M(n)		((n) << 3)
#define DPLLCR_STBY		(1 << 2)
#define DPLLCR_INCS_DPLL01_DOTCLKIN02	(0 << 0)
#define DPLLCR_INCS_DPLL01_DOTCLKIN13	(1 << 1)

/* DPLLC2R */
#define DPLLC2R_CODE		(0x95 << 24)
#define DPLLC2R_SELC		(1 << 12)
#define DPLLC2R_M(n)		((n) << 8)
#define DPLLC2R_FDPLL(n)	((n) << 0)	/* n=0 Setting prohibited */

/* DOOR */
#define DOOR_RGB(r, g, b)	(((r) << 18) | ((g) << 10) | ((b) << 2))
/* CDER */
#define CDER_RGB(r, g, b)	(((r) << 18) | ((g) << 10) | ((b) << 2))
/* BPOR */
#define BPOR_RGB(r, g, b)	(((r) << 18) | ((g) << 10) | ((b) << 2))

/* PnMR */
#define PnMR_VISL_VIN0		(0 << 26)	/* use Video Input 0 */
#define PnMR_VISL_VIN1		(1 << 26)	/* use Video Input 1 */
#define PnMR_VISL_VIN2		(2 << 26)	/* use Video Input 2 */
#define PnMR_VISL_VIN3		(3 << 26)	/* use Video Input 3 */
#define PnMR_YCDF_YUYV		(1 << 20)	/* YUYV format */
#define PnMR_TC_R		(0 << 17)	/* Tranparent color is PnTC1R */
#define PnMR_TC_CP		(1 << 17)	/* Tranparent color is color palette */
#define PnMR_WAE		(1 << 16)	/* Wrap around Enable */
#define PnMR_SPIM_NOTP		(4 << 12)	/* No Transparent */
#define PnMR_SPIM_ALP		(5 << 12)	/* Alpha Blending */
#define PnMR_SPIM_EOR		(6 << 12)	/* EOR */
#define PnMR_SPIM_TP_OFF	(1 << 14)	/* No Transparent Color */
#define PnMR_CPSL_CP1		(0 << 8)	/* Color Palette selected 1 */
#define PnMR_CPSL_CP2		(1 << 8)	/* Color Palette selected 2 */
#define PnMR_CPSL_CP3		(2 << 8)	/* Color Palette selected 3 */
#define PnMR_CPSL_CP4		(3 << 8)	/* Color Palette selected 4 */
#define PnMR_DC			(1 << 7)	/* Display Area Change */
#define PnMR_BM_MD		(0 << 4)	/* Manual Display Change Mode */
#define PnMR_BM_AR		(1 << 4)	/* Auto Rendering Mode */
#define PnMR_BM_AD		(2 << 4)	/* Auto Display Change Mode */
#define PnMR_BM_VC		(3 << 4)	/* Video Capture Mode */
#define PnMR_DDDF_8BPP		(0 << 0)	/* 8bit */
#define PnMR_DDDF_16BPP		(1 << 0)	/* 16bit or 32bit */
#define PnMR_DDDF_ARGB		(2 << 0)	/* ARGB */
#define PnMR_DDDF_YC		(3 << 0)	/* YC */
#define PnMR_DDDF_MASK		(3 << 0)

/* PnDDCR4 */
#define PnDDCR4_CODE		(0x7766 << 16)
#define PnDDCR4_VSPS		(1 << 13)
#define PnDDCR4_SDFS_RGB	(0 << 4)
#define PnDDCR4_SDFS_YC		(5 << 4)
#define PnDDCR4_SDFS_MASK	(7 << 4)
#define PnDDCR4_EDF_NONE	(0 << 0)
#define PnDDCR4_EDF_ARGB8888	(1 << 0)
#define PnDDCR4_EDF_RGB888	(2 << 0)
#define PnDDCR4_EDF_RGB666	(3 << 0)
#define PnDDCR4_EDF_MASK	(7 << 0)

/* ESCR */
#define ESCR_DCLKOINV		(1 << 25)
#define ESCR_DCLKSEL_DCLKIN	(0 << 20)
#define ESCR_DCLKSEL_CLKS	(1 << 20)
#define ESCR_DCLKSEL_MASK	(1 << 20)
#define ESCR_DCLKDIS		(1 << 16)
#define ESCR_SYNCSEL_OFF	(0 << 8)
#define ESCR_SYNCSEL_EXVSYNC	(2 << 8)
#define ESCR_SYNCSEL_EXHSYNC	(3 << 8)
#define ESCR_FRQSEL_MASK	(0x3f << 0)

/* DORCR */
#define DORCR_PG2T		(1 << 30)
#define DORCR_DK2S		(1 << 28)
#define DORCR_PG2D_DS1		(0 << 24)
#define DORCR_PG2D_DS2		(1 << 24)
#define DORCR_PG2D_FIX0		(2 << 24)
#define DORCR_PG2D_DOOR		(3 << 24)
#define DORCR_PG2D_MASK		(3 << 24)
#define DORCR_DR1D		(1 << 21)
#define DORCR_PG1D_DS1		(0 << 16)
#define DORCR_PG1D_DS2		(1 << 16)
#define DORCR_PG1D_FIX0		(2 << 16)
#define DORCR_PG1D_DOOR		(3 << 16)
#define DORCR_PG1D_MASK		(3 << 16)
#define DORCR_RGPV		(1 << 4)
#define DORCR_DPRS		(1 << 0)

/* DPTSR */
#define DPTSR_PnDK(n)		(1 << ((n) + 16))
#define DPTSR_PnTS(n)		(1 << (n))

#endif
