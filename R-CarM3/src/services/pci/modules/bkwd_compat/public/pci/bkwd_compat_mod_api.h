#ifndef _BKWD_COMPAT_MOD_API_H_
#define _BKWD_COMPAT_MOD_API_H_
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

#include <sys/types.h>

#include <pci/pci.h>


/*
 ===============================================================================
 bkwd_compat_mod_api_t

 This is the structure which provides the access API's for the backwards
 compatibility module of the PCI server.

 This module will be deprecated

*/
typedef struct
{
	uint_t struct_size;

	pci_devhdl_t (*attach_device)(uint32_t flags, uint_t idx, void *bufptr);
	pci_err_t (*detach_device)(pci_devhdl_t hdl);
	pci_bdf_t (*device_find)(uint_t idx, pci_vid_t vid, pci_did_t did, pci_ccode_t ccode);
	pci_err_t (*read_config_bus)(pci_bdf_t bdf, uint_t offset, uint_t cnt, size_t size, void *bufptr);
	pci_err_t (*read_config)(pci_devhdl_t hdl, uint_t offset, uint_t cnt, size_t size, void *bufptr);
	pci_err_t (*write_config_bus)(pci_bdf_t bdf, uint_t offset, uint_t cnt, size_t size, const void *bufptr);
	pci_err_t (*write_config)(pci_devhdl_t hdl, uint_t offset, uint_t cnt, size_t size, const void *bufptr);
	void (*rescan_bus)(void);
	void (*pci_present)(uint_t *lastbus, uint_t *version, uint_t *hardware);

} bkwd_compat_mod_api_t;

/*
 ===============================================================================
 The backward compatibility module provides an initialized function pointer
 table of type 'bkwd_compat_mod_api_t' and named BKWD_COMPAT_MODULE_ACCESS.
 The symbol can be found by the PCI library using the search string
 BKWD_COMPAT_MODULE_ACCESS_SYM.

 Additionally, this module may also provide an initialization function named
 BKWD_COMPAT_MODULE_INITFN which can be found by the PCI library using the search
 string BKWD_COMPAT_MODULE_INITFN_SYM.

 The PCI server modload_bkwd_compat() function will lookup
 BKWD_COMPAT_MODULE_INITFN_SYM first and if found call that function. It expects
 the 'bkwd_compat_mod_api_t **' parameter passed in to be initialized on
 successful return.

 If BKWD_COMPAT_MODULE_INITFN_SYM does not exist (it is optional) or it fails
 for some reason, then the symbol BKWD_COMPAT_MODULE_ACCESS_SYM is searched for
 in order to gain access to the backward compatibility module API's.

*/
#define STRINGX(s)	STRING(s)
#define STRING(s)	#s

#define BKWD_COMPAT_MODULE_ACCESS		bkwd_compat_mod_access
#define BKWD_COMPAT_MODULE_INITFN		bkwd_compat_mod_init

#define BKWD_COMPAT_MODULE_ACCESS_SYM	STRINGX(BKWD_COMPAT_MODULE_ACCESS)
#define BKWD_COMPAT_MODULE_INITFN_SYM	STRINGX(BKWD_COMPAT_MODULE_INITFN)



#endif	/* _BKWD_COMPAT_MOD_API_H_ */

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/services/pci/modules/bkwd_compat/public/pci/bkwd_compat_mod_api.h $ $Rev: 798837 $")
#endif
