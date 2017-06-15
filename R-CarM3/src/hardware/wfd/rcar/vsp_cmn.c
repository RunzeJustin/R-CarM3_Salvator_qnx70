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

#include <stdio.h>  
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/neutrino.h>
#include <sys/mman.h>
#include <hw/inout.h>
#include <errno.h>
#include <arm/r-car.h>
#include <unistd.h>
#include <atomic.h>
#include <sys/slog.h>

#include "vsp.h"

vsp_private_t VSP_LIST[] = {
	//VSPD0
	{
		.reg_base 	= VSPD0_REG_BASE,
		.reg_size 	= VSP_REG_SIZE,
		.irq		= VSPD0_IRQ,
		.cpg_stp_bit = 23,
	},
	//VSPD1
	{
		.reg_base 	= VSPD1_REG_BASE,
		.reg_size 	= VSP_REG_SIZE,
		.irq		= VSPD1_IRQ,
		.cpg_stp_bit = 22,
	},	
	//VSPD2
	{
		.reg_base 	= VSPD2_REG_BASE,
		.reg_size 	= VSP_REG_SIZE,
		.irq		= VSPD2_IRQ,
		.cpg_stp_bit = 21,
	},	
	//VSPI0
	{
		.reg_base 	= VSPI0_REG_BASE,
		.reg_size 	= VSP_REG_SIZE,
		.irq		= VSPI0_IRQ,
		.cpg_stp_bit = 31,
	},
	
};

const struct sigevent* (*vsp_intr[])(void* arg, int id) = {
	vspd0_intr,
	vspd1_intr,
	vspd2_intr, 
	vspi0_intr,
};

int get_bpp(int fmt){
	int bpp=0;
	switch( fmt ){
	case VSPCORE_RGB332: 	case VSPCORE_YUV444SP:	case VSPCORE_YUV422SP:
	case VSPCORE_YUV420SP:	case VSPCORE_YUV444P:	case VSPCORE_YUV422P:	
	case VSPCORE_YUV420P:	case VSPCORE_RGBCLUT:	case VSPCORE_YUVCLUT:
		bpp = 1;
		break;
		
	case VSPCORE_RGBX444:	case VSPCORE_RGB444X:	case VSPCORE_RGBx555:
	case VSPCORE_RGB555x:	case VSPCORE_RGB565:	case VSPCORE_RGBP444:
	case VSPCORE_RGB444P:	case VSPCORE_RGBP555:	case VSPCORE_RGB555P:
	case VSPCORE_BGRP444:	case VSPCORE_BGR444P:	case VSPCORE_BGRP555:
	case VSPCORE_BGR555P:	case VSPCORE_YUV422Itype0: case VSPCORE_YUV422Itype1:
		bpp = 2;
		break;

	case VSPCORE_RGBX666:	case VSPCORE_RGB666X:	case VSPCORE_RGBx666:
	case VSPCORE_RGB666x:	case VSPCORE_RGB888:	case VSPCORE_BGR888:
	case VSPCORE_BGRx666:	case VSPCORE_YUV444I:	case VSPCORE_YUV420I:
		bpp = 3;
		break;

	case VSPCORE_RGBPX666:	case VSPCORE_RGB666XP:	case VSPCORE_RGBX666P:
	case VSPCORE_RGBP666X:	case VSPCORE_RGBPx666:	case VSPCORE_RGBx666P:
	case VSPCORE_RGBP666:	case VSPCORE_RGB666P:	case VSPCORE_RGBP888:
	case VSPCORE_RGB888P:	case VSPCORE_RGBx666x:	case VSPCORE_RGBXX666:
	case VSPCORE_BGRX888:	case VSPCORE_RGBX565:
		bpp = 4;
		break;
	default:
		bpp = 0;
		break;
	}
	return bpp;
}

void get_stride(int fmt, int *mul, int *div, int *mulC, int *divC){
	/* Y */
	switch( fmt ){
	case VSPCORE_YUV444I: case VSPCORE_YUV420I:
		*mul = 3;
		*div = 1;
		break;
	case VSPCORE_YUV422Itype0: case VSPCORE_YUV422Itype1:
		*mul = 2;
		*div = 1;
		break;
	default:
		*mul = 1;
		*div = 1;
		break;
	}
	/* CbCr */
	switch( fmt ){
	case VSPCORE_YUV444SP:
		*mulC = 2;
		*divC = 1;
		break;
	case VSPCORE_YUV422P: case VSPCORE_YUV420P:
		*mulC = 1;
		*divC = 2;
		break;
	default:
		*mulC = 1;
		*divC = 1;
		break;
	}
	return;
}

