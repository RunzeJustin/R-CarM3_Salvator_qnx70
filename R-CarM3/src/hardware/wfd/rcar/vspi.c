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
#include <sys/slog.h>

#include "vsp.h"


inline struct sigevent* vspi_isr_call (void *arg, int id) 
{
	vsp_dev_t *dev = (vsp_dev_t *) arg;
	uint32_t intr_status;

	intr_status = dev->reg.vi6_ctrl->wpf_irq[0].irq_sta;
	if (intr_status & 1)
	{
        /* disable interrupt */
        dev->reg.vi6_ctrl->wpf_irq[0].irq_enb = 0;

        /* clear interrupt */
		dev->reg.vi6_ctrl->wpf_irq[0].irq_sta = ~intr_status;
        return &dev->intrevent;
	}
    return NULL;
}

const struct sigevent* vspi0_intr (void* arg, int id)
{
	return vspi_isr_call (arg,id);
}


unsigned int uds_compute_ratio(unsigned int input, unsigned int output)
{
	/* TODO: This is an approximation that will need to be refined. */
	return (input) * 4096 / (output );
}

int calc_bwidth(unsigned short ratio){
	if (ratio >= 4096) {
		/* Down-scaling */
		unsigned int mp;

		mp = ratio / 4096;
		mp = mp < 4 ? 1 : (mp < 8 ? 2 : 4);

		return 64 * 4096 * mp / ratio;
	} else {
		/* Up-scaling */
		return 64;
	}
}

void scale_frame_prepare (vsp_dev_t *dev, vsp_pipe_t *pipe)
{
	int bpp;
	int multiply = 0;
	int division = 0;
	int multiply_c = 0;
	int division_c = 0;
	int x_ratio;
	int y_ratio;
	
	uds_par_t *uds_par;
	wpf_par_t *wpf_par;
	
	int id = pipe->vsp_pipe_id;
		
	/* Enable WPF[id] interrupt */
	dev->reg.vi6_ctrl->wpf_irq[id].irq_enb = 1;
	
	/* Set parameters for RPF */
	set_rpf_var (dev,pipe);
	
	/* Set parameters for UDS */
	uds_par = &dev->param.uds_par[id];
	x_ratio				= uds_compute_ratio(pipe->src_rect[2], pipe->dst.width);
	y_ratio				= uds_compute_ratio(pipe->src_rect[3], pipe->dst.height);
	uds_par->uds_id 	= id;
	uds_par->ctrl		= (1<<30) | (1<<25) | VSP_UDSn_CTRL_AMDSLH; 
	uds_par->scale		= (x_ratio << 16) | y_ratio;
	uds_par->pass_bwidt	= calc_bwidth(x_ratio) << 16;
	uds_par->pass_bwidt	|= calc_bwidth(y_ratio);
	uds_par->clip_size	= (pipe->dst.width << 16) | pipe->dst.height;
	uds_par->alpth		= 0x0;
	uds_par->alpval		= 0x0;
	uds_par->fill_color	= 0x0;
		
	/* Set parameters for WFD */
	wpf_par = &dev->param.wpf_par[id];
	wpf_par->wpf_id = id;
	wpf_par->src_rpf = SRC_RPF_MASTER_LAYER_SET << id*2;
	wpf_par->dstm_addr_y = pipe->dst.addr.y_rgb;
	wpf_par->dstm_addr_c0 = pipe->dst.addr.c0;
	wpf_par->dstm_addr_c1 = pipe->dst.addr.c1;
	wpf_par->outfmt = pipe->dst.fmt;
	switch (wpf_par->outfmt)
	{
		case VSPCORE_RGBP888:
            wpf_par->dswap = 0xc;
			break;
		case VSPCORE_RGB565:
            wpf_par->dswap = 0xe;
			break;
		default:
            wpf_par->dswap = 0xf;
			break;
	}
	bpp = get_bpp(wpf_par->outfmt);
	get_stride(wpf_par->outfmt, &multiply, &division, &multiply_c, &division_c);
	wpf_par->dstm_stride_y = (pipe->dst.width * bpp * multiply) / division;
	wpf_par->dstm_stride_c = (pipe->dst.width * bpp * multiply_c) / division_c;	
    /* set clipping parameter */
	wpf_par->hszclip = VSP_WPF_HSZCLIP_HCEN;
    if( pipe->dst.width < INPUT_WPF_HSIZE_MAX)
        wpf_par->hszclip |= pipe->dst.width;
    else
        wpf_par->hszclip |= INPUT_WPF_HSIZE_MAX;
            

	wpf_par->vszclip = VSP_WPF_HSZCLIP_HCEN;
	wpf_par->vszclip |= pipe->dst.height;
    
}

