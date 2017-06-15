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
 
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "rcar_display.h"
#include "vsp.h"

#define PARSE_ERROR(line) { \
	SLOG_ERROR("Invalid graphics.conf: line %d\n",line); \
	return -1; \
}

#define NUM_DISPLAY_CONFIG_MAX 8

graphics_conf_t display_conf[NUM_DISPLAY_CONFIG_MAX];
int displayid;
int opt_display_begin, opt_framebuffer_begin;

compose_hdl vsp_compose = {
	.sub_hw_init = vsp_init,
	.sub_hw_fini = vsp_fini,
	.activate_pipeline = vsp_activate_pipeline,
	.deactivate_pipeline = vsp_deactivate_pipeline,
	.frame_update = vsp_frame_update,
	.sub_hw_frame_sync = NULL,
};

du_cfg_t du_cfg_list[] = {
	//DU0 configuration
	{
		.reg_off = DU0_OFF,
		.irq	= DU0_IRQ,
		.isr = rcardu_du0_isr,
		.grp_index = DU_GRP_0,
		.du_index = DU_CH_0,
		.pipe_ids = {1, 2, 3, 4},
		.use_planes = {1, 1, 1, 1},
		.pipe_num = 4,
		.scale_pipe = 1,
		.hw_compose = &vsp_compose,
		.ComposeHwId = VSPD0,
		.ExScaleHwId = DEVICE_NONE,
	},
	//DU1 configuration
	{
		.reg_off = DU1_OFF,
		.irq	= DU1_IRQ,
		.isr = rcardu_du1_isr,
		.grp_index = DU_GRP_0,
		.du_index = DU_CH_1,
		.pipe_ids = {5, 6, 7, 8},
		.use_planes = {3, 3, 3, 3},
		.pipe_num = 4,
		.scale_pipe = 5,
		.hw_compose = &vsp_compose,
		.ComposeHwId = VSPD1,
		.ExScaleHwId = DEVICE_NONE,
	},
	//DU2 configuration
	{
		.reg_off = DU2_OFF,
		.irq	= DU2_IRQ,
		.isr = rcardu_du2_isr,
		.grp_index = DU_GRP_1,
		.du_index = DU_CH_2,
		.pipe_ids = {9, 10, 11, 12},
		.use_planes = {1, 1, 1, 1},
		.pipe_num = 4,
		.scale_pipe = 9,
		.hw_compose = &vsp_compose,
		.ComposeHwId = VSPD2,
		.ExScaleHwId = DEVICE_NONE,
	},
};

int parse_config (char *opt,int line)
{
	char *str,*str1,*str2, *substr;

	if ((str = strstr (opt, "begin")))
	{
		str1 = str;
		if ((str = strstr (str1, "display")))
		{
			opt_display_begin = 1;
			displayid = atoi (str+strlen("display"));
			display_conf[displayid].usable = 1;
		}
		else if ((str = strstr (str1, "framebuffer")))
			opt_framebuffer_begin = 1;
		return 0;
	}
	if (opt_display_begin && (str = strstr (opt, "end")))
	{
		str1 = str;
		if ((str = strstr (str1, "display")))
			opt_display_begin = 0;
		return 0;
	}
	if (opt_display_begin && (str = strstr (opt, "pipeline")))
	{
		str1 = str;
		if ((str = strstr (str1, "=")))
			display_conf[displayid].pipeline = atoi (str+1);
		else PARSE_ERROR(line);
		return 0;
	}		
	if (opt_framebuffer_begin && (str = strstr (opt, "end")))
	{
		str1 = str;
		if ((str = strstr (str1, "class")))
			opt_framebuffer_begin = 0;
		return 0;
	}
	if (opt_display_begin && (str = strstr (opt, "video-mode")))
	{
		str1 = str;
		if ((str = strstr (str1, "=")))
		{
			str2 = str;
			str2++;
			if ((substr = strstr (str2, "x")))
				*substr = '\0';
			else PARSE_ERROR(line);
			display_conf[displayid].xres = atoi (str2);
			str2 =substr+1;
			if ((substr = strstr (str2, "@")))
			{
				*substr = '\0';
				display_conf[displayid].refresh_rate = atoi (substr+1);
			}
			display_conf[displayid].yres = atoi (str2);
		}
		return 0;
	}
	if (opt_framebuffer_begin && (str = strstr (opt, "display")))
	{
		str1 = str;
		if ((str = strstr (str1, "=")))
			displayid = atoi (str+1);
		else PARSE_ERROR(line);
		return 0;
	}
	if (opt_framebuffer_begin && (str = strstr (opt, "pipeline")))
	{
		str1 = str;
		if ((str = strstr (str1, "=")))
			display_conf[displayid].pipeline = atoi (str+1);
		else PARSE_ERROR(line);
		return 0;
	}
	return 0;
}

int get_config_data(du_dev_t *dev, const char *filename)
{
	char buf[256];
	char *opt,*c;
	int lineno=0, num_of_port=0, i;
	port_t *port = dev->ports;
	
	FILE *fd = fopen(filename, "r");
	memset (display_conf,0,sizeof(display_conf));

    if (fd == NULL)
    {
        /* No config file, use what we have */
        slogf(99, 1, "Can't locate %s file",filename);
        return 0;
    }

    while (fgets(buf, sizeof (buf), fd) != NULL) {
    	lineno++;
        c = buf;
        while ((*c == ' ') || (*c == '\t'))
            c++;
        if ((*c == '\015') || (*c== '\032') || (*c == '\0') || (*c == '\n') || (*c == '#'))
            continue;
        opt = c;
        while ((*c == '\015') || (*c== '\032') || ((*c != '\0') && (*c != '\n') && (*c != '#')))
            c++;
        *c = '\0';
		if (-1 == parse_config (opt,lineno))
			return 0;
    }
    fclose(fd);

	for (i=0;i<NUM_DISPLAY_CONFIG_MAX;i++)
	{
		if (display_conf[i].usable)
		{				
			port->default_pipeline = display_conf[i].pipeline;
			port++;
			num_of_port++;
		}
	}

	return num_of_port;
}

void bind_display_to_port (du_dev_t *dev)
{
	int i,j;
	int bound=0;
	port_t *port = dev->ports;
	for (i=0;i<NUM_DISPLAY_CONFIG_MAX;i++)
	{
		if (display_conf[i].usable)
		{
			bound = 0;
			for (j=0;j<RCARDU_WFD_MAX_NUMBER_OF_PORTS;j++)
			{
				if (IS_INCLUDED(display_conf[i].pipeline, du_cfg_list[j].pipe_ids, du_cfg_list[j].pipe_num))
				{	
				    SLOG_DEBUG("Display %d bound to DU%d",i,j);
					port->du_cfg = &du_cfg_list[j];
					port++;
					bound = 1;
					break;
				}
			}
			if (!bound) SLOG_ERROR("Can't bind display %d",i);
		} else
		{
			SLOG_DEBUG("Display %d is not usable",i);
		}
	}
}

int get_plane_from_pipeId (port_t *port, int pipeId)
{
	int i;
	
	for (i=0;i<RCARDU_MAX_NUMBER_PIPELINES;i++)
	{
		if (pipeId==port->du_cfg->pipe_ids[i])
		{
			break;
		}
	}
	return port->du_cfg->use_planes[i];
}
