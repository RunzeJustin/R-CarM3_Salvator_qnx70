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

#ifndef MACRO_H_
#define MACRO_H_

/* helper macros for device and port validation */
#define DEVICE_VALIDATE(ret)  if (!dev || dev->hdr.magic != DEVICE_MAGIC) {\
		DC_ERROR("!dev || dev->hdr.magic != DEVICE_MAGIC fail");\
        ret;\
}

#define PORT_CREATED(error, ret)  if (!port->created) { \
		DC_ERROR("port not created"); \
    	LOG_ERROR(error); \
    	ret; \
}

#define PORT_ATTACHED(error, ret)  if (!port->attached) { \
		DC_ERROR("port not attached"); \
    	LOG_ERROR(error); \
    	ret; \
}

#define PORTID_VALIDATE(error, ret) { \
	int i =0; \
	if (portId == WFD_INVALID_PORT_ID) { \
		DC_ERROR("portId is WFD_INVALID_PORT_ID"); \
		LOG_ERROR(error); \
		ret; \
	} \
	for (; i < dev->portsSize; ++i) { \
		if (portId == dev->ports[i].portId) { \
			break; \
		} \
	} \
	if (i >= dev->portsSize) { \
		DC_ERROR("port invalid portId"); \
		LOG_ERROR(error); \
		ret; \
	} \
}


#define PORT_FIND_BY_ID(error, ret) { \
	int i =0; \
	for (; i < dev->portsSize; ++i) { \
		if (portId == dev->ports[i].portId) { \
			break; \
		} \
	} \
	if (i >= dev->portsSize) { \
		DC_ERROR("port invalid portId"); \
		UNLOCK_DEVICE();\
		LOG_ERROR(error); \
		ret; \
	} \
	port = &(dev->ports[i]); \
}


#define PORT_VALIDATE(error, ret) { \
	int i =0; \
	for (; i < dev->portsSize; ++i) { \
		if (port == &(dev->ports[i]) && port->portId != WFD_INVALID_PORT_ID) { \
			break; \
		} \
	} \
	if (i >= dev->portsSize) { \
		DC_ERROR("port invalid"); \
		LOG_ERROR(error); \
		ret; \
	} \
}

#define MODE_VALIDATE(error, ret) { \
	int i =0; \
	for (; i < port->modesSize; ++i) { \
		if (mode == &(port->modes[i])) { \
			break; \
		} \
	} \
	if (i >= port->modesSize) { \
		DC_ERROR("mode invalid"); \
		LOG_ERROR(error); \
		ret; \
	} \
}


#define PIPE_FIND_BY_ID(error, ret) { \
	int i =0; \
	for (; i < dev->pipesSize; ++i) { \
		if (pipelineId == dev->pipes[i].pipeId) { \
			break; \
		} \
	} \
	if (i >= dev->pipesSize) { \
		DC_ERROR("pipe invalid portId"); \
		UNLOCK_DEVICE(); \
		LOG_ERROR(error); \
		ret; \
	} \
	pipe = &(dev->pipes[i]); \
}

#define PORT_VALIDATE_PIPELINE(error, ret) { \
	int i =0; \
	for (; i < port->bindablesSize; ++i) { \
		if (pipe->pipeId == (WFDHandle)port->bindables[i]) { \
			break; \
		} \
	} \
	if (i >= port->bindablesSize) { \
		DC_ERROR("pipe not valid for port"); \
		LOG_ERROR(error); \
		ret; \
	} \
}


#define PIPELINE_CREATED(error, ret)  if (!pipeline->created) { \
		DC_ERROR("pipe not created"); \
    	LOG_ERROR(error); \
    	ret; \
}

#define PIPELINE_VALIDATE(error, ret) {\
	int i = 0;\
	for (; i < dev->pipesSize; ++i) {\
		if (pipe == &dev->pipes[i]) {\
			if (!pipe->created) {\
				DC_ERROR("Pipe line not created");\
				LOG_ERROR(error);\
				ret ;\
			}\
			/* found */\
			break;\
		}\
	}\
	if (i == dev->pipesSize) {\
		DC_ERROR("Not pipeline/layer");\
		LOG_ERROR(error);\
		ret ;\
	}\
}

#define SOURCE_VALIDATE(ret)  if (!src || src->hdr.magic != SOURCE_MAGIC) {\
        DC_ERROR("SOURCE_MAGIC fail");\
        ret; \
}

#define DEST_VALIDATE(ret)  if (!dest || dest->hdr.magic != DEST_MAGIC) {\
        DC_ERROR("DEST_MAGIC fail");\
        ret; \
}

#endif /* MACRO_H_ */
