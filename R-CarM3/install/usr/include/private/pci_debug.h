#ifndef _PCI_DEBUG_H_
#define _PCI_DEBUG_H_
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
#include <sys/slog.h>

#include <pci/pci.h>

#include "private/dbgmod_api.h"


extern volatile const dbgmod_api_t *pci_dbg;

/*
 ===============================================================================
 DEBUG API's

 The following API is used to log various output points.
*/

#define slog_debug(vl, args...) \
		do { \
			if (pci_dbg != NULL) __pci_slog_debug((vl), args); \
		} while(0)


static inline void __attribute__((__format__(__printf__, 2, 3))) __pci_slog_debug(int_t verbosity_level, char *fmt, ...)
{
	va_list arglist;
	va_start(arglist, fmt);
	pci_dbg->log(_SLOG_DEBUG1, pci_verbosity, verbosity_level, fmt, arglist);
	va_end(arglist);
}



#endif	/* _PCI_DEBUG_H_ */

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/lib/pci/private/pci_debug.h $ $Rev: 798837 $")
#endif
