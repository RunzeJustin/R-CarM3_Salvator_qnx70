/*
 * $QNXLicenseC:
 * Copyright 2014-2015, QNX Software Systems. 
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
 

#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <screen/screen.h>
#include <screen/iomsg.h>

#include "rcar_display.h"


/* 64 bytes granularity for DC */
#define DC_WIDTH_GRANULARITY 16

/* 8 pixels granularity for SGX */
#define SGX_WIDTH_GRANULARITY 16

/* 1 pixels granularity for offscreen CPU accesible */
#define CPU_WIDTH_GRANULARITY 1

#define ALIGNMENT 4

WFD_API_CALL WFDErrorCode WFD_APIENTRY
wfdCreateWFDEGLImagesQNX(WFDDevice device, WFDint width, WFDint height,
    WFDint format, WFDint usage, WFDint count, WFDEGLImage *images) WFD_APIEXIT
{
    const int client_mapping_flags = WFD_USAGE_OPENGL_ES1_QNX |
                                     WFD_USAGE_OPENGL_ES2_QNX |
                                     WFD_USAGE_OPENVG_QNX     |
                                     WFD_USAGE_NATIVE_QNX;

    du_dev_t* dev = (du_dev_t*)device;
    char          name[32];
    int           i, rc;

    TRACE;

    DEVICE_VALIDATE(return WFD_ERROR_BAD_DEVICE)
	
    SLOG_DEBUG("wfdCreateWFDEGLImagesQNX(): width=%d, height=%d, format=%08X, usage=%08X, count=%d", width, height, format, usage, count);

    if (width <= 0 || height <= 0 || count <= 0 || !images)
    {
    	SLOG_ERROR("invalid width, height, count, or images argument");
        return WFD_ERROR_ILLEGAL_ARGUMENT;
    }

    if (usage & client_mapping_flags)
    {
        /* GPU stack requires client vaddr. */
        usage |= (WFD_USAGE_READ_QNX | WFD_USAGE_WRITE_QNX);
    }

    int bpp;

    switch (format) {
        case WFD_FORMAT_YUV420_QNX:
        case WFD_FORMAT_NV12_QNX:
        case WFD_FORMAT_YV12_QNX:
            bpp = 1;
            break;
        case WFD_FORMAT_RGBA4444_QNX:
        case WFD_FORMAT_RGBX4444_QNX:
        case WFD_FORMAT_RGBA5551_QNX:
        case WFD_FORMAT_RGBX5551_QNX:
        case WFD_FORMAT_RGB565_QNX:
        case WFD_FORMAT_YUY2_QNX:
        case WFD_FORMAT_UYVY_QNX:
        case WFD_FORMAT_YVYU_QNX:
            bpp = 2;
            break;
        case WFD_FORMAT_RGB888_QNX:
            bpp = 3;
            break;
        case WFD_FORMAT_RGBA8888_QNX:
        case WFD_FORMAT_RGBX8888_QNX:
            bpp = 4;
            break;
        default:
        	SLOG_DEBUG("Image format not supported:%d",format);
            return WFD_ERROR_NOT_SUPPORTED;
    }

    int granularity = CPU_WIDTH_GRANULARITY /* SGX_WIDTH_GRANULARITY */;

    if (format == WFD_FORMAT_YUV420_QNX || format == WFD_FORMAT_YV12_QNX) {
        granularity *= 2;
    }
 
    int strides[2];
    strides[0] = (width * bpp + (granularity-1)) & ~(granularity-1);

    if (usage & WFD_USAGE_ROTATION_QNX) {
        strides[1] = (height * bpp + (granularity-1)) & ~(granularity-1);
    } else {
        strides[1] = 0;
    }

    int size;
    if (format != WFD_FORMAT_YUV420_QNX && format != WFD_FORMAT_NV12_QNX && format != WFD_FORMAT_YV12_QNX) {
        if (usage & WFD_USAGE_ROTATION_QNX) {
            size = max(height * strides[0], width * strides[1]);
        } else {
            size = height * strides[0];
        }
    } else {
        size = (height + (height+1)/2) * strides[0];
    }

    for (i = 0; i < count; i++)
    {
        win_image_t* img = calloc(1, sizeof(*img));
        int pt_fd = -1;

        if (!img)
        {
            SLOG_ERROR("could not allocate native image");
            break;
        }

        img->width = width;
        img->height = height;
        img->format = format;
        img->usage = usage;
        img->size = size;
        img->strides[0] = strides[0];
        img->strides[1] = strides[1];
		
        if (format == WFD_FORMAT_YUV420_QNX ||
                format == WFD_FORMAT_NV12_QNX ||
                format == WFD_FORMAT_YV12_QNX) 
		{
            img->planar_offsets[1] = strides[0] * height;
            if (format == WFD_FORMAT_YUV420_QNX || format == WFD_FORMAT_YV12_QNX) {
                img->planar_offsets[2] = strides[0] * ((height * 5 + 1) / 4);
            }
        }

        /* The following calculation is to make image size be an even multiple of the page size */
        img->size = (img->size + sysconf( _SC_PAGE_SIZE ) - 1) & ~(sysconf( _SC_PAGE_SIZE ) - 1);
        img->flags = WIN_IMAGE_FLAG_PHYS_CONTIG;

        pt_fd = posix_typed_mem_open(RCARDU_POSIX_TYPED_MEM_PATH, O_RDWR, POSIX_TYPED_MEM_ALLOCATE_CONTIG);
        if (pt_fd == -1) {
            SLOG_ERROR("return: WFD_ERROR_OUT_OF_MEMORY (Can't open posix typed memory)");
            free(img);
            break;
        }

        img->vaddr = mmap64(0, img->size, PROT_READ|PROT_WRITE|PROT_NOCACHE, MAP_SHARED | MAP_NOINIT, pt_fd, 0);
        if (img->vaddr == MAP_FAILED) {
            SLOG_ERROR("return: WFD_ERROR_OUT_OF_MEMORY (Can't mmap posix typed memory)");
            close(pt_fd);
            free(img);
            break;
        }
        close(pt_fd);

        if (img->flags & WIN_IMAGE_FLAG_PHYS_CONTIG)
        {
            img->paddr = disp_phys_addr(img->vaddr);
            SLOG_DEBUG2("    img->paddr=%08llX", img->paddr);
        }

        /* create a shared memory object which will allow us to control cache options */
        snprintf(name, sizeof(name), "%d:%d", getpid(), i);
        img->fd = shm_open(name, O_RDWR|O_CREAT, 0777);
        if (img->fd == -1) {
            SLOG_ERROR("return: WFD_ERROR_OUT_OF_MEMORY (Can't create shared memory)");
            munmap(img->vaddr, img->size);
            free(img);
            break;
        }

        rc = shm_unlink(name);
        
        if ((rc = shm_ctl(img->fd, SHMCTL_PHYS, img->paddr, img->size))) {
            SLOG_ERROR("return: WFD_ERROR_OUT_OF_MEMORY (Can't set special flags on posix typed memory)");
            close(img->fd);
            munmap(img->vaddr, img->size);
            shm_unlink(name);
            free(img);
            break;
        }
        SLOG_DEBUG2("    img->vaddr=%08X", (unsigned int)img->vaddr);

        img->usage |= WFD_USAGE_DISPLAY_QNX;
        img->flags |= (1 << 15); /* equivalent to WIN_IMAGE_FLAG_UNCACHED_‎MAPPING */
        images[i] = img;
    }

    if (i < count)
    {
        for (--i; i >= 0; i--)
        {
            win_image_t* img = images[i];
            munmap(img->vaddr, img->size);
            close(img->fd);
            free(img);
            images[i] = NULL;
        }

        return WFD_ERROR_OUT_OF_MEMORY;
    }

    return WFD_ERROR_NONE;
}