void vsp_reset (int vsp_id)
{
	uintptr_t SRCR6_addr = (uintptr_t)mmap_device_io(4, RCAR_CPG_BASE + RCAR_CPG_SRCR6);
	uintptr_t SRSTCLR6_addr = (uintptr_t)mmap_device_io(4, RCAR_CPG_BASE + RCAR_CPG_SRSTCLR6);	
							  
	out32(SRCR6_addr, in32(SRCR6_addr) | (1 << VSP_LIST[vsp_id].cpg_stp_bit));
	delay(1);
	out32(SRSTCLR6_addr, in32(SRSTCLR6_addr) | (1 << VSP_LIST[vsp_id].cpg_stp_bit));
	munmap_device_io((uintptr_t)SRCR6_addr, 4);	
	munmap_device_io((uintptr_t)SRSTCLR6_addr, 4);		
}

int32_t vsp_ins_stop_processing(vsp_dev_t	*dev)
{
	unsigned int status;
	unsigned int loop_cnt = VSP_STATUS_LOOP_CNT;

	/* disable interrupt */
	dev->reg.vi6_ctrl->wpf_irq[0].irq_enb = 0;

	/* clear interrupt */
    dev->reg.vi6_ctrl->wpf_irq[0].irq_sta = 0;

	/* read status register */
	status = dev->reg.vi6_ctrl->status;

	if (status & VSP_STATUS_WPF0) {
		/* software reset */
        dev->reg.vi6_ctrl->sreset = VSP_SRESET_WPF0;
		/* waiting reset process */
		do {
			/* sleep */
			delay(20);		/* 20ms */

			/* read status register */
			status = dev->reg.vi6_ctrl->status;
		} while ((status & VSP_STATUS_WPF0) &&
			(--loop_cnt > 0));
	}

	return 0;
}

int is_vspd (vsp_dev_t *dev)
{
	return (dev->vsp_idx <= 2); // See VSP_LIST
}
int is_vspi (vsp_dev_t *dev)
{
	return ((dev->vsp_idx >=3) && (dev->vsp_idx <= 3)); // See VSP_LIST
}

