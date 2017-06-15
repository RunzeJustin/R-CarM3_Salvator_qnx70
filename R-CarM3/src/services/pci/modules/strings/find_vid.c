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
#include <string.h>

#include <pci/pci_strings.h>

#include "strings.h"

/*
 ===============================================================================
 pci_strings_find_vid

*/
const char *pci_strings_find_vid(const pci_vid_t vid)
{
	const char * const db_file_start = get_file_ptr();
	if (db_file_start == NULL) return NULL;
	else
	{
		const off64_t vend_offset = find_vid_offset(db_file_start, vid);
		if (vend_offset < 0) return NULL;
		else
		{
			const char * const vend_line = db_file_start + vend_offset;
			const char *vend_str = strchr(vend_line, '\t');
			const char * const nextline = next_line(vend_line);

			if ((vend_str == NULL) || (vend_str >= nextline)) return NULL;
			else
			{
				uint_t vend_str_len = (nextline - 1) - (vend_str + 1);
				char *str = calloc(1, vend_str_len + 1);
				memcpy(str, vend_str + 1, vend_str_len);
				str[vend_str_len] = '\0';
				return str;
			}
		}
	}
}

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/services/pci/modules/strings/find_vid.c $ $Rev: 798837 $")
#endif