WFD_API_CALL WFDErrorCode WFD_APIENTRY wfdDestroyWFDEGLImagesQNX(WFDDevice device,
    WFDint count, WFDEGLImage *images) WFD_APIEXIT
{
    du_dev_t* dev = (du_dev_t *)device;
    int           i;

    TRACE;

    DEVICE_VALIDATE(return WFD_ERROR_BAD_DEVICE)

    if (count <= 0 || !images)
    {
        SLOG_ERROR("invalid images argument");
        return WFD_ERROR_ILLEGAL_ARGUMENT;
    }

    for (i = count-1; i >= 0; i--)
    {
        win_image_t* img = images[i];

        SLOG_DEBUG("wfdDestroyWFDEGLImagesQNX(): vaddr=%08X", (unsigned int)img->vaddr);
        munmap(img->vaddr, img->size);
        close(img->fd);
        free(img);
    }

    return WFD_ERROR_NONE;
}

WFD_API_CALL WFDSource WFD_APIENTRY wfdCreateSourceFromImage(WFDDevice device, WFDPipeline pipeline,
    WFDEGLImage image, const WFDint *attribList) WFD_APIEXIT
{
    du_dev_t* dev=(du_dev_t*)device;
    pipe_t*       pipe=(pipe_t*)pipeline;
    source_t*     src;
    win_image_t*  img=(win_image_t*)image;
    int           i=0;

    TRACE;

    DEVICE_VALIDATE(return WFD_INVALID_HANDLE)
	
    for (i=0; i<dev->pipesSize; ++i)
    {
        if (pipe == &dev->pipes[i])
        {
            if (!pipe->created)
            {
                SLOG_ERROR("pipeline not created");
                LOG_ERROR(WFD_ERROR_BAD_HANDLE);
                return WFD_INVALID_HANDLE;
            }

            /* found */
            break;
        }
    }

    if (i==dev->pipesSize)
    {
        SLOG_ERROR("invalid pipeline");
        LOG_ERROR(WFD_ERROR_BAD_HANDLE);

        return WFD_INVALID_HANDLE;
    }

    if (!image)
    {
        SLOG_ERROR("invalid image");
        LOG_ERROR(WFD_ERROR_BAD_HANDLE);
        return WFD_INVALID_HANDLE;
    }

    if (img->width > dev->max_width || img->height > dev->max_height)
    {
        SLOG_ERROR("image width or height greater than max, Note: this is an error");
        LOG_ERROR(WFD_ERROR_INCONSISTENCY);
        return WFD_INVALID_HANDLE;
    }

    /* ensure format is supported by pipeline */
    if (pipeline != WFD_INVALID_HANDLE)
    {
		switch (img->format)
		{
			case SCREEN_FORMAT_RGB888:
			case SCREEN_FORMAT_RGBA8888:
			case SCREEN_FORMAT_RGBX8888:
			case SCREEN_FORMAT_RGB565:
			case SCREEN_FORMAT_RGBA5551:
			case SCREEN_FORMAT_YUY2:
			case SCREEN_FORMAT_NV12:
			case SCREEN_FORMAT_YUV420:
			case SCREEN_FORMAT_UYVY:
				 break;
			default:
			    SLOG_ERROR("invalid img format");
				return WFD_INVALID_HANDLE;
		}
    } else {
        SLOG_ERROR("invalid pipeline");
        return WFD_INVALID_HANDLE;
    }

    src = calloc(1, sizeof(*src));
    if (src == NULL)
    {
        SLOG_ERROR("calloc failed, out of memory?");
        LOG_ERROR(WFD_ERROR_OUT_OF_MEMORY);
        return WFD_INVALID_HANDLE;
    }

    src->hdr.magic = SOURCE_MAGIC;
    src->hdr.version = sizeof(*src);
    src->image = image;
    src->bpp = DISP_BYTES_PER_PIXEL(img->format);
    src->pixel_format = img->format; /* display format */
	
    SLOG_DEBUG("wfdCreateSourceFromImage(): img->vaddr=%08X, src->bpp=%d, src->pixel_format=%08X",
            (unsigned int)img->vaddr, src->bpp, src->pixel_format);

    return (WFDSource)src;
}

