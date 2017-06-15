/*
 * $QNXLicenseC:
 * Copyright 2011, QNX Software Systems. 
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

#ifndef __DEBUG_H_INCLUDED
#define __DEBUG_H_INCLUDED

#include <sys/slog.h>
#include <sys/slogcodes.h>
#include <pthread.h>

#define DEBUG_LVL
	int debug_lvl;\
	int displayhal_debug_lvl;
#define DEBUG_LVL_EXT
	extern int debug_lvl;\
	extern int displayhal_debug_lvl;

/* Enable DC debug level logging */
#define DC_LOGGING
#ifdef DC_LOGGING
#define DEBUG_LVL_INIT() \
    char value[8];\
    if(__khrGetDeviceConfigValue(deviceId, "debug_lvl", value, sizeof(value)) != EOK) {\
        debug_lvl = 0;\
    }\
    else {\
        debug_lvl = atoi(value); \
        if(debug_lvl < 0) { \
            debug_lvl = 1; \
        }\
    }\
    if(__khrGetDeviceConfigValue(deviceId, "displayhal_debug_lvl", value, sizeof(value)) != EOK) {\
        displayhal_debug_lvl = debug_lvl;\
    }\
    else {\
        displayhal_debug_lvl = atoi(value); \
        if(displayhal_debug_lvl < 0) { \
            displayhal_debug_lvl = 1; \
        }\
    }
#define DC_LOG(level, string, ...) \
    if(level <= debug_lvl) { \
        slogf(_SLOGC_GRAPHICS_DISPLAY, _SLOG_INFO, "wfd: %d %s - " string, pthread_self(), __FUNCTION__, ##__VA_ARGS__);\
    }
#else
#define DC_LOG(level, string, ...)
#define DEBUG_LVL_INIT()
#endif
  
/* Error logging macro */
#define DC_ERROR(error, ...) slogf(_SLOGC_GRAPHICS_DISPLAY, _SLOG_ERROR, "wfd ERROR: %d %s - " error, pthread_self(), __FUNCTION__, ##__VA_ARGS__)

/* Trace logging macro */
#define TRACE DC_LOG(5, "Trace start")
#define TRACE_EXIT DC_LOG(5, "Trace end")

/* Enable WFD API trace logging */
//#define TRACE_WFD_ENABLE
#if defined(TRACE_WFD_ENABLE)
#define TRACE_WFD slogf(_SLOGC_GRAPHICS_DISPLAY, _SLOG_ERROR, "%d %s - Trace start", pthread_self(), __FUNCTION__)
#define TRACE_WFD_EXIT slogf(_SLOGC_GRAPHICS_DISPLAY, _SLOG_ERROR, "%d %s - Trace end", pthread_self(), __FUNCTION__)
#else
#define TRACE_WFD
#define TRACE_WFD_EXIT
#endif

#endif /* __DEBUG_H_INCLUDED */
