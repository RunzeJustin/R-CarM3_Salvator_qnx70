#ifndef _CAP_SLOTID_H_
#define _CAP_SLOTID_H_
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

 This file contains public interfaces (functions and types) for accessing SLOTID
 specific capability data.

 The API's are only available after a successful call to pci_device_read_cap()

 ===============================================================================
*/
#define CAPID_SLOTID		0x4

/*
 ===============================================================================
 Slot Identification PCI capability specific types
*/
typedef struct
{
	int_t (*get_chassis_num)(pci_cap_t cap);
	pci_err_t (*set_chassis_num)(pci_cap_t cap, uint8_t chassis_num);
	int_t (*get_num_slots)(pci_cap_t cap);
	pci_err_t (*is_parent_bridge)(pci_cap_t cap, bool_t *first_in_chassis);

} cap_slotid_api_t;

#define SLOTID_API_VERSION		{.major = 1, .minor = 0}

#define SLOTID_API_CALL(_api_, _cap_, args...) \
		do { \
			assert((_cap_) != NULL);	/* bogus 'pci_cap_t' */ \
			assert((_cap_)->api_p != NULL);		/* 'pci_cap_t' not read */ \
			assert(((cap_slotid_api_t *)((_cap_)->api_p))->_api_ != NULL);	/* module does not implement API */ \
			const pci_version_t api_version = SLOTID_API_VERSION; \
			(_cap_)->api_version = api_version; \
			return ((cap_slotid_api_t *)((_cap_)->api_p))->_api_(_cap_, ## args); \
		} while(0)

/*
 ===============================================================================
 Slot Identification PCI capability specific API's
*/

/* get/set the chassis number. For get, a value < 0 indicates an error */
static inline int_t cap_slotid_get_chassis_num(pci_cap_t cap)
{
	SLOTID_API_CALL(get_chassis_num, cap);
}

static inline pci_err_t cap_slotid_set_chassis_num(pci_cap_t cap, uint8_t chassis_num)
{
	SLOTID_API_CALL(set_chassis_num, cap, chassis_num);
}

/* retrieve the number of slots the bridge supports. A value < 0 indicates an error */
static inline int_t cap_slotid_get_num_slots(pci_cap_t cap)
{
	SLOTID_API_CALL(get_num_slots, cap);
}

/* first in chassis ? */
static inline pci_err_t cap_slotid_is_parent_bridge(pci_cap_t cap, bool_t *first_in_chassis)
{
	SLOTID_API_CALL(is_parent_bridge, cap, first_in_chassis);
}



#endif	/* _CAP_SLOTID_H_ */

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/services/pci/modules/capabilities/public/pci/cap_slotid.h $ $Rev: 798837 $")
#endif
