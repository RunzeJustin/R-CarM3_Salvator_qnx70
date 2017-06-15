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
#include <arm/inout.h>
#include <arm/r-car.h>

extern void init_serscif(unsigned address, unsigned baud, unsigned clk);
extern void mem_copy(uint32_t prgStartAd, uint32_t sector_Ad, uint32_t imagesize);
struct startup_header	startup_hdr;

#define FLASH_IMAGE_ADDR  0x8740000
#define IMAGE_SCAN_SIZE     51200

static int rcar_boot_hyperflash(uint32_t addr)
{
    uint32_t    imageaddr, offset, ramaddr;
    void (*exec)();
    /* Get startup header */
    mem_copy(addr, FLASH_IMAGE_ADDR , IMAGE_SCAN_SIZE);
    // Just check the signature, no checksum
    imageaddr = image_scan(addr, addr + IMAGE_SCAN_SIZE);
    if (imageaddr == -1)
        return imageaddr;
    ser_putstr("Found image        @ 0x");
    ser_puthex(imageaddr);
    ser_putstr("\n");

    offset = imageaddr - addr;

    ramaddr = startup_hdr.ram_paddr + startup_hdr.paddr_bias - offset;
    mem_copy(ramaddr, FLASH_IMAGE_ADDR, startup_hdr.stored_size + offset);

    // Now do a full startup scan
    if ((imageaddr = image_scan(ramaddr, ramaddr + IMAGE_SCAN_SIZE)) == -1)
        return -1;

    ser_putstr("Jumping to startup @ 0x");
    ser_puthex(startup_hdr.startup_vaddr);
    ser_putstr("\n\n");
	if (offset == 0)
	{
		exec =  (void *)(intptr_t)startup_hdr.startup_vaddr;
	}
	else
	{
		exec =  (void *)(intptr_t)ramaddr;
	}
    exec();
    return 0;
}

int main(void)
{
	unsigned    image;
	
    init_serscif(RCAR_SCIF2_BASE, 115200, 14745600);

	ser_putstr("\n----------------------------------------------------------------\n");
	ser_putstr("| QNX Neutrino 7.0 IPL on the R-CarM3 (ARM Cortex-A57/A53 core) |\n");
    ser_putstr("----------------------------------------------------------------\n");

        image = 0x41000000;
		ser_putstr("load image from HyperFlash\n");
		rcar_boot_hyperflash(image);
		ser_putstr("load HyperFlash failed!\n");
        image = image_scan(image, image + 0x1000);
        if (image != 0xffffffff)  {
            ser_putstr("Image scan failed!\n");
        }

    return 0;
}

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL$ $Rev$")
#endif