void *VspLib_init(void *info, port_t *port)
{
	vsp_dev_t	*dev;
	vsp_info_t *vsp_info = (vsp_info_t *)info;
	int vsp_channel;
	int ret;
    unsigned int status;
    unsigned int loop_cnt = 50;
	pthread_mutexattr_t attr;
    
	vsp_channel = vsp_info->Id;
    uintptr_t FCPV_CFG0_add;
	
 	if (-1 == ThreadCtl(_NTO_TCTL_IO, 0)) {
	    perror("ThreadCtl");
	    return NULL;
	} 
		
	dev = malloc(sizeof(vsp_dev_t));
	if (!dev) {
	    return NULL;
	}

	memset ((void*)dev, 0, sizeof(vsp_dev_t));
	
	/* Get VSP information from channel number */
	dev->irq = VSP_LIST[vsp_channel].irq;
	dev->reg_base = VSP_LIST[vsp_channel].reg_base;
	dev->reg_size = VSP_LIST[vsp_channel].reg_size;
	dev->vsp_idx = vsp_channel;

	/* Get information about Master layer */
	dev->info.mWidth = vsp_info->mWidth;
	dev->info.mHeight = vsp_info->mHeight;
	dev->info.mFormat = vsp_info->mFormat;

	/* Reset VSP module */	
	vsp_reset(vsp_channel);

    /* Attach interrupt */
    dev->iid = InterruptAttach(dev->irq, vsp_intr[vsp_channel], dev,sizeof(*dev),0);
	if (dev->iid == -1)
    {
		perror("InterruptAttach");
		goto fail;
	}
	
	/* Get virtual address of VSP register */
	dev->reg_base_ptr = (uintptr_t)mmap_device_io(dev->reg_size, dev->reg_base);
		
	if (dev->reg_base_ptr == (uintptr_t)MAP_FAILED) {
	    perror("mmap_device_io with regbase");
	    goto fail;
	} 

	/* Get virtual address of modules in VSP */
	dev->reg.vi6_ctrl 	= (vi6_ctrl_t *)(dev->reg_base_ptr + VI6_CTRL_REG_OFF);
	dev->reg.rpf 		= (rpf_t *)(dev->reg_base_ptr + VI6_RPF_OFF);
	dev->reg.wpf 		= (wpf_t *)(dev->reg_base_ptr + VI6_WPF_OFF);
	dev->reg.dpr 		= (dpr_t *)(dev->reg_base_ptr + VI6_DPR_OFF);
	dev->reg.uds 		= (uds_t *)(dev->reg_base_ptr + VI6_UDS_OFF);
	dev->reg.bru 		= (bru_t *)(dev->reg_base_ptr + VI6_BRU_OFF);
	dev->reg.lif 		= (lif_t *)(dev->reg_base_ptr + VI6_LIF_OFF);
	dev->reg.dl  		= (dl_t *)(dev->reg_base_ptr + VI6_DL_CTRL);

	/* Enable clock */
	dev->reg.vi6_ctrl->clk_dcswt = 0x808;

    /* Init Display List */
    dev->pvdata = malloc(sizeof(struct vsp_private_data));
    if (!dev->pvdata) {
        goto fail;
    }
    int dl_mode = DL_MODE_AUTO_REPEAT;
    //	int dl_mode = DL_MODE_HEADER_LESS_AUTO_REPEAT;
    ret = vsp_dl_create(dev->pvdata, port, dl_mode);

    if (ret) {
        goto fail;
    }

    memset( &dev->pvdata->lock, 0, sizeof( intrspin_t ) );
    dev->pvdata->active = 0;
    vsp_dl_reset(dev->pvdata);

	/* If VSPD */
	if (is_vspd(dev))
	{
		/* Connect VSPD to DU */
		init_lif (dev);
		
		/* Init composition */
		bru_init (dev);
	}
    else if (is_vspi(dev))
    {
        /* Initialize event for VSPI */
        pthread_mutexattr_init (&attr);
        pthread_mutex_init(&dev->vsp_mutex,&attr);
        dev->irqchan = ChannelCreate(0);
        dev->irqcoid = ConnectAttach(0, 0, dev->irqchan, _NTO_SIDE_CHANNEL, 0);

        if (dev->iid != -1)
            SIGEV_PULSE_INIT(&dev->intrevent, dev->irqcoid, SIGEV_PULSE_PRIO_INHERIT, VSP_INT_EVENT, 0);
        else
        {
            ConnectDetach(dev->irqcoid);
            ChannelDestroy(dev->irqchan);
            goto fail;
        }
        
        /* initialize DL control register */
        dev->reg.dl->ctrl		= VI6_DL_CTRL_AR_WAIT | VI6_DL_CTRL_DL_ENABLE;;
        /* initialize DL swap register */
        dev->reg.dl->swap		= VI6_DL_SWAP_LWS;
        
        /*VSP reset*/
        vsp_ins_stop_processing(dev);
        
        /* Get virtual address of FCPV register */
        FCPV_CFG0_add = (uintptr_t)mmap_device_io(32, dev->reg_base + FCPV_OFFSET);
            
        if (FCPV_CFG0_add == (uintptr_t)MAP_FAILED) {
            perror("mmap_device_io with regbase");
            goto fail;
        }
        
        /* reset the FCP */
        status = in32(FCPV_CFG0_add + FCPV_STA);
        if (status & VSP_FCPV_STA_ACT) {
            /* reset */
            out32(FCPV_CFG0_add + FCPV_RST, VSP_FCPV_RST_SOFTRST);
            /* waiting reset process */
            do {
                /* sleep */
                delay(20);		/* 20ms */

                /* read status register */
                status = in32(FCPV_CFG0_add + FCPV_STA);
            } while ((status & VSP_FCPV_STA_ACT) &&
                (--loop_cnt > 0));
        }

        if (loop_cnt == 0)
            SLOG_WARNING("%s: happen to timeout after reset!!\n", __func__);
        
        out32(FCPV_CFG0_add + FCPV_CFG0, FCPV_CFG0_FCPVSEL);
        munmap_device_io(FCPV_CFG0_add, 32);
    }

	routing_init (dev);
	
	/* Set state to initialized */
	dev->state = VSP_INITIALIZED;
	return dev;
fail:
	dev->state = VSP_STOPPING;
	InterruptDetach(dev->iid);
	free(dev->pvdata->dlmemory);
	free(dev->pvdata);
	free(dev);
	return (0);
}

