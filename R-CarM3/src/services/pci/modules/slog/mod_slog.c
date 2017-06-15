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
#include <sys/slog.h>
#include <sys/slogcodes.h>
#include <unistd.h>
#include <stdio.h>
#include <pthread.h>

#include <pci/pci.h>
#include "private/slogmod_api.h"
#include "private/pci_slog.h"

/*
 ===============================================================================
 _base_verbosity_level

 This array sets the base line verbosity level for a given log type. The base
 verbosity level is compared to the global verbosity level (the verbosity
 parameter to slogmod_slog()) in order to determine whether an output message
 is sent to the system log.

 The additional parameter <verbosity_level> is a per call verbosity modifier
 which can be used to offset the global verbosity level. For example, with
 a default verbosity of 0 all slog_info() calls with a call specific verbosity
 level of 0 will be logged. If the information was not that important, then
 a call specific verbosity level modifier of +1 could be provided which would
 require the global verbosity level to be at least 1 greater than the base
 verbosity level. Similarly, slog_warn() will only log if the global verbosity
 is at least 1 (ie. its base verbosity level) however at specific points in the
 code, a call specific verbosity level of -1 could be provided which would
 lower the required threshold for output by 1 (in this case to 0 which would
 cause the message to be displayed always)

*/

static const uint_t _base_verbosity_level[] =
{
	[0 ... (_SLOG_SEVMAXVAL - 1)] = -1,
	[_SLOG_INFO] = 1,
	[_SLOG_WARNING] = 1,
	[_SLOG_ERROR] = 0,		// all error messages are logged by default
};
static struct
{
	const char * const hdr;
	uint_t len;
} _hdr_string[] =
{
	[0 ... (_SLOG_SEVMAXVAL - 1)] = {.hdr = NULL, .len = 0},
	[_SLOG_INFO] = {.hdr = "PCI INFO ", .len = sizeof("PCI INFO ")},
	[_SLOG_WARNING] = {.hdr = "PCI WARN ", .len = sizeof("PCI WARN ")},
	[_SLOG_ERROR] = {.hdr = "PCI ERROR", .len = sizeof("PCI ERROR")},
};

extern uint_t pci_base_verbosity;	// global base verbosity level

static void set_pci_base_verbosity(void)
{
	const char * const base_verbosity = getenv(PCI_ENVVAR_VERBOSITY);
	if ((base_verbosity != NULL) && (*base_verbosity != '\0'))
	{
		uint_t tmp_pci_base_verbosity = strtoul(base_verbosity, NULL, 0);
		slog_info(0, "%s = %u", PCI_ENVVAR_VERBOSITY, tmp_pci_base_verbosity);
		pci_base_verbosity = tmp_pci_base_verbosity;
	}
}

/*
 ===============================================================================
 slogmod_slog

*/
__attribute__ ((visibility ("internal")))
pci_err_t slogmod_slog(uint_t type, int_t verbosity, int_t verbosity_level, const char *fmt, va_list arglist)
{
	static pthread_once_t once = PTHREAD_ONCE_INIT;
	const char * const hdr_string = _hdr_string[type].hdr;

	pthread_once(&once, set_pci_base_verbosity);

	if (hdr_string == NULL) return PCI_ERR_SLOG_TYPE;
	else
	{
		const int_t base_verbosity_level = _base_verbosity_level[type];
		pci_err_t r = PCI_ERR_SLOG_VERBOSITY;

		if ((verbosity + (int_t)pci_base_verbosity) >= ((int_t)base_verbosity_level + verbosity_level))
		{
			const uint_t len = strlen(fmt);
			const uint_t hdr_len = _hdr_string[type].len;
			char *s_fmt = alloca(len + hdr_len + sizeof(",nn,nn,nn [4294967296:99]"));

			if (s_fmt == NULL) r = PCI_ERR_ENOMEM;
			{
				/*
				 * we prepend the SLOG message with a header string, the base verbosity level
				 * of the log message <type>, the relative verbosity of the specific log message,
				 * the current overall verbosity level (which is a combination of the PCI_BASE_VERBOSITY
				 * environment variable and the value of 'pci_verbosity', available to all programs
				 * which link against libpc) and the 'pid' of the caller. Because there will be similar
				 * slog messages for every process using libpci, the pid allows 'grep' filtering based on
				 * process id. It is not possible to use the major/minor numbers for this purpose
				 */
				s_fmt[0] = '\0';
				sprintf(s_fmt, "%s,%d,%d,%d [%u:%u]: ", hdr_string, base_verbosity_level,
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
}

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/services/pci/modules/slog/mod_slog.c $ $Rev: 808445 $")
#endif
