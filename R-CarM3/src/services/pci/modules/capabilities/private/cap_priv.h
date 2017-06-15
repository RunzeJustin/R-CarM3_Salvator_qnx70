#ifndef _CAP_PRIV_H_
#define _CAP_PRIV_H_
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

#include <stddef.h>

#include <pci/pci.h>

#include "private/pci_cap.h"
#include "private/pci_lib.h"

/*
 ===============================================================================

 This file contains private types and macros common to all capability modules

 ===============================================================================
*/


/*
 ===============================================================================
 pci_cap_priv_t

 This type is the internal representation of a 'pci_cap_t' and is used by all
 capability modules. The 'cap_specific' field is the capability specific portion
 of the structure and it is allocated along with the 'pci_cap_priv_t' by each
 module

*/
typedef struct
{
	/* these first 2 fields are a copy of the 'pci_cap_t' and must be first */
	pci_version_t api_version;
	const void *api_p;		// pointer to the module specific API's

	uint_t struct_size;	// the total size of this structure including capability specific section
	pci_bdf_t bdf;		// the unique device this capability is associated with
	_capid_t capid;		// the capability ID this structure refers to
	uint16_t cap_ptr;	// the PCI/PCIe configuration space offset to this capability
	cap_mod_access_t *access;	// access functions to the capability module
	uint32_t reserved;	// for future

	void *cap_specific;	// capability specific storage (must be last field of structure)
} pci_cap_priv_t;

/*
 ===============================================================================
 CAP_PRIV_INIT

 'pci_cap_priv_t' sructure initialization macro

*/
#define CAP_PRIV_INIT(_cp_, _mod_params_, _id_, _ptr_, _sz_extra_) \
		do { \
			(_cp_)->struct_size = sizeof(pci_cap_priv_t) + (_sz_extra_); \
			(_cp_)->bdf = (_mod_params_)->bdf; \
			(_cp_)->cap_ptr = (_ptr_); \
			(_cp_)->capid = (_id_); \
			(_cp_)->access = (_mod_params_)->access; \
			(_cp_)->api_p = (_cp_)->access->public_api; \
			(_cp_)->reserved = 0; \
			(_cp_)->cap_specific = &(_cp_)[1]; \
		} while(0)

/*
 ===============================================================================
 VALID_CAP_PRIV

 The following define is used (especially by the API routines) to validate an
 incoming 'pci_cap_t' received from the user for PCIe AER extended capabilities

 It is intended to be used as follows

 pci_cap_privt_t *cap_priv = (pci_cap_priv_t *)cap;

 if (!VALID_CAP_PRIV(cap_priv, <hdl>, capid_type_e, pci_capid_t))
 {
     <error handling>
 }

 _cap_ is a 'pci_cap_priv_t *' (which is a cast from a 'pci_cap_t *')
 _capid_type_ is a 'capid_type_e' (private/pci_cap.h)
 _capid_val_ is a uint16_t to accommodate both a 'pci_capid_t' and a 'pcie_capid_t'
 _hdl_ is optional and may be NULL. If not NULL, the _hdl_->bdf is checked against
 the _cap_->bdf and the _hdl_->coid is checked against 0

 Implementation Note

 The __check_hdl() function is used to allow NULL to be passed in to
 VALID_CAP_PRIV(). Without the function, the compiler does not like the use of
 the PCI_DEVHDL_DECODE_*() macros with a NULL (which it forces evaluation of
 with a "test ? true : false;" statement
*/
static inline bool_t __check_hdl(pci_devhdl_t hdl, pci_bdf_t bdf)
{
	const bool_t ok =
			(
				(PCI_DEVHDL_DECODE_BDF(hdl) == bdf) &&
				(PCI_DEVHDL_DECODE_COID(hdl) != PCI_DEVHDL_COID_XOR)	// cannot be 0
			);
	return ok;
}
#define VALID_CAP_PRIV(_cap_, _hdl_, _capid_type_, _capid_val_) \
		( \
			((_cap_) != NULL) && \
			((_cap_)->struct_size >= sizeof(*(_cap_))) && \
			((_cap_)->bdf != PCI_BDF_NONE) && \
			((_cap_)->capid == CAPID_ENCODE(_capid_type_, _capid_val_)) && \
			((_cap_)->cap_specific != NULL) && \
			(((_hdl_) == NULL) ? true : __check_hdl((_hdl_), (_cap_)->bdf)) \
		)

/* call the module specific API check function */
extern pci_err_t mod_api_version_check(pci_cap_t cap);
/* (_cap_) == NULL check is because the capability read can use the public API's */
#define VALID_API_VERSION(_cap_)	(((_cap_) == NULL) || (mod_api_version_check((_cap_)) == PCI_ERR_OK))



#endif	/* _CAP_PRIV_H_ */

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/services/pci/modules/capabilities/private/cap_priv.h $ $Rev: 798837 $")
#endif