void set_rpf_var (vsp_dev_t *dev, vsp_pipe_t *pipe)
{
	int multiply = 0;
	int division = 0;
	int multiply_c = 0;
	int division_c = 0;
	int rpf_id = pipe->vsp_pipe_id;
	rpf_par_t *rpf_par;
	
	PIPE_VALIDATE(rpf_id,return);
	
	rpf_par = &dev->param.rpf_par[rpf_id];
	rpf_par->active=1;
	rpf_par->rpf_id = rpf_id;
	
	/* Set parameter for RPF */
 	rpf_par->src_bsize 		= ((pipe->src_rect[2] & 0x1FFF) << 16) | (pipe->src_rect[3] & 0x1FFF);
	rpf_par->src_esize		= ((pipe->src_rect[2] & 0x1FFF) << 16) | (pipe->src_rect[3] & 0x1FFF);
	
	rpf_par->infmt			= pipe->src.fmt & 0x7F;
	rpf_par->alph_sel		= (0 << 28); /* use 0,1,4,8 packed alpha */
	
	if (pipe->src.fmt & OPACITY_FULL)	/* eg RGBX8888 */
	{
		rpf_par->alph_sel = (4 << 28);			/* Use fixed alpha value */
	}
	
	if (rpf_par->infmt <= 0x3f) 			/* RGB space */
	{
		rpf_par->infmt 		&= ~(1<<8);		/* Disable Color space conversion */
		switch (rpf_par->infmt) {
			case VSPCORE_BGR888:
				rpf_par->dswap		= 0xF;
				break;
			default:
				rpf_par->dswap		= 0xC;
				break;
		}
	}
	else									/* YUV space */
	{
		if (pipe->src.fmt != pipe->dst.fmt)
			rpf_par->infmt 		|= (1<<8);		/* Enable Color space conversion */
		rpf_par->dswap		= 0xF;
		rpf_par->alph_sel	= (4 << 28);	/* Fixed alpha */
		
		/* Check YC order of VSPCORE_YUV422Itype0 format */
		if ((rpf_par->infmt & 0x7f) == VSPCORE_YUV422Itype0)
		{
			if (pipe->src.fmt & ORDER_YUY2)
			{
				rpf_par->infmt |= (1<<15);	/*SPYCS=1*/
			}
			else if (pipe->src.fmt & ORDER_YVYU)
			{
				rpf_par->infmt |= (1<<15) | (1<<14); /*SPUVS,SPYCS=1*/
			}
		}
	}
	
	rpf_par->vrtcol_set		= 0xFF000000;	/* Fixed alpha value */
	rpf_par->loc			= (pipe->dst.hcoord << 16) | pipe->dst.vcoord;
	
	/* Get source memory stride */
	int bpp = get_bpp(rpf_par->infmt & 0x7f);
	get_stride(rpf_par->infmt, &multiply, &division, &multiply_c, &division_c);
	int stride_y = (pipe->src.width * bpp * multiply) / division;
	int stride_c = (pipe->src.width * bpp * multiply_c) / division_c;
	rpf_par->srcm_pstride	= ((stride_y & 0xFFFF) << 16) | (stride_c & 0xFFFF);

	rpf_par->srcm_addr_y	= pipe->src.addr.y_rgb + pipe->src_rect[1] * stride_y +
								pipe->src_rect[0] * bpp;
	rpf_par->srcm_addr_c0 	= pipe->src.addr.c0 + (pipe->src_rect[1] * stride_c)/2 +
								pipe->src_rect[0];
	rpf_par->srcm_addr_c1	= pipe->src.addr.c1 + (pipe->src_rect[1] * stride_c)/2 +
								pipe->src_rect[0] * bpp;			
}

