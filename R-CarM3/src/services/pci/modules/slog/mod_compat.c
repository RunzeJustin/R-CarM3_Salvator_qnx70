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
#include <pthread.h>
#include <stdlib.h>

#include <pci/pci.h>

#include "private/pci_mod.h"



/*
 ===============================================================================
 mod_compat

 This function is called to allow a module to determine whether or not it is
 compatible with <check_type> and <version>

 Generally, modules will be compatible so the function is written to filter out
 incompatibilities

*/
__attribute__ ((visibility ("internal")))
bool_t mod_compat(version_typecheck_e check_type, pci_version_t version)
{
	switch (check_type)
	{
		case version_typecheck_e_LIBRARY:
		{
			break;
		}
		case version_typecheck_e_SERVER:
		{
			break;
		}
		default: break;
	}
	return true;
}

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/services/pci/modules/slog/mod_compat.c $ $Rev: 798837 $")
#endif
