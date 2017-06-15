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

#include <sys/trace.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <atomic.h>
#include <hw/sysinfo.h>
#include <KHR/khronos_utils.h>
#include <confname.h>
#include <sys/conf.h>

#include "rcar_display.h"

/* Create a list of WFD extensions.
 *  WFD_QNX_egl_images
 *    - required by screen
 *    - indicates we provide these functions:
 *      wfdCreateWFDEGLImagesQNX, wfdDestroyWFDEGLImagesQNX
 *  WFD_QNX_vsync
 *    - indicates we provide the wfdWaitForVSyncQNX function
 *  WFD_QNX_port_mode_info
 *    - indicates we provide these portmode attributes:
 *      WFD_PORT_MODE_ASPECT_RATIO_QNX, WFD_PORT_MODE_PREFERRED_QNX
 *  WFD_QNX_bchs_filter
 *    - indicates we provide these pipeline attributes:
 *      WFD_PIPELINE_BRIGHTNESS_QNX, WFD_PIPELINE_CONTRAST_QNX,
 *      WFD_PIPELINE_HUE_QNX, WFD_PIPELINE_SATURATION_QNX
 *  WFD_QNX_pipeline_color_space / WFD_QNX_color_space
 *    - indicates we provide this pipeline attribute:
 *      WFD_PIPELINE_COLOR_SPACE_QNX
 *  WFD_QNX_port_brightness
 *    - indicates we provide these port attributes:
 *      WFD_PORT_CURRENT_BRIGHTNESS_QNX, WFD_PORT_BRIGHTNESS_QNX
 *  WFD_QNX_port_gamma_curve
 *    - indicates we provide these port attributes:
 *      WFD_PORT_RED_GAMMA_CURVE_QNX, WFD_PORT_GREEN_GAMMA_CURVE_QNX,
 *      WFD_PORT_BLUE_GAMMA_CURVE_QNX
 */

#undef RCARDU_EXT

#if WFD_QNX_egl_images
    #define RCARDU_EXT_IMG RCARDU_EXT("WFD_QNX_egl_images")
#else
    #define RCARDU_EXT_IMG
#endif

#if WFD_QNX_vsync
    #define RCARDU_EXT_VSYNC RCARDU_EXT("WFD_QNX_vsync")
#else
    #define RCARDU_EXT_VSYNC
#endif

#if WFD_QNX_port_mode_info
    #define RCARDU_EXT_MINFO RCARDU_EXT("WFD_QNX_port_mode_info")
#else
    #define RCARDU_EXT_MINFO
#endif

#if WFD_QNX_bchs_filter
    #define RCARDU_EXT_BCHS RCARDU_EXT("WFD_QNX_bchs_filter")
#else
    #define RCARDU_EXT_BCHS
#endif

#if WFD_QNX_pipeline_color_space
    #define RCARDU_EXT_PCOLORSPACE RCARDU_EXT("WFD_QNX_pipeline_color_space")
#else
    #define RCARDU_EXT_PCOLORSPACE
#endif

/* Define this for compatibility reason with 6.6.0 screen */
#if WFD_QNX_color_space
    #define RCARDU_EXT_COLORSPACE RCARDU_EXT("WFD_QNX_color_space")
#else
    #define RCARDU_EXT_COLORSPACE
#endif

#if WFD_QNX_port_brightness
    #define RCARDU_EXT_PBRIGHTNESS RCARDU_EXT("WFD_QNX_port_brightness")
#else
    #define RCARDU_EXT_PBRIGHTNESS
#endif

#if WFD_QNX_port_gamma_curve
    #define RCARDU_EXT_GAMMA_CURVE RCARDU_EXT("WFD_QNX_port_gamma_curve")
#else
    #define RCARDU_EXT_GAMMA_CURVE
#endif

#define RCARDU_EXT_LIST RCARDU_EXT_IMG RCARDU_EXT_VSYNC RCARDU_EXT_MINFO RCARDU_EXT_BCHS \
    RCARDU_EXT_PCOLORSPACE RCARDU_EXT_COLORSPACE RCARDU_EXT_PBRIGHTNESS RCARDU_EXT_GAMMA_CURVE

#define RCARDU_EXT(x) { .name=(x) },
static const struct
{
    char name[32];
} rcardu_extension_list[]={RCARDU_EXT_LIST};
#undef RCARDU_EXT