void set_wpf_var (vsp_dev_t *dev, vsp_pipe_t *pipe)
{
	int rpf_id = 0;
	rpf_par_t rpf_par;
	wpf_par_t *wpf_par;
    wpf_par = &dev->param.wpf_par[0];

	for (rpf_id=0;rpf_id < VSPD_INPUT_IMAGE_NUM;rpf_id++)
	{
		rpf_par = dev->param.rpf_par[rpf_id];
		if (rpf_par.active) /* Check if any pipeline is active */
		{
			wpf_par->src_rpf |= SRC_RPF_MASTER_LAYER_SET << 28 | SRC_RPF_SUB_LAYER_SET << rpf_id*2;
		}
		else
		{
			/* Deactivate RPF source */
			wpf_par->src_rpf &= ~(SRC_RPF_MASK << rpf_id*2);
		}
	}
    wpf_par->hszclip = 0;
    wpf_par->vszclip = 0;
    wpf_par->outfmt = 0x0013;
    wpf_par->dswap = 0;
    wpf_par->rndctrl = 0;
    wpf_par->dstm_stride_y = 0;
    wpf_par->dstm_stride_c = 0;
    wpf_par->dstm_addr_y = 0;
    wpf_par->dstm_addr_c0 = 0;
    wpf_par->dstm_addr_c1 = 0;
}

void routing_init (vsp_dev_t *dev)
{
	/* By default, RPFs are routed to BRUis */
	/* RPF route */
	dev->reg.dpr->rpf_route[0] = DPR_SET_NOT_USED;
	dev->reg.dpr->rpf_route[1] = DPR_SET_NOT_USED;
	dev->reg.dpr->rpf_route[2] = DPR_SET_NOT_USED;
	dev->reg.dpr->rpf_route[3] = DPR_SET_NOT_USED;
	dev->reg.dpr->rpf_route[4] = DPR_SET_NOT_USED;

	dev->reg.dpr->uds_route[0] = DPR_SET_NOT_USED;
	dev->reg.dpr->uds_route[1] = DPR_SET_NOT_USED;
	dev->reg.dpr->uds_route[2] = DPR_SET_NOT_USED;
	
	/* BRU route to WFD0 */
	dev->reg.dpr->bru_route = DPR_SET_WPF0;

	/* others */
	dev->reg.dpr->wpf_fporch[0] 	= DPR_SET_WPF_FPORCH;
	dev->reg.dpr->wpf_fporch[1] 	= DPR_SET_WPF_FPORCH;
	dev->reg.dpr->wpf_fporch[2] 	= DPR_SET_WPF_FPORCH;
	dev->reg.dpr->wpf_fporch[3] 	= DPR_SET_WPF_FPORCH;
	dev->reg.dpr->sru_route 		= DPR_SET_NOT_USED;
	dev->reg.dpr->lut_route 		= DPR_SET_NOT_USED;
	dev->reg.dpr->clu_route 		= DPR_SET_NOT_USED;
	dev->reg.dpr->hst_route 		= DPR_SET_NOT_USED;
	dev->reg.dpr->hsi_route 		= DPR_SET_NOT_USED;
	dev->reg.dpr->hgo_smppt 		= 0x73F;	
	dev->reg.dpr->hgt_smppt 		= 0x73F;	
	
	if (is_vspd(dev))
	{
		/* RPF route to BRU */
		dev->reg.dpr->rpf_route[0] = DPR_SET_BRUin0;
		dev->reg.dpr->rpf_route[1] = DPR_SET_BRUin1;
		dev->reg.dpr->rpf_route[2] = DPR_SET_BRUin2;
		dev->reg.dpr->rpf_route[3] = DPR_SET_BRUin3;
		dev->reg.dpr->rpf_route[4] = DPR_SET_BRUin4;

		/* BRU route to WFD0 */
		dev->reg.dpr->bru_route = DPR_SET_WPF0;
	}
	if (is_vspi(dev))
	{
		/* RPF route to UDS */
		dev->reg.dpr->rpf_route[0] = DPR_SET_UDS0;
		/* UDS route to WFD0 */
		dev->reg.dpr->uds_route[0] = DPR_SET_WPF0;
        
        dev->reg.dpr->bru_route = DPR_SET_NOT_USED;
	}
}

