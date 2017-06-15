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
#include <string.h>

#include <pci/pci_strings.h>

#include "strings.h"


/*
 ===============================================================================
 parse_dev_line

 Parse a device line from the pcidatabase.com tab delimited vendor and device
 information file and return the device id in the storage pointed to be <did>,
 the pointer to the device id string in the storage pointed to by <dev_str>
 and the length of the string in the storage pointed to by <dev_str_len>

 <line> should point to the first character of a new line (ie. the character
 after a '\n')

*/
__attribute__ ((visibility ("internal")))
bool_t parse_dev_line(const char * const line, pci_did_t *did, const char **dev_str, uint_t * const dev_str_len)
{
	const char *p = line;
	const char *p_next = next_line(p);

	if ((p == NULL) || (p_next == NULL) || (*p != '\t')) return false;	// first character of line must be a TAB
	else
	{
		char dev_id[((sizeof(*did) * 8) / 4) + 1];	// space for max digits + '\0'

		++p;	// skip over tab
		memcpy(dev_id, p, sizeof(dev_id) - 1);
		dev_id[sizeof(dev_id) - 1] = '\0';

		p += (sizeof(dev_id) - 1);
		if (*p != '\t') return false;	// next char must be a TAB
		else
		{
			*did = (pci_did_t)strtoul(dev_id, NULL, 16);
			*dev_str = p + 1;
			*dev_str_len = p_next - *dev_str - 1;
			return true;
		}
	}
}

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/services/pci/modules/strings/_parse_dev_line.c $ $Rev: 798837 $")
#endif
