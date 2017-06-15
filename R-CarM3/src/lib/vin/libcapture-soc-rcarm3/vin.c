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


#include <stdint.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/neutrino.h>
#include <hw/inout.h>
#include <sys/resmgr.h>
#include <sys/iofunc.h>
#include <sys/dispatch.h>
#include <errno.h>
#include <string.h>
#include <atomic.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <inttypes.h>
#include <devctl.h>
#include "vin.h"
#include <arm/r-car.h>
#include <screen/screen.h>

#define SOC_INFO	"MIPI-CSI2, VIN"
#define SOC_INIT	 rcar_vin_init
#define SOC_FINI	 rcar_vin_fini
#define SOC_UPDATE	 rcar_vin_update

extern rcar_imrlx4_init();
extern rcar_imrlx4_fini();
pthread_mutex_t mutex;

paddr_t rcar_vin_mphys(void *addr) 
{
	off64_t offset;

	if(mem_offset64(addr, NOFD, 1, &offset, 0) == -1) {
		return -1;
	}
	return offset;
}

void *rcar_vin_event_handler(void *data)
{
	volatile int vin_ints = 0;
	struct _pulse pulse;
	iov_t iov;
	int	rcvid;
	int slot = 0;

	SETIOV(&iov, &pulse, sizeof(pulse));
	
	rcar_context_t *p_soc = (rcar_context_t *)data;
	rcar_vin_t *vin = &p_soc->vin;
	
	vin->frm_end = 0;
	
	for (;;) {
		if ((rcvid = MsgReceivev(vin->chid, &iov, 1, NULL)) == -1)
			continue;

		switch (pulse.code) 
		{
			case RCAR_VIN_PULSE:
				atomic_set(&vin->frm_end, 1);
				vin_ints = in32(vin->vbase + RCAR_VIN_INTS);
				out32(vin->vbase + RCAR_VIN_INTS, vin_ints);	
				if((vin_ints & (1 << 1))||(vin_ints & (1 << 4))) {
					pthread_mutex_lock(&mutex);
					slot = in32(vin->vbase + RCAR_VIN_MS);
					slot &= (3 << 3);
					slot = slot >> 3;
					if(slot < vin->nbufs) {
						vin->buf.latest = slot;
						rcar_imrlx4_update_frame(vin->buf.addr[slot], (paddr_t)vin->frm_bufs[slot]);
					}
					pthread_mutex_unlock(&mutex);
				}
				InterruptUnmask(vin->irq, vin->iid);
				atomic_clr_value(&vin->frm_end, 1);
				break;
			case RCAR_VIN_IMR_PULSE:
				pthread_mutex_lock(&mutex);
				pthread_cond_broadcast(&vin->cond);
				pthread_mutex_unlock(&mutex);
				break;
			case RCAR_VIN_END:
				return NULL;
			default:
				if (rcvid)
					MsgReplyv(rcvid, ENOTSUP, &iov, 1);
				break;
		}		
	}
}

int rcar_vin_create_thread(capture_context_t context)
{
	rcar_context_t *p_soc = (rcar_context_t *)context;
	rcar_vin_t* vin = &p_soc->vin;
	
	ThreadCtl(_NTO_TCTL_IO, 0);
	
	if ((vin->chid = ChannelCreate(_NTO_CHF_DISCONNECT | _NTO_CHF_UNBLOCK)) == -1)
		return -1;

	if ((vin->coid = ConnectAttach(0, 0, vin->chid, _NTO_SIDE_CHANNEL, 0)) == -1)
		goto fail;

	pthread_attr_init(&vin->attr);
	pthread_attr_setschedpolicy(&vin->attr, SCHED_RR);
	vin->param.sched_priority = 21;
	pthread_attr_setschedparam(&vin->attr, &vin->param);
	pthread_attr_setinheritsched(&vin->attr, PTHREAD_EXPLICIT_SCHED);
	pthread_attr_setdetachstate(&vin->attr, PTHREAD_CREATE_DETACHED);
	pthread_attr_setstacksize(&vin->attr, 8192);

	vin->event.sigev_notify   = SIGEV_PULSE;
	vin->event.sigev_coid     = vin->coid;
	vin->event.sigev_code     = RCAR_VIN_PULSE;
	vin->event.sigev_priority = 21;

	// Create vin event handler 
	if (pthread_create(&vin->tid, &vin->attr, (void *)rcar_vin_event_handler, context)) {
		fprintf(stderr, "%s:  Unable to create event handler\n", __FUNCTION__);
		goto fail;
	}
	if ((vin->iid = InterruptAttachEvent(vin->irq, &vin->event, _NTO_INTR_FLAGS_TRK_MSK|_NTO_INTR_FLAGS_END)) == -1){
		fprintf(stderr,"%s: Interrupt attach failed.\n", __FUNCTION__);
		goto fail;
	}
	return 0;
	
fail:
	ConnectDetach(vin->coid);
	ChannelDestroy(vin->chid);
	return -1;
}

