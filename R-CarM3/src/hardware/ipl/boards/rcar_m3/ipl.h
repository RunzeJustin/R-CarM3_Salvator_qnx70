/*
 * $QNXLicenseC: 
 * Copyright 2007, 2008, QNX Software Systems.  
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



#ifndef __IPL_H_INCLUDED
#define __IPL_H_INCLUDED

#include <sys/platform.h>
#include <sys/startup.h>
#include <stdint.h>
/*Global definitions for some space that we share in the IPL code*/

extern struct startup_header	startup_hdr;

void copy (unsigned long dst, unsigned long src, unsigned long size);
extern unsigned long image_scan(unsigned long start, unsigned long end);

typedef struct _ser_dev_t {
	unsigned char	(*get_byte)(void);
	void			(*put_byte)(unsigned char);
	unsigned char	(*poll)(void);
} ser_dev;

unsigned scif_base;

#endif

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/6.6.0/trunk/hardware/ipl/lib/ipl.h $ $Rev: 787997 $")
#endif
