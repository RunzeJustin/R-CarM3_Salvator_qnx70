#ifndef _CAP_BR_SSVID_H_
#define _CAP_BR_SSVID_H_
/*
 * $QNXLicenseC:
 * Copyright (c) 2012, 2016 QNX Software Systems. All Rights Reserved.
 *
 * You must obtain a written license from and pay applicable license fees to QNX
 * Software Systems before you may reproduce, modify or distribute this software,
 * or any work that includes all or part of this software.   Free development
 * licenses are available for evaluation and non-commercial purposes.  For more
 * information visit http://licensing.qnx.com or email licensing@qnx.com.
 *
 * This file may contain contributions from others.  Please review this entire
 * file for other proprietary rights or license notices, as well as the QNX
 * Development Suite License Guide at http://licensing.qnx.com/license-guide/
 * for other information.
 * $
 */

#include <stdlib.h>
#include <dlfcn.h>
#include <assert.h>

#include <pci/pci.h>

/*
 ===============================================================================

 This file contains public interfaces (functions and types) for accessing
 BR_SSVID specific capability data.

 The API's are only available after a successful call to pci_device_read_cap()

 ===============================================================================
*/
#define CAPID_BR_SSVID		0xd

/*
 ===============================================================================
 Bridge Subsystem and Subsystem Vendor ID PCI capability specific types
*/
typedef struct
{
	pci_ssid_t (*get_ssid)(pci_cap_t cap);
	pci_ssvid_t (*get_ssvid)(pci_cap_t cap);

} cap_br_ssvid_api_t;

#define BR_SSVID_API_VERSION		{.major = 1, .minor = 0}

#define BR_SSVID_API_CALL(_api_, _cap_, args...) \
		do { \
			assert((_cap_) != NULL);	/* bogus 'pci_cap_t' */ \
			assert((_cap_)->api_p != NULL);		/* 'pci_cap_t' not read */ \
			assert(((cap_br_ssvid_api_t *)((_cap_)->api_p))->_api_ != NULL);	/* module does not implement API */ \
			const pci_version_t api_version = BR_SSVID_API_VERSION; \
			(_cap_)->api_version = api_version; \
			return ((cap_br_ssvid_api_t *)((_cap_)->api_p))->_api_(_cap_, ## args); \
		} while(0)

/*
 ===============================================================================
 Bridge Subsystem and Subsystem Vendor ID PCI capability specific API's
*/

/* get the Subsystem ID. A value of 0 indicates an error */
static inline pci_ssid_t cap_br_ssvid_get_ssid(pci_cap_t cap)
{
	BR_SSVID_API_CALL(get_ssid, cap);
}

/* get the Subsystem Vendor ID. A value of 0 indicates an error */
static inline pci_ssvid_t cap_br_ssvid_get_ssvid(pci_cap_t cap)
{
	BR_SSVID_API_CALL(get_ssvid, cap);
}


#endif	/* _CAP_BR_SSVID_H_ */

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/services/pci/modules/capabilities/public/pci/cap_br_ssvid.h $ $Rev: 798837 $")
#endif
