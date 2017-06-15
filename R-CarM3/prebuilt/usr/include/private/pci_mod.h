#ifndef _PCI_MOD_H_
#define _PCI_MOD_H_
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


typedef enum
{
	version_typecheck_e_LIBRARY,
	version_typecheck_e_MODULE,
	version_typecheck_e_SERVER,
} version_typecheck_e;


extern const pci_version_t bad_version;
#define BAD_VERSION		bad_version
#define IS_BAD_VERSION(_v_)		(((_v_).major == bad_version.major) && ((_v_).minor == bad_version.minor))
#define BAD_VERSION_INITIALIZER	{.major = 0xFFFFu, .minor = 0xFFFFu}

typedef enum
{
	module_id_e_HW = 1,
	module_id_e_CAP,
	module_id_e_BKWD_COMPAT,
	module_id_e_SERVER_BUSCFG,
	module_id_e_SLOG,
	module_id_e_DEBUG
} module_id_e;

typedef struct
{
	module_id_e id;
	uint32_t capid;		// is a _capid_t and is only for id = module_id_e_CAP
} module_id_t;

typedef struct
{
	pci_version_t my_version;
	bool_t (*mod_compat)(module_id_t *module_id, pci_version_t module_version);
} version_checker_t;

extern version_checker_t *version_check;

/*
 ===============================================================================
 function prototypes
*/
pci_err_t dlopen_mod(const char *mod_name, uint_t mode, void **modhdl);
pci_version_t mod_version(void);
pci_version_t lib_version(void);
bool_t mod_compat(version_typecheck_e check_type, pci_version_t version);
pci_err_t register_version_checker(version_checker_t *version_check);




#endif	/* _PCI_MOD_H_ */

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/lib/pci/private/pci_mod.h $ $Rev: 798837 $")
#endif
