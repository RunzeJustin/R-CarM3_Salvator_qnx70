#ifndef _CAP_VEND_H_
#define _CAP_VEND_H_
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

 This file contains public interfaces (functions and types) for generically
 accessing VEND specific capability data.

 The API's are only available after a successful call to pci_device_read_cap()

 Vendor specific API's, types and structure definitions are contained within a
 separate (vendor specific) cap_vend_<vend>.h file

 ===============================================================================
*/
#define CAPID_VEND		0x9

/*
 ===============================================================================
 Generic Vendor Specific PCI capability types

*/
typedef struct
{
	int_t (*get_num_bytes)(pci_cap_t cap);
	int_t (*get_offset)(pci_cap_t cap);
	pci_err_t (*read_bytes)(pci_cap_t cap, const uint_t num_bytes, uint8_t *buf);

} cap_vend_api_t;

#define VEND_API_VERSION		{.major = 1, .minor = 0}

#define VEND_API_CALL(_api_, _cap_, args...) \
		do { \
			assert((_cap_) != NULL);	/* bogus 'pci_cap_t' */ \
			assert((_cap_)->api_p != NULL);		/* 'pci_cap_t' not read */ \
			assert(((cap_vend_api_t *)((_cap_)->api_p))->_api_ != NULL);	/* module does not implement API */ \
			const pci_version_t api_version = VEND_API_VERSION; \
			(_cap_)->api_version = api_version; \
			return ((cap_vend_api_t *)((_cap_)->api_p))->_api_(_cap_, ## args); \
		} while(0)

/*
 ===============================================================================
 Generic Vendor Specific PCI capability API's
*/

/* get the number of vendor specific bytes in the capability. A value < 0 indicates an error */
static inline int_t cap_vend_get_num_bytes(pci_cap_t cap)
{
	VEND_API_CALL(get_num_bytes, cap);
}

/*
 * get the offset of the vendor specific bytes so that the registers can be read
 * and written with pci-device_cfg_rd*()/pci_device_cfg_wr*() respectively.
 * A value < 0 indicates an error
*/
static inline int_t cap_vend_get_offset(pci_cap_t cap)
{
	VEND_API_CALL(get_offset, cap);
}

/* read <num_bytes> into the caller allocated storage pointed to by <buf> */
static inline pci_err_t cap_vend_read_bytes(pci_cap_t cap, const uint_t num_bytes, uint8_t *buf)
{
	VEND_API_CALL(read_bytes, cap, num_bytes, buf);
}



#endif	/* _CAP_VEND_H_ */

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/services/pci/modules/capabilities/public/pci/cap_vend.h $ $Rev: 798837 $")
#endif
