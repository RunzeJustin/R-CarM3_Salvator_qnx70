#ifndef _CAP_SATACFG_H_
#define _CAP_SATACFG_H_
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

 This file contains public interfaces (functions and types) for accessing SATA
 Index/Data Configuration specific capability data.

 The API's are only available after a successful call to pci_device_read_cap()

 ===============================================================================
*/
#define CAPID_SATACFG		0x12

/*
 ===============================================================================
 Sata Configuration PCI capability specific types
*/
typedef struct
{
	int_t (*get_bar_num)(pci_cap_t cap);
	int_t (*get_bar_offset)(pci_cap_t cap);
	pci_ba_val_t (*get_addr)(pci_cap_t cap, pci_devhdl_t hdl);
	pci_version_t (*get_version)(pci_cap_t cap);

} cap_satacfg_api_t;

#define SATACFG_API_VERSION		{.major = 1, .minor = 0}

#define SATACFG_API_CALL(_api_, _cap_, args...) \
		do { \
			assert((_cap_) != NULL);	/* bogus 'pci_cap_t' */ \
			assert((_cap_)->api_p != NULL);		/* 'pci_cap_t' not read */ \
			assert(((cap_satacfg_api_t *)((_cap_)->api_p))->_api_ != NULL);	/* module does not implement API */ \
			const pci_version_t api_version = SATACFG_API_VERSION; \
			(_cap_)->api_version = api_version; \
			return ((cap_satacfg_api_t *)((_cap_)->api_p))->_api_(_cap_, ## args); \
		} while(0)

/*
 ===============================================================================
 SATA Configuration PCI capability specific API's
*/

/* get the bar number that the Index/Data pair reside in */
static inline pci_version_t cap_satacfg_get_version(pci_cap_t cap)
{
	SATACFG_API_CALL(get_version, cap);
}

/* get the bar number that the Index/Data pair reside in */
static inline int_t cap_satacfg_get_bar_num(pci_cap_t cap)
{
	SATACFG_API_CALL(get_bar_num, cap);
}

/* get the offset within the bar region that the Index/Data pair reside at */
static inline pci_err_t cap_satacfg_get_bar_offset(pci_cap_t cap)
{
	SATACFG_API_CALL(get_bar_offset, cap);
}

/*
 * get the address of the Index/Data pair. This function is a convenience function
 * for the user which will read the BAR information and obtain the final address
 * of the Index/Data pair
*/
static inline int_t cap_satacfg_get_addr(pci_devhdl_t hdl, pci_cap_t cap)
{
	SATACFG_API_CALL(get_addr, cap, hdl);
}




#endif	/* _CAP_SATACFG_H_ */

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/services/pci/modules/capabilities/public/pci/cap_satacfg.h $ $Rev: 798837 $")
#endif
