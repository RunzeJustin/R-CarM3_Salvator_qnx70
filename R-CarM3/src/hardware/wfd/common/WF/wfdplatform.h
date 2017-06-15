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

#ifndef _WFDPLATFORM_H_
#define _WFDPLATFORM_H_

#include <KHR/khrplatform.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef WFD_API_CALL
#define WFD_API_CALL KHRONOS_APICALL
#endif
#ifndef WFD_APIENTRY
#define WFD_APIENTRY KHRONOS_APIENTRY
#endif
#ifndef WFD_APIEXIT
#define WFD_APIEXIT KHRONOS_APIATTRIBUTES
#endif

typedef enum
{ WFD_FALSE = KHRONOS_FALSE,
  WFD_TRUE  = KHRONOS_TRUE
} WFDboolean;

typedef khronos_uint8_t             WFDuint8;
typedef khronos_int32_t             WFDint;
typedef khronos_float_t             WFDfloat;
typedef khronos_uint32_t            WFDbitfield;
typedef khronos_uint32_t            WFDHandle;
typedef khronos_utime_nanoseconds_t WFDtime;

#define WFD_FOREVER                 (0xFFFFFFFFFFFFFFFFLL)

typedef void*  WFDEGLDisplay; /* An opaque handle to an EGLDisplay */
typedef void*  WFDEGLSync;    /* An opaque handle to an EGLSyncKHR */
typedef void*  WFDEGLImage;   /* An opaque handle to an EGLImage */
typedef void*  WFDNativeStreamType;

#define WFD_INVALID_SYNC            ((WFDEGLSync)0)

#ifdef __cplusplus
}
#endif

#endif /* _WFDPLATFORM_H_ */

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL$ $Rev$")
#endif