uint32_t rcar_vin_compute_ratio(uint32_t input, uint32_t output)
{
	if (output > input)
		return input * 4096 / output;
	else
		return (input - 1) * 4096 / (output - 1);
}

uint32_t rcar_vin_get_bwidth(uint32_t ratio)
{
	uint32_t bwidth;
	uint32_t mant, frac;
	
	mant = (ratio & 0xF000) >> 12;
	frac = ratio & 0x0FFF;
	
	if (mant)
		bwidth = 64 * 4096 * mant / (4096 * mant + frac);
	else
		bwidth = 64;
	
	return bwidth;
}

void rcar_vin_enable_clock(capture_context_t context)
{
	uintptr_t CPG_CPGWPR_reg;
	uintptr_t SMSTPCR8_reg;
	uintptr_t MSTPSR8_reg;

	uint32_t tmp, mask;
	
	rcar_context_t *p_soc = (rcar_context_t *)context;

	CPG_CPGWPR_reg = mmap_device_io(4, 0xE6150900);
	SMSTPCR8_reg   = mmap_device_io(4, 0xE6150990);
	MSTPSR8_reg	   = mmap_device_io(4, 0xE61509A0);

	if(p_soc->active_dev)
		mask = (1 << 7);					// VIN4
	else
		mask = (1 << 11);					// VIN0

	/* Enale supply clock to module */
	tmp = in32(MSTPSR8_reg);
	tmp &= ~mask;
	out32(SMSTPCR8_reg, tmp);

	/* Unmap register */
	munmap_device_io(CPG_CPGWPR_reg, 4);
	munmap_device_io(SMSTPCR8_reg, 4);
	munmap_device_io(MSTPSR8_reg, 4);
}

void rcar_vin_disable_clock (capture_context_t context)
{
	uintptr_t SMSTPCR8_reg;
	uintptr_t CPG_CPGWPR_reg;
	uintptr_t MSTPSR8_reg;

	uint32_t tmp;
	
	rcar_context_t *p_soc = (rcar_context_t *)context;

	CPG_CPGWPR_reg = mmap_device_io(4, 0xE6150900);
	SMSTPCR8_reg   = mmap_device_io(4, 0xE6150990);
	MSTPSR8_reg	   = mmap_device_io(4, 0xE61509A0);

	if(p_soc->active_dev)
		tmp = (1 << 7);
	else
		tmp = (1 << 11);
	
	/* Stop supply clock to module */
	out32(CPG_CPGWPR_reg, tmp);
	out32(SMSTPCR8_reg, ~tmp);

	/* Unmap register */
	munmap_device_io(CPG_CPGWPR_reg, 4);
	munmap_device_io(SMSTPCR8_reg, 4);
	munmap_device_io(MSTPSR8_reg, 4);
}

