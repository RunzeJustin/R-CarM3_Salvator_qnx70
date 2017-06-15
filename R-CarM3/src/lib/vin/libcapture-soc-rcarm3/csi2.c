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

#include "csi2.h"

static void rcar_csi2_enable_clock(rcar_csi2_t *csi)
{
	uintptr_t CPG_CSI0CKCR_reg;
	uintptr_t CPG_CSIREFCKCR_reg;
	uintptr_t MSTPSR7_reg;
	uintptr_t SMSTPCR7_reg;

	uint32_t tmp;

	/* Map register */
	CPG_CSI0CKCR_reg 	= mmap_device_io(4, 0xE615000C);
	CPG_CSIREFCKCR_reg 	= mmap_device_io(4, 0xE6150034);
	MSTPSR7_reg 		= mmap_device_io(4, 0xE61501C4);
	SMSTPCR7_reg 		= mmap_device_io(4, 0xE615014C);

	/* Enable supply clock for CSI40 and CSI20 */
	tmp = in32(SMSTPCR7_reg);
	tmp &= ~((1 << 16)|(1 << 14));
	out32(SMSTPCR7_reg, tmp);

	/* Stop CSI0-Clock, set Division Ratio */
	tmp = 0x0000011F;
	out32(CPG_CSI0CKCR_reg, tmp);

	/* Supply CSI0-Clock */
	tmp &= ~(1 << 8);
	out32(CPG_CSI0CKCR_reg, tmp);

	/* Stop CSIREF-Clock, set Division Ratio */
	tmp = 0x00000107;
	out32(CPG_CSIREFCKCR_reg, tmp);

	/* Supply CSIREF-Clock */
	tmp &= ~(1 << 8);
	out32(CPG_CSIREFCKCR_reg, tmp);
	
	/* Unmap register */
	munmap_device_io(CPG_CSI0CKCR_reg, 4);
	munmap_device_io(CPG_CSIREFCKCR_reg, 4);
	munmap_device_io(MSTPSR7_reg, 4);
	munmap_device_io(SMSTPCR7_reg, 4);
	
	delay(10);
}

static int rcar_csi2_set_phy_freq(rcar_csi2_t* csi)
{
	csi2_info_t* info = csi->info;
	
	const uint32_t const hs_freq_range[43] = {
		0x00, 0x10, 0x20, 0x30, 0x01,  /* 0-4   */
		0x11, 0x21, 0x31, 0x02, 0x12,  /* 5-9   */
		0x22, 0x32, 0x03, 0x13, 0x23,  /* 10-14 */
		0x33, 0x04, 0x14, 0x05, 0x15,  /* 15-19 */
		0x25, 0x06, 0x16, 0x07, 0x17,  /* 20-24 */
		0x08, 0x18, 0x09, 0x19, 0x29,  /* 25-29 */
		0x39, 0x0A, 0x1A, 0x2A, 0x3A,  /* 30-34 */
		0x0B, 0x1B, 0x2B, 0x3B, 0x0C,  /* 35-39 */
		0x1C, 0x2C, 0x3C               /* 40-42 */
	};
	
	uint32_t bps_per_lane = RCAR_CSI_190MBPS;

	switch (csi->lanes) {
	case 1:
		bps_per_lane = RCAR_CSI_400MBPS;
		break;
	case 4:
		if (!info->interlace) {
			if ((info->width == 1920) && (info->height == 1080))
				bps_per_lane = RCAR_CSI_900MBPS;
			else if ((info->width == 1280) && (info->height == 720))
				bps_per_lane = RCAR_CSI_450MBPS;
			else if ((info->width == 720) && (info->height == 480))
				bps_per_lane = RCAR_CSI_190MBPS;
			else if ((info->width == 720) && (info->height == 576))
				bps_per_lane = RCAR_CSI_190MBPS;
			else if ((info->width == 640) && (info->height == 480))
				bps_per_lane = RCAR_CSI_100MBPS;
			else
				goto failed;
		} else {
			if ((info->width == 1920) && (info->height == 1080))
				bps_per_lane = RCAR_CSI_450MBPS;
			else
				goto failed;
		}
		break;
	default:
		fprintf(stderr, "%s: lanes is invalid (%d)\n", __FUNCTION__, csi->lanes);
		return -EINVAL;
	}
	
	out32(csi->vbase + RCAR_CSI2_PHYPLL, (hs_freq_range[bps_per_lane] << 16));
	return 0;
	
failed:
	fprintf(stderr, "%s: not support resolution (%dx%d%c)\n",
		 __FUNCTION__, info->width, info->height, (info->interlace) ? 'i' : 'p');
	return -EINVAL;
}

