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
 
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <atomic.h>
#include <errno.h>
#include <sys/neutrino.h>
#include <string.h>
#include <stdint.h>
#include <sys/mman.h>
#include <hw/inout.h>
#include "rcar_display.h"

#define		LVDCR0 			0xFEB90000
#define		LVDCR1 			0xFEB90004
#define		LVDPLLCR 		0xFEB90008
#define		LVDCTRCR 		0xFEB9000C
#define		LVDCHCR  		0xFEB90010

#define LVDCR0_DUSEL			(1 << 15)
#define LVDCR0_DMD			(1 << 12)
#define LVDCR0_LVMD_MASK		(0xf << 8)
#define LVDCR0_LVMD_SHIFT		8
#define LVDCR0_PLLEN			(1 << 4)
#define LVDCR0_BEN			(1 << 2)
#define LVDCR0_LVEN			(1 << 1)
#define LVDCR0_LVRES			(1 << 0)
/* Gen3 */
#define LVDCR0_PLLON			(1 << 4)
#define LVDCR0_PWD			(1 << 2)

#define LVDCR1_CKSEL			(1 << 15)
#define LVDCR1_CHSTBY(n)		(3 << (2 + (n) * 2))
#define LVDCR1_CLKSTBY			(3 << 0)

#define LVDPLLCR_CEEN			(1 << 14)
#define LVDPLLCR_FBEN			(1 << 13)
#define LVDPLLCR_COSEL			(1 << 12)
#define LVDPLLCR_PLLDLYCNT_150M		(0x1bf << 0)
#define LVDPLLCR_PLLDLYCNT_121M		(0x22c << 0)
#define LVDPLLCR_PLLDLYCNT_60M		(0x77b << 0)
#define LVDPLLCR_PLLDLYCNT_38M		(0x69a << 0)
#define LVDPLLCR_PLLDLYCNT_MASK		(0x7ff << 0)
/* Gen3 */
#define LVDPLLCR_PLLDLYCNT_42M		(0x14cb << 0)
#define LVDPLLCR_PLLDLYCNT_85M		(0x0a45 << 0)
#define LVDPLLCR_PLLDLYCNT_128M		(0x06c3 << 0)
#define LVDPLLCR_PLLDLYCNT_148M		(0x46c1 << 0)

#define LVDCTRCR_CTR3SEL_ZERO		(0 << 12)
#define LVDCTRCR_CTR3SEL_ODD		(1 << 12)
#define LVDCTRCR_CTR3SEL_CDE		(2 << 12)
#define LVDCTRCR_CTR3SEL_MASK		(7 << 12)
#define LVDCTRCR_CTR2SEL_DISP		(0 << 8)
#define LVDCTRCR_CTR2SEL_ODD		(1 << 8)
#define LVDCTRCR_CTR2SEL_CDE		(2 << 8)
#define LVDCTRCR_CTR2SEL_HSYNC		(3 << 8)
#define LVDCTRCR_CTR2SEL_VSYNC		(4 << 8)
#define LVDCTRCR_CTR2SEL_MASK		(7 << 8)
#define LVDCTRCR_CTR1SEL_VSYNC		(0 << 4)
#define LVDCTRCR_CTR1SEL_DISP		(1 << 4)
#define LVDCTRCR_CTR1SEL_ODD		(2 << 4)
#define LVDCTRCR_CTR1SEL_CDE		(3 << 4)
#define LVDCTRCR_CTR1SEL_HSYNC		(4 << 4)
#define LVDCTRCR_CTR1SEL_MASK		(7 << 4)
#define LVDCTRCR_CTR0SEL_HSYNC		(0 << 0)
#define LVDCTRCR_CTR0SEL_VSYNC		(1 << 0)
#define LVDCTRCR_CTR0SEL_DISP		(2 << 0)
#define LVDCTRCR_CTR0SEL_ODD		(3 << 0)
#define LVDCTRCR_CTR0SEL_CDE		(4 << 0)
#define LVDCTRCR_CTR0SEL_MASK		(7 << 0)

#define LVDCHCR_CHSEL_CH(n, c)		((((c) - (n)) & 3) << ((n) * 4))
#define LVDCHCR_CHSEL_MASK(n)		(3 << ((n) * 4))

int lvds_init(void *arg)
{
	port_t *port = (port_t *)arg;
	uintptr_t LVDPLLCR_val    =   (uintptr_t )MAP_DEVICE_FAILED;
	uintptr_t LVDCTRCR_val    =   (uintptr_t )MAP_DEVICE_FAILED;
	uintptr_t LVDCR0_val   	  =   (uintptr_t )MAP_DEVICE_FAILED;
	uintptr_t LVDCR1_val   	  =   (uintptr_t )MAP_DEVICE_FAILED;
	uintptr_t LVDCHCR_val     =   (uintptr_t )MAP_DEVICE_FAILED;
	
	uint32_t pllcr;
	uint32_t lvdcr0;
	int pixel_clk = (int)port->pixel_clk/1000;
	
	LVDPLLCR_val    = mmap_device_io(4 ,LVDPLLCR);
	LVDCTRCR_val    = mmap_device_io(4 ,LVDCTRCR);
	LVDCR0_val    	= mmap_device_io(4 ,LVDCR0);
	LVDCR1_val    	= mmap_device_io(4 ,LVDCR1);
	LVDCHCR_val    	= mmap_device_io(4 ,LVDCHCR);
	
	if ((pixel_clk >= 25175) && (pixel_clk < 42000))
		pllcr = LVDPLLCR_PLLDLYCNT_42M;
	else if (pixel_clk < 85000)
		pllcr = LVDPLLCR_PLLDLYCNT_85M;
	else if (pixel_clk < 128000)
		pllcr = LVDPLLCR_PLLDLYCNT_128M;
	else if (pixel_clk < 148500)
		pllcr = LVDPLLCR_PLLDLYCNT_148M;
	else {
		return -1;
	}
	
	out32(LVDPLLCR_val, pllcr);
	
	/* Hardcode the channels and control signals routing for now.
	 *
	 * HSYNC -> CTRL0
	 * VSYNC -> CTRL1
	 * DISP  -> CTRL2
	 * 0     -> CTRL3
	 *
	 */
	out32(LVDCTRCR_val, LVDCTRCR_CTR3SEL_ZERO |
			LVDCTRCR_CTR2SEL_DISP | LVDCTRCR_CTR1SEL_VSYNC |
			LVDCTRCR_CTR0SEL_HSYNC);	
			
	out32(LVDCHCR_val,
			LVDCHCR_CHSEL_CH(0, 0) | LVDCHCR_CHSEL_CH(1, 1) |
			LVDCHCR_CHSEL_CH(2, 2) | LVDCHCR_CHSEL_CH(3, 3)
		);

	/* Select the input, hardcode mode 0, enable LVDS operation and turn
	 * bias circuitry on.
	 */
	lvdcr0 = LVDCR0_PLLON;
	out32(LVDCR0_val, lvdcr0);

	lvdcr0 |= LVDCR0_PWD;
	out32(LVDCR0_val, lvdcr0);
	
	usleep(150);
	
	lvdcr0 |= LVDCR0_LVRES;
	out32(LVDCR0_val, lvdcr0);
	
	/* Turn all the channels on. */
	out32(LVDCR1_val, 0x00000155);
	
	delay(10);
	return 0;
}
