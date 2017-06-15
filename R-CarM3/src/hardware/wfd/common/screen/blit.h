/*
 * $QNXLicenseC:
 * Copyright 2010, QNX Software Systems. 
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

#ifndef _SCREEN_BLIT_H_INCLUDED
#define _SCREEN_BLIT_H_INCLUDED

#include <screen/iomsg.h>
#include <EGL/egl.h>

typedef void * win_handle_t;

struct win_blit_ctx;
typedef struct win_blit_ctx* win_blit_ctx_t;

typedef struct {
	int x1, y1, x2, y2;
} win_rect_t;

typedef struct {
	int sx, sy, sw, sh;
	int dx, dy, dw, dh;
	int transp;
	int premult_alpha;
	int global_alpha;
	int quality;
	int rotation;
	int flip;
	int mirror;
} win_blit_t;

typedef struct win_blit_module {
	/* module information */
	const char *name;
	void       *handle;
	int         loaded;

	/* interface */
	struct {
		int (*ctx_init)(win_blit_ctx_t *ctx);

		void (*ctx_fini)(win_blit_ctx_t ctx);

		win_handle_t (*alloc)(win_blit_ctx_t ctx, win_image_t *img);

		void (*free)(win_blit_ctx_t ctx, win_handle_t handle);

		int (*fill)(win_blit_ctx_t ctx, win_handle_t dst, const win_rect_t *rect, unsigned int color, unsigned char alpha);

		int (*blit)(win_blit_ctx_t ctx, win_handle_t src, win_handle_t dst, const win_blit_t *args);

		void (*flush)(win_blit_ctx_t ctx);

		void (*finish)(win_blit_ctx_t ctx);

		int (*prefx)(win_blit_ctx_t ctx, EGLSurface surf, const win_image_t *img);
	} funcs;
} win_blit_module_t;

#endif /* _SCREEN_BLIT_H_INCLUDED */