int rcar_csi2_init(int channel, rcar_csi2_t* csi)
{
	int ret = 0;
	
	uint32_t tmp = 0x10;
	uint32_t vcdt = 0;
	uint32_t vcdt2 = 0;
	
	if(channel) {
		csi->pbase = RCAR_CSI20_BASE;
		csi->lanes = 1;
	}
	else {
		csi->pbase = RCAR_CSI40_BASE;
		csi->lanes = 4;
	}
	
	/* Map base address */
	if ((csi->vbase = (uintptr_t)mmap_device_io(RCAR_CSI2_SIZE, csi->pbase)) == (uintptr_t)MAP_FAILED) {
        fprintf(stderr, "%s: CSI2 base mmap_device_io (0x%x) failed", __FUNCTION__, (uint32_t)csi->pbase);
        rcar_csi2_fini(channel, csi);
        return -1;
    }
	
	/* Supply clock for module */
	rcar_csi2_enable_clock(csi);

	/* Reflect registers immediately */
	out32(csi->vbase + RCAR_CSI2_TREF, 0x01);
	
	/* Reset CSI2 hardware */
	out32(csi->vbase + RCAR_CSI2_SRST, 0x01);
	delay(1);
	out32(csi->vbase + RCAR_CSI2_SRST, 0);
	out32(csi->vbase + RCAR_CSI2_PHTC, 0);
	
	/* Setting HS reception frequency */
	{
		switch (csi->lanes) 
		{
			case 1:
				/* First field number setting */
				out32(csi->vbase + RCAR_CSI2_FLD, (1 << 0)|(1 << 16));
				tmp |= 0x1;
				vcdt |= (0x1e | RCAR_CSI2_VCDT_VCDTN_EN); /* YUV422 8 bit */
				break;
			case 4:
				/* First field number setting */
				out32(csi->vbase + RCAR_CSI2_FLD, (1 << 0)|(1 << 17));
				tmp |= 0xF;
				vcdt |= (0x24 | RCAR_CSI2_VCDT_VCDTN_EN); /* RGB888 */
				break;
			default:
				fprintf(stderr, "%s: lanes is invalid (%d)\n", __FUNCTION__, csi->lanes);
				return -EINVAL;
		}
		
		/* set PHY frequency */
		ret = rcar_csi2_set_phy_freq(csi);
		if (ret < 0)
			return ret;
		
		/* Enable lanes */
		out32(csi->vbase + RCAR_CSI2_PHYCNT, tmp);
		out32(csi->vbase + RCAR_CSI2_PHYCNT, tmp | RCAR_CSI2_PHYCNT_SHUTDOWNZ);
		out32(csi->vbase + RCAR_CSI2_PHYCNT, tmp | RCAR_CSI2_PHYCNT_SHUTDOWNZ |
												   RCAR_CSI2_PHYCNT_RSTZ);
	}
	
	out32(csi->vbase + RCAR_CSI2_CHKSUM, 3 << 0);
	out32(csi->vbase + RCAR_CSI2_VCDT, vcdt);
	out32(csi->vbase + RCAR_CSI2_VCDT2, vcdt2);
	out32(csi->vbase + RCAR_CSI2_FRDT, 1 << 16);
	delay(1);
	out32(csi->vbase + RCAR_CSI2_LINKCNT, 0x82000000);
	out32(csi->vbase + RCAR_CSI2_LSWAP, 0x000000e4);
	
	/* Wait until video decoder power off */
	delay(10);
	{
		int timeout = 100;
		/* Read the PHY clock lane monitor register (PHCLM). */
		while (!(in32(csi->vbase + RCAR_CSI2_PHCLM) & 0x01) ) {
			usleep(10);
			if(!timeout--)
				break;
		}
		
		if (timeout == 0)
			fprintf(stderr, "%s: Timeout of reading the PHY clock lane\n", __FUNCTION__);
		
		timeout = 100;
		/* Read the PHY data lane monitor register (PHDLM). */
		while (!(in32(csi->vbase + RCAR_CSI2_PHDLM) & 0x01) ) {
			usleep(10);
			if(!timeout--)
				break;
		}
		
		if (timeout == 0)
			fprintf(stderr, "%s: Timeout of reading the PHY data lane\n", __FUNCTION__);
	}
	
	return 0;
}

int rcar_csi2_fini(int channel, rcar_csi2_t* csi)
{
	out32(csi->vbase + RCAR_CSI2_PHYCNT, 0);
	
	/* Reset CSI2 hardware */
	out32(csi->vbase + RCAR_CSI2_SRST, 0x01);
	delay(1);
	out32(csi->vbase + RCAR_CSI2_SRST, 0x00);
	
	/* Disable clock */
	//rcar_csi2_disable_clock(csi);
	
    if (csi->vbase) {
        munmap_device_io(csi->vbase, RCAR_CSI2_SIZE);
    }

    return 0;
}

