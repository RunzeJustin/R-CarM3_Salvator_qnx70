#ifndef _CAPX_SN_PRIV_H_
#define _CAPX_SN_PRIV_H_
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
#include <pci/cap_pcie.h>

#include "capx_priv.h"


/*
 ===============================================================================
 capx_sn_t

 The following types define the layout of the Device Serial Number PCIe
 extended capabilities structure as outlined in Fig 7-56 on pg 703 of the PCI
 Express Base Specification Rev 3.0., Nov 10, 2010

*/

// 32 bit message address structure
typedef struct __attribute__((packed,aligned(4)))
{
	capx_hdr_t hdr;
	uint32_t sernum_l;
	uint32_t sernum_h;
} capx_sn_t;




#endif	/* _CAPX_SN_PRIV_H_ */

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/services/pci/modules/capabilities/private/capx_sn_priv.h $ $Rev: 798837 $")
#endif