WFD_API_CALL WFDint WFD_APIENTRY
wfdGetStrings(WFDDevice device, WFDStringID name,
    const char **strings, WFDint stringsCount) WFD_APIEXIT
{
    du_dev_t* dev = (du_dev_t*)device;
    int ret = 0;

    TRACE;
    SLOG_DEBUG2("       device=%08X, name=%08X(%s), strings=%p, stringsCount=%d", device, name,
        name == WFD_VENDOR ? "WFD_VENDOR" : (name == WFD_RENDERER ? "WFD_RENDERER" :
        (name == WFD_VERSION ? "WFD_VERSION" : (name == WFD_EXTENSIONS ? "WFD_EXTENSIONS" : "UNKNOWN"))),
        strings, stringsCount);

    DEVICE_VALIDATE(return 0)

    if ((strings) && (stringsCount < 0)) {
        dev->error = WFD_ERROR_ILLEGAL_ARGUMENT;
        SLOG_DEBUG2("       return: 0 (WFD_ERROR_ILLEGAL_ARGUMENT)");
        return 0;
    }

    switch (name)
    {
        case WFD_VENDOR:
            if (!strings) {
                ret=1;
            } else {
                if (stringsCount >= 1) {
                    *strings = "QNX Software Systems";
                    ret=1;
                    SLOG_DEBUG2("       set string[%d]: %s", 0, *strings);
                }
            }
        case WFD_RENDERER:
            if (!strings) {
                ret=1;
            } else {
                if (stringsCount >= 1) {
                    if (dev->chip_type == RCAR_PRODUCT_H3) {
                        *strings = "R-CarH3";
                        ret=1;
                    } else if (dev->chip_type == RCAR_PRODUCT_M3W) {
                        *strings = "R-CarM3-W";
                        ret=1;
                    } else {
                        *strings = "Unknown";
                        ret=1;
                    }
                    SLOG_DEBUG2("       set string[%d]: %s", 0, *strings);
                }
            }
            break;
        case WFD_VERSION:
            if (!strings) {
                ret=1;
            } else {
                if (stringsCount >= 1) {
                    *strings = "1.0";
                    SLOG_DEBUG2("       set string[%d]: %s", 0, *strings);
                    ret=1;
                }
            }
            break;
        case WFD_EXTENSIONS:
            if (!strings) {
                ret = (WFDint)sizeof(rcardu_extension_list)/sizeof(rcardu_extension_list[0]);
            } else {
                for (ret = 0; ret < (WFDint)(sizeof(rcardu_extension_list) / sizeof(rcardu_extension_list[0])) && ret < stringsCount; ret++) {
                    strings[ret]=&rcardu_extension_list[ret].name[0];
                    SLOG_DEBUG2("       set string[%d]: %s", ret, strings[ret]);
                }
            }
            break;
        default:
            LOG_ERROR(WFD_ERROR_ILLEGAL_ARGUMENT);
            SLOG_DEBUG2("       return: 0 (WFD_ERROR_ILLEGAL_ARGUMENT)");
            return 0;
    }
    SLOG_DEBUG2("       return: %d", ret);
    return ret;
}

#define BILLION  1000000000L
struct timespec start, stop;
double accum;
	
int rcardu_wait_vsync(du_dev_t *dev, port_t *port)
{
    struct _pulse       pulse;
    iov_t               iov;
    uint64_t            MaxWaitTime =50*1000*1000;

	atomic_clr( &port->vsync_done, 0x01 );
    pthread_mutex_lock( &port->vsync_mutex );
    if (port->vsync_done & 1) 
    {
        pthread_mutex_unlock( &port->vsync_mutex );
        usleep(2);
        return 1;
    }
	
	atomic_add(&port->want_vsync_pulse, 1);

	SETIOV(&iov, &pulse, sizeof (pulse));

	while (1)
	{
		TimerTimeout(CLOCK_REALTIME, _NTO_TIMEOUT_RECEIVE, NULL, &MaxWaitTime, NULL);

		if (MsgReceivev(port->irqchan, &iov, 1, NULL) == -1)
			break;

		if (pulse.code == R_CAR_DU_VSYNC_PULSE)
			break;
	}

	atomic_set( &port->vsync_done, 0x01 );
	pthread_mutex_unlock( &port->vsync_mutex );
	usleep (1);
		
    return 1;
}

WFD_API_CALL WFDboolean WFD_APIENTRY
wfdIsExtensionSupported(WFDDevice device, const char *string) WFD_APIEXIT
{
    du_dev_t* dev = (du_dev_t*)device;
    size_t i;
    TRACE;
    SLOG_DEBUG2("       device=%08X, string=\"%s\"", device, string != NULL ? string : "<null>");

    DEVICE_VALIDATE(return WFD_FALSE)

    if (string == NULL) {
        dev->error = WFD_ERROR_ILLEGAL_ARGUMENT;
        SLOG_DEBUG2("       return: WFD_FALSE (WFD_ERROR_ILLEGAL_ARGUMENT)");
        return 0;
    }

    for (i=0; i<sizeof(rcardu_extension_list)/sizeof(rcardu_extension_list[0]); i++) {
        if (strcmp(string, &rcardu_extension_list[i].name[0])==0) {
            SLOG_DEBUG2("       return: WFD_TRUE");
            return WFD_TRUE;
        }
    }

    SLOG_DEBUG2("       return: WFD_FALSE");
    return WFD_FALSE;
}

