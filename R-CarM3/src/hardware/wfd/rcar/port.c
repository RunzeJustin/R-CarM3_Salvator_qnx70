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

#include <string.h>
#include <unistd.h>

#include "rcar_display.h"
#include "vsp_drv.h"

static float calculate_mode_refresh(const struct wfdcfg_timing *mode)
{
    uint32_t htotal = mode->hpixels + mode->hbp + mode->hsw + mode->hfp;
    uint32_t vtotal = mode->vlines + mode->vbp + mode->vsw + mode->vfp;

    return mode->pixel_clock_kHz * 1000.0f / htotal / vtotal;
}

WFD_API_CALL WFDint WFD_APIENTRY wfdEnumeratePorts(WFDDevice wfd_device, WFDint *portIds,
    WFDint portIdsCount, const WFDint *filterList) WFD_APIEXIT
{
    du_dev_t* dev=(du_dev_t *)wfd_device;
    int           port_number=0;

    TRACE;
    SLOG_DEBUG2("       device=%08X, portIds=%p, portIdsCount=%d, filterList=%p", wfd_device, portIds, portIdsCount, filterList);

    DEVICE_VALIDATE(return 0)

    if (portIds && (portIdsCount <= 0))
    {
        LOG_ERROR(WFD_ERROR_ILLEGAL_ARGUMENT);
        SLOG_DEBUG2("       return: 0 (portIds!=NULL && portIdsCount<=0)");
        return WFD_INVALID_HANDLE;
    }

    if ((filterList) && (*filterList != WFD_NONE)) {
        SLOG_DEBUG2("       return: 0 (filterList!=NULL && !=WFD_NONE)");
        return 0;
    }

    if (portIds != NULL)
    {
        /*
        On success, the number of portIds elements that were populated with port ID
        values is returned via the function return value.
        Thus elements 0 through [(the returned value) - 1] of portIds will contain
        valid ID values.
         */
        for (; (port_number < dev->portsSize) && (port_number<portIdsCount); port_number++)
        {
            portIds[port_number] = dev->ports[port_number].portId;
        }

        SLOG_DEBUG2("    Filled %d ports by request", port_number);
        return port_number;
    }

    /*
     * if wfdEnumeratePorts is called with portIds = NULL then no IDs are returned,
     * but the total number of available IDs is returned via the function return value.
     */
    SLOG_DEBUG2("    Return %d number of ports", dev->portsSize);
    return dev->portsSize;
}

WFD_API_CALL WFDPort WFD_APIENTRY
wfdCreatePort(WFDDevice wfd_device, WFDint portId, const WFDint *attribList) WFD_APIEXIT
{
    du_dev_t*       dev=(du_dev_t*)wfd_device;
    port_t*             port=NULL;
    const struct wfdcfg_timing* val_timing = NULL;

    TRACE;
    SLOG_DEBUG2("       device=%08X, portId=%d, attribList=%p", wfd_device, portId, attribList);

    DEVICE_VALIDATE(return WFD_INVALID_HANDLE)
    PORTID_VALIDATE(WFD_ERROR_ILLEGAL_ARGUMENT, return WFD_INVALID_HANDLE)

    SLOG_DEBUG2("    Creation of port %d", portId);

    if ((attribList!=NULL) && (*attribList!=WFD_NONE))
    {
        LOG_ERROR(WFD_ERROR_BAD_ATTRIBUTE);
        SLOG_DEBUG2("       return: WFD_INVALID_HANDLE (WFD_ERROR_BAD_ATTRIBUTE)");
        return WFD_INVALID_HANDLE;
    }

    if ((portId < 1) || (portId > RCARDU_WFD_MAX_NUMBER_OF_PORTS)) {
        SLOG_DEBUG2("       return: WFD_INVALID_HANDLE (WFD_ILLEGAL_ARGUMENT)");
        LOG_ERROR(WFD_ERROR_ILLEGAL_ARGUMENT);
        return WFD_INVALID_HANDLE;
    }

    LOCK_DEVICE();

    port = &(dev->ports[portId - 1]);
    STAILQ_INIT(&port->modelist);

    /* Build an option argument for our wfdcfg_port_create() */
    struct wfdcfg_keyval opts[]={
        {
            .key = WFDCFG_OPT_BOUND_DU_INDEX,
            .i = port->du_cfg->du_index,
            .p = NULL,
        },
        {   /* marks end of list */
            .key = NULL,
            .i = 0,
            .p = NULL,
        },
    };

    int err = wfdcfg_port_create(&port->cfglib_port, dev->cfglib_device, portId, opts);
    if (err) {
        port->created = WFD_FALSE;
        dev->error = WFD_ERROR_OUT_OF_MEMORY;
        UNLOCK_DEVICE();

        SLOG_ERROR("wfdcfg_port_create failed (port_id = %d, err = %d (%s))", portId, err, strerror(err));
        SLOG_DEBUG2("       return: WFD_INVALID_HANDLE (WFD_ERROR_OUT_OF_MEMORY - wfdcfg_port_create)");

        return WFD_INVALID_HANDLE;
    }

    err = wfdcfg_mode_list_create(&port->cfglib_modelist, port->cfglib_port, opts);
    if (err != EOK) {
        wfdcfg_port_destroy(port->cfglib_port);
        port->created = WFD_FALSE;
        dev->error = WFD_ERROR_OUT_OF_MEMORY;
        UNLOCK_DEVICE();

        SLOG_ERROR("wfdcfg_mode_list_create failed (port_id = %d, err = %d (%s)", portId, err, strerror(err));

        SLOG_DEBUG2("       return: WFD_INVALID_HANDLE (WFD_ERROR_OUT_OF_MEMORY - wfdcfg_mode_list_create)");
        return WFD_INVALID_HANDLE;
    }

    while ((val_timing = wfdcfg_mode_list_get_next(port->cfglib_modelist, val_timing))) {
        portmode_t *pmode = calloc(sizeof *pmode, 1);
        if (!pmode) {
            wfdcfg_port_destroy(port->cfglib_port);
            port->created = WFD_FALSE;
            dev->error = WFD_ERROR_OUT_OF_MEMORY;
            UNLOCK_DEVICE();
            SLOG_ERROR("wfdcfg_mode_list_create failed WFD_ERROR_OUT_OF_MEMORY");

            return WFD_INVALID_HANDLE;
        }
        pmode->timings = val_timing;
        pmode->refresh = calculate_mode_refresh(pmode->timings);
        if (pmode->timings->flags & WFDCFG_PREFERRED) {
            port->native_width = pmode->timings->hpixels;
            port->native_height = pmode->timings->vlines;
        }
        STAILQ_INSERT_TAIL(&port->modelist, pmode, list_entry);
    }

    port->gamma = LUT_DEF_GAMMA;
    port->gamma_range_min = LUT_MIN_GAMMA;
    port->gamma_range_max = LUT_MAX_GAMMA;

    port->created = WFD_TRUE;

    /* Execute external init function for this port */
    const struct wfdcfg_keyval* init_fnc_ext = wfdcfg_port_get_extension(port->cfglib_port, WFDCFG_EXT_FN_PORT_INIT1);
    if (init_fnc_ext) {
        ((wfdcfg_ext_fn_port_init1_t*)init_fnc_ext->p)(port->cfglib_port);
    }

    /* Set port type if wfdcfg provides it */
    const struct wfdcfg_keyval* port_type = wfdcfg_port_get_extension(port->cfglib_port, WFDCFG_EXT_PORT_TYPE);
    if (port_type) {
        port->type = port_type->i;
    } else {
        port->type = WFD_PORT_TYPE_OTHER;
    }

    const struct wfdcfg_keyval* phys_port_type = wfdcfg_port_get_extension(port->cfglib_port, WFDCFG_OPT_PHYSPORT_TYPE);
    if (phys_port_type) {
        port->physport_type = phys_port_type->i;
    } else {
        port->physport_type = RCAR_PHYSPORT_UNKNOWN;
    }

    /* Set port physical display size if wfdcfg provides it */
    const struct wfdcfg_keyval* port_size = wfdcfg_port_get_extension(port->cfglib_port, WFDCFG_EXT_PHYS_SIZE_MM);
    if (port_size) {
        port->physical_width = ((float*)port_type->p)[0];
        port->physical_height = ((float*)port_type->p)[1];
    } else {
        port->physical_width = 0.0f;
        port->physical_height = 0.0f;
    }

    UNLOCK_DEVICE();
    SLOG_DEBUG2("    Port %d has been created", portId);

    return ((WFDPort)port);
}

