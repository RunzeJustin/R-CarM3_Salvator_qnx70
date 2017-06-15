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
#include <sys/neutrino.h>
#include <hw/inout.h>
#include <sys/resmgr.h>
#include <sys/iofunc.h>
#include <sys/dispatch.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <inttypes.h>
#include <devctl.h>
#include "vin.h"
#include <arm/r-car.h>
#include <hw/i2c.h>

#define DECODER_INFO   	   "ADV7482"
#define DECODER_INIT		adv7482_init
#define DECODER_FINI		adv7482_fini
#define DECODER_UPDATE		adv7482_update

pthread_mutex_t mutex;

char* norm_none = CAPTURE_NORM_NONE;

capture_context_t capture_create_context(uint32_t flags)
{ 
	ThreadCtl(_NTO_TCTL_IO, 0);
	
	pthread_mutex_init(&mutex, NULL);
	pthread_mutex_lock(&mutex);

	if((rcar_context = calloc(1, sizeof(*rcar_context))) == NULL) {
		fprintf(stderr, "%s: calloc failed\n", __FUNCTION__);
		pthread_mutex_unlock(&mutex);
		return NULL;
	}
	
	rcar_context->enable = 0;
	rcar_context->is_runing = 0;
	rcar_context->source_idx = 0;
	rcar_context->active_dev = 0;
	rcar_context->video.update = 0;

	pthread_mutex_unlock(&mutex);
	return rcar_context;
}

int capture_create_buffers(capture_context_t context, uint32_t property)
{
	errno = ENOTSUP;
	return -1;
}

void capture_destroy_context( capture_context_t context )
{
	rcar_context_t *p_decoder = (rcar_context_t *)context;
	video_info_t* video = &p_decoder->video;
	int channel = p_decoder->active_dev;
	
	if(p_decoder) {
		pthread_mutex_lock(&mutex);
		p_decoder->is_runing = 0;
		
		DECODER_FINI(channel, video);
		free(p_decoder);
		pthread_mutex_unlock(&mutex);
	}
	
	pthread_mutex_destroy(&mutex);
}

int capture_update(capture_context_t context, uint32_t flags)
{ 
	rcar_context_t *p_decoder = (rcar_context_t *)context;
	video_info_t* video = &p_decoder->video;
	int channel = p_decoder->active_dev;
	
	pthread_mutex_lock(&mutex);
	
	if((p_decoder->enable) && (!p_decoder->is_runing)) {
		if(DECODER_INIT(channel, video)) {
			pthread_mutex_unlock(&mutex);
			return -1;
		}
		p_decoder->is_runing = 1;
	}
	else if((!p_decoder->enable) && (p_decoder->is_runing)) {
		if(DECODER_FINI(channel, video)) {
			pthread_mutex_unlock(&mutex);
			return -1;
		}
		p_decoder->is_runing = 0;
		return 0;
	}
	
	if(DECODER_UPDATE(channel, video)) {
		pthread_mutex_unlock(&mutex);
		return -1;
	}

	pthread_mutex_unlock(&mutex);
	return 0;	
}

int capture_get_frame( capture_context_t context, uint64_t timeout, uint32_t flags )
{
	errno = ENOTSUP;
	return -1;
}

int capture_release_frame( capture_context_t context, uint32_t idx )
{
	errno = ENOTSUP;
	return -1;
}

int capture_put_buffer( capture_context_t ctx, uint32_t idx, uint32_t flags )
{
	errno = ENOTSUP;
	return -1;
}

int capture_is_property(capture_context_t context, uint32_t prop)
{
	switch(prop)
	{
		case CAPTURE_ENABLE:
		case CAPTURE_PROPERTY_SRC_INDEX:
		case CAPTURE_PROPERTY_DEVICE:
		case CAPTURE_PROPERTY_CONTRAST:
		case CAPTURE_PROPERTY_BRIGHTNESS:
		case CAPTURE_PROPERTY_SATURATION:
		case CAPTURE_PROPERTY_HUE:
		case CAPTURE_PROPERTY_DEVICE_INFO:
		case CAPTURE_PROPERTY_CURRENT_NORM:
		case CAPTURE_PROPERTY_SRC_WIDTH:
		case CAPTURE_PROPERTY_SRC_HEIGHT:
		case CAPTURE_PROPERTY_SRC_FORMAT:
		{
			return 1;
		}
	}

	return 0;
}

