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


/*
 ===============================================================================
 parse_section_slots

 Parse the 'slots' parameter section between <start> and <end> inclusive.
 Any known valid parameters will have their values added to <cfg>.

 <start> is expected to be the first character following the SECTION_HDR_END_CHAR
 <end> is expected to by the last character of the section (either the character
 immediately before the next section or end-of-file)

*/
__attribute__ ((visibility ("internal")))
void parse_section_slots(const char *start, const char *end, hw_cfg_t *cfg)
{
	const char *p = start;
	const char *end_of_line;
	csd_assignment_t *tail = cfg->csd_list;

	p = skip_comment(p);
	while ((end_of_line = find_eol(p, end)) != NULL)
	{
		int record_len = end_of_line - p;

		if (record_len >= 13)		/* 1111;2222;0;0 (13 characters minimum) */
		{
			char *s = alloca(record_len + 1);
			char *saveptr;
			char *vid_record;

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
						char *chassis_slot_record = strtok_r(NULL, ";", &saveptr);

						if (chassis_slot_record != NULL)
						{
							pci_cs_t chassis = 0;
							pci_cs_t slot = 0;
							char *p = strchr(chassis_slot_record, ':');

							if (p != NULL)
							{
								if ((toupper(*chassis_slot_record) == 'C') && (isdigit(*(chassis_slot_record + 1))))
								{
									/* format is Cx:Sy */
									*p++ = '\0';
									chassis = strtoul(chassis_slot_record + 1, NULL, 0);
									if (toupper(*p) == 'S') slot = strtoul(p + 1, NULL, 0);
								}
							}
							else if (isdigit(*chassis_slot_record)) slot = strtoul(chassis_slot_record, NULL, 0);

							if (slot > 0)
							{
								uint_t device_number = 0;
								char *device_number_record = strtok_r(NULL, ";", &saveptr);

								if (device_number_record != NULL)
								{
									device_number = strtoul(device_number_record + 1, NULL, 10);

									if ((toupper(*device_number_record) != 'D') || (device_number >= PCI_BDF_MAX_DEVS))
									{
										slog_error(0, "\tIncorrectly formatted device number field");
										return;
									}
								}
								csd_assignment_t *csd_assignment = calloc(1, sizeof(*csd_assignment));
								if (csd_assignment != NULL)
								{
									/* fill in the chassis/slot assignment */
									csd_assignment->vid = vid;
									csd_assignment->did = did;
									csd_assignment->idx = idx;
									csd_assignment->csd = PCI_CSD(chassis, slot, device_number);
									csd_assignment->next = NULL;
									if (cfg->csd_list == NULL) cfg->csd_list = csd_assignment;	// first
									if (tail == NULL) tail = csd_assignment;
									else
									{
										tail->next = csd_assignment;
										tail = csd_assignment;
									}
									slog_info(0, "\tChassis/Slot/Device %u/%u/%u record for vid/did %x/%x @ idx %u",
												PCI_CHASSIS(PCI_CSD_CS(csd_assignment->csd)), PCI_SLOT(PCI_CSD_CS(csd_assignment->csd)),
												PCI_CSD_DEV(csd_assignment->csd), csd_assignment->vid, csd_assignment->did, csd_assignment->idx);
								}
								else slog_error(0, "\tUnable to allocate memory for chassis/slot assignment record");
							}
							else slog_error(0, "\tIncorrectly formatted chassis/slot field");
						}
						else slog_error(0, "\tIncorrectly formatted chassis/slot assignment record");
					}
					else slog_error(0, "\tIncorrectly formatted chassis/slot assignment record");
				}
				else slog_error(0, "\tIncorrectly formatted chassis/slot assignment record");
			}
			else slog_error(0, "\tIncorrectly formatted chassis/slot assignment record");
		}

		p = end_of_line + 1;
		p = skip_comment(p);
	}
}



#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/services/pci/modules/hw/src/parse/parse_section_slots.c $ $Rev: 798837 $")
#endif