WFD_API_CALL void WFD_APIENTRY
wfdDestroyPort(WFDDevice wfd_device, WFDPort wfd_port) WFD_APIEXIT
{
	du_dev_t	*dev = (du_dev_t *)wfd_device;
	port_t		*port = (port_t *)wfd_port;
	pipe_t		*pipe = NULL;
	TRACE;

	DEVICE_VALIDATE(return)
	PORT_VALIDATE(WFD_ERROR_BAD_HANDLE, return)
	PORT_CREATED(WFD_ERROR_BAD_HANDLE, return)
	LOCK_DEVICE();

	/* walk pipes set port invalid */
	for (; port->bound_pipesSize > 0;) {
		--port->bound_pipesSize;
		pipe = port->bound_pipes[port->bound_pipesSize];
		if (pipe && pipe->created) {
			pipe->port = WFD_INVALID_HANDLE;
			pipe->bound_port = WFD_INVALID_HANDLE;
			pipe->changes |= WFD_PIPELINE_CHANGES_BIND;
			port->changes |= WFD_PORT_CHANGES_PIPELINE;
			dev->changes |= WFD_DEVICE_CHANGES_PIPELINE;
		}
		port->bound_pipes[port->bound_pipesSize] = NULL;
	}
	wfdCommitPortUpdates(dev, port);
	if (port->du_cfg->ExScaleHwId != DEVICE_NONE)
	{
		munmap (port->dl_vrt, (DL_HEADER_SIZE + (DL_BODY_SIZE * DL_BODY_NUM_FOR_WORK))*DISPLAY_LIST_NUM);
	}
	UNLOCK_DEVICE();
}

WFD_API_CALL WFDint WFD_APIENTRY
wfdGetPortModes(WFDDevice wfd_device, WFDPort wfd_port,
	WFDPortMode *wfd_modes, WFDint wfd_modesSize) WFD_APIEXIT
{
    du_dev_t*       dev=(du_dev_t *)wfd_device;
    port_t*        port=(port_t *)wfd_port;
    portmode_t*    mode;
    WFDint found = 0;

    TRACE;
    SLOG_DEBUG2("       device=%08X, port=%08X, modes=%p, modesCount=%d", wfd_device, wfd_port, wfd_modes, wfd_modesSize);

    DEVICE_VALIDATE(return 0);
    PORT_VALIDATE(WFD_ERROR_BAD_HANDLE, return 0)
    PORT_CREATED(WFD_ERROR_BAD_HANDLE, return 0)
    PORT_ATTACHED(WFD_ERROR_NOT_SUPPORTED, return 0)


    if (wfd_modes && wfd_modesSize <= 0) {
        LOG_ERROR(WFD_ERROR_ILLEGAL_ARGUMENT);
        SLOG_DEBUG2("       return: 0 (WFD_ERROR_ILLEGAL_ARGUMENT)");
        return 0;
    }

    STAILQ_FOREACH(mode, &port->modelist, list_entry) {
        if (wfd_modes) {
            if (found >= wfd_modesSize) {
                break;
            }
            wfd_modes[found] = (WFDPortMode)mode;
            SLOG_DEBUG2("       return: modes[%d]=%08X", found, wfd_modes[found]);
        }
        found++;
    }

    SLOG_DEBUG2("       return: %d (done)", found);

    return found;
}

