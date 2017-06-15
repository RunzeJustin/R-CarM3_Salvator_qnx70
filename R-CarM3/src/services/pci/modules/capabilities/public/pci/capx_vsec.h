#ifndef _CAPX_VSEC_H_
#define _CAPX_VSEC_H_
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
#include <pci/cap_pcie.h>

/*
 ===============================================================================

 This file contains public interfaces (functions and types) for generically
 accessing VSEC specific capability data.

 The API's are only available after a successful call to cap_pcie_read_xtnd_cap()

 Vendor specific API's, types and structure definitions are contained within a
 separate (vendor specific) capx_vsec_<vend>.h file

 ===============================================================================
*/
#define CAPID_PCIe_VSEC		0x000B

/*
 ===============================================================================
 Generic Vendor Specific Extended PCIe capability types

 FIX ME

 Test the get_offset API

*/
typedef struct
{
	int_t (*version)(pcie_cap_t cap);
	int_t (*get_id)(pcie_cap_t cap);
	int_t (*get_revision)(pcie_cap_t cap);
	int_t (*get_num_bytes)(pcie_cap_t cap);
	int_t (*get_offset)(pcie_cap_t cap);
	pci_err_t (*read_bytes)(pcie_cap_t cap, const uint_t num_bytes, uint8_t *buf);

} capx_vsec_api_t;

#define VSEC_API_VERSION		{.major = 1, .minor = 0}

#define VSEC_API_CALL(_api_, _cap_, args...) \
		do { \
			assert((_cap_) != NULL);	/* bogus 'pcie_cap_t' */ \
			assert((_cap_)->api_p != NULL);		/* 'pcie_cap_t' not read */ \
			assert(((capx_vsec_api_t *)((_cap_)->api_p))->_api_ != NULL);	/* module does not implement API */ \
			const pci_version_t api_version = VSEC_API_VERSION; \
			(_cap_)->api_version = api_version; \
			return ((capx_vsec_api_t *)((_cap_)->api_p))->_api_(_cap_, ## args); \
		} while(0)

/*
 ===============================================================================
 Vendor Specific PCIe extended capability API's
*/

/* return the PCISIG assigned VSEC capability version. A value of < 0 indicates an error */
static inline int_t capx_vsec_version(pcie_cap_t cap)
{
	VSEC_API_CALL(version, cap);
}

/* get the vendor specific capability ID. A value < 0 indicates an error */
static inline int_t capx_vsec_get_id(pcie_cap_t cap)
{
	VSEC_API_CALL(get_id, cap);
}

/* get the vendor specific capability revision. A value < 0 indicates an error */
static inline int_t capx_vsec_get_revision(pcie_cap_t cap)
{
	VSEC_API_CALL(get_revision, cap);
}

/* get the number of vendor specific bytes in the capability. A value < 0 indicates an error */
static inline int_t capx_vsec_get_num_bytes(pcie_cap_t cap)
{
	VSEC_API_CALL(get_num_bytes, cap);
}

/*
 * get the offset of the vendor specific bytes so that the registers can be read
 * and written with pci-device_cfg_rd*()/pci_device_cfg_wr*() respectively.
 * A value < 0 indicates an error
*/
static inline int_t capx_vsec_get_offset(pcie_cap_t cap)
{
	VSEC_API_CALL(get_offset, cap);
}

/* read <num_bytes> into the caller allocated storage pointed to by <buf> */
static inline pci_err_t capx_vsec_read_bytes(pcie_cap_t cap, const uint_t num_bytes, uint8_t *buf)
{
	VSEC_API_CALL(read_bytes, cap, num_bytes, buf);
}



#endif	/* _CAPX_VSEC_H_ */

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/services/pci/modules/capabilities/public/pci/capx_vsec.h $ $Rev: 798837 $")
#endif