void scale_frame_update(vsp_dev_t *dev, vsp_pipe_t *pipe, uint32_t dst_offset, uint32_t dst_width)
{
	int id = pipe->vsp_pipe_id;
	uint32_t width;

	uint32_t l_pos;
	uint32_t r_pos;	
    uint32_t margin = 0; 
    static uint32_t temp = 0;
    uint32_t offset = 0;
    uint32_t clip_offset = 0;
    int bpp = 0;
    
    
    uint32_t ratio	= uds_compute_ratio(pipe->src_rect[2], pipe->dst.width);
	/* check scaling factor parameter */
    if (ratio < VSP_UDS_SCALE_8_1) {
		/* set partition margin */
		margin += 4;
	} else if (ratio < VSP_UDS_SCALE_4_1) {
		/* set partition margin */
		margin += 2;
	} else if (ratio < VSP_UDS_SCALE_2_1) {
		/* set partition margin */
		margin += 1;
	}
    
    width = pipe->dst.width;

	/* calculate partition position with margin */
	l_pos = (unsigned int)VSP_CLIP0(dst_offset, margin);
    
    if (width > dst_offset + dst_width + margin) {
        r_pos = (dst_offset + dst_width + margin);
	} else {
		r_pos = width;
		dst_width = width - dst_offset;
	}
    
    /* Update WPF parameters */
    /* replace destinationaddress */
    if (dst_offset != 0) {
        uint32_t val_hszclip = VSP_WPF_HSZCLIP_HCEN;
        val_hszclip |= dst_width;
        val_hszclip |= margin << 16;
        dev->param.wpf_par[id].hszclip 	    = val_hszclip;
        bpp = get_bpp(dev->param.wpf_par[id].outfmt);
        dev->param.wpf_par[id].dstm_addr_y 	+= INPUT_WPF_HSIZE_MAX*bpp;
        if (dev->param.wpf_par[id].dstm_addr_c0 > 0)
            dev->param.wpf_par[id].dstm_addr_c0 += INPUT_WPF_HSIZE_MAX*bpp;
        if (dev->param.wpf_par[id].dstm_addr_c1 > 0)
            dev->param.wpf_par[id].dstm_addr_c1 += dst_width*bpp;
	}
    
	uint32_t l_temp = l_pos;
	uint32_t r_temp = r_pos;

    /* Update UDS parameters */
    /* replace clipping size */
    dev->param.uds_par[id].clip_size    &= 0x0000FFFF;
    dev->param.uds_par[id].clip_size    |= ((r_temp - l_temp) << 16);  
    /* add horizontal filter phase of control register */    
    dev->param.uds_par[id].ctrl         |= VSP_UDSn_CTRL_AMDSLH;
    
    /* calculate scaling */
    l_temp *= ratio;
    r_temp *= ratio;

    /* replace scaling filter horizontal phase */
    if (l_temp & 0xfff)
    {
        dev->param.uds_par[id].hphase = (4096 - (l_temp & 0xfff)) << 16;
    }
    else
    {
        dev->param.uds_par[id].hphase = 0;
    }
    dev->param.uds_par[id].hphase |= (r_temp % 4096);
        
  
    /* Update RPF parameters*/
	l_pos = l_temp;
	r_pos = r_temp;

    
  	offset = VSP_ROUND_UP(l_pos, 4096);
	width = VSP_ROUND_DOWN(r_pos, 4096) - offset;

    /* replace horizontal input clipping register */
    clip_offset = offset%2;
    dev->param.uds_par[id].hszclip = clip_offset << 16;
    dev->param.uds_par[id].hszclip |= width;
    dev->param.uds_par[id].hszclip |= VSP_UDS_HSZCLIP_HCEN;
 
 	/* calculate source horizontal size */
	offset = VSP_ROUND_DOWN(offset - clip_offset, 1);
	width = VSP_ROUND_UP(width + clip_offset, 1);
 
	if ((width % 2) == 1)
		width++;

    
    if(temp > offset)
        temp = 0;
        
    offset -= temp;
    bpp = get_bpp(dev->param.rpf_par[id].infmt & 0x7f);

	dev->param.rpf_par[id].srcm_addr_y += offset*bpp;
	/* replace chroma address */
	if (dev->param.rpf_par[id].srcm_addr_c0)
		dev->param.rpf_par[id].srcm_addr_c0 += offset;
	if (dev->param.rpf_par[id].srcm_addr_c1)
		dev->param.rpf_par[id].srcm_addr_c1 += offset;

	/* replace alpha plane */
	if (dev->param.rpf_par[id].srcm_addr_ai) {
		dev->param.rpf_par[id].srcm_addr_ai += offset;
	}
    temp += offset;
	/* replace basic and extended read size */
	dev->param.rpf_par[id].src_bsize = width << 16;
	dev->param.rpf_par[id].src_bsize |= pipe->src.height;
	dev->param.rpf_par[id].src_esize = dev->param.rpf_par[id].src_bsize;
}