int capture_get_property_i(capture_context_t context, uint32_t prop, int32_t* value)
{
	rcar_context_t *p_decoder = (rcar_context_t *)context;
	video_info_t* video = &p_decoder->video;
	
	switch(prop)
	{
		case CAPTURE_PROPERTY_DEVICE:
			*value = p_decoder->active_dev;
			break;
		case CAPTURE_PROPERTY_SRC_INDEX:
			*value = p_decoder->source_idx;
			break;
		case CAPTURE_ENABLE:
			*value = p_decoder->enable;
			break;
		case CAPTURE_PROPERTY_CONTRAST:
			*value = (int32_t)video->con;
			break;
		case CAPTURE_PROPERTY_HUE:
			*value = (int32_t)video->hue;
			break;
		case CAPTURE_PROPERTY_BRIGHTNESS:
			*value = (int32_t)video->bri;
			break;
		case CAPTURE_PROPERTY_SATURATION:
			*value = (int32_t)video->sat;
			break;
		case CAPTURE_PROPERTY_SRC_WIDTH:
			*value = video->width;
			break;
		case CAPTURE_PROPERTY_SRC_HEIGHT:
			*value = video->height;
			break;
		case CAPTURE_PROPERTY_SRC_FORMAT:
			*value = video->interlace;
			break;
		case CAPTURE_PROPERTY_FRAME_FLAGS:
			if(video->interlace) {
				*value |= CAPTURE_FRAME_FLAG_INTERLACED;
			}
			break;
		default:
			errno = ENOTSUP;
			return -1;
	}
	return 0;	
}

int capture_get_property_p(capture_context_t context, uint32_t prop, void **value)
{	
	int tmp;
	rcar_context_t *p_decoder = (rcar_context_t *)context;
	video_info_t *video = &p_decoder->video;
	
	switch(prop)
	{
		case CAPTURE_PROPERTY_CURRENT_NORM:
			tmp = adv7482_signal(p_decoder->active_dev);
			if(tmp == 0) {
				*value = (void*)norm_none;
			}
			else {
				*value = (void*)video->format;
			}
			break;
		case CAPTURE_PROPERTY_DEVICE_INFO:
			strcpy((char*)(value), DECODER_INFO);
			break;
		default:
			errno = ENOTSUP;
			return -1;
	}

	return 0;
}

int capture_set_property_i(capture_context_t context, uint32_t prop, int32_t value)
{
	rcar_context_t *p_decoder = (rcar_context_t *)context;
	video_info_t *video = &p_decoder->video;
	int channel = p_decoder->active_dev;
	
	switch(prop)
	{
		case CAPTURE_PROPERTY_DEVICE:
			p_decoder->active_dev = value & 0xF;
			break;
		case CAPTURE_PROPERTY_SRC_INDEX:
			p_decoder->source_idx = value;
			break;
		case CAPTURE_ENABLE:
			if(value == 2) {
				adv7482_powerup(channel);
			}
			else {
				p_decoder->enable = value;
			}
			break;
		case CAPTURE_PROPERTY_CONTRAST:
			video->con = value;
			video->update |= DECODER_COLOR_CON_UPDATE;
			break;
		case CAPTURE_PROPERTY_HUE:
			video->hue = value;
			video->update |= DECODER_COLOR_HUE_UPDATE;
			break;
		case CAPTURE_PROPERTY_BRIGHTNESS:
			video->bri = value;
			video->update |= DECODER_COLOR_BRI_UPDATE;
			break;
		case CAPTURE_PROPERTY_SATURATION:
			video->sat = value;
			video->update |= DECODER_COLOR_SAT_UPDATE;
			break;
		default:
			errno = ENOTSUP;
			return -1;
	}
	
	return 0;
}

int capture_set_property_p(capture_context_t context, uint32_t prop, void *value)
{
	errno = ENOTSUP;
	return -1;
}
