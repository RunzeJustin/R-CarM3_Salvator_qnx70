/*
 * $QNXLicenseC:
 * Copyright 2014-2016, QNX Software Systems. 
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
#include <math.h>
#include <screen/screen.h>
#include <screen/iomsg.h>
#include "rcar_display.h"

WFD_API_CALL WFDint WFD_APIENTRY wfdEnumeratePipelines(WFDDevice device, WFDint *pipelineIds,
    WFDint pipelineIdsCount, const WFDint *filterList) WFD_APIEXIT
{
    du_dev_t*       dev=(du_dev_t *)device;
    WFDint              i=0;

    TRACE;
    SLOG_DEBUG2("       device=%08X, pipelineIds=%p, pipelineIdsCount=%d, filterList=%p", device, pipelineIds, pipelineIdsCount, filterList);

    DEVICE_VALIDATE(return 0)

    if (pipelineIds && pipelineIdsCount<=0)
    {
        LOG_ERROR(WFD_ERROR_ILLEGAL_ARGUMENT);
        SLOG_DEBUG2("       return: 0 (WFD_ERROR_ILLEGAL_ARGUMENT)");
        return 0;
    }

    if ((filterList) && (*filterList != WFD_NONE)) {
        LOG_ERROR(WFD_ERROR_ILLEGAL_ARGUMENT);
        SLOG_DEBUG2("       return: 0 (WFD_ERROR_ILLEGAL_ARGUMENT)");
        return 0;
    }

    if (pipelineIds)
    {
        for (i=0; (i<pipelineIdsCount) && (i<dev->pipesSize); i++)
        {
            pipelineIds[i]=dev->pipes[i].pipeId;
            SLOG_DEBUG2("       set: pipelineIds[%d]=%d ()", i, pipelineIds[i]);
        }
    }
    else
    {
        i=dev->pipesSize;
    }

    SLOG_DEBUG2("    Return %d pipelines count", i);
    return i;
}

WFD_API_CALL WFDPipeline WFD_APIENTRY
wfdCreatePipeline(WFDDevice device,
	WFDint pipelineId, const WFDint *attribList) WFD_APIEXIT
{
    du_dev_t* dev=(du_dev_t*)device;
    pipe_t*       pipe;

    TRACE;
    SLOG_DEBUG2("       device=%08X, pipelineId=%d, attribList=%p", device, pipelineId, attribList);

    DEVICE_VALIDATE(return 0)

    if ((attribList != NULL) && (*attribList != WFD_NONE)) {
        LOG_ERROR(WFD_ERROR_BAD_ATTRIBUTE);
        SLOG_DEBUG2("       return: WFD_INVALID_HANDLE (WFD_ERROR_BAD_ATTRIBUTE)");
        return WFD_INVALID_HANDLE;
    }

    LOCK_DEVICE();

    PIPE_FIND_BY_ID(WFD_ERROR_ILLEGAL_ARGUMENT, return WFD_INVALID_HANDLE)

	if (pipe->created) {
		LOG_ERROR_LOCKED(WFD_ERROR_IN_USE); //unlock_device inside
		SLOG_DEBUG2("       return: WFD_INVALID_HANDLE (WFD_ERROR_IN_USE)");
		return WFD_INVALID_HANDLE;
	}

    SLOG_DEBUG("%s: pipeId = %d", __FUNCTION__, pipe->pipeId);

	pipe->created = WFD_TRUE;
    
    scale_pipe_init(dev, pipe);
    
	UNLOCK_DEVICE();
	SLOG_DEBUG2("       return: %08X", (WFDPipeline)pipe);

	return (WFDPipeline)pipe;
}

WFD_API_CALL void WFD_APIENTRY
wfdDestroyPipeline(WFDDevice device, WFDPipeline pipeline) WFD_APIEXIT
{
	du_dev_t	*dev = (du_dev_t *)device;
	pipe_t		*pipe = (pipe_t *)pipeline;

	TRACE;
	SLOG_DEBUG2("       device=%08X, pipeline=%08X", device, pipeline);

	DEVICE_VALIDATE(return)
	PIPELINE_VALIDATE(WFD_ERROR_BAD_HANDLE, return)

	SLOG_DEBUG("%s: pipeId = %d", __FUNCTION__, pipe->pipeId);

	LOCK_DEVICE();

	if (pipe->src != WFD_INVALID_HANDLE) {
		pipe->src = WFD_INVALID_HANDLE;
		pipe->src_transition = WFD_TRANSITION_AT_VSYNC;
		pipe->changes |= WFD_PIPELINE_CHANGES_SOURCE;
		dev->changes |= WFD_DEVICE_CHANGES_PIPELINE;
	}

	if (pipe->port != WFD_INVALID_HANDLE) {
		pipe->port = WFD_INVALID_HANDLE;
		pipe->changes |= WFD_PIPELINE_CHANGES_BIND;
		dev->changes |= WFD_DEVICE_CHANGES_PIPELINE;
	}

	wfdCommitPipelineUpdates(dev, pipe);
	memset(pipe, 0, sizeof(*pipe));
    
    scale_pipe_quit(dev, pipe);

	UNLOCK_DEVICE();

	SLOG_DEBUG2("       return: (done)");
    return;
}

WFD_API_CALL void WFD_APIENTRY
wfdBindSourceToPipeline(WFDDevice device, WFDPipeline pipeline,
WFDSource source, WFDTransition transition,
	const WFDRect *region) WFD_APIEXIT
{
	du_dev_t	*dev = (du_dev_t *)device;
	pipe_t		*pipe = (pipe_t *)pipeline;
	source_t	*src = (source_t *)source;
	WFDint		width = 0, height = 0;

	TRACE;
	SLOG_DEBUG2("       device=%08X, pipeline=%08X, source=%08X, transition=%08X, region={%d, %d, %d, %d}", device, pipeline, source, transition,
        region==NULL ? 0 : region->offsetX, region==NULL ? 0 : region->offsetY, region==NULL ? 0 : region->width, region==NULL ? 0 : region->height);


	DEVICE_VALIDATE(return)
	PIPELINE_VALIDATE(WFD_ERROR_BAD_HANDLE, return)

	SLOG_DEBUG("%s: pipeId = %d", __FUNCTION__, pipe->pipeId);

	if (transition != WFD_TRANSITION_IMMEDIATE &&
		transition != WFD_TRANSITION_AT_VSYNC) {
		SLOG_ERROR("Invalid transition vsync and immediate");
		LOG_ERROR(WFD_ERROR_ILLEGAL_ARGUMENT);
		return;
	}

	LOCK_DEVICE();

	if (source != WFD_INVALID_HANDLE) {
		width = ((win_image_t *)src->image)->width;
		height = ((win_image_t *)src->image)->height;
	} 

	pipe->src = (WFDEGLImage)source;
	pipe->src_transition = transition;

	if (region) {
		if (region->offsetX >= 0 && region->offsetX < width) {
			pipe->region.offsetX = region->offsetX;
			if (region->offsetX + region->width < width) {
				pipe->region.width = region->width;
			} else {
				pipe->region.width = width - region->offsetX;
			}
		} else {
			pipe->region.offsetX = 0;
			pipe->region.width = 0;
		}
		if (region->offsetY >= 0 && region->offsetY < height) {
			pipe->region.offsetY = region->offsetY;
			if (region->offsetY + region->height < height) {
				pipe->region.height = region->height;
			} else {
				pipe->region.height = height - region->offsetY;
			}
		} else {
			pipe->region.offsetY = 0;
			pipe->region.height = 0;
		}
	} else {
		pipe->region.offsetX = 0;
		pipe->region.offsetY = 0;
		pipe->region.width = width;
		pipe->region.height = height;
	}

	if (pipe->src != pipe->bound_src) {
		pipe->changes |= WFD_PIPELINE_CHANGES_SOURCE;
		if (pipe->port != WFD_INVALID_HANDLE) {
			pipe->port->changes |= WFD_PORT_CHANGES_PIPELINE;
			dev->changes = 1;
		}
	} 
	else {
		pipe->changes &= ~(WFD_PIPELINE_CHANGES_SOURCE|WFD_PIPELINE_CHANGES_RECT);
	}

	UNLOCK_DEVICE();
}

WFD_API_CALL WFDint WFD_APIENTRY
wfdGetPipelineAttribi(WFDDevice device,
	WFDPipeline pipeline, WFDPipelineConfigAttrib attrib) WFD_APIEXIT
{
	WFDint      value = 0;
	TRACE;

	wfdGetPipelineAttribiv(device, pipeline, attrib, 1, &value);
	return value;
}

WFD_API_CALL WFDfloat WFD_APIENTRY
wfdGetPipelineAttribf(WFDDevice device,
	WFDPipeline pipeline, WFDPipelineConfigAttrib attrib) WFD_APIEXIT
{
	WFDfloat    value = 0.0f;
	TRACE;

	wfdGetPipelineAttribfv(device, pipeline, attrib, 1, &value);
	return value;
}

WFD_API_CALL void WFD_APIENTRY
wfdGetPipelineAttribiv(WFDDevice device, WFDPipeline pipeline,
	WFDPipelineConfigAttrib attrib, WFDint count, WFDint *value) WFD_APIEXIT
{
	du_dev_t	*dev = (du_dev_t *)device;
	pipe_t		*pipe = (pipe_t *)pipeline;
	TRACE;

	DEVICE_VALIDATE(return)
	PIPELINE_VALIDATE(WFD_ERROR_BAD_HANDLE, return)

	SLOG_DEBUG("%s: pipeId = %d", __FUNCTION__, pipe->pipeId);
	if (value == NULL || count <= 0 || count > 16) {
		LOG_ERROR(WFD_ERROR_ILLEGAL_ARGUMENT);
		return;
	}

	switch (attrib) {
		case WFD_PIPELINE_ID:
			if (count != 1) {
				LOG_ERROR(WFD_ERROR_ILLEGAL_ARGUMENT);
				return;
			}
			*value = pipe->pipeId;
			break;
		case WFD_PIPELINE_PORTID:
			if (count != 1) {
				LOG_ERROR(WFD_ERROR_ILLEGAL_ARGUMENT);
				return;
			};
			if (pipe->port == WFD_INVALID_HANDLE) {
				*value = WFD_INVALID_PORT_ID;
			} else {
				*value = 1;
			}
			break;
		case WFD_PIPELINE_LAYER:
			if (count != 1) {
				LOG_ERROR(WFD_ERROR_ILLEGAL_ARGUMENT);
				return;
			}
#if 0
			if (pipe->pipeId != TC_LAYER_PN_CUR && pipe->pipeId != MMP2_LAYER_TV_CUR) {
				*value = pipe->pipeId;
			} else {
				*value = WFD_INVALID_PIPELINE_LAYER;
			}
#else
			*value = pipe->pipeId;
#endif
			break;
		case WFD_PIPELINE_SHAREABLE:
			if (count != 1) {
				LOG_ERROR(WFD_ERROR_ILLEGAL_ARGUMENT);
				return;
			}
			*value = pipe->shareable;
			break;
		case WFD_PIPELINE_FLIP:
		case WFD_PIPELINE_MIRROR:
			if (count != 1) {
				LOG_ERROR(WFD_ERROR_ILLEGAL_ARGUMENT);
				return;
			}
			*value = WFD_FALSE;
			break;
		case WFD_PIPELINE_ROTATION_SUPPORT:
			if (count != 1) {
				LOG_ERROR(WFD_ERROR_ILLEGAL_ARGUMENT);
				return;
			}
			*value = WFD_ROTATION_SUPPORT_LIMITED;
			break;
		case WFD_PIPELINE_ROTATION:
			if (count != 1) {
				LOG_ERROR(WFD_ERROR_ILLEGAL_ARGUMENT);
				return;
			}
			*value = pipe->rotation;
			break;

		case WFD_PIPELINE_DIRECT_REFRESH:
			if (count != 1) {
				LOG_ERROR(WFD_ERROR_ILLEGAL_ARGUMENT);
				return;
			}
			*value = WFD_TRUE;
			break;
		case WFD_PIPELINE_MAX_SOURCE_SIZE:
			if (count != 2) {
				LOG_ERROR(WFD_ERROR_ILLEGAL_ARGUMENT);
				return;
			}
			value[0] = dev->max_width;
			value[1] = dev->max_height;
			break;
		case WFD_PIPELINE_SOURCE_RECTANGLE:
			if (count != 4) {
				LOG_ERROR(WFD_ERROR_ILLEGAL_ARGUMENT);
				return;
			}
			LOCK_DEVICE();
			value[0] = pipe->src_rect[0];
			value[1] = pipe->src_rect[1];
			value[2] = pipe->src_rect[2];
			value[3] = pipe->src_rect[3];
			UNLOCK_DEVICE();
			break;
		case WFD_PIPELINE_SCALE_FILTER:
			if (count != 1) {
				LOG_ERROR(WFD_ERROR_ILLEGAL_ARGUMENT);
				return;
			}
			*value = WFD_SCALE_FILTER_NONE;
			break;
		case WFD_PIPELINE_DESTINATION_RECTANGLE:
			if (count != 4) {
				LOG_ERROR(WFD_ERROR_ILLEGAL_ARGUMENT);
				return;
			}
			LOCK_DEVICE();	
			value[0] = pipe->dst_rect[0];
			value[1] = pipe->dst_rect[1];
			value[2] = pipe->dst_rect[2];
			value[3] = pipe->dst_rect[3];
			UNLOCK_DEVICE();
			break;
		case WFD_PIPELINE_TRANSPARENCY_ENABLE:
			if (count != 1) {
				LOG_ERROR(WFD_ERROR_ILLEGAL_ARGUMENT);
				return;
			}
			LOCK_DEVICE();	
			*value = pipe->transparency;
			UNLOCK_DEVICE();
			break;
		case WFD_PIPELINE_GLOBAL_ALPHA:
			if (count != 1) {
				LOG_ERROR(WFD_ERROR_ILLEGAL_ARGUMENT);
				return;
			}
			*value = 255;
			break;
		default:
			LOG_ERROR(WFD_ERROR_BAD_ATTRIBUTE);
			return;
	}
}

WFD_API_CALL void WFD_APIENTRY
wfdGetPipelineAttribfv(WFDDevice device, WFDPipeline pipeline,
	WFDPipelineConfigAttrib attrib, WFDint count, WFDfloat *value) WFD_APIEXIT
{
	du_dev_t	*dev = (du_dev_t *)device;
	pipe_t		*pipe = (pipe_t *)pipeline;
	port_t *port = pipe->port;

	TRACE;

	DEVICE_VALIDATE(return)
	PIPELINE_VALIDATE(WFD_ERROR_BAD_HANDLE, return)

	SLOG_DEBUG("%s: pipeId = %d", __FUNCTION__, pipe->pipeId);

	switch (attrib) {
		case WFD_PIPELINE_SCALE_RANGE:
			if (count != 2) {
				LOG_ERROR(WFD_ERROR_ILLEGAL_ARGUMENT);
				return;
			}
			if (pipe->pipeId == port->du_cfg->scale_pipe)
			{
				value[0] = (float)1/(float)DOWN_SCALE_MAX;
				value[1] = (float)UP_SCALE_MAX;
			}
			else
			{
				value[0] = NO_SCALE;
				value[1] = NO_SCALE;
			}
			break;
		default:
			LOG_ERROR(WFD_ERROR_BAD_ATTRIBUTE);
			return;
	}
}

WFD_API_CALL void WFD_APIENTRY
wfdSetPipelineAttribi(WFDDevice device, WFDPipeline pipeline,
	WFDPipelineConfigAttrib attrib, WFDint value) WFD_APIEXIT
{
	TRACE;
	wfdSetPipelineAttribiv(device, pipeline, attrib, 1, &value);
}

WFD_API_CALL void WFD_APIENTRY
wfdSetPipelineAttribf(WFDDevice device, WFDPipeline pipeline,
	WFDPipelineConfigAttrib attrib, WFDfloat value) WFD_APIEXIT
{
	TRACE;
	wfdSetPipelineAttribfv(device, pipeline, attrib, 1, &value);
}

WFD_API_CALL void WFD_APIENTRY
wfdSetPipelineAttribiv(WFDDevice device, WFDPipeline pipeline,
WFDPipelineConfigAttrib attrib, WFDint count,
	const WFDint *value) WFD_APIEXIT
{
	du_dev_t	*dev = (du_dev_t *)device;
	pipe_t		*pipe = (pipe_t *)pipeline;
	TRACE;

	DEVICE_VALIDATE(return)
	PIPELINE_VALIDATE(WFD_ERROR_BAD_HANDLE, return)

	SLOG_DEBUG("%s: pipeId = %d", __FUNCTION__, pipe->pipeId);

	switch (attrib) {
		case WFD_PIPELINE_SOURCE_RECTANGLE:
			if (count >= 4) {
				LOCK_DEVICE();
				if (pipe->src_rect[0] != value[0] || pipe->src_rect[1] != value[1] ||
				    pipe->src_rect[2] != value[2] || pipe->src_rect[3] != value[3]) {
					pipe->src_rect[0] = value[0];
					pipe->src_rect[1] = value[1];
					pipe->src_rect[2] = value[2];
					pipe->src_rect[3] = value[3];
					pipe->changes |= WFD_PIPELINE_CHANGES_SOURCE_RECTANGLE;
				}
				UNLOCK_DEVICE();
			}
			break;
	case WFD_PIPELINE_FLIP:
	case WFD_PIPELINE_MIRROR:
		if (count >= 1) {
			if (*value) {
				LOG_ERROR(WFD_ERROR_ILLEGAL_ARGUMENT);
			}
		}
		break;
	case WFD_PIPELINE_ROTATION:
		if (count >= 1) {
			if (*value) {
				LOG_ERROR(WFD_ERROR_ILLEGAL_ARGUMENT);
			}
		}
		break;
	case WFD_PIPELINE_SCALE_FILTER:
		if (count >= 1) {
			if (*value != WFD_SCALE_FILTER_NONE) {
				LOG_ERROR(WFD_ERROR_ILLEGAL_ARGUMENT);
			}
		}
		break;
	case WFD_PIPELINE_DESTINATION_RECTANGLE:
		if (count >= 4) {
			LOCK_DEVICE();
			if (pipe->dst_rect[0] != value[0] || pipe->dst_rect[1] != value[1] ||
				pipe->dst_rect[2] != value[2] || pipe->dst_rect[3] != value[3]) {
				pipe->dst_rect[0] = value[0];
				pipe->dst_rect[1] = value[1];
				pipe->dst_rect[2] = value[2];
				pipe->dst_rect[3] = value[3];
				pipe->changes |= WFD_PIPELINE_CHANGES_DESTINATION_RECTANGLE;
			}
			UNLOCK_DEVICE();
		}
		break;
	case WFD_PIPELINE_TRANSPARENCY_ENABLE:
		if (count >= 1) {
			if (pipe->pipeId != pipe->port->du_cfg->scale_pipe) {
				switch (*value) {
					case WFD_TRANSPARENCY_SOURCE_ALPHA:
					case WFD_TRANSPARENCY_SOURCE_COLOR:
					case WFD_TRANSPARENCY_NONE:
						LOCK_DEVICE();
						pipe->transparency = *value;
						UNLOCK_DEVICE();
						break;
					default:
						LOG_ERROR(WFD_ERROR_ILLEGAL_ARGUMENT);
						break;
				}
			} else {
				if (*value != WFD_TRANSPARENCY_NONE) {
					LOG_ERROR(WFD_ERROR_ILLEGAL_ARGUMENT);
					break;
				}
				LOCK_DEVICE();
				pipe->transparency = *value;
				UNLOCK_DEVICE();				
			}
		}
		break;
	case WFD_PIPELINE_GLOBAL_ALPHA:
		if (count >= 1) {
			if (*value >= 256) {
				LOG_ERROR(WFD_ERROR_ILLEGAL_ARGUMENT);
			} else {
				LOCK_DEVICE();
				pipe->global_alpha_active = 1;
				pipe->global_alpha = *value & 0xff;
				UNLOCK_DEVICE();
			}
		}
		break;
	default:
		LOG_ERROR(WFD_ERROR_BAD_ATTRIBUTE);
		break;
	}
}


WFD_API_CALL void WFD_APIENTRY
wfdSetPipelineAttribfv(WFDDevice device, WFDPipeline pipeline,
WFDPipelineConfigAttrib attrib, WFDint count,
	const WFDfloat *value) WFD_APIEXIT
{
	du_dev_t	*dev = (du_dev_t *)device;
	pipe_t		*pipe = (pipe_t *)pipeline;
	int		i;
	TRACE;

	if (!dev) {
		return;
	}

	SLOG_DEBUG("%s: pipeId = %d", __FUNCTION__, pipe->pipeId);

	for (i=0; i < dev->pipesSize; ++i) {
		if (pipe == &dev->pipes[i]) {
			if (!pipe->created) {
				SLOG_ERROR("Pipe line not created");
				LOG_ERROR(WFD_ERROR_BAD_HANDLE);
				return;
			}
			break;
		}
	}
	if (i == dev->pipesSize) {
		SLOG_ERROR("Not pipeline/pipe");
		LOG_ERROR(WFD_ERROR_BAD_HANDLE);
		return;
	}

	if (value == NULL || count <= 0) {
		LOG_ERROR(WFD_ERROR_ILLEGAL_ARGUMENT);
		return;
	}

	LOG_ERROR(WFD_ERROR_BAD_ATTRIBUTE);
}

WFD_API_CALL WFDint WFD_APIENTRY
wfdGetPipelineTransparency(WFDDevice device, WFDPipeline pipeline,
	WFDbitfield *trans, WFDint transSize) WFD_APIEXIT
{
	du_dev_t	*dev = (du_dev_t *)device;
	pipe_t		*pipe = (pipe_t *)pipeline;
	int		i, rc = 0;

	TRACE;

	SLOG_DEBUG("%s: pipeId = %d", __FUNCTION__, pipe->pipeId);

	if (!dev) {
		return 0;
	}

	for (i=0; i < dev->pipesSize; ++i) {
		if (pipe == &dev->pipes[i]) {
			if (!pipe->created) {
				SLOG_ERROR("Pipeline not created");
				LOG_ERROR(WFD_ERROR_BAD_HANDLE);
			return 0;
			}
			break;
		}
	}
	if (i == dev->pipesSize) {
		SLOG_ERROR("Not pipeline/pipe");
		LOG_ERROR(WFD_ERROR_BAD_HANDLE);
		return 0;
	}

	if (trans && transSize <= 0) {
		LOG_ERROR(WFD_ERROR_BAD_HANDLE);
		return 0;
	}

	if (trans) {
		trans[0] = WFD_TRANSPARENCY_NONE;
		trans[1] = WFD_TRANSPARENCY_SOURCE_COLOR;
		trans[2] = WFD_TRANSPARENCY_GLOBAL_ALPHA;
		trans[3] = WFD_TRANSPARENCY_SOURCE_ALPHA;
		rc = 4;
	}

	return rc;
}

WFD_API_CALL void WFD_APIENTRY
wfdSetPipelineTSColor(WFDDevice device, WFDPipeline pipeline,
	WFDTSColorFormat colorFormat, WFDint count, const void *color) WFD_APIEXIT
{
	du_dev_t *dev = (du_dev_t *)device;
	pipe_t     *pipe = (pipe_t *)pipeline;
	int         i;
	TRACE;

	if (!dev) {
		return;
	}

	SLOG_DEBUG("%s: pipeId = %d", __FUNCTION__, pipe->pipeId);

	for (i=0; i < dev->pipesSize; ++i) {
		if (pipe == &dev->pipes[i]) {
			if (!pipe->created) {
				SLOG_ERROR("Pipe line not created");
				LOG_ERROR(WFD_ERROR_BAD_HANDLE);
				return;
			}
			break;
		}
	}
	if (i == dev->pipesSize) {
		SLOG_ERROR("Not pipeline/pipe");
		LOG_ERROR(WFD_ERROR_BAD_HANDLE);
		return;
	}

	LOG_ERROR(WFD_ERROR_NOT_SUPPORTED);
}

WFD_API_CALL void WFD_APIENTRY
wfdBindMaskToPipeline(WFDDevice device, WFDPipeline pipeline,
	WFDMask mask, WFDTransition transition) WFD_APIEXIT
{
	du_dev_t	*dev = (du_dev_t *)device;
	pipe_t		*pipe = (pipe_t *)pipeline;
	int		i;
	TRACE;

	if (!dev) {
		return;
	}

	SLOG_DEBUG("%s: pipeId = %d", __FUNCTION__, pipe->pipeId);

	for (i=0; i < dev->pipesSize; ++i) {
		if (pipe == &dev->pipes[i]) {
			if (!pipe->created) {
				SLOG_ERROR("Pipe line not created");
				LOG_ERROR(WFD_ERROR_BAD_HANDLE);
				return;
			}
			break;
		}
	}
	if (i == dev->pipesSize) {
		SLOG_ERROR("Not pipeline/pipe");
		LOG_ERROR(WFD_ERROR_BAD_HANDLE);
		return;
	}

	LOG_ERROR(WFD_ERROR_NOT_SUPPORTED);
}

WFD_API_CALL WFDint WFD_APIENTRY
wfdGetPipelineLayerOrder(WFDDevice device,
	WFDPort port, WFDPipeline pipeline) WFD_APIEXIT
{
	du_dev_t	*dev = (du_dev_t *)device;
	pipe_t		*pipe = (pipe_t *)pipeline;
	TRACE;

	if (!dev) {
		return WFD_INVALID_PIPELINE_LAYER;
	}
	PIPELINE_VALIDATE(WFD_ERROR_BAD_HANDLE, return WFD_INVALID_PIPELINE_LAYER);

	return pipe->layer;
}

int
wfdCommitPipelineUpdates(du_dev_t *dev, pipe_t *pipe)
{
	compose_hdl *hw_compose_hdl;
    int dst_width = pipe->dst_rect[2];
    int dst_height = pipe->dst_rect[3];

    source_t *source = NULL;
	port_t *port = pipe->port;
	
    TRACE;
    SLOG_DEBUG2("       dev=%08X, pipe=%08X", dev, pipe);

    if (!port)
    {
    	SLOG_DEBUG("commit pipeline: no port");
        return 0;
    }
	hw_compose_hdl = port->du_cfg->hw_compose;

	SLOG_DEBUG("%s: pipeId = %d", __FUNCTION__, pipe->pipeId);

    if (!IS_INCLUDED(pipe->pipeId, port->du_cfg->pipe_ids, port->du_cfg->pipe_num))
    {
        SLOG_DEBUG2("       return (invalid pipeline ID: %d)", pipe->pipeId);
        return 0;
    }

    if ((!dst_width) || (!dst_height))
    {
        SLOG_DEBUG2("       return (no size)");
        hw_compose_hdl->deactivate_pipeline(dev,pipe);
        return 0;
    }

    source = (source_t *)pipe->src;
    if (!source)
    {
    	SLOG_DEBUG2("       return (no source)");
		pipe->bound_src = 0;
		hw_compose_hdl->deactivate_pipeline(dev,pipe);
        return 0;
    }

	hw_compose_hdl->activate_pipeline(dev,pipe);
	if (hw_compose_hdl->frame_update)
		hw_compose_hdl->frame_update(dev,pipe);
	
	pipe->bound_src = pipe->src;
	pipe->bound_port = pipe->port;
	pipe->changes = 0;

	SLOG_DEBUG2("       return: %s",
		pipe->src_transition == WFD_TRANSITION_AT_VSYNC ? "WFD_TRANSITION_AT_VSYNC":"OTHER");
	return (pipe->src_transition == WFD_TRANSITION_AT_VSYNC);
}