static void vspi_set_part_full_to_dl(vsp_dev_t *dev)
{
	uds_par_t *uds_par;
	wpf_par_t *wpf_par;
	rpf_par_t *rpf_par;
	struct dl_body *body;
    unsigned int *body_temp;
 	struct display_header *head = (struct display_header *)(dev->pvdata->dlmemory->vaddr);
    memset(head, 0, DL_HEADER_SIZE);
	head->display_list[0].plist =
		VSP_VP_TO_INT(dev->pvdata->dlmemory->paddr + DL_HEADER_SIZE);
        
	/* set pointer */
	body_temp = (unsigned int *)head;
	body_temp += ((head->display_list[0].num_bytes + DL_HEADER_SIZE) >> 2);
	/* get free body */
    body = &dev->pvdata->dlmemory->body[0];
    body->dlist = (struct display_list*)body_temp;
    body->reg_count = 0;
    body->paddr = head->display_list[0].plist;
    
    uds_par = &dev->param.uds_par[0];
    rpf_par = &dev->param.rpf_par[0];
    wpf_par = &dev->param.wpf_par[0];

    vsp_rpf_to_dl_core(dev->pvdata, dev, rpf_par, 0,  body);
    vsp_uds_to_dl_core(dev->pvdata, dev, uds_par, body);
    vsp_wpf_to_dl_core(dev->pvdata, dev, wpf_par, body);
    
    head->display_list[0].num_bytes = body->reg_count*8;
    
	/* finalize DL header */
	head->pnext_header = head->display_list[0].plist + DL_BODY_SIZE*DL_BODY_NUM_FOR_WORK;
	head->int_auto = 2;    
}

