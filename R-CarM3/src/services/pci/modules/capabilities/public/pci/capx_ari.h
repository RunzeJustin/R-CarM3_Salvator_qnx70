#ifndef _CAPX_ARI_H_
#define _CAPX_ARI_H_
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
 Device Extended Alternate Routing ID Interpretation specific capability data.

 The API's are only available after a successful call to cap_pcie_read_xtnd_cap()

 ===============================================================================
*/
#define CAPID_PCIe_ARI		0x000E

/*
 ===============================================================================
 ARI PCIe extended capability specific types
*/

typedef struct
{
	int_t (*version)(pcie_cap_t cap);
	pci_err_t (*group_mfvc_enable)(pcie_cap_t cap, pci_devhdl_t hdl);
	pci_err_t (*group_mfvc_disable)(pcie_cap_t cap, pci_devhdl_t hdl);
	pci_err_t (*group_acs_enable)(pcie_cap_t cap, pci_devhdl_t hdl);
	pci_err_t (*group_acs_disable)(pcie_cap_t cap, pci_devhdl_t hdl);
	int_t (*group_get)(pcie_cap_t cap);
	pci_err_t (*group_set)(pcie_cap_t cap, pci_devhdl_t hdl, uint_t group_num);
	pci_bdf_t (*next_func)(pcie_cap_t cap, pcie_cap_t *cap_next);

} capx_ari_api_t;

#define ARI_API_VERSION		{.major = 1, .minor = 0}

#define ARI_API_CALL(_api_, _cap_, args...) \
		do { \
			assert((_cap_) != NULL);	/* bogus 'pci_cap_t' */ \
			assert((_cap_)->api_p != NULL);		/* 'pci_cap_t' not read */ \
			assert(((capx_ari_api_t *)((_cap_)->api_p))->_api_ != NULL);	/* module does not implement API */ \
			const pci_version_t api_version = ARI_API_VERSION; \
			(_cap_)->api_version = api_version; \
			return ((capx_ari_api_t *)((_cap_)->api_p))->_api_(_cap_, ## args); \
		} while(0)

/*
 ===============================================================================
 ARI PCIe extended capability specific API's
*/

/* return the ARI capability version. A value of < 0 indicates an error */
static inline int_t capx_ari_version(pcie_cap_t cap)
{
	ARI_API_CALL(version, cap);
}

/* enable MFVC function groups. <cap> must be for function 0 */
static inline pci_err_t capx_group_mfvc_enable(pci_devhdl_t hdl, pcie_cap_t cap)
{
	ARI_API_CALL(group_mfvc_enable, cap, hdl);
}
/* disable MFVC function groups. <cap> must be for function 0 */
static inline pci_err_t capx_group_mfvc_disable(pci_devhdl_t hdl, pcie_cap_t cap)
{
	ARI_API_CALL(group_mfvc_disable, cap, hdl);
}

/* enable ACS function groups. <cap> must be for function 0 */
static inline pci_err_t capx_group_acs_enable(pci_devhdl_t hdl, pcie_cap_t cap)
{
	ARI_API_CALL(group_acs_enable, cap, hdl);
}
/* disable ACS function groups. <cap> must be for function 0 */
static inline pci_err_t capx_group_acs_disable(pci_devhdl_t hdl, pcie_cap_t cap)
{
	ARI_API_CALL(group_acs_disable, cap, hdl);
}

/* get the group number for the function associated with <cap>. -1 indicates an error */
static inline int_t capx_ari_group_get(pcie_cap_t cap)
{
	ARI_API_CALL(group_get, cap);
}
/* set the group number for the function associated with <cap> */
static inline pci_err_t capx_ari_group_set(pci_devhdl_t hdl, pcie_cap_t cap, uint_t group_num)
{
	ARI_API_CALL(group_set, cap, hdl, group_num);
}

/* get the 'pci_bdf_t' for the next implemented ARI function. The first call
 * should pass the ARI <cap> for function 0. If <cap_next> != NULL, it can be
 * passed as <cap> in subsequent calls in order to retrieve the BDF for all
 * implemented functions. The caller is responsible for free()'ing <cap_next>
 * (if its not NULL) when they are done with it */
static inline pci_bdf_t capx_ari_next_func(pcie_cap_t cap, pcie_cap_t *cap_next)
{
	ARI_API_CALL(next_func, cap, cap_next);
}



#endif	/* _CAPX_ARI_H_ */

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/services/pci/modules/capabilities/public/pci/capx_ari.h $ $Rev: 798837 $")
#endif
