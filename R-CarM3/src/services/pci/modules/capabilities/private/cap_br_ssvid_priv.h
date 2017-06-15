#ifndef _CAP_BR_SSVID_PRIV_H_
#define _CAP_BR_SSVID_PRIV_H_
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
 cap_br_ssvid_t

 The following types define the layout of the Slot Identification PCI
 capabilities structure as outlined in Fig 3-4 on pg 56 of the PCI-to-PCI Bridge
 Architecture Specification, Rev 1.2., Jun 9, 2003

*/

// 32 bit message address structure
typedef struct __attribute__((packed,aligned(4)))
{
	pci_capid_t  capid;
	uint8_t  next;
	uint16_t reserved;
	uint16_t ssvid;
	uint16_t ssid;
} cap_br_ssvid_t;




#endif	/* _CAP_BR_SSVID_PRIV_H_ */

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/services/pci/modules/capabilities/private/cap_br_ssvid_priv.h $ $Rev: 798837 $")
#endif