WFD_API_CALL WFDint WFD_APIENTRY
wfdEnumerateDevices(WFDint *deviceIds, WFDint deviceIdsCount, const WFDint *filterList) WFD_APIEXIT
{
    TRACE;

    if (deviceIds && deviceIdsCount <= 0)
    {
        return 0;
    }

    if (deviceIds)
    {
        *deviceIds = 1;
    }

    return 1;
}

WFD_API_CALL WFDDevice WFD_APIENTRY
wfdCreateDevice(WFDint deviceId, const WFDint *attribList) WFD_APIEXIT
{
    du_dev_t* dev;
    int it,pipe_order=0;
    TRACE;
    SLOG_DEBUG2("       deviceId=%d, attribList=%p", deviceId, attribList);

    if (deviceId!=1)
    {
        SLOG_DEBUG2("       return: WFD_INVALID_HANDLE (wrong device id)");
        return WFD_INVALID_HANDLE;
    }

    dev = calloc(1, sizeof(*dev));
    if (dev == NULL)
    {
        SLOG_DEBUG2("       return: WFD_INVALID_HANDLE (can't allocate device structure)");
        return WFD_INVALID_HANDLE;
    }

    /* wfdCreateDevice supports attribList set to WFD_NONE or NULL only */
    if (attribList) {
        if (*attribList != WFD_NONE) {
            SLOG_DEBUG2("       return: WFD_INVALID_HANDLE (unsupported attributes)");
            free(dev);
            return WFD_INVALID_HANDLE;
        }
    }

    dev->hdr.magic = DEVICE_MAGIC;
    dev->hdr.version = sizeof(*dev);
    if (pthread_mutex_init(&dev->mutex, NULL) != 0) {
        SLOG_DEBUG2("       return: WFD_INVALID_HANDLE (can't initialize mutex)");
        free(dev);
        return WFD_INVALID_HANDLE;
    }

    if (wfdcfg_device_create(&dev->cfglib_device, deviceId, NULL) != EOK) {
        SLOG_DEBUG2("       return: WFD_INVALID_HANDLE (can't create wfdcfg device)");
        pthread_mutex_destroy(&dev->mutex);
        free(dev->cfglib_device);
        free(dev);
        return WFD_INVALID_HANDLE;
    }

    dev->max_width  = 0xfff;
    dev->max_height = 0xfff;
    get_chip_info(dev);
    dev->eventSize = RCARDU_MAX_NUMBER_EVENTS;
    SLOG_DEBUG2("    Parsing device config ...");
    dev->portsSize = parse_device_config(dev);
	if (!dev->portsSize)
	{
		slogf(99, 1, "Unable to initialize ports\n");
		return WFD_INVALID_HANDLE;
	}
	SLOG_DEBUG2("    portSize:%d",dev->portsSize);
	bind_display_to_port(dev);
	dev->pipesSize = 0;
	
	SLOG_DEBUG2("    Filling port data ...");
	/* Set port information */
	for (it=0;it<dev->portsSize;it++)
	{
		dev->ports[it].portId = it + 1; // display port id begin at 1
		dev->ports[it].type = dev->ports[it].du_cfg->type;
		dev->ports[it].vsync_counter = 0;
		dev->ports[it].bgcolor = 0xffff00ff;
		dev->ports[it].attached = WFD_TRUE;
		dev->ports[it].bindablesSize = dev->ports[it].du_cfg->pipe_num;

		int pipeId,i;
		for (i = 0; i < dev->ports[it].du_cfg->pipe_num; i++)
		{
			pipeId = dev->ports[it].du_cfg->pipe_ids[i];
			dev->ports[it].bindables[i] = pipeId;

			/* Filling pipes data */
			SLOG_DEBUG2("    Filling pipes data ... pipe_order = %d, pipeId = %d",pipe_order,pipeId);
			dev->pipes[pipe_order].pipeId = pipeId;
			dev->pipes[pipe_order].plane = get_plane_from_pipeId (&dev->ports[it],pipeId);
			dev->pipes[pipe_order].layer = pipe_order;
			dev->pipes[pipe_order].shareable = 0;
			dev->pipes[pipe_order].direct_refresh = 1;
			dev->pipes[pipe_order].rotation_support = 0;
			dev->pipes[pipe_order].rotation = 0;
			dev->pipes[pipe_order].transparency = 0;
			dev->pipes[pipe_order].chroma_enabled = 0;
			dev->pipes[pipe_order].source_color = 0xff00ff;
			dev->pipes[pipe_order].mask = 1 << 31;
			dev->pipes[pipe_order].interlaced=0;
			pipe_order++;
		}
		
		/* BTW, calculate number of pipelines of all ports */
		dev->pipesSize += dev->ports[it].bindablesSize;
	}
	
	SLOG_DEBUG2("    Initializing adapter ...");
	
    if (rcardu_init(dev) == -1)
    {
        pthread_mutex_destroy(&dev->mutex);
        rcardu_fini(dev);
        free(dev->cfglib_device);
        free(dev);
        return WFD_INVALID_HANDLE;
    }
    SLOG_DEBUG2("    Done. dev = %p", dev);
    return (WFDDevice)dev;
}

