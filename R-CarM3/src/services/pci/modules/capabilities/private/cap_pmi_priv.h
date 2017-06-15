#ifndef _CAP_PMI_PRIV_H_
#define _CAP_PMI_PRIV_H_
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
 cap_pmi_t

 The following types define the layout of the Power Management Interface PCI
 capabilities structure as outlined in Fig 3-3 on pg 24 of the PCI Bus Power
 Management Interface Specification, Rev 1.2., Mar 3, 2004

*/

// 32 bit message address structure
typedef struct __attribute__((packed,aligned(4)))
{
	pci_capid_t  capid;
	uint8_t  next;
	uint16_t cap_reg;
	uint16_t ctrl_stat_reg;
	uint8_t bridge_ext_reg;		// only implemented in bridges
	uint8_t data_reg;
} cap_pmi_reg_t;

typedef struct
{
	pci_cmd_t bus_master;		// preserves the state of the bus master bit when leaving D0 state
	cap_pmi_reg_t reg;
} cap_pmi_t;



#endif	/* _CAP_PMI_PRIV_H_ */

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/services/pci/modules/capabilities/private/cap_pmi_priv.h $ $Rev: 798837 $")
#endif
