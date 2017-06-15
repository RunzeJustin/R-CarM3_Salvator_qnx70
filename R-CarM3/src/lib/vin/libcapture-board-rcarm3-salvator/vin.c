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

#include "vin.h"

#define DECODER_LIB		"libcapture-decoder-adv7482.so"
#define SOC_LIB			"libcapture-soc-rcarm3.so"

rcar_context_t *rcar_context;
context_api_t decoder_api;
context_api_t soc_api;
pthread_mutex_t mutex;

int load_lib(void* hdl, const char* pathname, int mode)
{
    context_api_t *hdl_api = (context_api_t*)hdl;
    hdl_api->hdl = dlopen(pathname,mode);
    if(hdl_api->hdl == NULL ){
        fprintf(stderr, "%s: DLL Error %s\n",__FUNCTION__, dlerror());
        return -1;
    }
    hdl_api->capture_create_context = dlsym(hdl_api->hdl, "capture_create_context");
    hdl_api->capture_destroy_context = dlsym(hdl_api->hdl, "capture_destroy_context");
    hdl_api->capture_is_property = dlsym(hdl_api->hdl, "capture_is_property");
    hdl_api->capture_update = dlsym(hdl_api->hdl, "capture_update");
    hdl_api->capture_set_property_i = dlsym(hdl_api->hdl, "capture_set_property_i");
    hdl_api->capture_set_property_p = dlsym(hdl_api->hdl, "capture_set_property_p");
    hdl_api->capture_get_property_i = dlsym(hdl_api->hdl, "capture_get_property_i");
    hdl_api->capture_get_property_p = dlsym(hdl_api->hdl, "capture_get_property_p");
    hdl_api->capture_get_frame = dlsym(hdl_api->hdl, "capture_get_frame");
    hdl_api->capture_release_frame = dlsym(hdl_api->hdl, "capture_release_frame");
    hdl_api->capture_create_buffers = dlsym(hdl_api->hdl, "capture_create_buffers");
    hdl_api->capture_put_buffer = dlsym(hdl_api->hdl, "capture_put_buffer");
    if (hdl_api->capture_create_context == 0	||
        hdl_api->capture_destroy_context == 0	||
        hdl_api->capture_is_property == 0 		||
        hdl_api->capture_update == 0	 		||
        hdl_api->capture_set_property_i == 0	||
        hdl_api->capture_set_property_p == 0 	||
        hdl_api->capture_get_property_i == 0 	||
        hdl_api->capture_get_property_p == 0 	||
        hdl_api->capture_get_frame == 0 		||
        hdl_api->capture_release_frame == 0 	||	
        hdl_api->capture_create_buffers == 0 	||		
        hdl_api->capture_put_buffer == 0) {
        fprintf(stderr, "%s: Couldn't find all the necessary library %s functions\n", __FUNCTION__, pathname);
        return -1;
        dlclose(hdl_api->hdl);
    }
    return 0;
}

capture_context_t capture_create_context(uint32_t flags)
{
    int ret = 0;
    
    pthread_mutex_init(&mutex, NULL);
    ret = pthread_mutex_lock(&mutex);
    if(ret != EOK) {
        fprintf(stderr, "%s: pthread_mutex_lock failed", __FUNCTION__);
        pthread_mutex_unlock(&mutex);
        return NULL;
    }
    else {
        if ((load_lib((void*)&decoder_api, DECODER_LIB, 0) == -1) || 
            (load_lib((void*)&soc_api, SOC_LIB, 0) == -1 )) {
                pthread_mutex_unlock(&mutex);
                return NULL;
        }
    
        if((rcar_context = calloc(1, sizeof(*rcar_context))) == NULL) {
            fprintf(stderr, "%s: calloc failed\n", __FUNCTION__);
            return NULL;
        }
        
        rcar_context->decoder = decoder_api.capture_create_context(flags);
        rcar_context->soc = soc_api.capture_create_context(flags);
    
        rcar_context->n_devices = 2;
        rcar_context->active_dev = 0;
        rcar_context->source_idx = 0;
        rcar_context->enable = 0;
        rcar_context->is_runing = 0;

        pthread_mutex_unlock(&mutex);
        return rcar_context;
    }
    
    pthread_mutex_unlock(&mutex);
    return NULL;
}

int capture_create_buffers(capture_context_t context, uint32_t property)
{
    rcar_context_t *p_context = (rcar_context_t*)context;
    capture_context_t soc_context = (capture_context_t)p_context->soc;
    return soc_api.capture_create_buffers(soc_context, property);
}

