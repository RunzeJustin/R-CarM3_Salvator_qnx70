/*
 * $QNXLicenseC:
 * Copyright 2009, QNX Software Systems. 
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

#ifndef _CONF_H_
#define _CONF_H_

#include <stdio.h>

#include <KHR/khrplatform.h>

typedef enum {
    __khr_none,
    __khr_khronos,
    __khr_egl_display,
    __khr_wfd_device,
    __khr_winmgr,
    __khr_globals,
    __khr_display,
    __khr_class,
    __khr_program,
    __khr_mtouch,
    __khr_mtouch_filter
} __khr_sections_t;

typedef struct {
    int layer;

    enum {
        __KHR_NATIVE_FRAMEBUFFER,
        __KHR_GLES1_FRAMEBUFFER,
        __KHR_GLES2_FRAMEBUFFER,
        __KHR_VG_FRAMEBUFFER,
    } type;

    int format;
    int src_alpha;
    int chroma;
    int srckey;
} __khr_framebuffer_t;

KHRONOS_APICALL FILE * __khrOpenGraphicsConf(void);

KHRONOS_APICALL FILE * __khrOpenWinmgrConf(__khr_sections_t *psection);

KHRONOS_APICALL int __khrEnumerateDevices(int *ids, int count);

KHRONOS_APICALL int __khrEnumerateDisplays(int *ids, int count);

KHRONOS_APICALL int __khrGetConfigValue(const char *key, char *val, int size);

KHRONOS_APICALL int __khrGetDeviceConfigValue(int device_id, const char *key, char *val, int size);

KHRONOS_APICALL int __khrGetDisplayConfigValue(int display_id, const char *key, char *val, int size);

KHRONOS_APICALL void * __khrLoadLibraryString(char *dlls);

#endif /* _CONF_H_ */

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL$ $Rev$")
#endif
