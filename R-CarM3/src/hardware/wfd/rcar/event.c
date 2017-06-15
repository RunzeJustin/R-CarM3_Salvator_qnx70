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

#include <stdlib.h>
#include "rcar_display.h"

WFD_API_CALL WFDEvent WFD_APIENTRY
wfdCreateEvent(WFDDevice device, const WFDint *attribList) WFD_APIEXIT
{
	du_dev_t *dev = (du_dev_t *)device;
	int i = 0;
	TRACE;

	DEVICE_VALIDATE(return 0)
	for ( ; i < dev->eventSize; i++) {
		if (dev->events[i].created == 0) {
			dev->events[i].created = 1;
			return i+1;
		}
	}
	return 0;
}

WFD_API_CALL void WFD_APIENTRY
wfdDestroyEvent(WFDDevice device, WFDEvent event) WFD_APIEXIT
{
	du_dev_t *dev = (du_dev_t *)device;
	int evt_idx = (int)event -1;
	TRACE;

	DEVICE_VALIDATE(return )
	dev->events[evt_idx].created = 0;
}

WFD_API_CALL WFDint WFD_APIENTRY
wfdGetEventAttribi(WFDDevice device,
    WFDEvent event, WFDEventAttrib attrib) WFD_APIEXIT
{
	du_dev_t *dev = (du_dev_t *)device;
	int evt_idx = (int)event -1;
	TRACE;
	DEVICE_VALIDATE(return 0)
	switch(attrib) {
		case WFD_EVENT_PIPELINE_BIND_QUEUE_SIZE:
			break;
		case WFD_EVENT_TYPE:
			return dev->events[evt_idx].type;
		case WFD_EVENT_PORT_ATTACH_PORT_ID:
			return dev->events[evt_idx].attached_portId;
		case WFD_EVENT_PORT_ATTACH_STATE:
			return dev->events[evt_idx].attached_state;
		case WFD_EVENT_PORT_PROTECTION_PORT_ID:
		case WFD_EVENT_PIPELINE_BIND_PIPELINE_ID:
		case WFD_EVENT_PIPELINE_BIND_SOURCE:
		case WFD_EVENT_PIPELINE_BIND_MASK:
		case WFD_EVENT_PIPELINE_BIND_QUEUE_OVERFLOW:
			break;
		default:
			LOG_ERROR(WFD_ERROR_BAD_ATTRIBUTE);
		return 0;
	}

	return 0;
}

WFD_API_CALL void WFD_APIENTRY
wfdDeviceEventAsync(WFDDevice device, WFDEvent event,
    WFDEGLDisplay display, WFDEGLSync sync) WFD_APIEXIT
{
	TRACE;
}

WFD_API_CALL WFDEventType WFD_APIENTRY
wfdDeviceEventWait(WFDDevice device,
    WFDEvent event, WFDtime timeout) WFD_APIEXIT
{
	TRACE;
	return WFD_INVALID_HANDLE;
}

WFD_API_CALL void WFD_APIENTRY
wfdDeviceEventFilter(WFDDevice device,
    WFDEvent event, const WFDEventType *filter) WFD_APIEXIT
{
	TRACE;
}
