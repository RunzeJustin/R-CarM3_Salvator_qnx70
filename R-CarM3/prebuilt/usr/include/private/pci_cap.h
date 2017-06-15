#ifndef _PCI_CAP_H_
#define _PCI_CAP_H_
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

#include <pci/pci.h>
#include <pci/cap_pcie.h>

#include "private/pci_mod.h"


/*
 * some or all of the fields of the 'cap_mod_params_t' structure are required
 * depending on the API function being called
 */
struct cap_mod_access_s;
typedef struct
{
	pid_t pid;
	pci_devhdl_t hdl;
	pci_reqType_t reqType;
	pci_bdf_t bdf;
	struct cap_mod_access_s *access;
} cap_mod_params_t;

typedef struct cap_mod_access_s
{
	uint_t struct_size;
	pci_version_t (*mod_version)(void);
	bool_t (*mod_compat)(version_typecheck_e check_type, pci_version_t version);

	pci_err_t (*read_cap)(cap_mod_params_t *, uint_t cap_ptr, pci_cap_t *cap);
	/*
	 * re (*enable_cap)() and (*disable_cap)() function pointers. Either NONE or
	 * BOTH of these function pointers should be initialized. If a capability
	 * cannot be disabled (ie. it is permanently enabled like PCIe), then BOTH the
	 * (*enable_cap)() and (*disable_cap)() function pointers should be NULL. If an
	 * (*enable_cap)() is provided, then a (*disable_cap)() should also be provided
	 * and it should return PCI_ERR_OK even if it does nothing. Otherwise the
	 * 'cap_enabled_entry_t' associated with the 'bdf_entry_t' when enable is
	 * successfully called, will never be released
	 *
	 * (*isenabled_cap)() MUST be implemented if (*enable_cap)() != NULL. It is
	 * called in the context of the calling process (ie. no message pass)
	*/
	pci_err_t (*enable_cap)(cap_mod_params_t *cap_mod_params, pci_cap_t cap);
	pci_err_t (*disable_cap)(cap_mod_params_t *cap_mod_params, pci_cap_t cap);
	pci_err_t (*isenabled_cap)(cap_mod_params_t *cap_mod_params, pci_cap_t cap, bool_t *enabled);

	void *public_api;	// pointer to the public API functions for the capability module
} cap_mod_access_t;


typedef enum
{
	capid_type_e_NONE = 0,
	capid_type_e_PCI,
	capid_type_e_PCIe,

} capid_type_e;

typedef struct __attribute__((packed, aligned(4)))
{
	uint8_t reserved;
	uint8_t type;		// capid_type_e
	uint16_t val;		// large enough for both a pci_capid_t and a pcie_capid_t
} __capid_t;
typedef uint32_t _capid_t;

#define CAPID_TYPE(_capid_)					(((__capid_t *)&(_capid_))->type)
#define CAPID_VAL(_capid_)					(((__capid_t *)&(_capid_))->val)
#define CAPID_ENCODE(_type_, _val_)			capid_encode((_type_), (_val_))
static inline _capid_t capid_encode(capid_type_e type, uint16_t val)
{
	__capid_t capid = {.reserved = 0, .type = type, .val = val};
	return *((_capid_t *)&capid);
}

pci_err_t cap_mod_read(const pci_bdf_t bdf, const _capid_t capid, uint_t cap_ptr, pci_cap_t *cap);
cap_mod_access_t *cap_mod_find(const pci_bdf_t bdf, const _capid_t capid, pci_err_t *err);

/*
 ===============================================================================
 All capability modules provide an initialized function pointer table of
 type 'cap_mod_access_t' and named CAP_MODULE_ACCESS. The symbol can be found
 by the PCI library using the search string CAP_MODULE_ACCESS_SYM.

 Additionally, these modules may also provide an initialization function named
 CAP_MODULE_INITFN which can be found by the PCI library using the search string
 CAP_MODULE_INITFN_SYM.

 The PCI server modload_cap() function will lookup CAP_MODULE_INITFN_SYM first
 and if found call that function. It expects the 'cap_mod_access_t **' parameter
 passed in to be initialized on successful return.

 If CAP_MODULE_INITFN_SYM does not exist (it is optional) or it fails for some
 reason, then the symbol CAP_MODULE_ACCESS_SYM is searched for in order to gain
 access to the module API's.

 IMPORTANT: These symbol names can never change if backward compatibility is to
 	 	 	be maintained
*/
#define STRINGX(s)	STRING(s)
#define STRING(s)	#s

#define CAP_MODULE_ACCESS		cap_mod_access
#define CAP_MODULE_INITFN		cap_mod_init

#define CAP_MODULE_ACCESS_SYM	STRINGX(CAP_MODULE_ACCESS)
#define CAP_MODULE_INITFN_SYM	STRINGX(CAP_MODULE_INITFN)



#endif	/* _PCI_CAP_H_ */

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/lib/pci/private/pci_cap.h $ $Rev: 798837 $")
#endif
