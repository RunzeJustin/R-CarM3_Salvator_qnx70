#ifndef _CAPX_VSEC_PRIV_H_
#define _CAPX_VSEC_PRIV_H_
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

#include "capx_priv.h"

/*
 ===============================================================================
 capx_vsec_t

 The following types define the layout of the generic portion of a Vendor
 Specific Extended PCIe capabilities structure as outlined on pg 745 of the PCIe
 Base Specification version 3

 Implementation Note
 -------------------
 When the number of bytes provided in the capability is obtained from the device
 it includes the 'capx_hdr_t' as well as the vsec_id and the length_and_revision
 fields.

*/

typedef struct __attribute__((packed,aligned(4)))
{
	capx_hdr_t hdr;
	struct
	{
		uint16_t id;
		uint16_t length_and_revision;
	} vsec;
	uint8_t  bytes[0];	/* an array of vendor specific bytes */
} capx_vsec_t;



#endif	/* _CAPX_VSEC_PRIV_H_ */

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/services/pci/modules/capabilities/private/capx_vsec_priv.h $ $Rev: 798837 $")
#endif
