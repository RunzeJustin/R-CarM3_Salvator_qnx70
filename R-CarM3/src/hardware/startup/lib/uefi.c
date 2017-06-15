/*
 * $QNXLicenseC:
 * Copyright 2008, QNX Software Systems. 
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

#include "startup.h"
#include <hw/uefi.h>

EFI_HANDLE			efi_image_handle;
EFI_SYSTEM_TABLE	*efi_system_table;

void *
uefi_find_config_tbl(const EFI_GUID *const uid) {
	EFI_CONFIGURATION_TABLE	*tbl;
	unsigned				i;

	tbl = efi_system_table->ConfigurationTable;
	for(i = 0; i < efi_system_table->NumberOfTableEntries; ++i) {
		if(memcmp(&tbl->VendorGuid, uid, sizeof(*uid)) == 0) {
			return tbl->VendorTable;
		}
		++tbl;
	}
	return NULL;
}

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION( "$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/hardware/startup/lib/uefi.c $ $Rev: 780356 $" );
#endif
