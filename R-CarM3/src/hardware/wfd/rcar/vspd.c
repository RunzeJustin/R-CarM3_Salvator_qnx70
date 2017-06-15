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
#include <sys/slog.h>

#include "vsp.h"

int vspd_run_dl(struct vsp_private_data *vdata, vsp_dev_t *dev, void *dl, int dl_mode)
{
	int ret = 0;

	if (vdata->active) {
		ret = vsp_dl_swap(vdata, dev, dl);
	} else {
		dev->reg.vi6_ctrl->wpf_irq[0].irq_sta = 0;
		dev->reg.vi6_ctrl->dsp_irq.dsp_irq_sta = 0;
		vdata->active = 1;
		ret = vsp_dl_start(vdata, dev, dl, dl_mode);
	}

	return ret;
}

static int vspd_dl_output_du_head_mode(	struct vsp_private_data *vdata, vsp_dev_t *dev)
{
	int i;
	struct dl_body *body;
	/* get free body */
	body = vsp_dl_get_body(vdata);
	if (body == NULL) {
		return EXIT_FAILURE;
	}

	/* set rpf info */
	for (i = 0; i < VSPD_INPUT_IMAGE_NUM; i++) {
		vsp_rpf_to_dl_core(vdata, dev, &dev->param.rpf_par[i], i, body);
	}

	/* set wpf info */
	vsp_wpf_to_dl_core(vdata, dev, &dev->param.wpf_par[0], body);

	vdata->dlmemory->head[0].dl_body_list[0] = body;

	return vspd_run_dl(vdata, dev, &vdata->dlmemory->head[0], DL_MODE_AUTO_REPEAT);
}

static int vspd_dl_output_du_head_less(struct vsp_private_data *vdata, vsp_dev_t *dev)
{
	int i;
	struct dl_body *body;
	TRACE;
	body = vsp_dl_get_single_body(vdata->dlmemory);
	if (body == NULL) {
		return EXIT_FAILURE;
	}

	for (i = 0; i < VSPD_INPUT_IMAGE_NUM; i++) {
		vsp_rpf_to_dl_core(vdata, dev, &dev->param.rpf_par[i], i, body);
	}

	vsp_wpf_to_dl_core(vdata, dev, &dev->param.wpf_par[0], body);

	return vspd_run_dl(vdata, dev, body, DL_MODE_HEADER_LESS_AUTO_REPEAT);
}



inline void vspd_isr_call (void *arg, int id) 
{
	vsp_dev_t *dev = (vsp_dev_t *) arg;
	uint32_t intr_status;

	intr_status = dev->reg.vi6_ctrl->wpf_irq[0].irq_sta;
	if (intr_status & VI6_WPFn_IRQ_DFE)
		vsp_dl_irq_dl_frame_end(dev->pvdata, dev);
	else
		vsp_dl_irq_frame_end(dev->pvdata, dev);
	
	intr_status = dev->reg.vi6_ctrl->dsp_irq.dsp_irq_sta & VI6_DISP_IRQ_STA_DST;
	if (intr_status) {
		dev->reg.vi6_ctrl->dsp_irq.dsp_irq_sta = ~intr_status;
		vsp_dl_irq_display_start(dev->pvdata, dev);
	}
}

const struct sigevent* vspd0_intr (void* arg, int id)
{
	vspd_isr_call (arg,id);
	return NULL;	
}

const struct sigevent* vspd1_intr (void* arg, int id)
{
	vspd_isr_call (arg,id);
	return NULL;	
}

const struct sigevent* vspd2_intr (void* arg, int id)
{
	vspd_isr_call (arg,id);
	return NULL;
}


int vspd_dl_output_du(struct vsp_private_data *vdata, vsp_dev_t *dev, int pipeid)
{
	int ret = EXIT_SUCCESS;
	TRACE;
	if (vdata->dlmemory->dl_mode == DL_MODE_HEADER_LESS_AUTO_REPEAT){
		/* header less auto repeat mode */
		return vspd_dl_output_du_head_less(vdata, dev);
	}
	else if(vdata->dlmemory->dl_mode == DL_MODE_AUTO_REPEAT){
		/* normal auto repeat mode */
		return vspd_dl_output_du_head_mode(vdata, dev);
	} else {
		ret = EXIT_FAILURE;
	}
	return ret;
}

int	VspLib_frame_update( void *arg, vsp_pipe_t *pipe )
{
	vsp_dev_t *dev = (vsp_dev_t*) arg;
	int id = pipe->vsp_pipe_id;
	PIPE_VALIDATE(id,return -1);

	set_rpf_var (dev,pipe);
	set_wpf_var (dev,pipe);
	vspd_dl_output_du(dev->pvdata, dev, id);
	
	dev->state = VSP_RUNNING; 
	return dev->state;
}

void VspLib_activate_pipe ( void *arg, int pipeId )
{
	PIPE_VALIDATE(pipeId,return);

	vsp_dev_t *dev = (vsp_dev_t *)arg;
	rpf_par_t *rpf_par = &dev->param.rpf_par[pipeId];
	rpf_par->active = 1;
}

void VspLib_deactivate_pipe ( void *arg, int pipeId )
{
	TRACE;
	SLOG_DEBUG("arg=%p, pipeId = %d",arg,pipeId);
	if (!arg) {
		SLOG_ERROR("%s: no dev",__FUNCTION__);
		return;
	}
	vsp_dev_t *dev = (vsp_dev_t *)arg;
	PIPE_VALIDATE(pipeId,return);

	rpf_par_t *rpf_par = &dev->param.rpf_par[pipeId];
	int active = rpf_par->active;
	rpf_par->active = 0;
	if(active){
		vspd_dl_output_du(dev->pvdata, dev, pipeId);
	}
}
