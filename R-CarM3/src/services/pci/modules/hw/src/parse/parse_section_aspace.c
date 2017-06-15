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

#include "private/pci_slog.h"
#include "parse.h"


static void add_aspace_filter(const char *param, char *param_val, val_type_e type, const char delim, void *extra);

static bool_t valid_aspace_filter_param_char(char c);


static known_param_names_t known_params[] =
{
	{
		.name = "ASPACE_FILTER",
		.len = sizeof("ASPACE_FILTER") - 1,
		.type = val_type_e_STRING,
		.multi_val = true,
		.delim=':',
		.param_handler = add_aspace_filter,
	},
};


/*
 ===============================================================================
 parse_section_aspace

 Parse the 'aspace' parameter section between <start> and <end> inclusive.
 Any known valid parameters will have their values added to <cfg>.

 <start> is expected to be the first character following the SECTION_HDR_END_CHAR
 <end> is expected to by the last character of the section (either the character
 immediately before the next section or end-of-file)

 Section entries must exist 1 per line (line continuation is permitted however
 the '=' must appear before the line continuation character). Each line consists
 of a parameter name, an '=' and 1 or more parameter values separated by a
 section specific token

 Valid entries are added to the cfg->aspace_filter_list

*/
__attribute__ ((visibility ("internal")))
void parse_section_aspace(const char *start, const char *end, hw_cfg_t *cfg)
{
	const char *p = start;
	const char *end_of_line;

	p = skip_comment(p);
	while ((end_of_line = find_eol(p, end)) != NULL)
	{
		const char *equal_sign = find_token('=', p, end_of_line);
		if (equal_sign != NULL)
		{
			uint_t param_len;
			const char *param = find_name(p, equal_sign - 1, &param_len, NULL);
			if (param != NULL)
			{
				const known_param_names_t const *known_param = find_param(param, known_params, NELEMENTS(known_params));
				if (known_param != NULL)
				{
					if (strncmp(known_param->name, "ASPACE_FILTER", known_param->len) == 0)
					{
						/* the entire parameter string will be parsed by the handler */
						uint_t val_len;
						const char *val = find_name(equal_sign + 1, end_of_line - 1, &val_len, valid_aspace_filter_param_char);
						if (val != NULL)
						{
							param_handler_f handler = known_param->param_handler;
							if (handler != NULL)
							{
								char *param_val_str;

								MAKE_TMP_STRING(param_val_str, val, val_len);

								handler(known_param->name, param_val_str, known_param->type, known_param->delim, cfg);
							}

						}
					}
				}
			}
		}
		p = end_of_line + 1;
		p = skip_comment(p);
	}
}