void rcar_vin_scale(rcar_vin_t* vin)
{
	uint32_t ratio_h, ratio_v;
	uint32_t bwidth_h, bwidth_v;
	uint32_t clip_size;
	
	cam_info_t *cam = &vin->cam; 
	
	ratio_h = rcar_vin_compute_ratio(cam->cw, cam->dw);
	ratio_v = rcar_vin_compute_ratio(cam->ch, cam->dh);
	
	bwidth_h = rcar_vin_get_bwidth(ratio_h);
	bwidth_v = rcar_vin_get_bwidth(ratio_v);
		
	if(cam->interlace) {
		clip_size = ((cam->dw - 1) << 16) | (cam->dh/2 - 1);
	}
	else {
		clip_size = ((cam->dw - 1) << 16) | (cam->dh - 1);
	}
		
	out32(vin->vbase + RCAR_VIN_MC, in32(vin->vbase + RCAR_VIN_MC) | RCAR_VIN_MC_SCLE);
	out32(vin->vbase + RCAR_VIN_UDS_CTRL, RCAR_VIN_UDS_CTRL_AMD);
	out32(vin->vbase + RCAR_VIN_UDS_SCALE, (ratio_h << 16) | ratio_v);
	out32(vin->vbase + RCAR_VIN_UDS_PASS_BW, (bwidth_h << 16) | bwidth_v);
	out32(vin->vbase + RCAR_VIN_UDS_CLIPSIZE, clip_size);

	out32(vin->vbase + RCAR_VIN_IS, (cam->dw + 31) & ~0x1f);
	//out32(vin->vbase + RCAR_VIN_IS, (cam->dw + 15) & ~0x0f);
}

void rcar_vin_crop(rcar_vin_t* vin)
{
	cam_info_t *cam = &vin->cam; 
	
	if(cam->cw == 0)
		cam->cw = cam->sw - cam->cx;
	
	if(cam->ch == 0)
		cam->ch = cam->sh - cam->cy;
	
	/* Set Start/End Pixel/Line Pre-Clip */
	out32(vin->vbase + RCAR_VIN_SPPRC, cam->cx);
	out32(vin->vbase + RCAR_VIN_EPPRC, cam->cx + cam->cw - 1);
	
	switch (cam->interlace) 
	{
		case 1:
			out32(vin->vbase + RCAR_VIN_SLPRC, cam->cy/2);
			out32(vin->vbase + RCAR_VIN_ELPRC, (cam->cy + cam->ch)/2 - 1);
			break;
		case 0:
			out32(vin->vbase + RCAR_VIN_SLPRC, cam->cy);
			out32(vin->vbase + RCAR_VIN_ELPRC, cam->cy + cam->ch - 1);
			break;
	}
}

