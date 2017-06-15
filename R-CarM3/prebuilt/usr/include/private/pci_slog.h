#ifndef _PCI_SLOG_H_
#define _PCI_SLOG_H_
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

#include "private/slogmod_api.h"


extern volatile const slogmod_api_t *pci_slog;

/*
 ===============================================================================
 SLOG API's

 The following API's are used to log various output points.
 There are 3 functions based on the severity of the information.
 The global variable 'pci_verbosity' is also used to control the amount of
 output

 The 'verbosity_level' is used to control the amount of output for a given
 log type (INFO, WARNING, ERROR, etc). It represents a delta from the base
 verbosity level for the logging type.
 For example,
*/

#define slog_info(vl, args...) \
		do { \
			if (pci_slog != NULL) __pci_slog_info((vl), args); \
		} while(0)

#define slog_warn(vl, args...) \
		do { \
			if (pci_slog != NULL) __pci_slog_warn((vl), args); \
		} while(0)

#define slog_error(vl, args...) \
		do { \
			if (pci_slog != NULL) __pci_slog_error((vl), args); \
		} while(0)

/* for runtime performance reasons, don't use these directly */
static inline void __attribute__((__format__(__printf__, 2, 3))) __pci_slog_info(int_t verbosity_level, char *fmt, ...)
{
	va_list arglist;
	va_start(arglist, fmt);
	pci_slog->log(_SLOG_INFO, pci_verbosity, verbosity_level, fmt, arglist);
	va_end(arglist);
}
static inline void __attribute__((__format__(__printf__, 2, 3))) __pci_slog_warn(int_t verbosity_level, char *fmt, ...)
{
	va_list arglist;
	va_start(arglist, fmt);
	pci_slog->log(_SLOG_WARNING, pci_verbosity, verbosity_level, fmt, arglist);
	va_end(arglist);
}
static inline void __attribute__((__format__(__printf__, 2, 3))) __pci_slog_error(int_t verbosity_level, char *fmt, ...)
{
	va_list arglist;
	va_start(arglist, fmt);
	pci_slog->log(_SLOG_ERROR, pci_verbosity, verbosity_level, fmt, arglist);
	va_end(arglist);
}




#endif	/* _PCI_SLOG_H_ */

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/lib/pci/private/pci_slog.h $ $Rev: 798837 $")
#endif