WFD_API_CALL WFDint WFD_APIENTRY wfdGetPortModeAttribi(WFDDevice wfd_device, WFDPort wfd_port,
    WFDPortMode wfd_mode, WFDPortModeAttrib attrib) WFD_APIEXIT
{
    du_dev_t*       dev  = (du_dev_t*)wfd_device;
    port_t*         port = (port_t*)wfd_port;
    portmode_t*     mode = (portmode_t*)wfd_mode;

    TRACE;
    SLOG_DEBUG2("       device=%08X, port=%08X, mode=%08X, attrib=%08X", wfd_device, wfd_port, wfd_mode, attrib);

    DEVICE_VALIDATE(return 0)
    PORT_VALIDATE(WFD_ERROR_BAD_HANDLE, return 0)
    PORT_CREATED(WFD_ERROR_BAD_HANDLE, return 0)
    PORT_ATTACHED(WFD_ERROR_NOT_SUPPORTED, return 0)

    if (mode == NULL) {
        dev->error = WFD_ERROR_ILLEGAL_ARGUMENT;
        SLOG_DEBUG2("       return: 0 (WFD_ERROR_ILLEGAL_ARGUMENT)");
        return 0;
    }

    switch (attrib)
    {
        case WFD_PORT_MODE_WIDTH:
            SLOG_DEBUG2("       return: %d (WFD_PORT_MODE_WIDTH)", mode->timings->hpixels);
            return mode->timings->hpixels;
        case WFD_PORT_MODE_HEIGHT:
            SLOG_DEBUG2("       return: %d (WFD_PORT_MODE_HEIGHT)", mode->timings->vlines);
            return mode->timings->vlines;
        case WFD_PORT_MODE_REFRESH_RATE:
            SLOG_DEBUG2("       return: %d (WFD_PORT_MODE_REFRESH_RATE)", (WFDint)(mode->refresh + 0.5f));
            return (WFDint)(mode->refresh + 0.5f);
        case WFD_PORT_MODE_FLIP_MIRROR_SUPPORT:
            SLOG_DEBUG2("       return: %d (WFD_PORT_MODE_FLIP_MIRROR_SUPPORT)", mode->mirror);
            return mode->mirror;
        case WFD_PORT_MODE_ROTATION_SUPPORT:
            SLOG_DEBUG2("       return: %d (WFD_PORT_MODE_ROTATION_SUPPORT)",mode->rotation_support);
            return mode->rotation_support;
        case WFD_PORT_MODE_INTERLACED:
            SLOG_DEBUG2("       return: %d (WFD_PORT_MODE_INTERLACED)",mode->interlaced);
            return mode->interlaced;
#if WFD_QNX_port_mode_info
        case WFD_PORT_MODE_PREFERRED_QNX:
            SLOG_DEBUG2("       return: %s (WFD_PORT_MODE_PREFERRED_QNX)", (mode->timings->flags & WFDCFG_PREFERRED) ? "WFD_TRUE" : "WFD_FALSE");
            return (mode->timings->flags & WFDCFG_PREFERRED) ? WFD_TRUE : WFD_FALSE;
#endif
        default:
             LOG_ERROR(WFD_ERROR_BAD_ATTRIBUTE);
             SLOG_DEBUG("WFD_ERROR_BAD_ATTRIBUTE");
             break;
    }

    return 0;
}

WFD_API_CALL WFDfloat WFD_APIENTRY
wfdGetPortModeAttribf(WFDDevice wfd_device, WFDPort wfd_port,
	WFDPortMode wfd_mode, WFDPortModeAttrib attrib) WFD_APIEXIT
{
	du_dev_t	*dev  = (du_dev_t *)wfd_device;
	port_t		*port = (port_t *)wfd_port;
	portmode_t	*mode = (portmode_t *)wfd_mode;

	TRACE;
	SLOG_DEBUG2("       device=%08X, porthdl=%08X, mode=%08X, attrib=%08X", wfd_device, wfd_port, wfd_mode, attrib);

	DEVICE_VALIDATE(return 0.0f)
	PORT_VALIDATE(WFD_ERROR_BAD_HANDLE, return 0.0f)
	PORT_CREATED(WFD_ERROR_BAD_HANDLE, return 0.0f)
	PORT_ATTACHED(WFD_ERROR_NOT_SUPPORTED, return 0.0f)

	switch (attrib) {
		case WFD_PORT_MODE_REFRESH_RATE:
		    SLOG_DEBUG2("       return: %f (WFD_PORT_MODE_REFRESH_RATE)", (WFDfloat)mode->refresh);
		    return (WFDfloat)mode->refresh;
#if WFD_QNX_port_mode_info
        case WFD_PORT_MODE_ASPECT_RATIO_QNX:
            SLOG_DEBUG2("       return: %f (WFD_PORT_MODE_ASPECT_RATIO_QNX)", (WFDfloat)port->active_mode->timings->hpixels /
                    (WFDfloat)port->active_mode->timings->vlines);
            return (WFDfloat)port->active_mode->timings->hpixels / (WFDfloat)port->active_mode->timings->vlines;
#endif /* WFD_QNX_port_mode_info */
		default:
			LOG_ERROR(WFD_ERROR_BAD_ATTRIBUTE);
			break;
	}

	SLOG_DEBUG2("       return: 0.0f (WFD_ERROR_BAD_ATTRIBUTE)");
	return 0.0f;
}

WFD_API_CALL void WFD_APIENTRY
wfdSetPortMode(WFDDevice wfd_device, WFDPort wfd_port, WFDPortMode wfd_mode) WFD_APIEXIT
{
	du_dev_t	*dev = (du_dev_t *)wfd_device;
	port_t		*port = (port_t *)wfd_port;
	portmode_t	*mode = (portmode_t *)wfd_mode;

	TRACE;
	SLOG_DEBUG2("       device=%08X, port=%08X, mode=%08X", wfd_device, wfd_port, wfd_mode);

	DEVICE_VALIDATE(return )
	PORT_VALIDATE(WFD_ERROR_BAD_HANDLE, return)
	PORT_CREATED(WFD_ERROR_BAD_HANDLE, return)
	PORT_ATTACHED(WFD_ERROR_NOT_SUPPORTED, return)

	if (mode == NULL){
		SLOG_ERROR("Can't set port %d mode. (NULL)",port->portId);
		LOG_ERROR(WFD_ERROR_BAD_HANDLE);
		SLOG_DEBUG2("       return: (failed)");
		return;
	}
	if (port->active_mode != mode) {
		port->active_mode = mode;
		port->changes |= WFD_PORT_CHANGES_MODE;
	}
	SLOG_DEBUG("Current mode of port %d is %dx%d @ %fHz", port->portId,
			port->active_mode->timings->hpixels,
			port->active_mode->timings->vlines,
			port->active_mode->refresh);
	SLOG_DEBUG2("       return: (done)");
}

WFD_API_CALL WFDPortMode WFD_APIENTRY
wfdGetCurrentPortMode(WFDDevice wfd_device, WFDPort wfd_port) WFD_APIEXIT
{
	du_dev_t	*dev = (du_dev_t *)wfd_device;
	port_t		*port = (port_t *)wfd_port;

	TRACE;
	SLOG_DEBUG2("       device=%08X, port=%08X", wfd_device, wfd_port);

	DEVICE_VALIDATE(return WFD_INVALID_HANDLE)
	PORT_VALIDATE(WFD_ERROR_BAD_HANDLE, return WFD_INVALID_HANDLE)
	PORT_CREATED(WFD_ERROR_BAD_HANDLE, return WFD_INVALID_HANDLE)
	PORT_ATTACHED(WFD_ERROR_NOT_SUPPORTED, return WFD_INVALID_HANDLE)

	if (port->active_mode != NULL) {
	    SLOG_DEBUG2("       return: %08X (done)", (WFDPortMode)port->active_mode);
		return (WFDPortMode)port->active_mode;
	}

	LOG_ERROR(WFD_ERROR_NOT_SUPPORTED);

	return WFD_INVALID_HANDLE;
}