/*
 ===============================================================================
 add_aspace_filter

 This function will process the address space filter record of the HW
 configuration file 'aspace' section

 The format of the filter parameter for this section of the file is as follows.
 All 3 fields are required

 <section type>:<section start>:<section end>

 <section type> must be MEM or IO (case invariant)
 both <section start> and <section end> are required
 <section start> must be < <section end>
*/
static void add_aspace_filter(const char *param, char *param_val, val_type_e type, const char delim_c, void *extra)
{
	hw_cfg_t *cfg = (hw_cfg_t *)extra;
	const char delim[] = {[0] = delim_c, [1] = '\0'};
	char *saveptr;
	char *type_record = strtok_r(param_val, delim, &saveptr);

	if ((type_record == NULL) || ((strcasecmp(type_record, "MEM") != 0) && (strcasecmp(type_record, "IO") != 0)))
	{
		slog_error(0, "\t%s - incorrectly formatted ASPACE_FILTER <type> parameter", param);
	}
	else
	{
		/* parse the address space filter record */
		char *aspace_filter_record_str;
		struct
		{
			pci_ba_val_t start;
			pci_ba_val_t end;
			pci_ba_val_t align;
		} aspace_filter_record;

		memset(&aspace_filter_record, 0, sizeof(aspace_filter_record));

		if ((aspace_filter_record_str = strtok_r(NULL, delim, &saveptr)) != NULL)
		{
			aspace_filter_record.start = strtoull(aspace_filter_record_str, NULL, 0);

			if ((aspace_filter_record_str = strtok_r(NULL, delim, &saveptr)) != NULL)
			{
				aspace_filter_record.end = strtoull(aspace_filter_record_str, NULL, 0);

				if ((aspace_filter_record_str = strtok_r(NULL, delim, &saveptr)) != NULL)
				{
					aspace_filter_record.align = strtoull(aspace_filter_record_str, NULL, 0);
				}
			}
			else slog_error(0, "\tIncorrectly formatted ASPACE_FILTER <end> parameter");
		}
		else slog_error(0, "\tIncorrectly formatted ASPACE_FILTER <start> parameter");

		const pci_asType_e type = (strcasecmp(type_record, "MEM") == 0) ? pci_asType_e_MEM : pci_asType_e_IO;
		const pci_asAttr_e min_align = (type == pci_asType_e_MEM) ? pci_asAttr_e_ALIGN_MEM_MIN : pci_asAttr_e_ALIGN_IO_MIN;

		if (aspace_filter_record.end <= aspace_filter_record.start)
		{
			slog_error(0, "\tIncorrectly formatted ASPACE_FILTER, end %"PRIx64" <= start %"PRIx64"",
							aspace_filter_record.end, aspace_filter_record.start);
		}
		else if ((aspace_filter_record.start & (max(aspace_filter_record.align, min_align) - 1)) != 0)
		{
			slog_error(0, "\tIncorrectly formatted ASPACE_FILTER, start %"PRIx64" does not meet %"PRIx64" alignment requirement",
							aspace_filter_record.start, max(aspace_filter_record.align, min_align));
		}
		else if (((aspace_filter_record.end + 1) & (max(aspace_filter_record.align, min_align) - 1)) != 0)
		{
			slog_error(0, "\tIncorrectly formatted ASPACE_FILTER, end %"PRIx64" does not meet %"PRIx64" alignment requirement",
							aspace_filter_record.end + 1, max(aspace_filter_record.align, min_align));
		}
		else
		{
			aspace_filter_t *aspace_filter = calloc(1, sizeof(*aspace_filter));
			if (aspace_filter != NULL)
			{
				/* find the last entry on the list */
				aspace_filter_t *tail = cfg->aspace_filter_list;
				if (tail != NULL)
				{
					while (tail->next != NULL) {
						tail = tail->next;
					}
					assert(tail != NULL);
				}

				aspace_filter->asmap.ba.addr = aspace_filter_record.start;
				aspace_filter->asmap.ba.size = aspace_filter_record.end - aspace_filter_record.start + 1;
				aspace_filter->asmap.ba.type = type;
				aspace_filter->asmap.ba.attr |= ilog2(max(aspace_filter_record.align, min_align));

				aspace_filter->next = NULL;
				if (cfg->aspace_filter_list == NULL) cfg->aspace_filter_list = aspace_filter;	// first
				if (tail != NULL) tail->next = aspace_filter;

				slog_info(0, "\tAdded Address Space Filter record %"PRIx64" --> %"PRIx64", align: %"PRIx64"",
						aspace_filter->asmap.ba.addr, aspace_filter->asmap.ba.addr + aspace_filter->asmap.ba.size - 1,
						(uint64_t)1 << (aspace_filter->asmap.ba.attr & pci_asAttr_e_ALIGN));
			}
		}
	}
}

/*
 ===============================================================================
 valid_aspace_filter_param_char

 This function will return true or false depending on whether <c> is a valid
 character for an address space reservation in the 'buscfg' variables section.

*/
static bool_t valid_aspace_filter_param_char(char c)
{
	return isxdigit(c) ||
			(c == ':') ||
			(toupper(c) == 'X') ||
			(toupper(c) == 'M') ||
			(toupper(c) == 'E') ||
			(toupper(c) == 'I') ||
			(toupper(c) == 'O');
}


#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/services/pci/modules/hw/src/parse/parse_section_aspace.c $ $Rev: 811187 $")
#endif
