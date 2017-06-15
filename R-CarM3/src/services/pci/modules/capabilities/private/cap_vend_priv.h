#ifndef _CAP_VEND_PRIV_H_
#define _CAP_VEND_PRIV_H_
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


/*
 ===============================================================================
 cap_vend_t

 The following types define the layout of the generic portion of a Vendor
 Specific PCI capabilities structure as outlined on pg 330 of the PCI Base
 Specification version 3

 Implementation Note
 -------------------
 When the number of bytes provided in the capability is obtained from the device
 it includes the 2 bytes for the 'capid' and 'next' fields. This will be
 corrected for when the capability is read so that 'num_bytes' represents the
 size of the 'bytes[]' array

*/

typedef struct __attribute__((packed,aligned(4)))
{
	pci_capid_t  capid;
	uint8_t  next;
	uint8_t  num_bytes;
	uint8_t  bytes[0];	/* an array of vendor specific bytes */
} cap_vend_t;



#endif	/* _CAP_VEND_PRIV_H_ */

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/services/pci/modules/capabilities/private/cap_vend_priv.h $ $Rev: 798837 $")
#endif