void rcar_vin_setup(capture_context_t context)
{
	int i;
	uint32_t input_is_yuv = 0;
	uint32_t output_is_yuv = 0;
	uint32_t vnmc, dmr, dmr2, ifmd, interrupt;
	uint32_t mem_size;
	
	rcar_context_t *p_soc = (rcar_context_t *)context;
	rcar_vin_t *vin = &p_soc->vin;
	cam_info_t *cam = &vin->cam;
	img_info_t img;
	
	/* Enable clock */
	rcar_vin_enable_clock(context);
	
	/* Interlace interface */
	if(p_soc->active_dev) {
		vnmc = RCAR_VIN_MC_IM_FULL;
	} else {
		if(!cam->interlace) {
			vnmc = RCAR_VIN_MC_IM_ODD_EVEN;
		} else {
			vnmc = RCAR_VIN_MC_IM_FULL;
		}
	}
	
	/* Input interface */
	if(p_soc->active_dev) {
		vnmc |= RCAR_VIN_MC_INF_YUV8_BT656;
		input_is_yuv = 1;
	} else {
		vnmc |= RCAR_VIN_MC_INF_RGB888;
	}
	
	/* Output format */
	switch(cam->dfmt)
	{
		case SCREEN_FORMAT_RGBA8888:
			dmr = RCAR_VIN_DMR_EXRGB | RCAR_VIN_DMR_DTMD_ARGB;
			img.bpp = 4;
			break;
		case SCREEN_FORMAT_RGB565:
			dmr = 0;
			img.bpp = 2;
			break;
		case SCREEN_FORMAT_UYVY:
			dmr = 0;
			output_is_yuv = 1;
			img.bpp = 2;
			break;
		case SCREEN_FORMAT_RGBA5551:
			dmr = RCAR_VIN_DMR_DTMD_ARGB;
			img.bpp = 2;
			break;
	}
	
	/* Update on field change */
	vnmc |= RCAR_VIN_MC_VUP;
	
	/* If input and output use the same colorspace, use bypass mode */ 
	if (input_is_yuv == output_is_yuv) {
		vnmc |= RCAR_VIN_MC_BPS;
	}
	
	/* Capture from CSI2 */
	vnmc &= ~RCAR_VIN_MC_DPINE;
	
	/* Select CSI virtual channel to capture */
	if(p_soc->active_dev) {
		ifmd = RCAR_VIN_CSI_IFMD_CSI_CHSEL(1);
		ifmd |= (RCAR_VIN_CSI_IFMD_DES1 | RCAR_VIN_CSI_IFMD_DES0);
	} else {
		ifmd = RCAR_VIN_CSI_IFMD_CSI_CHSEL(0);
	}
	
	/* Bus param */
	dmr2 = RCAR_VIN_DMR2_FTEV;
	if(p_soc->active_dev == 0)
		dmr2 |= (RCAR_VIN_DMR2_VPS | RCAR_VIN_DMR2_HPS);
	
	/* Maximum supported capture buffer */
	vin->nbufs = vin->frm_nbufs;
	if(vin->frm_nbufs >= RCAR_VIN_MAX_BUFFER)
		vin->nbufs = RCAR_VIN_MAX_BUFFER;
	
	//Mmap middle buffer
	mem_size = cam->dh * (cam->dw * img.bpp);
	for(i = 0; i < vin->nbufs; i++) { 
		vin->buf.addr[i] = (uintptr_t)mmap(0, mem_size, PROT_READ | PROT_WRITE | PROT_NOCACHE,
														MAP_ANON | MAP_PHYS | MAP_PRIVATE, NOFD, 0);
		memset((void*)vin->buf.addr[i], 0, mem_size);
		out32(vin->vbase + RCAR_VIN_MB(i), rcar_vin_mphys((void *)vin->buf.addr[i]));
	}
	
	/* Interrupt type */
	interrupt = cam->interlace ? RCAR_VIN_IE_EFE : RCAR_VIN_IE_FIE;
	
	/* Apply setup */
	out32(vin->vbase + RCAR_VIN_CSI_IFMD, ifmd);
	out32(vin->vbase + RCAR_VIN_INTS, interrupt);
	out32(vin->vbase + RCAR_VIN_DMR, dmr);
	out32(vin->vbase + RCAR_VIN_DMR2, dmr2);
	out32(vin->vbase + RCAR_VIN_MC, vnmc | RCAR_VIN_MC_ME);
	
	/* Crop and scale */
	rcar_vin_crop(vin);
	rcar_vin_scale(vin);
	
	/* Wait for interrupt */
	if(rcar_vin_create_thread(context)) {
		fprintf(stderr, "%s: create interrupt handler failed\n", __FUNCTION__);
	}
	
	/* IMRLX4 init */
	img.hcoid = vin->coid;
	img.pulse = RCAR_VIN_IMR_PULSE;
	img.channel = 0;
	img.cx = cam->cx;
	img.cy = cam->cy;
	img.dw = cam->dw;
	img.dh = cam->dh;
	rcar_imrlx4_init(img);

	/* Start */
	out32(vin->vbase + RCAR_VIN_IE, interrupt);
	out32(vin->vbase + RCAR_VIN_FC, RCAR_VIN_FC_C_FRAME); 
}

int rcar_vin_update(capture_context_t context)
{
	rcar_context_t *p_soc = (rcar_context_t *)context;
	rcar_vin_t *vin = &p_soc->vin;
	cam_info_t* cam = &vin->cam;
	
	if(cam->update & SOC_SCALING_UPDATE) {
		rcar_vin_scale(vin);
		cam->update &= ~SOC_SCALING_UPDATE;
	}
	
	if(cam->update & SOC_CROPPING_UPDATE) {
		rcar_vin_crop(vin);
		cam->update &= ~SOC_CROPPING_UPDATE;
	}

	return 0;
}

