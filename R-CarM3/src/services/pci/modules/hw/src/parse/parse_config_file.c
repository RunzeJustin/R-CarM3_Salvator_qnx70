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

#include <stdio.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <string.h>
#include <ctype.h>

#include "private/pci_slog.h"
#include "parse.h"


static known_section_types_t known_section_types[] =
{
	{.name = "interrupts", 	.len = sizeof("interrupts") - 1, .section_parser = parse_section_interrupts},
	{.name = "slots", 	.len = sizeof("slots") - 1, .section_parser = parse_section_slots},
	{.name = "aspace", 	.len = sizeof("aspace") - 1, .section_parser = parse_section_aspace},
	{.name = "rbar", 	.len = sizeof("rbar") - 1, .section_parser = parse_section_rbar},
};


/*
 ===============================================================================
 process_section_hw_specific

 This function pointer is provided to allow for a HW specific module to hook
 into the processing of a HW configuration file.

 The intention is that in some situations we may want to add a section to a
 HW configuration file that has no applicability to any other HW module.

 Although we could just add the section definition to 'hw_cfg_t' and similarly
 add it to the 'known_section_types[]' above, if this section truly was not
 applicable to any other HW module we would be unnecessarily increasing the
 size of every other module for a section that they will never see.

 Instead, to accommodate such scenarios, a HW specific module can set this
 function pointer to its own 'process_section_xxx()' function. As each section
 is found, the HW specific module will be given an opportunity to handle the
 section itself. The function returns true or false depending on whether or not
 it processed the section. If it does not process the section and the section is
 known, the default processing of the common parsing code will be done.

*/
__attribute__ ((visibility ("internal")))
process_section_hw_specific_f process_section_hw_specific = NULL;


static bool_t find_section(const char *start, const char *end, section_info_t *section_info, hw_cfg_t *cfg);
static void process_section(section_info_t *section_info, hw_cfg_t *cfg);


/*
 ===============================================================================
 parse_config_file

 This function is called to parse a HW configuration file and fill in the
 <cfg> parameter

*/
__attribute__ ((visibility ("internal")))
void parse_config_file(const char const *fname, hw_cfg_t *cfg)
{
	if (fname == NULL) return;
	else if (*fname == '\0') return;
	{
		struct stat s;
		const char const *fp = MAP_FAILED;
		int fd = open(fname, O_RDONLY);

		if ((fd >= 0) && (fstat(fd, &s) == 0) &&
			((fp = mmap(NULL, s.st_size, PROT_READ, MAP_SHARED, fd, 0)) != MAP_FAILED))
		{
			/* lets parse the file */
			section_info_t section_info;
			const char *end_of_file = fp + s.st_size - 1;	// points to the last valid byte of the file

			slog_info(0, "++ HW Config file '%s' parse start ++", fname);
			find_section(fp, end_of_file, &section_info, cfg);
			slog_info(0, "++ HW Config file '%s' parse end ++", fname);
		}
		else
		{
			slog_warn(0, "%s environment variable set to %s but could not be opened or otherwise read",
							PCI_ENVVAR_HW_CONFIG, fname);
		}
		if (fp != MAP_FAILED) munmap((void *)fp, s.st_size);
		if (fd >= 0) close(fd);
	}
}

/*
 ===============================================================================
 find_section

 This function will search for a valid section tag between <start> and <end>
 inclusive. A valid section tag consists of the SECTION_HDR_START_CHAR, a
 section name and a SECTION_HDR_END_CHAR. The section name may be surrounded by
 whitespace characters only (tabs or spaces).

 Finding a valid section tag does not mean the section is known, only that a
 valid section tag exists

 If a valid section tag is found, the <section> info is updated. As part of
 updating the <section_info>, the end of the section is required. This
 information is obtained by searching for the next section. If the next section
 is found, the end of the current section is 1 byte less than the start of the
 next. If its not found, the end of the section is <end>. As we recurse down
 finding sections, eventually no more will be found. At that point we process
 the section (starting from the last section found) and unwinding back up to
 the beginning.

 The reason for processing this way is so that we don't duplicate the section
 processing in order to find the next section

*/
static bool_t find_section(const char *start, const char *end, section_info_t *section_info, hw_cfg_t *cfg)
{
	bool_t section_found = false;
	const char *section_hdr_start = find_token(SECTION_HDR_START_CHAR, start, end);

	if (section_hdr_start != NULL)
	{
		const char *section_hdr_end = find_token(SECTION_HDR_END_CHAR, section_hdr_start + 1, end);

		if (section_hdr_end != NULL)
		{
			/* determine whether a valid name exists between section_hdr_start and section_hdr_end */
			uint_t section_name_len;
			const char *section_name = find_name(section_hdr_start + 1, section_hdr_end - 1, &section_name_len, NULL);
			if (section_name != NULL)
			{
				section_found = true;

				/* section tag exists. Find the end of the section by searching for the next section tag */
				section_info_t next_section_info;
				bool_t next_section_found = find_section(section_hdr_end + 1, end, &next_section_info, cfg);

				section_info->start = section_hdr_start;
				section_info->name.str = section_name;
				section_info->name.len = section_name_len;
				section_info->params.start = section_hdr_end + 1;

				if (next_section_found) section_info->params.end = next_section_info.start - 1;
				else section_info->params.end = end;

				bool_t section_processed = false;
				if (process_section_hw_specific != NULL)
				{
					section_processed = process_section_hw_specific(section_info, cfg, NULL);
				}
				if (!section_processed) process_section(section_info, cfg);
			}
		}
	}
	return section_found;
}

/*
 ===============================================================================
  process_section

*/
static void process_section(section_info_t *section_info, hw_cfg_t *cfg)
{
	uint_t i;
	/* determine whether this is a known section and if so, parse it */
	for (i=0; i<NELEMENTS(known_section_types); i++)
	{
		if (strncasecmp(section_info->name.str, known_section_types[i].name, section_info->name.len) == 0)
		{
			/* parse the parameters for the section */
			void (*parse_section)() = known_section_types[i].section_parser;
			char *str;

			MAKE_TMP_STRING(str, section_info->name.str, section_info->name.len);
			slog_info(0, "+++ Parsing section [%s] start +++", str);
			parse_section(section_info->params.start, section_info->params.end, cfg);
			slog_info(0, "+++ Parsing section [%s] end +++", str);
			break;
		}
	}
}

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/services/pci/modules/hw/src/parse/parse_config_file.c $ $Rev: 798837 $")
#endif
