/*
 * $QNXLicenseC: 
 * Copyright 2014, QNX Software Systems.  
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

#include "genmac.h"
#include "variant.h"

#define PAGE_SIZE	(0x1000)
#define MAC_SIZE	(6)	// MAC Address is always 6 bytes

int main (int argc, char *argv[])
{
	uint64_t	macaddr;
	chip_uid_info	chipuid;

	if ((argc != 1) && ((argc != 2) || strcmp(argv[1], "-m")))
	{
		fprintf(stderr, "%s: Usage\n", argv[0]);
		return 1;
	}

	/*
	 * Map a page of memory using the physical address specified by the variant
	 */
#ifndef ID_BASE_ADDR
#error ID_BASE_ADDR must be specified in order to mmap memory
#endif
	chipuid.id_base = mmap_device_memory(NULL, PAGE_SIZE, PROT_READ|PROT_NOCACHE, 0, ID_BASE_ADDR);

	if (chipuid.id_base == MAP_FAILED)
	{
		fprintf(stderr, "%s: Unable to mmap memory, exiting...\n", argv[0]);
		return EXIT_FAILURE;
	}

	if (!get_uid(&chipuid))
	{
		fprintf(stderr, "%s: Unable to determine Unique ID, exiting...\n", argv[0]);
		free_uid_resources(&chipuid);
		return EXIT_FAILURE;
	}

	int i;
	if (argc == 1)
	{
		printf("%s", "Unique ID: 0x");
		for (i=(chipuid.uid_size-1); i>=0; i--)
		{
			printf("%02hhx", chipuid.uid[i]);
		}
		printf("\n");
	}
	else
	{
		macaddr = 0;

		/*
		 * Copy the UID into the MAC address
		 * If the UID contains less bytes than the MAC address the bytes that don't get copied will be zero.
		 */
		for (i=0; i<MAC_SIZE; i++)
		{
			macaddr |= chipuid.uid[i] << (8*i);
		}

		/*
		 * XOR any additional bytes
		 */
		if (chipuid.uid_size > MAC_SIZE)
		{
			for (i=MAC_SIZE; i<chipuid.uid_size; i++)
			{
				macaddr ^= chipuid.uid[i] << (8*(i % MAC_SIZE));
			}
		}

		/* Clear multicast bit, set admin */
		macaddr &= 0xFEFFFFFFFFFFLL;
		macaddr |= 0x020000000000LL;
		printf("%012llx\n", macaddr);
	}

	free_uid_resources(&chipuid);
	return EXIT_SUCCESS;
}

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/hardware/support/genmac/genmac.c $ $Rev: 742582 $")
#endif
