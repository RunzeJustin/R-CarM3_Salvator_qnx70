#ifndef _CAP_MSI_PRIV_H_
#define _CAP_MSI_PRIV_H_
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
 MSI Control register bit field masks
*/
#define MSI_PVM_CAPABLE_MASK		(1u << 8)
#define MSI_64BIT_CAPABLE_MASK		(1u << 7)
#define MSI_MULTIMSG_CAPABLE_MASK	(7u << 1)
#define MSI_MULTIMSG_ENABLE_MASK	(7u << 4)
#define MSI_CAPABILITY_ENABLE_MASK	(1u << 0)


/*
 ===============================================================================
 cap_msi_*_t

 The following types define the layout of the MSI capabilities structure as
 outlined in Fig 6-9 on pg 233 of the PCI Local Bus Specification Rev 3.0.

 The last type, cap_msi_t is a union of the previous types for the entire
 MSI capability structure
*/
// MSI capability header
typedef struct __attribute__((packed,aligned(4)))
{
	pci_capid_t  capid;
	uint8_t  next;
	uint16_t msi_ctrl_reg;
} cap_msi_hdr_t;

// 32 bit message address structure
typedef struct __attribute__((packed,aligned(4)))
{
	cap_msi_hdr_t hdr;
	uint32_t addr;
	uint16_t data;
} cap_msi_32_t;

// 64 bit message address structure
typedef struct __attribute__((packed,aligned(4)))
{
	cap_msi_hdr_t hdr;
	uint32_t addr;
	uint32_t addr_u;	// Most significant 32 bits
	uint16_t data;
} cap_msi_64_t;

// 32 bit message address with per vector masking structure
typedef struct __attribute__((packed,aligned(4)))
{
	cap_msi_hdr_t hdr;
	uint32_t addr;
	uint16_t data;
	uint16_t reserved;
	uint32_t mask;
	uint32_t pend;
} cap_msi_32_pvm_t;

// 64 bit message address with per vector masking structure
typedef struct __attribute__((packed,aligned(4)))
{
	cap_msi_hdr_t hdr;
	uint32_t addr;
	uint32_t addr_u;	// Most significant 32 bits
	uint16_t data;
	uint16_t reserved;
	uint32_t mask;
	uint32_t pend;
} cap_msi_64_pvm_t;

typedef struct
{
	uint_t nirq_req;	// desired number of IRQ's
	union
	{
		cap_msi_hdr_t hdr;
		cap_msi_32_t msi_32;
		cap_msi_64_t msi_64;
		cap_msi_32_pvm_t msi_32_pvm;
		cap_msi_64_pvm_t msi_64_pvm;
	} u;
} cap_msi_t;



#endif	/* _CAP_MSI_PRIV_H_ */

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/services/pci/modules/capabilities/private/cap_msi_priv.h $ $Rev: 798837 $")
#endif
