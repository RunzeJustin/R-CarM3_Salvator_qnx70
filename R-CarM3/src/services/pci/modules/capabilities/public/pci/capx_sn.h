#ifndef _CAPX_SN_H_
#define _CAPX_SN_H_
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

#include <stdint.h>
#include <inttypes.h>

#include <pci/pci.h>
#include <pci/cap_pcie.h>

/*
 ===============================================================================

 This file contains public interfaces (functions and types) for accessing PCIe
 Device Extended Serial Number specific capability data.

 The API's are only available after a successful call to pci_device_read_cap()

 ===============================================================================
*/
#define CAPID_PCIe_SN		0x3

/*
 ===============================================================================
 Device Serial Number PCIe extended capability specific types
*/
typedef uint64_t	capx_sn_sernum_t;

typedef struct
{
	int_t (*version)(pcie_cap_t cap);
	capx_sn_sernum_t (*read_sernum)(pcie_cap_t cap);

} capx_sn_api_t;

#define SN_API_VERSION		{.major = 1, .minor = 0}

#define SN_API_CALL(_api_, _cap_, args...) \
		do { \
			assert((_cap_) != NULL);	/* bogus 'pci_cap_t' */ \
			assert((_cap_)->api_p != NULL);		/* 'pci_cap_t' not read */ \
			assert(((capx_sn_api_t *)((_cap_)->api_p))->_api_ != NULL);	/* module does not implement API */ \
			const pci_version_t api_version = SN_API_VERSION; \
			(_cap_)->api_version = api_version; \
			return ((capx_sn_api_t *)((_cap_)->api_p))->_api_(_cap_, ## args); \
		} while(0)
/*
 ===============================================================================
 Device Serial Number PCIe extended capability specific API's
*/

/* return the Device Serial Number capability version. A value of < 0 indicates an error */
static inline int_t capx_sn_version(pcie_cap_t cap)
{
	SN_API_CALL(version, cap);
}

static inline capx_sn_sernum_t capx_sn_read_sernum(pcie_cap_t cap)
{
	SN_API_CALL(read_sernum, cap);
}



#endif	/* _CAPX_SN_H_ */

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/services/pci/modules/capabilities/public/pci/capx_sn.h $ $Rev: 798837 $")
#endif