WFD_API_CALL WFDSource WFD_APIENTRY wfdCreateSourceFromStream(WFDDevice device, WFDPipeline pipeline,
    WFDNativeStreamType stream, const WFDint *attribList) WFD_APIEXIT
{
    TRACE;

    return WFD_INVALID_HANDLE;
}

WFD_API_CALL void WFD_APIENTRY wfdDestroySource(WFDDevice device, WFDSource source) WFD_APIEXIT
{
    du_dev_t* dev=(du_dev_t*)device;
    source_t*     src=(source_t*)source;
    TRACE;

    DEVICE_VALIDATE(return )
    SOURCE_VALIDATE(return)

    free(src);
}

WFD_API_CALL WFDMask WFD_APIENTRY wfdCreateMaskFromImage(WFDDevice device, WFDPipeline pipeline,
    WFDEGLImage image, const WFDint *attribList) WFD_APIEXIT
{
    TRACE;
    return WFD_INVALID_HANDLE;
}

WFD_API_CALL WFDMask WFD_APIENTRY wfdCreateMaskFromStream(WFDDevice device,
    WFDPipeline pipeline, WFDNativeStreamType stream, const WFDint *attribList) WFD_APIEXIT
{
    TRACE;
    return WFD_INVALID_HANDLE;
}

WFD_API_CALL void WFD_APIENTRY wfdDestroyMask(WFDDevice device, WFDMask mask) WFD_APIEXIT
{
    TRACE;
}

WFD_API_CALL WFDDestinationQNX WFD_APIENTRY wfdCreateDestinationFromImageQNX(WFDDevice device, WFDPort port, WFDEGLImage image, const WFDint *attribList) WFD_APIEXIT
{
    TRACE;
    return WFD_INVALID_HANDLE;
}

WFD_API_CALL WFDDestinationQNX WFD_APIENTRY  wfdCreateDestinationFromStreamQNX(WFDDevice device, WFDPort port, WFDNativeStreamType stream, const WFDint *attribList) WFD_APIEXIT
{
    TRACE;
    return WFD_INVALID_HANDLE;
}
WFD_API_CALL void WFD_APIENTRY  wfdDestroyDestinationQNX(WFDDevice device, WFDDestinationQNX destination) WFD_APIEXIT
{
	TRACE;
}
WFD_API_CALL WFDErrorCode WFD_APIENTRY wfdBindDestinationToPortQNX(WFDDevice device, WFDPort port, WFDDestinationQNX destination, WFDTransition transition) WFD_APIEXIT
{
    return WFD_ERROR_ILLEGAL_ARGUMENT;
}