int rcar_vin_fini(capture_context_t context)
{
	int i = 0, loop;
	rcar_context_t *p_soc = (rcar_context_t *)context;
	rcar_vin_t *vin = &p_soc->vin;
	rcar_csi2_t *csi = &p_soc->csi;
	
	if(p_soc->is_runing == 0) {
		return 0;
	}
	
	/* Disable HDMI audio */
	if(p_soc->active_dev == 0) {
		audio_stop();
		audio_deinit();
	}
	
	/* Stop VIN */
	out32(vin->vbase + RCAR_VIN_FC, 0); 
	out32(vin->vbase + RCAR_VIN_IE, 0); 
	out32(vin->vbase + RCAR_VIN_MC, 0);
	
	loop = 100;
	while (1) {
		--loop;
		if (!loop || !(vin->frm_end & 1))
		break;
		usleep(1000);
    }
	
	for(i = 0; i < vin->nbufs; i++) {
		out32(vin->vbase + RCAR_VIN_MB(i), 0);	
	}
	
	MsgSendPulse(vin->coid, 21, RCAR_VIN_END, 0);
	usleep(10);
	
	pthread_cancel(vin->tid);
	pthread_join(vin->tid, NULL);
	InterruptDetach(vin->iid);
	ConnectDetach(vin->coid);
	ChannelDestroy(vin->chid);
	
	pthread_cond_destroy(&vin->cond);
	pthread_mutex_destroy(&vin->mutex);
	
	munmap_device_io(vin->vbase, RCAR_VIN_SIZE);
	
	//rcar_vin_disable_clock(context);
	
	/* Stop CSI2 */
	rcar_csi2_fini(p_soc->active_dev, csi);
	
	return 0;
}

int rcar_vin_init(capture_context_t context)
{
	pthread_condattr_t attr;
	
	rcar_context_t *p_soc = (rcar_context_t *)context;
	rcar_vin_t *vin = &p_soc->vin;
	
	/* Physical base address and interrupt number */
	switch(p_soc->active_dev) {
		case 0:
			vin->pbase = RCAR_VIN0_BASE;
			vin->irq = RCAR_INTCSYS_VIN0;
			break;
		case 1:
			vin->pbase = RCAR_VIN4_BASE;
			vin->irq = RCAR_INTCSYS_VIN4;
			break;
		default:
			fprintf(stderr, "%s: Not supported capture device\n", __FUNCTION__);
			return -1;
	}
	
	/* Map base address */
	if ((vin->vbase = (uintptr_t)mmap_device_io(RCAR_VIN_SIZE, vin->pbase)) == (uintptr_t)MAP_FAILED) {
        fprintf(stderr, "%s: VIN base mmap_device_io (0x%x) failed", __FUNCTION__, (uint32_t)vin->pbase);
        rcar_vin_fini(context);
        return (errno);
    }
	
	/* Signal */
	pthread_mutex_init(&vin->mutex, NULL);
	pthread_condattr_init(&attr);
    pthread_condattr_setclock(&attr, CLOCK_MONOTONIC);
	pthread_cond_init(&vin->cond, &attr);
	
	/* Enable HDMI audio */
	if(p_soc->active_dev == 0) {
		audio_setup(p_soc->screen_idx);
		audio_start();
	}
	
	/* Enable VIN */
	rcar_vin_setup(context);
	
	return 0;
}

capture_context_t capture_create_context(uint32_t flags)
{	
	ThreadCtl(_NTO_TCTL_IO, 0);
	
	pthread_mutex_init(&mutex, NULL);
	pthread_mutex_lock(&mutex);
	
	if((rcar_context = calloc(1, sizeof(rcar_context_t))) == NULL) {
		fprintf(stderr, "%s: calloc failed\n", __FUNCTION__);
		pthread_mutex_unlock(&mutex);
		return NULL;
	}
	
	rcar_context->active_dev = 0;
	rcar_context->is_runing = 0;
	rcar_context->enable = 0;

	pthread_mutex_unlock(&mutex);
	return rcar_context;
}

int capture_create_buffers(capture_context_t context, uint32_t property)
{
	errno = ENOTSUP;
	return -1;
}

void capture_destroy_context(capture_context_t context)
{
	rcar_context_t *p_soc = (rcar_context_t *)context;
	
	if(p_soc) {
		pthread_mutex_lock(&mutex);
		SOC_FINI(context);
		free(p_soc);
		rcar_imrlx4_fini();
		pthread_mutex_unlock(&mutex);
	}
	
	pthread_mutex_destroy(&mutex);
}