void bru_init (vsp_dev_t *dev)
{
	/* BRU control */
	dev->reg.bru->inctrl = 0;
	
	/* 	Set virtual RPF for composition 
		It is used as Master layer, and need to be always active */
	dev->reg.bru->virrpf_size = (dev->info.mWidth << 16) | (dev->info.mHeight);
	dev->reg.bru->virrpf_loc = 0;
	dev->reg.bru->virrpf_col = 0;

	/* BRU channel A */
	dev->reg.bru->bru_chan[0].ctrl = (1 << 31) | //Blend operation
									(4 << 20) |	//VIR_RPF is input to DST
									(0 << 16) | //BRUin0 is input to SRC
									(0 << 4) |	//No Color Data ROP operator
									(0 << 0); 	//No Alpha Data ROP operator
	dev->reg.bru->bru_chan[0].bld = (0 << 31) | (3 << 28) | (2 << 24);			
	
	/* BRU channel B */
	dev->reg.bru->bru_chan[1].ctrl = (1 << 31) | //Blend operation
									(0 << 20) |	//selecting DST doesn't affect for channel B
									(0 << 16) | 
									(0 << 4) |	//No Color Data ROP operator
									(0 << 0); 	//No Alpha Data ROP operator	
	dev->reg.bru->bru_chan[1].bld = (0 << 31) | (3 << 28) | (2 << 24);
	
	dev->reg.bru->rop 			= 	(0 << 31) | //Raster operation
									(1 << 20) |	//BRUin1 is input to ROP
									(0 << 16) | 
									(0 << 4) |	//No Color Data ROP operator
									(0 << 0); 	//No Alpha Data ROP operator		

	/* BRU channel C */
	dev->reg.bru->bru_chan[2].ctrl = (1 << 31) | //Blend operation
									(0 << 20) |	//selecting DST doesn't affect for channel C
									(2 << 16) | //BRUin2 is input to SRC
									(0 << 4) |	//No Color Data ROP operator
									(0 << 0); 	//No Alpha Data ROP operator
	dev->reg.bru->bru_chan[2].bld = (0 << 31) | (3 << 28) | (2 << 24);

	/* BRU channel D */
	dev->reg.bru->bru_chan[3].ctrl = (1 << 31) | //Blend operation
									(0 << 20) |	//selecting DST doesn't affect for channel D
									(3 << 16) | //BRUin3 is input to SRC
									(0 << 4) |	//No Color Data ROP operator
									(0 << 0); 	//No Alpha Data ROP operator								
	dev->reg.bru->bru_chan[3].bld = (0 << 31) | (3 << 28) | (2 << 24);
	
	/* BRU channel E */
	dev->reg.bru->bru_chan_5.ctrl = (1 << 31) | //Blend operation
									(0 << 20) |	//selecting DST doesn't affect for channel D
									(5 << 16) | //BRUin4 is input to SRC
									(0 << 4) |	//No Color Data ROP operator
									(0 << 0); 	//No Alpha Data ROP operator
									
	dev->reg.bru->bru_chan_5.bld = (0 << 31) | (3 << 28) | (2 << 24);
}

void init_lif (vsp_dev_t *dev)
{
	dev->reg.lif->ctrl = 0x1900003;
	dev->reg.lif->csbth = 0x51400c8;
}

void start_wpf (vsp_dev_t *dev, int wfd_id)
{
	/* Issue WPF command */
	dev->reg.vi6_ctrl->cmd[wfd_id] = CMD_START;
}

int vsp_wait_event(vsp_dev_t *dev)
{
   struct _pulse       pulse;
    iov_t               iov;
    uint64_t            MaxWaitTime =50*1000*1000;

    pthread_mutex_lock(&dev->vsp_mutex);

	SETIOV(&iov, &pulse, sizeof (pulse));

	while (1)
	{
		TimerTimeout(CLOCK_REALTIME, _NTO_TIMEOUT_RECEIVE, NULL, &MaxWaitTime, NULL);

		if (MsgReceivev(dev->irqchan, &iov, 1, NULL) == -1)
			break;

		if (pulse.code == VSP_INT_EVENT)
			break;
	}
	pthread_mutex_unlock(&dev->vsp_mutex);
		
    return 1;
}

void VspLib_fini (void *arg)
{
	vsp_dev_t *dev = (vsp_dev_t *)arg;
	dev->state = VSP_STOPPING;
	InterruptDetach(dev->iid);
	vsp_dl_destroy (dev->pvdata);
    if(is_vspi(dev))
        pthread_mutex_destroy(&dev->vsp_mutex);
	munmap_device_io (dev->reg_base_ptr, dev->reg_size);
  free(dev->pvdata);
  free (dev);
}
