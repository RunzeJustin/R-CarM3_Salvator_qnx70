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


static bool_t find_irq_token(const char **fp, pci_irq_t *irq);
static bool_t find_intpin_token(const char **fp, char *intpin_c);
static bool_t find_bdf_token(const char **fp, pci_bdf_t *bdf);

/*
 ===============================================================================
 parse_section_interrupts

 Parse the 'interrupts' parameter section between <start> and <end> inclusive.
 Any known valid parameters will have their values added to <cfg>.

 <start> is expected to be the first character following the SECTION_HDR_END_CHAR
 <end> is expected to by the last character of the section (either the character
 immediately before the next section or end-of-file)

 The format of this section of the file is as follows. All 3 fields are required

 Bx:Dy:Fz  Pin  IRQ

 or for ARI devices

 Bx:Fz     Pin  IRQ

 x, y and z specify the Bus, Device and Function respectively
 Pin identifies the PCI defined interrupt pin A, B, C or D. A value of '-'
     indicates a don't care value that will not be used in the match criteria
 IRQ specifies which IRQ the device interrupts on

 Valid entries are added to the cfg->irq_list

*/
__attribute__ ((visibility ("internal")))
void parse_section_interrupts(const char *start, const char *end, hw_cfg_t *cfg)
{
	const char *p = start;
	const char *end_of_line;
	pin_to_irq_map_t *tail = cfg->irq_list;

	p = skip_comment(p);
	while ((end_of_line = find_eol(p, end)) != NULL)
	{
		pci_bdf_t bdf = PCI_BDF_NONE;	// quiet compiler
		char intpin_c;
		pci_irq_t irq;
		const char *fp = p;

		if (find_bdf_token(&fp, &bdf) &&
			find_intpin_token(&fp, &intpin_c) &&
			find_irq_token(&fp, &irq))
		{
			pin_to_irq_map_t *irq_map = calloc(1, sizeof(*irq_map));
			if (irq_map != NULL)
			{
				irq_map->bdf = bdf;
				irq_map->intpin = intpin_c;
				irq_map->irq = irq;
				irq_map->next = NULL;
				if (cfg->irq_list == NULL) cfg->irq_list = irq_map;	// first
				if (tail == NULL) tail = irq_map;
				else
				{
					tail->next = irq_map;
					tail = irq_map;
				}
				slog_info(0, "\tIRQ map B%u:D%u:F%u %sINT%c --> irq %d",
							PCI_BUS(irq_map->bdf), PCI_DEV(irq_map->bdf), PCI_FUNC(irq_map->bdf),
							PCI_IS_ARI(bdf) ? "(ARI) " : " ",
							irq_map->intpin, irq_map->irq);
			}
		}
		p = end_of_line + 1;
		p = skip_comment(p);
	}
}

static int_t scan_bdf_val(const char *p, uint_t max_val_len, uint_t *val)
{
	char val_s[4] = {'\0'};		// bus can be up to 3 digits
	uint_t i;

	for(i=0; i<max_val_len; i++, ++p)
	{
		if (!isdigit(*p)) break;
		else val_s[i] = *p;
	}
	if (i == 0) return -1;
	else
	{
		*val = strtoul(val_s, NULL, 0);
		return i;
	}
}

static bool_t find_bdf_token(const char **fp, pci_bdf_t *bdf)
{
	int_t n;
	uint_t bus, dev, func;

	if ((**fp == 'B') && ((n = scan_bdf_val(++(*fp), 3, &bus)) > 0))
	{
		*fp += n;
		if (**fp == ':')
		{
			++(*fp);
			if ((**fp == 'D') && ((n = scan_bdf_val(++(*fp), 2, &dev)) > 0))
			{
				*fp += n;
				if (**fp == ':')
				{
					++(*fp);
					if ((**fp == 'F') && ((n = scan_bdf_val(++(*fp), 1, &func)) > 0))
					{
						*fp += n;
						if ((bus < PCI_MAX_BUSES) && (dev < PCI_BDF_MAX_DEVS) && (func < PCI_BDF_MAX_FUNCS))
						{
							*bdf = PCI_BDF(bus, dev, func);
							return isspace(**fp);	// must have trailing whitespace
						}
					}
				}
			}
			else if ((**fp == 'F') && ((n = scan_bdf_val(++(*fp), 2, &func)) > 0))
			{
				*fp += n;
				if ((bus < PCI_MAX_BUSES) && (func < PCI_ARI_MAX_FUNCS))
				{
					*bdf = PCI_BDF_ARI(bus, func);
					return isspace(**fp);	// must have trailing whitespace
				}
			}
		}
	}
	return false;
}

static bool_t find_intpin_token(const char **fp, char *intpin_c)
{
	while (**fp != '\n')
	{
		const char *s = *fp;
		++(*fp);

		if ((isalpha(*s) || (*s == '-')) && isspace(*(s - 1)) && isspace(*(s + 1)))
		{
			/* we've found an alpha surrounded by whitespace */
			*intpin_c = *s;
			return (strchr("ABCD-", toupper(*s)) != NULL) ? true : false;
		}
	}
	return false;
}

static bool_t find_irq_token(const char **fp, pci_irq_t *irq)
{
	while (**fp != '\n')
	{
		const char *s = *fp;
		++(*fp);

		if (isalnum(*s) && isspace(*(s - 1)))
		{
			/* we've found an alphanumeric */
			errno = 0;
			*irq = strtol(s, NULL, 0);
			return (errno == 0) ? true : false;
		}
	}
	return false;
}

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/services/pci/modules/hw/src/parse/parse_section_interrupts.c $ $Rev: 798837 $")
#endif
