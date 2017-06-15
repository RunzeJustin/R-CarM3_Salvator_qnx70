#ifndef _PCI_STRINGS_H_
#define _PCI_STRINGS_H_
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

/*
 ===============================================================================
 Strings Module

 This optional module provides API's for retrieving strings for PCI class codes
 and vendor/device ID's. It's main purpose is for utilities and tools that
 assist in the identification of devices by displaying human readable text.

 For Class codes, the strings are fixed and correspond to the entries in
 <pci/pci_ccode.h>

 For vendor and device ID's, the information is obtained from a <tab> delimited
 database file maintained at http://www.pcidatabase.com however the actual file
 to use is identified with the PCI_ENVVAR_STRINGS_FILE environment variable.
 This allows local additions, changes and corrections to be easily made and also
 allows for the removal of vendor/device ID's which will never exist in a given
 system thus reducing the search overhead when the strings module is required

*/

#define PCI_ENVVAR_STRINGS_FILE				"PCI_STRINGS_FILE"
#define PCI_ENVVAR_STRINGS_FILE_DEFAULT		"/etc/system/config/pci/pcidatabase.com-tab_delimited.txt"

/*
 ===============================================================================
 pci_strings_find_vid
 pci_strings_find_did

 Return a pointer to the Vendor/Device ID string respectively. The string
 returned must be released with free() when the caller is done with it

*/
const char *pci_strings_find_vid(const pci_vid_t vid);
const char *pci_strings_find_did(const pci_vid_t vid, const pci_did_t did);

/*
 ===============================================================================
 pci_strings_find_ccode

 Return a pointer to the class code specified by <ccode>. The string returned
 is fixed and DOES NOT need to be released as it does for pci_strings_find_vid()
 and pci_strings_find_did()

*/
const char *pci_strings_find_ccode(const pci_ccode_t ccode);



#endif	/* _PCI_STRINGS_H_ */

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/services/pci/modules/strings/public/pci/pci_strings.h $ $Rev: 798837 $")
#endif