int capture_update(capture_context_t context, uint32_t flags)
{
	rcar_context_t *p_soc = (rcar_context_t *)context;
	
	pthread_mutex_lock(&mutex);
	
	if((p_soc->enable) && (!p_soc->is_runing)) {
		if(SOC_INIT(context)) {
			pthread_mutex_unlock(&mutex);
			return -1;
		}
		p_soc->is_runing = 1;
	}
	else if((!p_soc->enable) && (p_soc->is_runing)) {
		if(SOC_FINI(context)) {
			pthread_mutex_unlock(&mutex);
			return -1;
		}
		p_soc->is_runing = 0;
		return 0;
	}
	
	if(SOC_UPDATE(context)) {
		pthread_mutex_unlock(&mutex);
		return -1;
	}
	
	pthread_mutex_unlock(&mutex);
	return 0;	
}

int capture_get_frame(capture_context_t context, uint64_t timeout, uint32_t flags)
{	
	int ret;
	struct timespec from;
	struct timespec to;
	uint64_t time_from;
	uint64_t time_to;
	
	rcar_context_t *p_soc = (rcar_context_t *)context;
	rcar_vin_t *vin = &p_soc->vin;
	
	pthread_mutex_lock(&mutex);
	
	if(p_soc->enable == 0) {
		pthread_mutex_unlock(&mutex);
		errno = ECANCELED;
		return -1;
	}
	
	clock_gettime(CLOCK_MONOTONIC, &from);
	time_from = timespec2nsec(&from);
	
	if(timeout == CAPTURE_TIMEOUT_INFINITE) {
		time_to = CAPTURE_TIMEOUT_INFINITE;
	}
	else {
		time_to = time_from + timeout;
	}
	nsec2timespec(&to, time_to);
		
	while(1) {
		ret = pthread_cond_timedwait(&vin->cond, &mutex, &to);
		if (ret == EOK) {
			break;
		}	
		else if(ret == ETIMEDOUT)
		{
			pthread_mutex_unlock(&mutex);
			errno = ETIMEDOUT;
			return -1;
		}
		else {
			pthread_mutex_unlock(&mutex);
			return -1;	
		}				
	}
			
	if(vin->buf.latest < vin->nbufs) {
		vin->buf.release = vin->buf.latest;
		pthread_mutex_unlock(&mutex);
		return vin->buf.release;
	}
	
	pthread_mutex_unlock(&mutex);
	return 0;
}

int capture_release_frame(capture_context_t context, uint32_t idx)
{	
	rcar_context_t *p_soc = (rcar_context_t *)context;
	rcar_vin_t *vin = &p_soc->vin;
	
	if(idx > vin->nbufs) {
		errno = EINVAL;
		return -1;
	}
	
	vin->buf.done[idx] = 1;

	return 0;
}

int capture_put_buffer(capture_context_t ctx, uint32_t idx, uint32_t flags)
{
	return 0;
}

int capture_is_property(capture_context_t context, uint32_t prop)
{
	switch(prop)
	{
		case CAPTURE_PROPERTY_DEVICE_INFO:
		case CAPTURE_PROPERTY_DEVICE:
		case CAPTURE_ENABLE:
		case CAPTURE_PROPERTY_SRC_WIDTH:
		case CAPTURE_PROPERTY_SRC_HEIGHT:
		case CAPTURE_PROPERTY_SRC_FORMAT:
		case CAPTURE_PROPERTY_NORM:
		case CAPTURE_PROPERTY_DST_WIDTH:
		case CAPTURE_PROPERTY_DST_HEIGHT:
		case CAPTURE_PROPERTY_DST_FORMAT:
		case CAPTURE_PROPERTY_DST_STRIDE:
		case CAPTURE_PROPERTY_CROP_X:
		case CAPTURE_PROPERTY_CROP_Y:
		case CAPTURE_PROPERTY_CROP_WIDTH:
		case CAPTURE_PROPERTY_CROP_HEIGHT:
		case CAPTURE_PROPERTY_FRAME_FLAGS:
		case CAPTURE_PROPERTY_FRAME_TIMESTAMP:
		case CAPTURE_PROPERTY_FRAME_NBUFFERS:
		case CAPTURE_PROPERTY_FRAME_BUFFERS:
		{
			return 1;
		}
	}
	return 0;
}