WFD_API_CALL WFDint WFD_APIENTRY
wfdGetPortAttribi(WFDDevice wfd_device,
	WFDPort wfd_port, WFDPortConfigAttrib attrib) WFD_APIEXIT
{
	WFDint value = 0;

	TRACE;
	SLOG_DEBUG2("       device=%08X, port=%08X, attrib=%08X", wfd_device, wfd_port, attrib);

	wfdGetPortAttribiv(wfd_device, wfd_port, attrib, 1, &value);

	return value;
}

WFD_API_CALL WFDfloat WFD_APIENTRY
wfdGetPortAttribf(WFDDevice wfd_device,
	WFDPort wfd_port, WFDPortConfigAttrib attrib) WFD_APIEXIT
{
	WFDfloat value = 0.0f;

	TRACE;
	SLOG_DEBUG2("       device=%08X, port=%08X, attrib=%08X", wfd_device, wfd_port, attrib);

	wfdGetPortAttribfv(wfd_device, wfd_port, attrib, 1, &value);

	return value;
}

WFD_API_CALL void WFD_APIENTRY wfdGetPortAttribiv(WFDDevice wfd_device, WFDPort wfd_port,
    WFDPortConfigAttrib attrib, WFDint count, WFDint *value) WFD_APIEXIT
{
    du_dev_t* dev=(du_dev_t*)wfd_device;
    port_t*       port=(port_t*)wfd_port;
    int           i=0;

    TRACE;
    SLOG_DEBUG2("       device=%08X, port=%08X, attrib=%08X, count=%d, value=%p", wfd_device, wfd_port, attrib, count, value);

    DEVICE_VALIDATE(return )
    PORT_VALIDATE(WFD_ERROR_BAD_HANDLE, return )
    PORT_CREATED(WFD_ERROR_BAD_HANDLE, return )
    PORT_ATTACHED(WFD_ERROR_NOT_SUPPORTED, return )

    if (value == NULL || count <= 0 || count > 16)
    {
        SLOG_DEBUG2("       return: (WFD_ERROR_ILLEGAL_ARGUMENT, value==NULL || count<=0)");
        LOG_ERROR(WFD_ERROR_ILLEGAL_ARGUMENT);
        return;
    }

    switch (attrib)
    {
        case WFD_PORT_ID:
            if (count != 1) {
                SLOG_DEBUG2("       return: (WFD_ERROR_ILLEGAL_ARGUMENT, WFD_PORT_ID)");
             LOG_ERROR(WFD_ERROR_ILLEGAL_ARGUMENT);
             return;
            }
            LOCK_DEVICE();
            *value=port->portId;
            UNLOCK_DEVICE();
            SLOG_DEBUG2("       return: %d (WFD_PORT_ID)", *value);
            break;
        case WFD_PORT_TYPE:
            if (count != 1) {
                SLOG_DEBUG2("       return: (WFD_ERROR_ILLEGAL_ARGUMENT, WFD_PORT_TYPE)");
                LOG_ERROR(WFD_ERROR_ILLEGAL_ARGUMENT);
                return;
            }
            LOCK_DEVICE();
            *value=port->type;
            UNLOCK_DEVICE();
            SLOG_DEBUG2("       return: %08X (WFD_PORT_TYPE)", *value);
            break;
        case WFD_PORT_DETACHABLE:
             if (count != 1) {
                 SLOG_DEBUG2("       return: (WFD_ERROR_ILLEGAL_ARGUMENT, WFD_PORT_DETACHABLE)");
                 LOG_ERROR(WFD_ERROR_ILLEGAL_ARGUMENT);
                 return;
             }
             LOCK_DEVICE();
             *value=port->detachable;
             UNLOCK_DEVICE();
             SLOG_DEBUG2("       return: %s (WFD_PORT_DETACHABLE)", *value == WFD_FALSE ? "WFD_FALSE" : "WFD_TRUE");
             break;
        case WFD_PORT_ATTACHED:
            if (count != 1) {
                SLOG_DEBUG2("       return: (WFD_ERROR_ILLEGAL_ARGUMENT, WFD_PORT_ATTACHED)");
                LOG_ERROR(WFD_ERROR_ILLEGAL_ARGUMENT);
                return;
            }
            LOCK_DEVICE();
            *value=port->attached;
            UNLOCK_DEVICE();
            SLOG_DEBUG2("       return: %s (WFD_PORT_ATTACHED)", *value == WFD_FALSE ? "WFD_FALSE" : "WFD_TRUE");
             break;
        case WFD_PORT_NATIVE_RESOLUTION:
             if (count!=2)
             {
                LOG_ERROR(WFD_ERROR_ILLEGAL_ARGUMENT);
                return;
             }
             LOCK_DEVICE();
             value[0] = port->native_width;
             value[1] = port->native_height;
             UNLOCK_DEVICE();
             SLOG_DEBUG2("       return: %dx%d (WFD_PORT_NATIVE_RESOLUTION)", value[0], value[1]);
             break;
		case WFD_PORT_FILL_PORT_AREA:
            if (count != 1) {
                SLOG_DEBUG2("       return: (WFD_ERROR_ILLEGAL_ARGUMENT, WFD_PORT_FILL_PORT_AREA)");
                LOG_ERROR(WFD_ERROR_ILLEGAL_ARGUMENT);
                return;
            }
            LOCK_DEVICE();
            *value = WFD_FALSE;
            UNLOCK_DEVICE();
            SLOG_DEBUG2("       return: %s (WFD_PORT_FILL_PORT_AREA)", *value == WFD_FALSE ? "WFD_FALSE" : "WFD_TRUE");
			break;
		case WFD_PORT_BACKGROUND_COLOR:
            if (count == 1) {
                /* Renesas DU implements fully transparent background */
                LOCK_DEVICE();
                value[0] = (port->bgcolor & 0x00FFFFFF)<<8;
                UNLOCK_DEVICE();
                SLOG_DEBUG2("       return: %08X (WFD_PORT_BACKGROUND_COLOR)", value[0]);
            } else if (count == 3) {
                WFDuint8* value8 = (WFDuint8*)value;

                LOCK_DEVICE();
                value8[0] = (port->bgcolor & 0x00FF0000) >> 16; /* red */
                value8[1] = (port->bgcolor & 0x0000FF00) >> 8;  /* green */
                value8[2] = (port->bgcolor & 0x000000FF);       /* blue */
                UNLOCK_DEVICE();
                SLOG_DEBUG2("       return: %d, %d, %d (WFD_PORT_BACKGROUND_COLOR)", value8[0], value8[1], value8[2]);
            } else {
                SLOG_DEBUG2("       return: (WFD_ERROR_ILLEGAL_ARGUMENT, WFD_PORT_BACKGROUND_COLOR)");
                LOG_ERROR(WFD_ERROR_ILLEGAL_ARGUMENT);
            }
			break;
		case WFD_PORT_FLIP:
            if (count != 1) {
                SLOG_DEBUG2("       return: (WFD_ERROR_ILLEGAL_ARGUMENT, WFD_PORT_FLIP)");
                LOG_ERROR(WFD_ERROR_ILLEGAL_ARGUMENT);
                return;
            }
            *value = WFD_FALSE;
            SLOG_DEBUG2("       return: %s (WFD_PORT_FLIP)", *value == WFD_FALSE ? "WFD_FALSE" : "WFD_TRUE");
            break;
		case WFD_PORT_MIRROR:
            if (count != 1) {
                SLOG_DEBUG2("       return: (WFD_ERROR_ILLEGAL_ARGUMENT, WFD_PORT_MIRROR)");
                LOG_ERROR(WFD_ERROR_ILLEGAL_ARGUMENT);
                return;
            }
            *value = WFD_FALSE;
            SLOG_DEBUG2("       return: %s (WFD_PORT_MIRROR)", *value == WFD_FALSE ? "WFD_FALSE" : "WFD_TRUE");
            break;
		case WFD_PORT_ROTATION:
            if (count != 1) {
                SLOG_DEBUG2("       return: (WFD_ERROR_ILLEGAL_ARGUMENT, WFD_PORT_ROTATION)");
                LOG_ERROR(WFD_ERROR_ILLEGAL_ARGUMENT);
                return;
            }
            *value = WFD_FALSE;
            SLOG_DEBUG2("       return: %s (WFD_PORT_ROTATION)", *value == WFD_FALSE ? "WFD_FALSE" : "WFD_TRUE");
            break;
		case WFD_PORT_POWER_MODE:
            if (count != 1) {
                SLOG_DEBUG2("       return: (WFD_ERROR_ILLEGAL_ARGUMENT, WFD_PORT_POWER_MODE)");
                LOG_ERROR(WFD_ERROR_ILLEGAL_ARGUMENT);
                return;
            }
            LOCK_DEVICE();
            *value = port->power_mode;
            UNLOCK_DEVICE();
            SLOG_DEBUG2("       return: %08X (WFD_PORT_POWER_MODE)", *value);
			break;
		case WFD_PORT_PARTIAL_REFRESH_SUPPORT:
            if (count != 1) {
                SLOG_DEBUG2("       return: (WFD_ERROR_ILLEGAL_ARGUMENT, WFD_PORT_PARTIAL_REFRESH_SUPPORT)");
                LOG_ERROR(WFD_ERROR_ILLEGAL_ARGUMENT);
                return;
            }
            *value = WFD_PARTIAL_REFRESH_NONE;
            SLOG_DEBUG2("       return: %08X (WFD_PORT_PARTIAL_REFRESH_SUPPORT)", *value);
			break;
		case WFD_PORT_PARTIAL_REFRESH_MAXIMUM:
            if (count != 2) {
                SLOG_DEBUG2("       return: (WFD_ERROR_ILLEGAL_ARGUMENT, WFD_PORT_PARTIAL_REFRESH_MAXIMUM)");
                LOG_ERROR(WFD_ERROR_ILLEGAL_ARGUMENT);
                return;
            }
            value[0] = 0;
            value[1] = 0;
            SLOG_DEBUG2("       return: %dx%d (WFD_PORT_PARTIAL_REFRESH_MAXIMUM)", value[0], value[1]);
            break;
		case WFD_PORT_PARTIAL_REFRESH_ENABLE:
            if (count != 1) {
                SLOG_DEBUG2("       return: (WFD_ERROR_ILLEGAL_ARGUMENT, WFD_PORT_PARTIAL_REFRESH_SUPPORT)");
                LOG_ERROR(WFD_ERROR_ILLEGAL_ARGUMENT);
                return;
            }
            *value = WFD_PARTIAL_REFRESH_NONE;
            SLOG_DEBUG2("       return: %08X (WFD_PORT_PARTIAL_REFRESH_ENABLE)", *value);
            break;
		case WFD_PORT_PARTIAL_REFRESH_RECTANGLE:
            if (count != 4) {
                SLOG_DEBUG2("       return: (WFD_ERROR_ILLEGAL_ARGUMENT, WFD_PORT_PARTIAL_REFRESH_RECTANGLE)");
                LOG_ERROR(WFD_ERROR_ILLEGAL_ARGUMENT);
                return;
            }
            value[0] = 0;
            value[1] = 0;
            value[2] = 0;
            value[3] = 0;
            SLOG_DEBUG2("       return: %d, %d, %d, %d (WFD_PORT_PARTIAL_REFRESH_RECTANGLE)", value[0], value[1], value[2], value[3]);
            break;
		case WFD_PORT_PIPELINE_ID_COUNT:
            if (count != 1) {
                SLOG_DEBUG2("       return: (WFD_ERROR_ILLEGAL_ARGUMENT, WFD_PORT_PIPELINE_ID_COUNT)");
                LOG_ERROR(WFD_ERROR_ILLEGAL_ARGUMENT);
                return;
            }
            LOCK_DEVICE();
            *value = port->bindablesSize;
            UNLOCK_DEVICE();
            SLOG_DEBUG2("       return: %d (WFD_PORT_PIPELINE_ID_COUNT)", *value);
            break;
		case WFD_PORT_BINDABLE_PIPELINE_IDS:
		    /* Do not check for count here, since Screen always passes 16 as count */
		    LOCK_DEVICE();
			for (i = 0; i <  port->bindablesSize; i++) {
				value[i] = port->bindables[i];
				SLOG_DEBUG2("       return: value[%d]=%d (WFD_PORT_BINDABLE_PIPELINE_IDS)", i, value[i]);
			}
			UNLOCK_DEVICE();
			break;
		case WFD_PORT_PROTECTION_ENABLE:
            if (count != 1) {
                SLOG_DEBUG2("       return: (WFD_ERROR_ILLEGAL_ARGUMENT, WFD_PORT_PROTECTION_ENABLE)");
                LOG_ERROR(WFD_ERROR_ILLEGAL_ARGUMENT);
                return;
            }
            *value = WFD_FALSE;
            SLOG_DEBUG2("       return: %s (WFD_PORT_PROTECTION_ENABLE)", *value == WFD_FALSE ? "WFD_FALSE" : "WFD_TRUE");
			break;
		default:
			LOG_ERROR(WFD_ERROR_BAD_ATTRIBUTE);
			SLOG_DEBUG2("       return: WFD_ERROR_BAD_ATTRIBUTE");
	}
    return;
}

