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
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <pthread.h>
#include <sys/slog.h>
#include <sys/slogcodes.h>

#include <pci/pci.h>
#include "private/dbgmod_api.h"
#include "private/pci_lib.h"

/*
 ===============================================================================
 dbgmod_log

 TODO - for server debug logging, allow for some sort of 'pid' filtering to
 	 	 help reduce the slog volume. This is particularly valuable for tracing
 	 	 writes (handled by the server) for a specific driver
*/
__attribute__ ((visibility ("internal")))
pci_err_t dbgmod_log(uint_t type, int_t verbosity, int_t verbosity_level, const char *fmt, va_list arglist)
{
	extern uint_t pci_base_verbosity;
	const int_t base_verbosity_level = 1;
	pci_err_t r = PCI_ERR_SLOG_VERBOSITY;

/*	if (pci_runtime_flags & pci_runtime_flags_e_SUPPRESS_DEBUG) r = PCI_ERR_OK;
	else*/ if ((verbosity + (int_t)pci_base_verbosity) >= ((int_t)base_verbosity_level + verbosity_level))
	{
		const uint_t len = strlen(fmt);
		const uint_t hdr_len = sizeof("PCI DEBUG");
		char *s_fmt = alloca(len + hdr_len + sizeof(",nn,nn,nn [4294967296:99]"));

		if (s_fmt == NULL) r = PCI_ERR_ENOMEM;
		{
			/*
			 * we prepend the DEBUG message with a header string, the base verbosity level
			 * of the log message <type>, the relative verbosity of the specific log message,
			 * the current overall verbosity level (which is a combination of the PCI_BASE_VERBOSITY
			 * environment variable and the value of 'pci_verbosity', available to all programs
			 * which link against libpc) and the 'pid' of the caller. Because there will be similar
			 * slog messages for every process using libpci, the pid allows 'grep' filtering based on
			 * process id. It is not possible to use the major/minor numbers for this purpose
			 */
			s_fmt[0] = '\0';
			sprintf(s_fmt, "PCI DEBUG,%d,%d,%d [%u:%u]: ", base_verbosity_level,
					verbosity_level, verbosity + pci_base_verbosity, getpid(), pthread_self());
			strncat(s_fmt, fmt, len);
#if 1
			r = (vslogf(_SLOGC_PCI, type, s_fmt, arglist) < 0) ? PCI_ERR_SLOG : PCI_ERR_OK;
#else
			strcat(s_fmt, "\n");
			r = (vprintf(s_fmt, arglist) < 0) ? PCI_ERR_SLOG : PCI_ERR_OK;
			fflush(stdout); usleep(50 * 1000);
#endif
		}
	}
	return r;
}

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/services/pci/modules/debug/mod_debug.c $ $Rev: 798837 $")
#endif
