#ifndef _STRINGS_H_
#define _STRINGS_H_
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

#include <pci/pci.h>
#include <pci/pci_ccode.h>


extern const char *fp_end;


/* prototypes */

extern const char * const get_file_ptr(void);
extern const off64_t find_vid_offset(const char * const db_file_start, const pci_vid_t vid);
extern bool_t parse_dev_line(const char * const line, pci_did_t *did, const char **dev_str, uint_t * const dev_str_len);
extern bool_t parse_vend_line(const char * const line, pci_vid_t *vid, const char **vend_str, uint_t * const vend_str_len);
extern const char *next_line(const char *p);



#endif	/* _STRINGS_H_ */

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/services/pci/modules/strings/strings.h $ $Rev: 798837 $")
#endif