WFD_API_CALL void WFD_APIENTRY
wfdGetPortAttribfv(WFDDevice wfd_device, WFDPort wfd_port,
	WFDPortConfigAttrib attrib, WFDint count, WFDfloat *value) WFD_APIEXIT
{
	du_dev_t	*dev = (du_dev_t *)wfd_device;
	port_t		*port = (port_t *)wfd_port;

	TRACE;
	SLOG_DEBUG2("       device=%08X, port=%08X, attrib=%08X, count=%d, value=%p", wfd_device, wfd_port, attrib, count, value);

	DEVICE_VALIDATE(return )
	PORT_VALIDATE(WFD_ERROR_BAD_HANDLE, return )
	PORT_CREATED(WFD_ERROR_BAD_HANDLE, return )
	PORT_ATTACHED(WFD_ERROR_NOT_SUPPORTED, return )

	if (value == NULL || count <= 0 || count > 16) {
		LOG_ERROR(WFD_ERROR_ILLEGAL_ARGUMENT);
		return;
	}

	switch (attrib) {
        case WFD_PORT_BACKGROUND_COLOR:
             if (count != 3) {
                 LOG_ERROR(WFD_ERROR_ILLEGAL_ARGUMENT);
                 SLOG_DEBUG2("       return: (WFD_ERROR_ILLEGAL_ARGUMENT, WFD_PORT_BACKGROUND_COLOR)");
                 return;
             }
             LOCK_DEVICE();
             value[0]=(WFDfloat)((port->bgcolor >> 24) & 0x000000FF) / 255.0f; /* red */
             value[1]=(WFDfloat)((port->bgcolor >> 16) & 0x000000FF) / 255.0f; /* green */
             value[2]=(WFDfloat)((port->bgcolor >> 8) & 0x000000FF) / 255.0f;  /* blue */
             UNLOCK_DEVICE();
             SLOG_DEBUG2("       return: %f, %f, %f (WFD_PORT_BACKGROUND_COLOR)", value[0], value[1], value[2]);
             break;
		case WFD_PORT_PHYSICAL_SIZE:
			if (count != 2) {
				LOG_ERROR(WFD_ERROR_ILLEGAL_ARGUMENT);
				SLOG_DEBUG2("       return: (WFD_ERROR_ILLEGAL_ARGUMENT, WFD_PORT_PHYSICAL_SIZE)");
				return;
			}
			LOCK_DEVICE();
			value[0] = port->physical_height;
			value[1] = port->physical_width;
			UNLOCK_DEVICE();
			SLOG_DEBUG2("       return: %f, %f (WFD_PORT_PHYSICAL_SIZE)", value[0], value[1]);
			break;
		case WFD_PORT_GAMMA_RANGE:
			if (count != 2) {
				LOG_ERROR(WFD_ERROR_ILLEGAL_ARGUMENT);
				SLOG_DEBUG2("       return: (WFD_ERROR_ILLEGAL_ARGUMENT, WFD_PORT_PHYSICAL_SIZE)");
				return;
			}
			LOCK_DEVICE();
			value[0] = port->gamma_range_min;
			value[1] = port->gamma_range_max;
			UNLOCK_DEVICE();
			SLOG_DEBUG2("       return: %f, %f (WFD_PORT_GAMMA_RANGE)", value[0], value[1]);
			break;
		case WFD_PORT_GAMMA:
			if (count != 1) {
				LOG_ERROR(WFD_ERROR_ILLEGAL_ARGUMENT);
				SLOG_DEBUG2("       return: (WFD_ERROR_ILLEGAL_ARGUMENT, WFD_PORT_GAMMA)");
				return;
			}
			LOCK_DEVICE();
			*value = port->gamma;
			UNLOCK_DEVICE();
			SLOG_DEBUG2("       return: %f (WFD_PORT_GAMMA)", value[0]);
			break;
		default:
		    SLOG_DEBUG2("       return: (WFD_ERROR_BAD_ATTRIBUTE: %08X)", attrib);
			LOG_ERROR(WFD_ERROR_BAD_ATTRIBUTE);
	}

    return;
}

