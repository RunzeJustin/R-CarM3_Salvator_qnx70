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
#include <pthread.h>
#include "vsp.h"


int WfdToVspFormat (int fmt)
{
	int VspFormat=0;
    switch (fmt) {
        case WFD_FORMAT_YUV420_QNX:		//YCbCr4:2:0 interleaved
			VspFormat = VSPCORE_YUV420I;
			break;
        case WFD_FORMAT_NV12_QNX:		//YCbCr4:2:0 semi planar
			VspFormat = VSPCORE_YUV420SP;
			break;
        case WFD_FORMAT_YV12_QNX:		//YCbCr4:2:0 planar
            VspFormat = VSPCORE_YUV420P; 			
            break;
        case WFD_FORMAT_YUY2_QNX:		//YCbCr4:2:2 interleaved type 0
			VspFormat = VSPCORE_YUV422Itype0 | ORDER_YUY2;
			break;
		case WFD_FORMAT_UYVY_QNX:
			VspFormat = VSPCORE_YUV422Itype0 | ORDER_UYVY;
			break;
		case WFD_FORMAT_YVYU_QNX:
			VspFormat = VSPCORE_YUV422Itype0 | ORDER_YVYU;
			break;
        case WFD_FORMAT_RGBA4444_QNX:
			VspFormat = VSPCORE_RGBP444;
			break;
        case WFD_FORMAT_RGBX4444_QNX:
			VspFormat = VSPCORE_RGBX444;
			break;			
        case WFD_FORMAT_RGBA5551_QNX:
			VspFormat = VSPCORE_RGBP555;
			break;			
        case WFD_FORMAT_RGBX5551_QNX:
			VspFormat = VSPCORE_RGBx555;
			break;		
        case WFD_FORMAT_RGB565_QNX:
			VspFormat = VSPCORE_RGB565;
			break;			
        case WFD_FORMAT_RGB888_QNX:
			VspFormat = VSPCORE_BGR888;
            break;
        case WFD_FORMAT_RGBA8888_QNX:
			VspFormat = VSPCORE_RGBP888;
			break;
        case WFD_FORMAT_RGBX8888_QNX:
            VspFormat = VSPCORE_RGBP888 | OPACITY_FULL;
            break;
        default:
            SLOG_ERROR("WfdToVspFormat: format not supported:%d",fmt);
            break;
    }
	return VspFormat;
}

void *vsp_init (void *device, void *disp){
 	port_t *port 			= (port_t *)disp;
	void *vsp_dev;
	vsp_info_t vsp_info;

	vsp_info.Id = port->du_cfg->ComposeHwId;
	/* Set master layer information for VSP */
	vsp_info.mHeight = port->active_mode->timings->vlines;
	vsp_info.mWidth = port->active_mode->timings->hpixels;
	vsp_info.mFormat = WfdToVspFormat (WFD_FORMAT_RGBA8888_QNX);
			
 	vsp_dev = VspLib_init (&vsp_info, port);
	if (!vsp_dev)
	{
		perror ("VspLib_init");
		return NULL;
	}

	return vsp_dev;
}

void vsp_fini (void *device, void *disp){
	port_t *port = (port_t *)disp;
	if (port->du_cfg->compose_dev)
		VspLib_fini (port->du_cfg->compose_dev);
}

void vsp_activate_pipeline(void *device, void *pipeline) {
	pipe_t *pipe = (pipe_t *)pipeline;
	port_t *port = pipe->port;
	void *compose_dev = port->du_cfg->compose_dev;
	
 	int vsp_pipe_id = (pipe->pipeId-1)%VSP_COMPOSITION_PIPELINE_MAX;
	VspLib_activate_pipe ( compose_dev, vsp_pipe_id );
}

void vsp_deactivate_pipeline(void *device, void *pipeline) {
	TRACE;
	pipe_t *pipe = (pipe_t *)pipeline;
	port_t *port = pipe->port;
	void *compose_dev = port->du_cfg->compose_dev;
 	int vsp_pipe_id = (pipe->pipeId-1)%VSP_COMPOSITION_PIPELINE_MAX;
	VspLib_deactivate_pipe ( compose_dev, vsp_pipe_id );
}

void vsp_frame_update(void *device, void *pipeline)
{
	pipe_t *pipe 		= (pipe_t *)pipeline;
	port_t *port 		= (port_t *)pipe->port;
	source_t *source 	= (source_t *)pipe->src;
	win_image_t *img 	= (win_image_t *)source->image;
	void *compose_dev 		= port->du_cfg->compose_dev;
	vsp_pipe_t vsp_pipe;

	/* set parameters */
	vsp_pipe.vsp_pipe_id = (pipe->pipeId-1)%VSP_COMPOSITION_PIPELINE_MAX;
	if (scaling_possible(pipe))
	{
		win_image_t img_dst;
		int32_t lresult = Run_Scaling(pipe, &img_dst);
		if(R_VSP_OK != lresult)
		{
			return;
		}
		vsp_pipe.src.addr.y_rgb = img_dst.paddr;
		vsp_pipe.src.addr.c0 = img_dst.paddr + img_dst.planar_offsets[1];
		vsp_pipe.src.addr.c1 = img_dst.paddr + img_dst.planar_offsets[2];
		vsp_pipe.src.fmt = WfdToVspFormat (img_dst.format);
		vsp_pipe.src.width = img_dst.width;
		vsp_pipe.src.height = img_dst.height;
		vsp_pipe.src_rect[0] = pipe->src_rect[0];
		vsp_pipe.src_rect[1] = pipe->src_rect[1];
		vsp_pipe.src_rect[2] = pipe->dst_rect[2];
		vsp_pipe.src_rect[3] = pipe->dst_rect[3];
	}
	else
	{
		vsp_pipe.src.addr.y_rgb = img->paddr;
		vsp_pipe.src.addr.c0 = img->paddr + img->planar_offsets[1];
		vsp_pipe.src.addr.c1 = img->paddr + img->planar_offsets[2];
		vsp_pipe.src.fmt = WfdToVspFormat (img->format);
		if ((pipe->transparency == WFD_TRANSPARENCY_NONE) && (img->format == WFD_FORMAT_RGBA8888_QNX))
			vsp_pipe.src.fmt |= OPACITY_FULL;
		vsp_pipe.src.width = img->width;
		vsp_pipe.src.height = img->height;
		vsp_pipe.src_rect[0] = pipe->src_rect[0];
		vsp_pipe.src_rect[1] = pipe->src_rect[1];
		vsp_pipe.src_rect[2] = pipe->src_rect[2];
		vsp_pipe.src_rect[3] = pipe->src_rect[3];
	}
 		
	vsp_pipe.dst.width = pipe->dst_rect[2];
	vsp_pipe.dst.height = pipe->dst_rect[3];
	vsp_pipe.dst.hcoord = pipe->dst_rect[0];
	vsp_pipe.dst.vcoord = pipe->dst_rect[1];
	
	VspLib_frame_update(compose_dev, &vsp_pipe);
}
