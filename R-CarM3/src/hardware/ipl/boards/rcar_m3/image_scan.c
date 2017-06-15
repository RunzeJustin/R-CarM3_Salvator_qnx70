/*
 * $QNXLicenseC: 
 * Copyright 2016, QNX Software Systems.
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

#include "ipl.h"

/*
 * We offer an alternative to use these evolved image_xxxx_2 functions because of
 * 1. It's not necessary to do checksum() on scanning image. e.g. It's way faster for XIP device 
 *    to do checksum after it has been loaded to memory
 * 2. It's uncommon nowadays to store two version of IFS in the same media
 * 3. It's not necessary to copy the startup header at every image_xxx() function 
 * 4. Also, we may not need to move startup code if it's already in place
 */
extern void mem_copy(uint32_t prgStartAd, uint32_t sector_Ad, uint32_t imagesize);

unsigned long image_scan (unsigned long start, unsigned long end) 
{
	struct startup_header *hdr;
	/* 
	 * image starts on word boundary
	 * We need this scan because it could have 8 raw bytes in front of imagefs
	 * depending on how the IFS is programmed
	 */
	for (; start < end; start += 4) {
		hdr = (struct startup_header *)(start);

		/* No endian issues here since stored "naturally" */
		if (hdr->signature == STARTUP_HDR_SIGNATURE)
			break;
	}
	if (start >= end)
	{
		return (-1L);
	}

	mem_copy(&startup_hdr, start , sizeof(startup_hdr));
	/* now we got the image signature */
	return (start);
}

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/6.6.0/trunk/hardware/ipl/lib/image_2.c $ $Rev: 723412 $")
#endif