WFD_API_CALL void WFD_APIENTRY
wfdSetPortAttribi(WFDDevice wfd_device, WFDPort wfd_port,
	WFDPortConfigAttrib attrib, WFDint value) WFD_APIEXIT
{
	TRACE;

	wfdSetPortAttribiv(wfd_device, wfd_port, attrib, 1, &value);
}

WFD_API_CALL void WFD_APIENTRY
wfdSetPortAttribf(WFDDevice wfd_device, WFDPort wfd_port,
	WFDPortConfigAttrib attrib, WFDfloat value) WFD_APIEXIT
{
	TRACE;

	wfdSetPortAttribfv(wfd_device, wfd_port, attrib, 1, &value);
}

WFD_API_CALL void WFD_APIENTRY
wfdSetPortAttribiv(WFDDevice wfd_device, WFDPort wfd_port,
	WFDPortConfigAttrib attrib, WFDint count, const WFDint *value) WFD_APIEXIT
{
	du_dev_t	*dev = (du_dev_t *)wfd_device;
	port_t		*port = (port_t *)wfd_port;

	TRACE;
	SLOG_DEBUG2("       device=%08X, port=%08X, attrib=%08X, count=%d, value=%p", wfd_device, wfd_port, attrib, count, value);

	DEVICE_VALIDATE(return )
	PORT_VALIDATE(WFD_ERROR_BAD_HANDLE, return )
	PORT_CREATED(WFD_ERROR_BAD_HANDLE, return )
	PORT_ATTACHED(WFD_ERROR_NOT_SUPPORTED, return )

	if (value == NULL || count < 0 || count > 4) {
		LOG_ERROR(WFD_ERROR_ILLEGAL_ARGUMENT);
		SLOG_DEBUG2("       return: (WFD_ERROR_ILLEGAL_ARGUMENT, value = NULL or count < 0, count > 4)");
		return;
	}

    switch (attrib)
    {
        case WFD_PORT_BACKGROUND_COLOR:
             if (count == 1) {
                 LOCK_DEVICE();
                 port->bgcolor = (*value & 0xFFFFFF00)>>8;
                 port->changes |= WFD_PORT_CHANGES_BACKGROUND_COLOR;
                 UNLOCK_DEVICE();
             } else  if (count == 3) {
                 WFDuint8* value8 = (WFDuint8*)value;
                 LOCK_DEVICE();
                 port->bgcolor = 0x00000000;
                 port->bgcolor |= ((uint32_t)value8[0]) << 16;
                 port->bgcolor |= ((uint32_t)value8[1]) << 8;
                 port->bgcolor |= ((uint32_t)value8[0]);
                 port->changes |= WFD_PORT_CHANGES_BACKGROUND_COLOR;
                 UNLOCK_DEVICE();
             } else {
                 LOG_ERROR(WFD_ERROR_ILLEGAL_ARGUMENT);
                 SLOG_DEBUG2("       return: (WFD_ERROR_ILLEGAL_ARGUMENT, WFD_PORT_BACKGROUND_COLOR)");
                 return;
             }
             SLOG_DEBUG2("       set: %08X (WFD_PORT_BACKGROUND_COLOR)", port->bgcolor);
             break;
        case WFD_PORT_POWER_MODE:
             LOCK_DEVICE();
             port->power_mode = *value;
             UNLOCK_DEVICE();
             SLOG_DEBUG2("       set: %08X (WFD_PORT_POWER_MODE)", port->power_mode);
             break;
        default:
             LOG_ERROR(WFD_ERROR_BAD_ATTRIBUTE);
             SLOG_DEBUG2("       return: (WFD_ERROR_BAD_ATTRIBUTE: %08X)", attrib);
             break;
    }

    SLOG_DEBUG2("       return: (done)");
    return;
}