void capture_destroy_context(capture_context_t context)
{
    rcar_context_t *p_context = (rcar_context_t*)context;
    capture_context_t decoder_context = (capture_context_t)p_context->decoder;
    capture_context_t soc_context = (capture_context_t)p_context->soc;
    
    if(p_context) {
        pthread_mutex_lock(&mutex);
        decoder_api.capture_destroy_context(decoder_context);
        soc_api.capture_destroy_context(soc_context);
        if(soc_api.hdl) {
            dlclose(soc_api.hdl);
            memset((void*)&soc_api, 0, sizeof(context_api_t));
        }
        if(decoder_api.hdl) {
            dlclose(decoder_api.hdl);
            memset((void*)&decoder_api, 0, sizeof(context_api_t));
        }
        free(p_context);
        pthread_mutex_unlock(&mutex);
    }
    
    pthread_mutex_destroy(&mutex);
}

int capture_update(capture_context_t context, uint32_t flags)
{
    int  srcw;
    int  srch;
    int  interlace;
    char *std;
    
    rcar_context_t *p_context = (rcar_context_t*)context;
    capture_context_t decoder_context = (capture_context_t)p_context->decoder;
    capture_context_t soc_context = (capture_context_t)p_context->soc;
    
    if(decoder_api.capture_update(decoder_context, flags))
        return -1;
        
    if((p_context->enable) && (!p_context->is_runing)) {
        
        /* Tell SoC about input video */
        if(decoder_api.capture_get_property_p(decoder_context, CAPTURE_PROPERTY_CURRENT_NORM, (void**)&std)) {
            fprintf(stderr, "%s: Get video standard failed\n", __FUNCTION__);
            return -1;
        }
        if(soc_api.capture_set_property_p(soc_context, CAPTURE_PROPERTY_NORM, (void*)std)) {
            fprintf(stderr, "%s: Set video standard failed\n", __FUNCTION__);
            return -1;
        }

        printf("%s\n", std);

        if(decoder_api.capture_get_property_i(decoder_context, CAPTURE_PROPERTY_SRC_WIDTH, &srcw)) {
            fprintf(stderr, "%s: Get source width size failed\n", __FUNCTION__);
            return -1;
        }
        if(soc_api.capture_set_property_i(soc_context, CAPTURE_PROPERTY_SRC_WIDTH, srcw)) {
            fprintf(stderr, "%s: Set source width size failed\n", __FUNCTION__);
            return -1;
        }
        if(decoder_api.capture_get_property_i(decoder_context, CAPTURE_PROPERTY_SRC_HEIGHT, &srch)) {
            fprintf(stderr, "%s: Get source height size failed\n", __FUNCTION__);
            return -1;
        }
        if(soc_api.capture_set_property_i(soc_context, CAPTURE_PROPERTY_SRC_HEIGHT, srch)) {
            fprintf(stderr, "%s: Set source height size failed\n", __FUNCTION__);
            return -1;
        }
        if(decoder_api.capture_get_property_i(decoder_context, CAPTURE_PROPERTY_SRC_FORMAT, &interlace)) {
            fprintf(stderr, "%s: Get source format failed\n", __FUNCTION__);
            return -1;
        }
        if(soc_api.capture_set_property_i(soc_context, CAPTURE_PROPERTY_SRC_FORMAT, interlace)) {
            fprintf(stderr, "%s: Set source format failed\n", __FUNCTION__);
            return -1;
        }
        
        /* Enable CSI2 */
        if(soc_api.capture_set_property_i(soc_context, CAPTURE_ENABLE, 2)) {
            return -1;
        }
    
        /* Power up decoder */
        if(decoder_api.capture_set_property_i(decoder_context, CAPTURE_ENABLE, 2)) {
            return -1;
        }
    }
    
    /* Enable VIN */
    if(soc_api.capture_update(soc_context, flags)) {
        return -1;
    }
    
    if((!p_context->enable) && (p_context->is_runing)) {
        p_context->is_runing = 0;
    }
    else if((p_context->enable) && (!p_context->is_runing)) {
        p_context->is_runing = 1;
    }
    
    return 0;	
}

int capture_get_frame(capture_context_t context, uint64_t timeout, uint32_t flags)
{
    rcar_context_t *p_context = (rcar_context_t *)context;
    capture_context_t soc_context = (capture_context_t)p_context->soc;
    return soc_api.capture_get_frame(soc_context, timeout, flags);
}

int capture_release_frame(capture_context_t context, uint32_t idx)
{
    rcar_context_t *p_context = (rcar_context_t*)context;
    capture_context_t soc_context = (capture_context_t)p_context->soc;
    return soc_api.capture_release_frame(soc_context, idx);
}

int capture_put_buffer(capture_context_t ctx, uint32_t idx, uint32_t flags)
{
    rcar_context_t *p_context = (rcar_context_t *)ctx;
    capture_context_t soc_context = (capture_context_t)p_context->soc;
    return soc_api.capture_put_buffer(soc_context, idx, flags);
}

int capture_is_property(capture_context_t context, uint32_t prop)
{
    rcar_context_t *p_context = (rcar_context_t*)context;
    capture_context_t decoder_context = (capture_context_t)p_context->decoder;
    capture_context_t soc_context = (capture_context_t)p_context->soc;

    if(decoder_api.capture_is_property(decoder_context, prop)) {
        return 1;
    } 
    else if(soc_api.capture_is_property(soc_context, prop)) {
        return 1;
    }

    return 0;
}

