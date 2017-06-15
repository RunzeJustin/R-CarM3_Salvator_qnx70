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
#include <string.h>
#include <ctype.h>
#include <assert.h>

#include "parse.h"


/*
 ===============================================================================
 skip_comment

 skip over comments (anything after COMMENT_CHAR) to the end-of-line
*/
__attribute__ ((visibility ("internal")))
const char *skip_comment(const char *p)
{
	while (*p == COMMENT_CHAR)
	{
		while (*p != '\n') ++p;
		++p;
		if (*p == '\r') ++p;
	}
	return p;
}

/*
 ===============================================================================
 skip_line_continuation

 skip over the '\n' following a line continuation character
*/
__attribute__ ((visibility ("internal")))
const char *skip_line_continuation(const char *p)
{
	while (*p == LINE_CONTINUATION_CHAR)
	{
		while (*p != '\n') ++p;
		++p;
		if (*p == '\r') ++p;
	}
	return p;
}

/*
 ===============================================================================
 find_token

 Find <token> in the range between <start> and <end> inclusive.
 Returns a pointer to the token or NULL if not found

 To be found, the token must exist on a uncommented line

*/
__attribute__ ((visibility ("internal")))
const char *find_token(const char token, const char *start, const char *end)
{
	const char *p = start;
	while (p <= end)
	{
		p = skip_comment(p);
		if (*p == token) return p;
		else ++p;
	}
	return NULL;
}

/*
 ===============================================================================
 find_eol

 Find EOL (end-of-line) in the range between <start> and <end> inclusive.
 Returns a pointer to the EOL or NULL if not found

 If a line continuation character is encountered, the next '\n' is ignored

*/
__attribute__ ((visibility ("internal")))
const char *find_eol(const char *start, const char *end)
{
	const char *p = start;
	while (p <= end)
	{
//		p = skip_comment(p);
		if (*p == LINE_CONTINUATION_CHAR)
		{
			const char *next_eol = find_token('\n', p + 1, end);
			if (next_eol != NULL) return find_eol(next_eol + 1, end);
			else return NULL;	// malformed file
		}
		else if (*p == '\n') return p;
		else ++p;
	}
	return NULL;
}

/*
 ===============================================================================
 find_name

 Locate a valid name between <start> and <end> inclusive and if found return a
 pointer to the name and set <*len> otherwise return NULL.

 A valid name consists of set of valid characters (as determined by valid_char())
 surrounded by 0 or more valid whitespace characters (tab or space) between
 <start> and <end> inclusive

 If the caller does not provide a 'valid_char_f', the default
 valid_var_name_char() function will be used

*/
static const char *find_name_start(const char *start, const char *end, valid_char_f valid_char);
static const char *find_name_end(const char *start, const char *end, valid_char_f valid_char);
static bool_t valid_var_name_char(char c);

__attribute__ ((visibility ("internal")))
const char *find_name(const char *start, const char *end, uint_t *len, valid_char_f valid_char)
{
	if (valid_char == NULL) valid_char = valid_var_name_char;

	const char *name_start = find_name_start(start, end, valid_char);
	const char *name_end = find_name_end(name_start, end, valid_char);

	if  ((name_start == NULL) || (name_end == NULL)) return NULL;
	else
	{
		*len = name_end - name_start + 1;
		return name_start;
	}
}

/*
 ===============================================================================
 find_param

 Search <nentries> of the 'known_param_names_t' array <known_params> for the
 entry name <param_name>. If found return a pointer to the entry else return
 NULL

 The search is case invariant

*/
__attribute__ ((visibility ("internal")))
const known_param_names_t const *
find_param(const char *param_name, const known_param_names_t const *known_params, const uint_t nentries)
{
	uint_t i;

	for (i=0; i<nentries; i++)
	{
		if (strncasecmp(param_name, known_params[i].name, known_params[i].len) == 0)
		{
			return &known_params[i];
		}
	}
	return NULL;
}

/*
 ===============================================================================
 valid_var_name_char

 This function will return true or false depending on whether <c> is a valid
 character for a variable name.

 Each section parser has its own validity function for variable values

*/
static bool_t valid_var_name_char(char c)
{
	return isalnum(c) || (c == '_');
}

/*
 ===============================================================================
 find_name_start

 Helper for find_name()

 Locate the start of a name (designated by alphanumerics) between <start> and
 <end> inclusive.

 Return a pointer to the start of the alphanumeric name if found or NULL if not
 found

 A valid name must only be preceded by tabs or spaces
*/
static const char *find_name_start(const char *start, const char *end, valid_char_f valid_char)
{
	const char *name = NULL;
	const char *p = start;

	while (1)
	{
		p = skip_comment(p);
		if (p > end) break;

		if (name == NULL)
		{
			if (isblank(*p)) ++p;	// permitted
			else if (valid_char(*p))
			{
				/* found */
				name = p;
				break;
			}
			else break;
		}
	}
	return name;
}

/*
 ===============================================================================
 find_name_end

 Helper for find_name()

 Locate the end of a name (designated by alphanumerics) between <start> and
 <end> inclusive.

 Return a pointer to the end of the alphanumeric name if found or NULL if not
 found

 A valid name must only contain alphanumerics and be followed by tabs or spaces
 <start> should point to the start of an alphanumeric string. The first tab or
 space found will mark the end of the name and what will be returned as long
 as the trailing characters are only tabs or spaces
*/
static const char *find_name_end(const char *start, const char *end, valid_char_f valid_char)
{
	if (start == NULL) return NULL;
	else
	{
		const char *p = start;
		const char *name_end = NULL;

		while (1)
		{
			p = skip_comment(p);
			if (p > end) break;

			if (name_end == NULL)
			{
				if (isblank(*p)) name_end = p - 1;
				else if (!valid_char(*p)) return NULL;
				else if (p == end) name_end = p;
			}
			else
			{
				/* found the end. Make sure trailing characters are only tabs or spaces */
				if (!isblank(*p))
				{
					/* illegal trailing character */
					name_end = NULL;
					break;
				}
			}
			++p;
		}
		return name_end;
	}
}

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/services/pci/modules/hw/src/parse/parse_support.c $ $Rev: 798837 $")
#endif