WFD_API_CALL void WFD_APIENTRY
wfdSetPortAttribfv(WFDDevice wfd_device, WFDPort wfd_port,
	WFDPortConfigAttrib attrib, WFDint count, const WFDfloat *value) WFD_APIEXIT
{
	du_dev_t	*dev = (du_dev_t *)wfd_device;
	port_t		*port = (port_t *)wfd_port;

	TRACE;
	SLOG_DEBUG2("       device=%08X, port=%08X, attrib=%08X, count=%d, value=%p", wfd_device, wfd_port, attrib, count, value);

	DEVICE_VALIDATE(return )
	PORT_VALIDATE(WFD_ERROR_BAD_HANDLE, return )
	PORT_CREATED(WFD_ERROR_BAD_HANDLE, return )
	PORT_ATTACHED(WFD_ERROR_NOT_SUPPORTED, return )

	if (value == NULL || count < 0) {
		LOG_ERROR(WFD_ERROR_ILLEGAL_ARGUMENT);
		SLOG_DEBUG2("       return: (WFD_ERROR_ILLEGAL_ARGUMENT, value == NULL or  count < 0)");
		return;
	}

	switch (attrib) {
        case WFD_PORT_BACKGROUND_COLOR:
             if (count == 3) {
                 LOCK_DEVICE();
                 port->bgcolor = 0x00000000;
                 port->bgcolor |= ((uint32_t)(value[0] * 255.0f)) << 24; /* red */
                 port->bgcolor |= ((uint32_t)(value[1] * 255.0f)) << 16; /* green */
                 port->bgcolor |= ((uint32_t)(value[2] * 255.0f)) << 8;  /* blue */
                 UNLOCK_DEVICE();
                 SLOG_DEBUG2("       return: set: %f, %f, %f (WFD_PORT_BACKGROUND_COLOR)", value[0], value[1], value[2]);
             } else {
                 LOG_ERROR(WFD_ERROR_ILLEGAL_ARGUMENT);
                 SLOG_DEBUG2("       return: (WFD_ERROR_ILLEGAL_ARGUMENT, WFD_PORT_BACKGROUND_COLOR)");
             }
             break;
		case WFD_PORT_GAMMA:
            if (count == 1) {
                if ((*value >= port->gamma_range_min) && (*value <= port->gamma_range_max)) {
                    LOCK_DEVICE();
                    port->gamma = *value;
                    port->changes |= WFD_PORT_CHANGES_GAMMA;
                    UNLOCK_DEVICE();
                    SLOG_DEBUG2("       return: set: %f (WFD_PORT_GAMMA)", value[0]);
                } else {
                    LOG_ERROR(WFD_ERROR_ILLEGAL_ARGUMENT);
                    SLOG_DEBUG2("       return: out-of-range (WFD_ERROR_ILLEGAL_ARGUMENT, WFD_PORT_GAMMA)");
                }
            } else {
                LOG_ERROR(WFD_ERROR_ILLEGAL_ARGUMENT);
                SLOG_DEBUG2("       return: (WFD_ERROR_ILLEGAL_ARGUMENT, WFD_PORT_GAMMA)");
            }
            break;
		default:
			LOG_ERROR(WFD_ERROR_BAD_ATTRIBUTE);
			SLOG_DEBUG2("       return: (WFD_ERROR_BAD_ATTRIBUTE: %08X)", attrib);
			break;
	}

    return;
}

