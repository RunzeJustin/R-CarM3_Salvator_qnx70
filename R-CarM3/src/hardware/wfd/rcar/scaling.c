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
 
 
#include "rcar_display.h"
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <devctl.h>
#include <string.h>
#include <screen/screen.h>
#include <screen/iomsg.h>
#include "vsp.h"



void scale_pipe_init(du_dev_t* dev, pipe_t* pipe)
{
	int i;
	TRACE;
	SLOG_DEBUG2("       pipeId=%d",pipe->pipeId);
	for(i = 0;i < dev->portsSize;i++)
	{
		if((pipe->pipeId == dev->ports[i].du_cfg->scale_pipe) && 
			(dev->ports[i].du_cfg->ExScaleHwId != DEVICE_NONE))
		{
			pipe->scale.obuffer[0] = NULL;
			pipe->scale.obuffer[1] = NULL;
			pipe->scale.obuffer_phys[0] = 0;
			pipe->scale.obuffer_phys[1] = 0;
			pipe->scale.cur_idx = 0;
			pipe->scale.last_dst_width = 0;
			pipe->scale.last_dst_height = 0;
			SLOG_DEBUG2("       return. (done)");
			return;
		}
	}
	SLOG_DEBUG2("       return. (not a scale pipe)");
}

void scale_pipe_quit(du_dev_t *dev, pipe_t* pipe)
{
    int i;
    for(i = 0; i < 2; i++)
	{
		if (NULL != pipe->scale.obuffer[i])
		{
			munmap (pipe->scale.obuffer[i], pipe->scale.last_dst_width*pipe->scale.last_dst_height*4);
		}
	}
}

int scaling_possible (pipe_t *pipe)
{
	port_t *port = (port_t *)pipe->port;
	return ((pipe->pipeId == port->du_cfg->scale_pipe) &&
							port->du_cfg->scaling_dev);
}

void *exscale_init (void *disp)
{
 	port_t *port = (port_t *)disp;
	void *vsp_dev;
	vsp_info_t vsp_info;
	
	vsp_info.Id = port->du_cfg->ExScaleHwId;
 	vsp_dev = VspLib_init (&vsp_info, port);

	if (!vsp_dev)
	{
		perror ("VspLib_init");
		return NULL;
	}

	return vsp_dev;
}

int32_t Run_Scaling(pipe_t *pipe, win_image_t* img_dst)
{
	port_t *port 		= (port_t *)pipe->port;
	source_t *source 	= (source_t *)pipe->src;
	win_image_t *img_src 	= (win_image_t *)source->image;
	void *scaling_dev 		= port->du_cfg->scaling_dev;
	vsp_pipe_t scale;
    int i;
	int pt_fd = -1;
    
	if(pipe->scale.last_dst_width != pipe->dst_rect[2] || pipe->scale.last_dst_height != pipe->dst_rect[3])
	{
		for(i = 0; i < 2; i++)
		{
			if (NULL != pipe->scale.obuffer[i])
			{
				munmap (pipe->scale.obuffer[i], pipe->scale.last_dst_width*pipe->scale.last_dst_height*4);
				pipe->scale.obuffer[i] = NULL;
			}
		}

		unsigned int screensize = pipe->dst_rect[2] * pipe->dst_rect[3] * 4;
		for(i = 0; i < 2; i++)
		{
			pt_fd = posix_typed_mem_open(RCARDU_POSIX_TYPED_MEM_PATH, O_RDWR, POSIX_TYPED_MEM_ALLOCATE_CONTIG);
			if (pt_fd == -1) 
			{
				SLOG_ERROR("Run_Scaling: return: WFD_ERROR_OUT_OF_MEMORY (Can't open posix typed memory)");
				return R_VSP_NG;
			}
			pipe->scale.obuffer[i] = mmap64(NULL, screensize, PROT_READ | PROT_WRITE | PROT_NOCACHE, MAP_SHARED, pt_fd, 0);
			if (pipe->scale.obuffer[i] == MAP_FAILED)
			{
				SLOG_ERROR("Run_Scaling: WFD_ERROR_OUT_OF_MEMORY (Can't mmap posix typed memory)");
				close(pt_fd);
				return R_VSP_NG;
			}
			pipe->scale.obuffer_phys[i] = (uint32_t)disp_phys_addr(pipe->scale.obuffer[i]);
			pipe->scale.last_dst_width = pipe->dst_rect[2];
			pipe->scale.last_dst_height = pipe->dst_rect[3];
			close(pt_fd);
		}
	}

	pipe->scale.cur_idx = (pipe->scale.cur_idx == 0) ? 1 : 0;
   
    
	/* set parameters */
	scale.src.addr.y_rgb = img_src->paddr;
	scale.src.addr.c0 = img_src->paddr + img_src->planar_offsets[1];
	scale.src.addr.c1 = img_src->paddr + img_src->planar_offsets[2];
	scale.src.fmt = WfdToVspFormat (img_src->format);
	scale.src.width = img_src->width;
	scale.src.height = img_src->height;
 	scale.src_rect[0] = pipe->src_rect[0];
	scale.src_rect[1] = pipe->src_rect[1];
	scale.src_rect[2] = pipe->src_rect[2];
	scale.src_rect[3] = pipe->src_rect[3];
	
	scale.dst.width = pipe->dst_rect[2];
	scale.dst.height = pipe->dst_rect[3];
	scale.dst.fmt = WfdToVspFormat (/* img_src->format */ WFD_FORMAT_RGBA8888_QNX);	/* hardcode output color format to RGBA8888 */
	scale.dst.addr.y_rgb = pipe->scale.obuffer_phys[pipe->scale.cur_idx];
	scale.dst.addr.c0 = pipe->scale.obuffer_phys[pipe->scale.cur_idx] + scale.dst.width*scale.dst.height;
	scale.dst.addr.c1 = 0;
    
    VspLib_scale_start(scaling_dev, &scale);
    
	img_dst->width = pipe->dst_rect[2];
	img_dst->height = pipe->dst_rect[3];
	img_dst->format = WFD_FORMAT_RGBA8888_QNX; /* hardcode output color format to RGBA8888 */
	img_dst->paddr = pipe->scale.obuffer_phys[pipe->scale.cur_idx];
	img_dst->vaddr = pipe->scale.obuffer[pipe->scale.cur_idx];
	img_dst->strides[0] = pipe->dst_rect[2] * 4;
	img_dst->planar_offsets[1] = pipe->dst_rect[2]*pipe->dst_rect[3];

	if (img_dst->format == WFD_FORMAT_YUV420_QNX || img_dst->format == WFD_FORMAT_YV12_QNX)
	{
		img_dst->planar_offsets[2] = pipe->dst_rect[2] * ((pipe->dst_rect[3] * 5 + 1) / 4);
	}
	else
	{
		img_dst->planar_offsets[2] = 0;
	}
    return R_VSP_OK;
}
