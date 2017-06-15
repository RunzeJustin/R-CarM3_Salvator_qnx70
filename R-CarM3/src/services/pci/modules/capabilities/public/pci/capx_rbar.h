#ifndef _CAPX_RBAR_H_
#define _CAPX_RBAR_H_
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
 Resizable BAR specific capability data.

 The API's are only available after a successful call to cap_pcie_read_xtnd_capid()

 ===============================================================================
*/
#define CAPID_PCIe_RBAR		0x0015

/*
 ===============================================================================
 Resizable BAR PCIe extended capability specific types
*/

typedef struct
{
	int_t (*version)(pcie_cap_t cap);
	pci_err_t (*get_supported_sizes)(pcie_cap_t cap, const uint_t bar_num, uint32_t *size_vector);
	pci_err_t (*get_selected_size)(pcie_cap_t cap, const uint_t bar_num, pci_ba_val_t *size);
	pci_err_t (*set_selected_size)(pcie_cap_t cap, pci_devhdl_t hdl, const uint_t bar_num, const pci_ba_val_t size);

} capx_rbar_api_t;

#define RBAR_API_VERSION		{.major = 1, .minor = 0}

#define RBAR_API_CALL(_api_, _cap_, args...) \
		do { \
			assert((_cap_) != NULL);	/* bogus 'pci_cap_t' */ \
			assert((_cap_)->api_p != NULL);		/* 'pci_cap_t' not read */ \
			assert(((capx_rbar_api_t *)((_cap_)->api_p))->_api_ != NULL);	/* module does not implement API */ \
			const pci_version_t api_version = RBAR_API_VERSION; \
			(_cap_)->api_version = api_version; \
			return ((capx_rbar_api_t *)((_cap_)->api_p))->_api_(_cap_, ## args); \
		} while(0)
/*
 ===============================================================================
 Resizable BAR PCIe extended capability specific API's
*/

/* return the RBAR capability version. A value of < 0 indicates an error */
static inline int_t capx_rbar_version(pcie_cap_t cap)
{
	RBAR_API_CALL(version, cap);
}

static inline pci_err_t capx_rbar_get_supported_sizes(pcie_cap_t cap, const uint_t bar_num, uint32_t *size_vector)
{
	RBAR_API_CALL(get_supported_sizes, cap, bar_num, size_vector);
}

/* get the currently selected BAR size for <bar_num> */
static inline pci_err_t capx_rbar_get_selected_size(pcie_cap_t cap, const uint_t bar_num, pci_ba_val_t *size)
{
	RBAR_API_CALL(get_selected_size, cap, bar_num, size);
}

/* set the currently selected BAR size for <bar_num> */
static inline pci_err_t capx_rbar_set_selected_size(pci_devhdl_t hdl, pcie_cap_t cap, const uint_t bar_num, const pci_ba_val_t size)
{
	RBAR_API_CALL(set_selected_size, cap, hdl, bar_num, size);
}


#endif	/* _CAPX_RBAR_H_ */

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/services/pci/modules/capabilities/public/pci/capx_rbar.h $ $Rev: 798837 $")
#endif
