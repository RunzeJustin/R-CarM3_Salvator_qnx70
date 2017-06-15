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

#include <stdio.h>

#include <pci/pci.h>

#include "private/pci_mod.h"


static const char * const version_str = MODULE_VERSION;

/*
 ===============================================================================
 mod_version

 the 'pci_version_t' is returned

 This function is used by the module to obtain their version number so that the
 information can be used by the PCI server or libpci for compatibility checks

 Implementation Note
 -------------------.
 The module version number consists of a major and minor number separated by an
 underscore (_)

*/
__attribute__ ((visibility ("internal")))
pci_version_t mod_version(void)
{
	static pci_version_t version = BAD_VERSION_INITIALIZER;

	if (IS_BAD_VERSION(version))
	{
		uint_t major, minor;
		/*
		 * why don't we just pass version.* as the parameters? Seems scanf() wants
		 * a uint_t * provided with the %u. A cast will silence the compiler but causes
		 * grief for big endian machines
		 */
		int n = sscanf(version_str, "%u_%u", &major, &minor);

		if (n == 2)
		{
			version.major = major;
			version.minor = minor;
		}
	}
	return version;
}

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/services/pci/modules/_common/mod_version.c $ $Rev: 798837 $")
#endif
