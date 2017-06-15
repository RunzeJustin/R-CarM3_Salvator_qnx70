#ifndef _DBGMOD_API_H_
#define _DBGMOD_API_H_
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

#include <stdarg.h>

#include <pci/pci.h>

#include "private/pci_mod.h"


/*
 ===============================================================================
 dbgmod_api_t

 This is the structure which provides the access API's for the DEBUG module of
 the PCI server

 In order to allow future extensions and retain backwards compatibility, this
 structure can only ever be extended. Fields can never be removed or reordered.
 The 'struct_size' field ensures this compatibility with the PCI library as
 follows.
 	 - If this structure is extended and used with an existing libpci.so, the
 	   new fields will not be known to libpci.so and hence not used but the
 	   interface will continue to function properly since libpci.so idea of
 	   sizeof(dbgmod_api_t) will be <= 'struct_size' of the new structure
 	 - If this structure is extended and a new libpci.so delivered but that
 	   library is used with an older DEBUG module, libpci.so's idea of
 	   sizeof(dbgmod_api_t) will be > 'struct_size' of the old DEBUG
 	   module and hence will be rejected

 So, if the API's for a new DEBUG module are extended, the new module can be
 used with an existing libpci.so, however none of the new features of the DEBUG
 module will be utilized. If the API's for a new DEBUG module are extended and a
 new libpci.so provided which utilizes those new API's, the new libpci.so can
 only be used with a new DEBUG module, which is exactly what one would expect.

*/
typedef struct
{
	uint_t struct_size;
	pci_version_t (*mod_version)(void);
	bool_t (*mod_compat)(version_typecheck_e check_type, pci_version_t version);
	pci_err_t (*log)(uint_t type, int_t verbosity, int_t verbosity_level, const char *fmt, va_list arglist);

} dbgmod_api_t;

/*
 ===============================================================================
 The DEBUG module provides an initialized function pointer table of type
 'dbgmod_api_t' and named DEBUG_MODULE_ACCESS. The symbol can be found by the
 PCI library using the search string DEBUG_MODULE_ACCESS_SYM.

 Additionally, this module may also provide an initialization function named
 DEBUG_MODULE_INITFN which can be found by the PCI library using the search
 string DEBUG_MODULE_INITFN_SYM.

 The PCI server modload_dbg() function will lookup DEBUG_MODULE_INITFN_SYM first
 and if found call that function. It expects the 'dbgmod_api_t **' parameter
 passed in to be initialized on successful return.

 If DEBUG_MODULE_INITFN_SYM does not exist (it is optional) or it fails for
 some reason, then the symbol DEBUG_MODULE_ACCESS_SYM is searched for in order
 to gain access to the DEBUG module API's.

 IMPORTANT: These symbol names can never change if backward compatibility is to
 	 	 	be maintained
*/
#define STRINGX(s)	STRING(s)
#define STRING(s)	#s

#define DEBUG_MODULE_ACCESS		dbgmod_access
#define DEBUG_MODULE_INITFN		dbgmod_init

#define DEBUG_MODULE_ACCESS_SYM	STRINGX(DEBUG_MODULE_ACCESS)
#define DEBUG_MODULE_INITFN_SYM	STRINGX(DEBUG_MODULE_INITFN)



#endif	/* _DBGMOD_API_H_ */

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/services/pci/modules/debug/private/dbgmod_api.h $ $Rev: 798837 $")
#endif
