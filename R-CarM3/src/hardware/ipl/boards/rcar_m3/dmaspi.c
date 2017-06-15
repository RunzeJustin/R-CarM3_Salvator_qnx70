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
#include <stdint.h>
#include <arm/r-car.h>
#include <hw/inout.h>

void InitDma01_Data(uint32_t prgStartAd, uint32_t sector_Ad, uint32_t accessCount)
{
	uint32_t base = RCAR_SYSDMAC0_BASE + RCAR_SYSDMAC_REGS;
	//DMA Setting
	out32(base + RCAR_SYSDMAC_DMASAR,  sector_Ad);		//	RPC area
	out32(base + RCAR_SYSDMAC_DMADAR,  prgStartAd);		//	
	out32(base + RCAR_SYSDMAC_DMATCR,  accessCount);		//
	out32(base + RCAR_SYSDMAC_DMACHCR,  0x00105409);		//64Byte/AutoRequest mode

}

void StartDma01(void)
{
	out16(RCAR_SYSDMAC0_BASE + RCAR_SYSDMAC_DMAOR, 0x0301);
}

uint32_t WaitDma01(void)
{
	uint32_t dataL=0;
	uint32_t base = RCAR_SYSDMAC0_BASE + RCAR_SYSDMAC_REGS;
	////////////////////////////////
	// DMA transfer complite check
	////////////////////////////////
	while(1){
		dataL = in32(base + RCAR_SYSDMAC_DMACHCR);
		if(dataL & (1 << 1)){
			out32(base + RCAR_SYSDMAC_DMACHCR, in32(base + RCAR_SYSDMAC_DMACHCR) & ~(1 << 1));		// TE Clear
			break;
		}
		if(dataL & (1 << 31)){
			out32(base + RCAR_SYSDMAC_DMACHCR, in32(base + RCAR_SYSDMAC_DMACHCR) & ~(1 << 31));		// CAE Clear
			return(1);
		}
	}
	out16(RCAR_SYSDMAC0_BASE + RCAR_SYSDMAC_DMAOR, 0x0000);				//0: Disables DMA transfers on all channels
	
	return(0);
}

void DisableDma01(void)
{
	uint32_t base = RCAR_SYSDMAC0_BASE + RCAR_SYSDMAC_REGS;
	
	out32(base + RCAR_SYSDMAC_DMACHCR, in32(base + RCAR_SYSDMAC_DMACHCR) & 0x00105410);			//64Byte/AutoRequest mode
}

void ClearDmaCh01(void)
{
	out32(RCAR_SYSDMAC0_BASE + RCAR_SYSDMAC_DMACHCLR, in32(RCAR_SYSDMAC0_BASE + RCAR_SYSDMAC_DMACHCLR) | (1 << 1));
}

void mem_copy(uint32_t prgStartAd, uint32_t sector_Ad, uint32_t imagesize)
{
	uint32_t paddingOffset=0;
	uint32_t accessCount=0;
	
	//calculate padding size
	paddingOffset = (imagesize + 0x3F ) & ~0x3F;
	
	//accessCount = imagesize/64;
	accessCount = paddingOffset >> 6;
	
	
	//DMA Setting
	InitDma01_Data(prgStartAd, sector_Ad, accessCount);

	StartDma01();
	
	WaitDma01();

	DisableDma01();

	ClearDmaCh01();
}

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL$ $Rev$")
#endif