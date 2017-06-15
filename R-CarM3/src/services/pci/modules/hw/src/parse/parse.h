#ifndef _PARSE_H_
#define _PARSE_H_
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

#include <alloca.h>
#include <string.h>

#include "hw_cfg.h"


#define SLOG_PREFIX_STR			"Parse HW Config File: "

#define COMMENT_CHAR				'#'
#define SECTION_HDR_START_CHAR		'['
#define SECTION_HDR_END_CHAR		']'
#define LINE_CONTINUATION_CHAR		'\\'


/* make a null terminated string 'v_new' out of 'v'. Temporary local storage for 'v_new' will be provided */
#define MAKE_TMP_STRING(v_new, v, v_len) \
		do { \
			(v_new) = alloca((v_len) + 1); \
			memcpy((v_new), (v), (v_len)); \
			(v_new)[(v_len)] = '\0'; \
		} while(0)

/*
 ===============================================================================
 known_section_types

*/
typedef struct
{
	const char const *name;
	const uint_t len;
	void (*section_parser)(const char *section_start, const char *eof, hw_cfg_t *cfg);
} known_section_types_t;

/*
 ===============================================================================
 known_names_t

*/
typedef enum
{
	val_type_e_STRING,
	val_type_e_INTEGER,
	val_type_e_REAL,

} val_type_e;

typedef void (*param_handler_f)(const char *param, char *param_val, val_type_e type, const char delim, void *extra);

typedef struct
{
	const char const *name;
	const uint_t len;
	val_type_e type;		// type of value
	bool_t multi_val;		// whether or not the parameter can have multiple values
	const char delim;		// if multiple values are accepted, this char delimits each entry
	param_handler_f param_handler;
} known_param_names_t;

/*
 ===============================================================================
 valid_char_f

*/
typedef bool_t (*valid_char_f)(const char c);

/*
 ===============================================================================
 section_info_t

*/
typedef struct
{
	const char *start;		// pointer to first byte of section
	struct
	{
		const char *str;	// pointer to the section name
		uint_t len;			// length of the section name
	} name;
	struct
	{
		const char *start;	// pointer to first byte of section parameters
		const char *end;	// pointer to last byte of section
	} params;
} section_info_t;


typedef bool_t (*process_section_hw_specific_f)(const section_info_t *section_info, hw_cfg_t *cfg, void *arg);
extern process_section_hw_specific_f process_section_hw_specific;

/*
 ===============================================================================
 server function prototypes
*/
const char *find_eol(const char *start, const char *end);
const char *find_token(const char token, const char *start, const char *end);
const char *find_name(const char *start, const char *end, uint_t *len, valid_char_f valid_char);
const char *skip_comment(const char *p);
const char *skip_line_continuation(const char *p);
const known_param_names_t const *
find_param(const char *param_name, const known_param_names_t const *known_params, const uint_t nentries);

void parse_section_interrupts(const char *start, const char *end, hw_cfg_t *cfg);
void parse_section_slots(const char *start, const char *end, hw_cfg_t *cfg);
void parse_section_aspace(const char *start, const char *end, hw_cfg_t *cfg);
void parse_section_rbar(const char *start, const char *end, hw_cfg_t *cfg);



#endif	/* _PARSE_H_ */

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/services/pci/modules/hw/src/parse/parse.h $ $Rev: 798837 $")
#endif