WFD_API_CALL WFDErrorCode WFD_APIENTRY
wfdDestroyDevice(WFDDevice device) WFD_APIEXIT
{
	du_dev_t *dev = (du_dev_t *)device;
	TRACE;

	DEVICE_VALIDATE(return WFD_ERROR_BAD_DEVICE)

	rcardu_fini(dev);

	pthread_mutex_destroy(&dev->mutex);
	free(dev->cfglib_device);
	free(dev);

	return WFD_ERROR_NONE;
}

WFD_API_CALL void WFD_APIENTRY
wfdDeviceCommit(WFDDevice device,
	WFDCommitType type, WFDHandle handle) WFD_APIEXIT
{
	du_dev_t	*dev = (du_dev_t *)device;
	port_t		*port = NULL;
	pipe_t		*pipe = NULL;
	int		wait_for_vsync = 0;	/* CHECK */
	int		i;

	TRACE;

	DEVICE_VALIDATE(return)

	/* TODO :we must make sure updates can be done before making any changes */
	if (type == WFD_COMMIT_ENTIRE_DEVICE) {
		LOCK_DEVICE();
		if (dev->changes) {
			for ( i = 0; i < dev->portsSize; i++ ) {
				if (dev->ports[i].changes) {
					/*
					* TODO - what we really want to do here is to have a multiple
					* wait_for_vsync function down below.
					*/
					wait_for_vsync |= wfdCommitPortUpdates(dev,&dev->ports[i]);
                    /* Choose any port for vsync waiting. If screen do not */
                    /* care about visual appearance how WFD driver should care? */
                    port = &dev->ports[i];
				}
			}
			dev->changes = 0;
		}
		UNLOCK_DEVICE();
	} else if (type == WFD_COMMIT_ENTIRE_PORT) {
		port = (port_t *)handle;
		PORT_VALIDATE(WFD_ERROR_BAD_HANDLE, return);
		PORT_CREATED(WFD_ERROR_BAD_HANDLE, return);
			LOCK_DEVICE();
		if (port->changes) {
			wait_for_vsync = wfdCommitPortUpdates(dev,port);
		}
			UNLOCK_DEVICE();
	} else if (type == WFD_COMMIT_PIPELINE) {
			LOCK_DEVICE();
		pipe = (pipe_t *)handle;
		port=pipe->port;
		wait_for_vsync = wfdCommitPipelineUpdates(dev, pipe);
			UNLOCK_DEVICE();
	}


	if (wait_for_vsync && port) {
		rcardu_wait_vsync(dev, port);
	}

}

WFD_API_CALL WFDint WFD_APIENTRY
wfdGetDeviceAttribi(WFDDevice device, WFDDeviceAttrib attrib) WFD_APIEXIT
{
	du_dev_t *dev = (du_dev_t *)device;

	TRACE;

	DEVICE_VALIDATE(return 0)

	switch (attrib) {
		case WFD_DEVICE_ID:
			return 1;
		default:
			LOG_ERROR(WFD_ERROR_BAD_ATTRIBUTE);
			return 0;
	}
}

WFD_API_CALL void WFD_APIENTRY
wfdSetDeviceAttribi(WFDDevice device,
	WFDDeviceAttrib attrib, WFDint value) WFD_APIEXIT
{
	du_dev_t *dev = (du_dev_t *)device;

	TRACE;

	DEVICE_VALIDATE(return )

    switch (attrib) {
        case WFD_DEVICE_ID:
             if (value != 1) {
                 dev->error = WFD_ERROR_ILLEGAL_ARGUMENT;
             }
             break;
        default:
             dev->error = WFD_ERROR_BAD_ATTRIBUTE;
             break;
    }

	LOG_ERROR(WFD_ERROR_BAD_ATTRIBUTE);
}

WFD_API_CALL WFDErrorCode WFD_APIENTRY
wfdGetError(WFDDevice device) WFD_APIEXIT
{
	du_dev_t	*dev = (du_dev_t *)device;
	WFDErrorCode	err;

	TRACE;

	DEVICE_VALIDATE(return 0)

	LOCK_DEVICE();

	err = dev->error;
	dev->error = WFD_ERROR_NONE;

	UNLOCK_DEVICE();

	return err;
}


