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
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <string.h>
#include <ctype.h>

#include <pci/pci_strings.h>

#include "strings.h"
#include "private/pci_slog.h"


__attribute__ ((visibility ("internal"))) const char *fp_end = NULL;

/*
 ===============================================================================
 get_file_ptr

*/
__attribute__ ((visibility ("internal")))
const char * const get_file_ptr(void)
{
	static const char *fp = NULL;
	static bool_t already_logged_nonexistent = false;

	if (fp == NULL)
	{
		const char *fname = getenv(PCI_ENVVAR_STRINGS_FILE);
		if (fname == NULL) fname = PCI_ENVVAR_STRINGS_FILE_DEFAULT;

		if (fname != NULL)
		{
			int fd = open(fname, O_RDONLY);
			if (fd > 0)
			{
				struct stat s;
				if (fstat(fd, &s) == 0)
				{
					fp = mmap(NULL, s.st_size, PROT_READ, MAP_SHARED, fd, 0);
					if (fp != MAP_FAILED)
					{
						fp_end = fp + s.st_size;
						/* file has now been found. Log that information and reset the flag */
						if (already_logged_nonexistent)
						{
							slog_info(1, "Located %s", fname);
							already_logged_nonexistent = false;	/* in case it goes away again */
						}
					}
					else
					{
						fp = NULL;
						slog_error(-1, "Unable to mmap file %s, %s", fname, pci_strerror(errno));
					}
				}
				else slog_error(-1, "Unable to stat file %s, %s", fname, pci_strerror(errno));

				close(fd);
			}
			else if ((errno != ENOENT) || !already_logged_nonexistent)
			{
				slog_error(0, "Unable to open file %s, %s", fname, pci_strerror(errno));
				if (!already_logged_nonexistent) already_logged_nonexistent = true;
			}
		}
	}
	return fp;
}

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/services/pci/modules/strings/_get_file_ptr.c $ $Rev: 801904 $")
#endif