int capture_get_property_i(capture_context_t context, uint32_t prop, int32_t *value)
{
	rcar_context_t *p_soc = (rcar_context_t *)context;
	
	switch(prop)
	{
		case CAPTURE_PROPERTY_DEVICE:
			*value = p_soc->active_dev;
			break;
		case CAPTURE_ENABLE:
			*value = p_soc->enable;
			break;
		default:
			errno = ENOTSUP;
			return -1;
	}
	return 0;
}

int capture_get_property_p(capture_context_t context, uint32_t prop, void **value)
{
	switch(prop)
	{
		case CAPTURE_PROPERTY_DEVICE_INFO:
			strcpy((char*)(value), SOC_INFO);
			break;
		default:
			errno = ENOTSUP;
			return -1;
	}
	return 0;
}

int capture_set_property_i(capture_context_t context, uint32_t prop, int32_t value)
{
	rcar_context_t *p_soc = (rcar_context_t *)context;
	rcar_vin_t *vin = &p_soc->vin;
	cam_info_t *cam = &vin->cam;
	
	switch(prop)
	{
		case CAPTURE_PROPERTY_DEVICE:
			p_soc->active_dev = value & 0xF;
			p_soc->screen_idx = (value & 0xF0) >> 4;
			break;
		case CAPTURE_ENABLE:
			/* CSI2 init */
			if(value == 2) {
				rcar_csi2_t *csi = &p_soc->csi;
				csi->info = (csi2_info_t*)cam;
				rcar_csi2_init(p_soc->active_dev, csi);
			}
			else {
				p_soc->enable = value;
			}
			break;
		case CAPTURE_PROPERTY_SRC_WIDTH:
			cam->sw = value;
			break;
		case CAPTURE_PROPERTY_SRC_HEIGHT:
			cam->sh = value;
			break;
		case CAPTURE_PROPERTY_SRC_FORMAT:
			cam->interlace = value;
			break;
		case CAPTURE_PROPERTY_DST_WIDTH:
			cam->dw = value;
			cam->update |= SOC_SCALING_UPDATE;
			break;
		case CAPTURE_PROPERTY_DST_HEIGHT:
			cam->dh = value;
			cam->update |= SOC_SCALING_UPDATE;
			break;
		case CAPTURE_PROPERTY_DST_FORMAT:
			cam->dfmt = value;
			break;
		case CAPTURE_PROPERTY_DST_STRIDE:
			cam->dstride = value;
			break;
		case CAPTURE_PROPERTY_CROP_X:
			cam->cx = value;
			cam->update |= SOC_CROPPING_UPDATE;
			break;
		case CAPTURE_PROPERTY_CROP_Y:
			cam->cy = value;
			cam->update |= SOC_CROPPING_UPDATE;
			break;
		case CAPTURE_PROPERTY_CROP_WIDTH:
			cam->cw = value;
			cam->update |= SOC_CROPPING_UPDATE;
			break;
		case CAPTURE_PROPERTY_CROP_HEIGHT:
			cam->ch = value;
			cam->update |= SOC_CROPPING_UPDATE;
			break;
		case CAPTURE_PROPERTY_FRAME_NBUFFERS:
			vin->frm_nbufs = value;
			break;
		default:
			errno = ENOTSUP;
			return -1;
	}
	return 0;
}

int capture_set_property_p(capture_context_t context, uint32_t prop, void *value)
{	
	rcar_context_t *p_soc = (rcar_context_t *)context;
	rcar_vin_t *vin = &p_soc->vin;
	cam_info_t *cam = &vin->cam;
	
	switch(prop)
	{
		case CAPTURE_PROPERTY_NORM:
			strcpy(cam->sfmt, (char*)value);
			break;
		case CAPTURE_PROPERTY_FRAME_BUFFERS:
			vin->frm_bufs = (void**)value;
			break;
		case CAPTURE_PROPERTY_FRAME_TIMESTAMP:
			/* Not implemented yet */
			break;
		case CAPTURE_PROPERTY_FRAME_FLAGS:
			/* Not implemented yet */
			break;
		default:
			errno = ENOTSUP;
			return -1;
	}
	return 0;
}
