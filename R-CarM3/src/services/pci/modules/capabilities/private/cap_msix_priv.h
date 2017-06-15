#ifndef _CAP_MSIX_PRIV_H_
#define _CAP_MSIX_PRIV_H_
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
#include <pci/cap_msix.h>


/*
 ===============================================================================
 MSI-X Control register bit field masks
*/
#define MSIX_CAPABILITY_ENABLE_MASK	(1u << 15)
#define MSIX_FUNCTION_MASK			(1u << 14)
#define MSIX_TABLE_SIZE_MASK		(0x7FFu << 0)

/*
 ===============================================================================
 MSI-X Vector register and PBA bit field masks
*/
#define MSIX_VECTOR_TABLE_BIR_MASK		0x7u
#define MSIX_VECTOR_TABLE_OFFSET_MASK	((uint32_t)~MSIX_VECTOR_TABLE_BIR_MASK)
#define MSIX_VECTOR_ENABLE_MASK			(1u << 0)
/* the next define only applies when the TPH Requester Capability is present */
#define MSIX_VECTOR_STEERING_TAG_MASK	(0xFFFFu << 16)

#define MSIX_PBA_BIR_MASK				0x7u
#define MSIX_PBA_OFFSET_MASK			((uint32_t)~MSIX_PBA_BIR_MASK)

/*
 ===============================================================================
 cap_msix_*_t

 The following types define the layout of the MSI-X capabilities structure as
 outlined in Fig 6-10, 6-11 and 6-12 on pgs 238,239 of the PCI Local Bus
 Specification Rev 3.0.

*/
// MSI-X capability structure header
typedef struct __attribute__((packed,aligned(4)))
{
	pci_capid_t  capid;
	uint8_t  next;
	uint16_t msix_ctrl_reg;
	uint32_t vector_offset;
	uint32_t pba_offset;
} cap_msix_hdr_t;

// MSI-X vector (table entry) structure
typedef struct __attribute__((packed,aligned(4)))
{
	uint32_t addr;
	uint32_t addr_u;
	uint32_t data;
	uint32_t ctrl;
} cap_msix_vector_t;

// MSI-X capability structure (cap_specific)
typedef struct __attribute__((packed,aligned(4)))
{
	cap_msix_hdr_t hdr;
	uint_t nirq_req;			// the requested number of interrupts to use
	cap_msix_vector_t *vector;	// pointer to the memory mapped vector table (PCI server context, should not need to be volatile)
	cap_msix_pba_t *pba;		// pointer to the memory mapped pending bits array (PCI server context)
	int_t intdisp[0];			// disposition map array (entries with -1 disable the use of the interrupt)
} cap_msix_t;



#endif	/* _CAP_MSIX_PRIV_H_ */

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/services/pci/modules/capabilities/private/cap_msix_priv.h $ $Rev: 798837 $")
#endif
