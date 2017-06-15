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
 
#ifndef _VSP_H_
#define _VSP_H_

#include <pthread.h>
#include <screen/screen.h>
#include <screen/iomsg.h>
#include "rcar_display.h"
#include "vsp_drv.h"
#include "vspd.h"
#include "vspi.h"


int is_vspd (vsp_dev_t *dev);
int is_vspi (vsp_dev_t *dev);
void vsp_activate_pipeline(void *device, void *pipeline);
void vsp_deactivate_pipeline(void *device, void *pipeline);
void *vsp_init (void *device, void *disp);
void vsp_fini (void *device, void *disp);
void vsp_frame_update(void *device, void *pipeline);
int WfdToVspFormat (int fmt);
int VspLib_scale_start( void *arg, vsp_pipe_t *pipe );

void *VspLib_init( void *info, port_t *port);
void VspLib_fini (void *arg);
int	VspLib_frame_update( void *arg, vsp_pipe_t *pipe );
void VspLib_activate_pipe ( void *arg, int pipeId );
void VspLib_deactivate_pipe ( void *arg, int pipeId );
void VspLib_compose_update_sync (void *arg);

int scaling_possible (pipe_t *pipe);
int32_t Run_Scaling(pipe_t *pipe, win_image_t* img_dst);

void set_rpf_var (vsp_dev_t *dev, vsp_pipe_t *pipe);
void set_wpf_var (vsp_dev_t *dev, vsp_pipe_t *pipe);

void vsp_dl_set_to_core(struct vsp_private_data *vdata, vsp_dev_t* dev, uintptr_t reg, uint32_t data, struct dl_body *body);
void vsp_rpf_to_dl_core(struct vsp_private_data *vdata, vsp_dev_t* dev, rpf_par_t *in, int rpf_id, struct dl_body *body);
void vsp_uds_to_dl_core(struct vsp_private_data *vdata, vsp_dev_t* dev, uds_par_t *in,	struct dl_body *body);
void vsp_wpf_to_dl_core(struct vsp_private_data *vdata, vsp_dev_t* dev, wpf_par_t *out, struct dl_body *body);

int vsp_dl_start(struct vsp_private_data *vdata, vsp_dev_t *dev, void *dl, int dl_mode);
struct dl_body *vsp_dl_get_body(struct vsp_private_data *vdata);
struct dl_body *vsp_dl_get_single_body(struct dl_memory *dlmemory);
void vsp_dl_irq_dl_frame_end(struct vsp_private_data *vdata, vsp_dev_t *dev);
void vsp_dl_irq_frame_end(struct vsp_private_data *vdata, vsp_dev_t *dev);
int vsp_dl_irq_display_start(struct vsp_private_data *vdata, vsp_dev_t *dev);
int vsp_dl_create(struct vsp_private_data *vdata, port_t *port, int dl_mode);
void vsp_dl_reset(struct vsp_private_data *vdata);
int vsp_dl_swap(struct vsp_private_data *vdata, vsp_dev_t *dev, void *dl);
void vsp_dl_destroy(struct vsp_private_data *vdata);

void set_rpf (vsp_dev_t *dev, rpf_par_t *rpf_par);
void set_wpf (vsp_dev_t *dev, wpf_par_t *wpf_par);
void set_uds (vsp_dev_t *dev, uds_par_t *uds_par);
int vsp_wait_event(vsp_dev_t *dev);
void start_wpf (vsp_dev_t *dev, int wfd_id);
void routing_init (vsp_dev_t *dev);
int get_bpp(int fmt);
void get_stride(int fmt, int *mul, int *div, int *mulC, int *divC);
void init_lif (vsp_dev_t *dev);
void bru_init (vsp_dev_t *dev);
extern vsp_private_t VSP_LIST[];
#endif