void vspi_set_part_diff_to_dl(vsp_dev_t *dev, struct display_header *pre_head, uint32_t par_indx)
{
	struct display_header *head;
    struct dl_body *body;
    unsigned int *body_temp;
	/* set next frame auto start of previous header */
	pre_head->int_auto = 1;

	head = (struct display_header *)
		VSP_DL_HARD_TO_VIRT(pre_head->pnext_header);
    
	/* initialize DL header */
	memset(head, 0, DL_HEADER_SIZE);
	head->display_list[0].plist = pre_head->pnext_header + DL_HEADER_SIZE;

	/* set pointer */
	body_temp = (unsigned int *)head;
	body_temp += ((head->display_list[0].num_bytes + DL_HEADER_SIZE) >> 2);
	/* get free body */
    body = &dev->pvdata->dlmemory->body[par_indx];
    body->dlist = (struct display_list*)body_temp;
    body->reg_count = 0;
    body->paddr = head->display_list[0].plist;
    
    vsp_dl_set_to_core(dev->pvdata, dev, (uintptr_t)&dev->reg.rpf[0].src_bsize, dev->param.rpf_par[0].src_bsize, body);   
    vsp_dl_set_to_core(dev->pvdata, dev, (uintptr_t)&dev->reg.rpf[0].src_esize, dev->param.rpf_par[0].src_esize, body);   
    vsp_dl_set_to_core(dev->pvdata, dev, (uintptr_t)&dev->reg.rpf[0].srcm_addr_y, dev->param.rpf_par[0].srcm_addr_y, body);   
    vsp_dl_set_to_core(dev->pvdata, dev, (uintptr_t)&dev->reg.rpf[0].srcm_addr_c0, dev->param.rpf_par[0].srcm_addr_c0, body);   
    vsp_dl_set_to_core(dev->pvdata, dev, (uintptr_t)&dev->reg.rpf[0].srcm_addr_c1, dev->param.rpf_par[0].srcm_addr_c1, body);   
    vsp_dl_set_to_core(dev->pvdata, dev, (uintptr_t)&dev->reg.rpf[0].srcm_addr_ai, dev->param.rpf_par[0].srcm_addr_ai, body);   
    vsp_dl_set_to_core(dev->pvdata, dev, (uintptr_t)&dev->reg.uds[0].ctrl, dev->param.uds_par[0].ctrl, body);   
    vsp_dl_set_to_core(dev->pvdata, dev, (uintptr_t)&dev->reg.uds[0].clip_size, dev->param.uds_par[0].clip_size, body);   
    vsp_dl_set_to_core(dev->pvdata, dev, (uintptr_t)&dev->reg.uds[0].hphase, dev->param.uds_par[0].hphase, body);   
    vsp_dl_set_to_core(dev->pvdata, dev, (uintptr_t)&dev->reg.uds[0].hszclip, dev->param.uds_par[0].hszclip, body);   
    vsp_dl_set_to_core(dev->pvdata, dev, (uintptr_t)&dev->reg.wpf[0].dstm_addr_y, dev->param.wpf_par[0].dstm_addr_y, body);   
    vsp_dl_set_to_core(dev->pvdata, dev, (uintptr_t)&dev->reg.wpf[0].dstm_addr_c0, dev->param.wpf_par[0].dstm_addr_c0, body);   
    vsp_dl_set_to_core(dev->pvdata, dev, (uintptr_t)&dev->reg.wpf[0].dstm_addr_c1, dev->param.wpf_par[0].dstm_addr_c1, body);   
    vsp_dl_set_to_core(dev->pvdata, dev, (uintptr_t)&dev->reg.wpf[0].hszclip, dev->param.wpf_par[0].hszclip, body);   

    head->display_list[0].num_bytes = body->reg_count*8;
   
	/* finalize DL header */
	head->pnext_header = head->display_list[0].plist + DL_BODY_SIZE*DL_BODY_NUM_FOR_WORK;
	head->int_auto = 2;
}

int	VspLib_scale_start( void *arg, vsp_pipe_t *pipe )
{
	uint32_t offset=0;
    uint32_t width;
    uint32_t par_indx = 0;
    uint32_t div_size = INPUT_WPF_HSIZE_MAX;
	vsp_dev_t *dev = (vsp_dev_t*) arg;
 	struct display_header *pre_head =
		(struct display_header *)(dev->pvdata->dlmemory->vaddr);
    
	pipe->vsp_pipe_id = 0;
	scale_frame_prepare (dev,pipe);	
    width = pipe->dst.width;
    
    while (offset < width)
    {
        /* Update frame parameters before scaling */
        scale_frame_update(dev, pipe, offset, div_size);
        
        if(offset == 0)
        {
            /* Scale first partition */
            vspi_set_part_full_to_dl(dev);
        }
        else
        {
            /* Scale second and more partition */
            vspi_set_part_diff_to_dl(dev, pre_head, par_indx);
            
            /* Update DL header */
            pre_head = (struct display_header *)
				VSP_DL_HARD_TO_VIRT(pre_head->pnext_header);
        }                                      
        offset += div_size;
        par_indx ++;
    }
    
	dev->reg.dl->ctrl = VI6_DL_CTRL_AR_WAIT | VI6_DL_CTRL_DL_ENABLE;
	/* DL LWORD swap */
	dev->reg.dl->swap = VI6_DL_SWAP_LWS;
    /* Wrtie address to DL*/
    dev->reg.dl->hdr_addr0		= dev->pvdata->dlmemory->paddr;

	dev->reg.vi6_ctrl->wpf_irq[0].irq_sta = ~0x0002;
	dev->reg.vi6_ctrl->wpf_irq[0].irq_enb = 0x0002;

	/* __asm__ __volatile__("dsb"); */
    /* Start Scaling */
    start_wpf (dev, 0);
    /* Wait Scaling finish */
    vsp_wait_event(dev);
	dev->state = VSP_RUNNING; 
	return dev->state;
}