int capture_get_property_i(capture_context_t context, uint32_t prop, int32_t *value)
{
    int decoder_val = 0;
    int soc_val = 0;
    rcar_context_t *p_context = (rcar_context_t *)context;
    capture_context_t decoder_context = (capture_context_t)p_context->decoder;
    capture_context_t soc_context = (capture_context_t)p_context->soc;
    
    if(value == NULL) {
        errno = EINVAL;
        return -1;
    }
    
    switch(prop) {
        case CAPTURE_PROPERTY_NDEVICES:
            *value = p_context->n_devices;
            break;
        case CAPTURE_ENABLE:
            if((decoder_api.capture_get_property_i(decoder_context, prop, &decoder_val)) ||
                soc_api.capture_get_property_i(soc_context, prop, &soc_val)) {
                    errno = ENOTSUP;
                    return -1;
                }
            *value = decoder_val & soc_val;
            break;
        default: 
            if(decoder_api.capture_get_property_i(decoder_context, prop, &decoder_val)) {
                if(soc_api.capture_get_property_i(soc_context, prop, &soc_val)) {
                    errno = ENOTSUP;
                    return -1;
                }
                *value = soc_val;
                break;
            }
            *value = decoder_val;
            break;
    }
    return 0;
}

int capture_get_property_p(capture_context_t context, uint32_t prop, void **value)
{
    char decoder_info[64];
    char soc_info[64];
    rcar_context_t *p_context = (rcar_context_t *)context;
    capture_context_t decoder_context = (capture_context_t)p_context->decoder;
    capture_context_t soc_context = (capture_context_t)p_context->soc;
    
    if(value == NULL) {
        errno = EINVAL;
        return -1;
    }
    
    if(prop == CAPTURE_PROPERTY_DEVICE_INFO) {
        if(*value == NULL) {
            *value = calloc(1, 128);
        }
        if((decoder_api.capture_get_property_p(decoder_context, prop, (void**)&decoder_info)) ||
            (soc_api.capture_get_property_p(soc_context, prop, (void**)&soc_info))) {
                errno = ENOTSUP;
                return -1;
        }
        snprintf((char*)(*value), 127, "Capture driver for RCarM3 SalvatorX board.\n" 
                                       "Decoder: %s\nSoC: %s\n", decoder_info, soc_info);
        return 0;
    }
    else {
        if(decoder_api.capture_get_property_p(decoder_context, prop, value)) {
            if(soc_api.capture_get_property_p(soc_context, prop, value)) {
                errno = ENOTSUP;
                return -1;
            }
        }
        return 0;
    }
}

int capture_set_property_i( capture_context_t context, uint32_t prop, int32_t value)
{
    rcar_context_t *p_context = (rcar_context_t *)context;
    capture_context_t decoder_context = (capture_context_t)p_context->decoder;
    capture_context_t soc_context = (capture_context_t)p_context->soc;
    
    switch(prop){
    case CAPTURE_PROPERTY_DEVICE:
        if((value >= 0) && ((value & 0xF) < p_context->n_devices)) {
            if((soc_api.capture_set_property_i(soc_context, prop, value)) || 
               (decoder_api.capture_set_property_i(decoder_context, prop, value))) {
                   errno = ENOTSUP;
                   return -1;
            }
            p_context->active_dev = value & 0xF;
        } else {
            errno = EINVAL;
            return -1;
        }
        break;
    case CAPTURE_PROPERTY_SRC_INDEX:
        if(decoder_api.capture_set_property_i(decoder_context, prop, value)) {
            errno = ENOTSUP;
            return -1;
        }
        p_context->source_idx = value;
        break;
    case CAPTURE_ENABLE:
        if((soc_api.capture_set_property_i(soc_context, prop, value)) || 
           (decoder_api.capture_set_property_i(decoder_context, prop, value))) {
            errno = ENOTSUP;
            return -1;
        }
        p_context->enable = value;
        break;
    default: 
        if((decoder_api.capture_set_property_i(decoder_context, prop, value)) &&
           (soc_api.capture_set_property_i(soc_context, prop, value))) {
                errno = ENOTSUP;
                return -1;
        }
        break;
    }
    return 0;
}

int capture_set_property_p( capture_context_t context, uint32_t prop, void *value )
{
    rcar_context_t* p_context = (rcar_context_t *)context;
    capture_context_t decoder_context = (capture_context_t)p_context->decoder;
    capture_context_t soc_context = (capture_context_t)p_context->soc;
    
    if((decoder_api.capture_set_property_p(decoder_context, prop, value)) &&
       (soc_api.capture_set_property_p(soc_context, prop, value))) {
        errno = ENOTSUP;
        return -1;
    }
    return 0;
}