WFD_API_CALL void WFD_APIENTRY
wfdBindPipelineToPort(WFDDevice wfd_device,
	WFDPort wfd_port, WFDPipeline pipeline) WFD_APIEXIT
{
	du_dev_t *dev = (du_dev_t *)wfd_device;
	port_t   *port = (port_t *)wfd_port;
	pipe_t   *pipe = (pipe_t *)pipeline;
	int it;

	TRACE;

	DEVICE_VALIDATE(return )
	PORT_VALIDATE(WFD_ERROR_BAD_HANDLE, return )
	PORT_CREATED(WFD_ERROR_BAD_HANDLE, return )
	PORT_ATTACHED(WFD_ERROR_NOT_SUPPORTED, return )
	PORT_VALIDATE_PIPELINE(WFD_ERROR_BAD_HANDLE, return)

	SLOG_DEBUG2("       device=%08X, port=%08X (id=%d), pipeline=%08X (id=%d)", wfd_device, wfd_port, port->portId,
	        pipeline,pipe->pipeId);

    if (port->bound_pipesSize >= RCARDU_MAX_NUMBER_PIPELINES) {
        LOG_ERROR(WFD_ERROR_OUT_OF_MEMORY);
        SLOG_DEBUG2("       return: (WFD_ERROR_OUT_OF_MEMORY, too many bound pipelines)");
        return;
    }

    for (it = 0; it < port->bound_pipesSize; it++) {
        if (port->bound_pipes[it] == pipe) {
            SLOG_DEBUG2("       return: (done, already bound)");
            return;
        }
    }

	LOCK_DEVICE();

	pipe->port = port;
	port->bound_pipes[port->bound_pipesSize++] = pipe;
    pipe->changes |= WFD_PIPELINE_CHANGES_BIND;
    port->changes |= WFD_PORT_CHANGES_PIPELINE;
    dev->changes |= WFD_DEVICE_CHANGES_PIPELINE;

	UNLOCK_DEVICE();

	SLOG_DEBUG2("       return: (done)");
    return;
}

WFD_API_CALL WFDint WFD_APIENTRY
wfdGetDisplayDataFormats(WFDDevice wfd_device, WFDPort wfd_port,
	WFDDisplayDataFormat *format, WFDint formatSize) WFD_APIEXIT
{
	du_dev_t	*dev = (du_dev_t *)wfd_device;
	port_t		*port = (port_t *)wfd_port;

	TRACE;
	SLOG_DEBUG2("       device=%08X, port=%08X, format=%p, formatSize=%d", wfd_device, wfd_port, format, formatSize);

	DEVICE_VALIDATE(return 0)
	PORT_VALIDATE(WFD_ERROR_BAD_HANDLE, return 0)
	PORT_CREATED(WFD_ERROR_BAD_HANDLE, return 0)
	PORT_ATTACHED(WFD_ERROR_NOT_SUPPORTED, return 0)

	if (format && formatSize <= 0) {
		LOG_ERROR(WFD_ERROR_ILLEGAL_ARGUMENT);
		SLOG_DEBUG2("       return: 0 (formatCount<=0)");
		return 0;
	}

	if (format != NULL) {
		*format = WFD_DISPLAY_DATA_FORMAT_NONE;
		SLOG_DEBUG2("       set: format[0]=WFD_DISPLAY_DATA_FORMAT_NONE");
	}

	SLOG_DEBUG2("       return: 1 (done)");
	return 1;
}

WFD_API_CALL WFDint WFD_APIENTRY
wfdGetDisplayData(WFDDevice wfd_device, WFDPort wfd_port,
	WFDDisplayDataFormat format, WFDuint8 *data, WFDint dataSize) WFD_APIEXIT
{
	du_dev_t	*dev = (du_dev_t *)wfd_device;
	port_t		*port = (port_t *)wfd_port;

	TRACE;
	SLOG_DEBUG2("       device=%08X, port=%08X, format=%08X, data=%p, dataSize=%d", wfd_device, wfd_port, format, data, dataSize);

	DEVICE_VALIDATE(return 0)
	PORT_VALIDATE(WFD_ERROR_BAD_HANDLE, return 0)
	PORT_CREATED(WFD_ERROR_BAD_HANDLE, return 0)
	PORT_ATTACHED(WFD_ERROR_NOT_SUPPORTED, return 0)

	if (data && dataSize <= 0) {
		LOG_ERROR(WFD_ERROR_ILLEGAL_ARGUMENT);
		SLOG_DEBUG2("       return: 0 (dataCount<=0)");
		return 0;
	}	

	if (format != WFD_DISPLAY_DATA_FORMAT_NONE) {
		LOG_ERROR(WFD_ERROR_ILLEGAL_ARGUMENT);
		SLOG_DEBUG2("       return: 0 (non supported format)");
		return 0;
	}

	SLOG_DEBUG2("       return: 0 (done)");
	return 0;
}


WFD_API_CALL WFDint WFD_APIENTRY
wfdWaitForVSyncQNX(WFDDevice wfd_device, WFDPort wfd_port)
{
	du_dev_t	*dev = (du_dev_t *)wfd_device;
	port_t		*port = (port_t *)wfd_port;

	DEVICE_VALIDATE(return 0)
	PORT_VALIDATE(WFD_ERROR_BAD_HANDLE, return 0)
	PORT_CREATED(WFD_ERROR_BAD_HANDLE, return 0)

	return rcardu_wait_vsync(dev, port);
}


int 
wfdCommitPortUpdates(du_dev_t *dev, port_t *port) 
{
	int	i = 0;
	int	wait_for_vsync = 0;

	TRACE;
	SLOG_DEBUG2("       port_id=%d. port->changes=0x%x", port->portId,port->changes);

	if (port->changes)
    {
        /* commit port changes */
        if (port->changes & WFD_PORT_CHANGES_MODE)
        {
            SLOG_DEBUG("WFD_PORT_CHANGES_MODE");
            SLOG_DEBUG2("    Setting real video mode ...");
            port_init(dev,port);
            port->changes &= ~WFD_PORT_CHANGES_MODE;
        }

        if (port->changes & WFD_PORT_CHANGES_PIPELINE){
            SLOG_DEBUG("WFD_PORT_CHANGES_PIPELINE");
            /* loop through pipelines looking for pipes
            associated with this port that are created */
            for (i = 0; i < dev->pipesSize; ++i){
                if (dev->pipes[i].port == port && dev->pipes[i].created){
                    /* got a change commit it */
                    wait_for_vsync |= wfdCommitPipelineUpdates(dev, &(dev->pipes[i]));
                }
            }
            port->changes &= ~WFD_PORT_CHANGES_PIPELINE;
        }

        if (port->changes & WFD_PORT_CHANGES_GAMMA) {
            SLOG_DEBUG("WFD_PORT_CHANGES_GAMMA");
            //TODO: Handle new gamma value at port->gamma
        }

        if ( port->changes & WFD_PORT_CHANGES_BACKGROUND_COLOR) {
            SLOG_DEBUG("WFD_PORT_CHANGES_BACKGROUND_COLOR");
            //TODO: Handle new background color value at port->bgcolor;
        }

        if (port->changes){
            SLOG_DEBUG("Unhandled WFD_PORT_CHANGES: %#x",port->changes);
        }
        port->changes = 0;
    }

	return wait_for_vsync;
}
