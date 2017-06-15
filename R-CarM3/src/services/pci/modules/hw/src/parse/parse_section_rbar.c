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
#include <string.h>
#include <assert.h>
#include <ctype.h>
#include <string.h>
#include <limits.h>
#include <stdio.h>

#include "private/pci_slog.h"
#include "parse.h"


/*
 ===============================================================================
 parse_section_rbar

 Parse the 'rbar' parameter section between <start> and <end> inclusive.
 Any known valid parameters will have their values added to <cfg>.

 <start> is expected to be the first character following the SECTION_HDR_END_CHAR
 <end> is expected to by the last character of the section (either the character
 immediately before the next section or end-of-file)

*/
__attribute__ ((visibility ("internal")))
void parse_section_rbar(const char *start, const char *end, hw_cfg_t *cfg)
{
	const char *p = start;
	const char *end_of_line;
	rbar_override_t *tail = cfg->rbar_override_list;
	uint_t record_count = 0;

	p = skip_comment(p);
	while ((end_of_line = find_eol(p, end)) != NULL)
	{
		int record_len = end_of_line - p;

		if (record_len >= 15)		/* 1234;abcd;1;1:8 (15 characters minimum) */
		{
			char *s = alloca(record_len + 1);
			char *saveptr;
			char *vid_record;

			++record_count;
			memcpy(s, p, record_len);
			s[record_len] = '\0';
			vid_record = strtok_r(s, ";", &saveptr);
			if (vid_record != NULL)
			{
				const pci_vid_t vid = (pci_vid_t)strtoul(vid_record, NULL, 16);
				char *did_record = strtok_r(NULL, ";", &saveptr);

				if (did_record != NULL)
				{
					const pci_did_t did = (pci_did_t)strtoul(did_record, NULL, 16);
					char *idx_record = strtok_r(NULL, ";", &saveptr);

					if (idx_record != NULL)
					{
						const uint_t idx = (uint_t)strtoul(idx_record, NULL, 0);
						char *bar_setting_record = strtok_r(NULL, ";", &saveptr);
						uint_t bar_setting_record_num = 0;
						struct rbar_info_s rbar_info[6] = {[0 ... 5] = {.num = -1, .size = 0}};
						bool_t bad_rbar_record = false;

						while (bar_setting_record_num < NELEMENTS(rbar_info))
						{
							if (bar_setting_record != NULL)
							{
								char *p = strchr(bar_setting_record, ':');

								/* format must be bar:size */
								if ((p != NULL) && isdigit(*bar_setting_record))
								{
									*p++ = '\0';
									uint_t bar_num = strtoul(bar_setting_record, NULL, 10);
									errno = 0;
									pci_ba_val_t size = strtoul(p, NULL, 0);
									if (errno == ERANGE)
									{
										errno = 0;
										size = strtoull(p, NULL, 0);
									}

									if ((bar_num < 6) && (size > 0) && (size != ULLONG_MAX) && (errno != ERANGE))
									{
										rbar_info[bar_setting_record_num].num = bar_num;
										rbar_info[bar_setting_record_num].size = size;
									}
								}
							}
							if ((rbar_info[bar_setting_record_num].num == -1) || (rbar_info[bar_setting_record_num].size == 0))
							{
								bad_rbar_record = (bar_setting_record_num == 0);
								if (bad_rbar_record)
								{
									slog_error(0, "\tMissing or incorrectly formatted record %u", record_count);
								}
								break;
							}
							else
							{
								bar_setting_record = strtok_r(NULL, ";", &saveptr);
								++bar_setting_record_num;
							}
						}
						if (!bad_rbar_record)
						{
							rbar_override_t *rbar_overrides = calloc(1, sizeof(*rbar_overrides));
							if (rbar_overrides != NULL)
							{
								uint_t i;

								/* fill in the bar_num/size assignment */
								rbar_overrides->vid = vid;
								rbar_overrides->did = did;
								rbar_overrides->idx = idx;
								for (i=0; i<NELEMENTS(rbar_overrides->bar); i++) {
									rbar_overrides->bar[i] = rbar_info[i];
								}
								rbar_overrides->next = NULL;
								if (cfg->rbar_override_list == NULL) cfg->rbar_override_list = rbar_overrides;	// first
								if (tail == NULL) tail = rbar_overrides;
								else
								{
									tail->next = rbar_overrides;
									tail = rbar_overrides;
								}
								/* all the slog_*() calls do this so do it here so we don't waste time unnecessarily */
								if (pci_slog != NULL)
								{
									/* build a nicely formatted log message */
									char buf[NELEMENTS(rbar_overrides->bar) * sizeof("x=xxxxxxxxxxxxxxxx, ")] = {'\0'};
									int_t n = 0;

									for (i=0; i<NELEMENTS(rbar_overrides->bar); i++)
									{
										if (rbar_overrides->bar[i].num >= 0)
										{
											if (n == 0) {
												n += sprintf(&buf[n], "%u=%"PRIx64"", rbar_overrides->bar[i].num, rbar_overrides->bar[i].size);
											}
											else {
												n += sprintf(&buf[n], ", %u=%"PRIx64"", rbar_overrides->bar[i].num, rbar_overrides->bar[i].size);
											}
										}
									}
									assert(n < sizeof(buf));

									slog_info(0, "\tRBAR Override record %u: vid/did %x/%x @ idx %u: %s", record_count,
												 rbar_overrides->vid, rbar_overrides->did, rbar_overrides->idx, buf);
								}
							}
							else
							{
								slog_error(0, "\tUnable to allocate memory for RBAR override record %u", record_count);
								break;
							}
						}
					}
					else slog_error(0, "\tMissing or incorrectly formatted device index field");
				}
				else slog_error(0, "\tMissing or incorrectly formatted Device ID field");
			}
			else slog_error(0, "\tMissing or incorrectly formatted Vendor ID field");
		}
		else if (record_len > 0)
		{
			/* its too short, but identify it */
			slog_error(0, "\tMissing or incorrectly formatted record %u", record_count);
			++record_count;
		}

		p = end_of_line + 1;
		p = skip_comment(p);
	}
}



#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/services/pci/modules/hw/src/parse/parse_section_rbar.c $ $Rev: 798837 $")
#endif
