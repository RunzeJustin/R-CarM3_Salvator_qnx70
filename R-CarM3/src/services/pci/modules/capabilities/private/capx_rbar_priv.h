#ifndef _CAPX_RBAR_PRIV_H_
#define _CAPX_RBAR_PRIV_H_
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
 capx_rbar_t

 The following types define the layout of the Resizable BAR PCIe extended
 capabilities structure as outlined in Fig 7-108 on pg 759 of the PCI
 Express Base Specification Rev 3.0., Nov 10, 2010

*/
// 32 bit message address structure
typedef struct __attribute__((packed,aligned(4)))
{
	uint16_t num_rbars;
	uint16_t rbar_vector;
	struct rbar_capability_s
	{
		capx_hdr_t hdr;
		struct rbar_entry_s
		{
			uint32_t size_vector;
			struct
			{
				uint8_t idx;	// bits 2:0 (for bar[0], bits 7:5 also contain the # of RBARs
				uint8_t size;	// bits 12:8
			} control;
			uint16_t reserved;
		} bar[6];
	} capability;
} capx_rbar_t;



#endif	/* _CAPX_RBAR_PRIV_H_ */

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/services/pci/modules/capabilities/private/capx_rbar_priv.h $ $Rev: 798837 $")
#endif
