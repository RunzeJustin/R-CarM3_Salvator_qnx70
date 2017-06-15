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
#include <ctype.h>

#include <pci/pci_strings.h>

#include "strings.h"


static struct vid_cache_s
{
	pci_vid_t vid;
	off64_t offset;
} *vid_cache = NULL;
static uint_t num_vid_cache_entries = 0;

static const off64_t find_cached_vid(const pci_vid_t vid)
{
	uint_t i;
	for (i=0; i<num_vid_cache_entries; i++)
	{
		if (vid_cache[i].vid == vid) return vid_cache[i].offset;
	}
	return -1;
}

static void add_cached_vid(const pci_vid_t vid, const off64_t offset)
{
	struct vid_cache_s *new_vid_cache = realloc(vid_cache, (num_vid_cache_entries + 1) * sizeof(*new_vid_cache));
	if (new_vid_cache != NULL)
	{
		new_vid_cache[num_vid_cache_entries].vid = vid;
		new_vid_cache[num_vid_cache_entries].offset = offset;
		vid_cache = new_vid_cache;
		++num_vid_cache_entries;
	}
}

/*
 ===============================================================================
 find_vid_offset

*/
__attribute__ ((visibility ("internal")))
const off64_t find_vid_offset(const char * const db_file_start, const pci_vid_t vid)
{
	off64_t vid_offset = find_cached_vid(vid);

	/* if the <vid> is not yet cached, search the file */
	if (vid_offset < 0)
	{
		const char *vend_line = db_file_start;
		while (vend_line != NULL)
		{
			if (isxdigit(*vend_line))
			{
				pci_vid_t vend_id;
				const char *vend_str;
				uint_t vend_str_len;
				bool_t parse_ok = parse_vend_line(vend_line, &vend_id, &vend_str, &vend_str_len);

				if (parse_ok && (vend_id == vid))
				{
					vid_offset = (off64_t)(vend_line - db_file_start);
					add_cached_vid(vid, vid_offset);
					break;
				}
			}
			vend_line = next_line(vend_line);
		}
	}
	return vid_offset;
}

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/services/pci/modules/strings/_find_vid_offset.c $ $Rev: 798837 $")
#endif
