#ifndef _CAP_PCIE_PRIV_H_
#define _CAP_PCIE_PRIV_H_
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


#define _PCIe_ECAM_ARI_BIT		28
#define PCIe_IS_ARI(_bdf_)		(((_bdf_) & (1u << _PCIe_ECAM_ARI_BIT)) != 0)

#define PCIe_ECAM_BDF_FUNC(_bdf_)		(PCIe_IS_ARI(_bdf_) ? _PCIe_ECAM_ARI_FUNC((_bdf_)) : _PCIe_ECAM_BDF_FUNC((_bdf_)))
#define PCIe_ECAM_BDF_DEV(_bdf_)		(PCIe_IS_ARI(_bdf_) ? _PCIe_ECAM_ARI_DEV((_bdf_)) : _PCIe_ECAM_BDF_DEV((_bdf_)))
#define PCIe_ECAM_BDF_BUS(_bdf_)		(PCIe_IS_ARI(_bdf_) ? _PCIe_ECAM_ARI_BUS((_bdf_)) : _PCIe_ECAM_BDF_BUS((_bdf_)))
#define PCIe_ECAM_BDF(_b_, _d_, _f_, _r_) \
		((uint32_t) \
			( \
				(((_b_) & 0xFFu) << 20) | \
				(((_d_) & 0x1Fu) << 15) | \
				(((_f_) & 0x7u) << 12) | \
				(((_r_) & 0xFFCu) << 0) \
			) \
		)

#define PCIe_ECAM_ARI(_b_, _f_, _r_) \
		((uint32_t) \
			( \
				(((_b_) & 0x7Fu) << 20) | \
				(((_f_) & 0xFFu) << 12) | \
				(((_r_) & 0xFFCu) << 0) | \
				(1u << _PCIe_ECAM_ARI_BIT) \
			) \
		)

#define _PCIe_ECAM_BDF_REG(_bdf_)		((((uint32_t)(_bdf_)) >> 0) & 0xFFCu)
#define _PCIe_ECAM_BDF_FUNC(_bdf_)		((((uint32_t)(_bdf_)) >> 12) & 0x7u)
#define _PCIe_ECAM_BDF_DEV(_bdf_)		((((uint32_t)(_bdf_)) >> 15) & 0x1Fu)
#define _PCIe_ECAM_BDF_BUS(_bdf_)		((((uint32_t)(_bdf_)) >> 20) & 0xFFu)

#define _PCIe_ECAM_ARI_REG(_bdf_)		((((uint32_t)(_bdf_)) >> 0) & 0xFFCu)
#define _PCIe_ECAM_ARI_FUNC(_bdf_)		((((uint32_t)(_bdf_)) >> 12) & 0xFFu)
#define _PCIe_ECAM_ARI_DEV(_bdf_)		((uint32_t)0)
#define _PCIe_ECAM_ARI_BUS(_bdf_)		((((uint32_t)(_bdf_)) >> 20) & 0xFFu)

/*
 ===============================================================================
 PCIe_OFFSET

 macro to convert a PCIe offset into a PCI offset
*/
#define PCIe_XCAP_OFFSET(_off_)		(0x100 + (_off_))

/*
 ===============================================================================
 cap_pcie_*_t

 The following types define the layout of the PCIe capabilities structure as
 outlined in Fig 7-10 on pg 605 of the PCIe Base Specification Rev 3.0.

 The first 3 types define the layout of 1) the header, 2) the device/link and
 slots capability, control and status registers and 3) the root capability,
 control and status registers

 The next 5 structures layout the structure for 1) root ports, 2) port with
 slots, 3) devices with links, 4) the root complex andd 5) common device
 registers

 The last type, cap_pcie_t is a union of the previous 5 types for the entire
 PCIe capability structure
*/
// PCIe capability header
typedef struct __attribute__((packed,aligned(4)))
{
	pci_capid_t  capid;
	uint8_t  next;
	uint16_t pcie_cap_reg;
} cap_pcie_hdr_t;

// device/link/slot capability, control and status registers
typedef struct __attribute__((packed,aligned(4)))
{
	uint32_t cap;
	uint16_t control;
	uint16_t status;
} cap_pcie_dls_t;

// root capability, control and status registers
typedef struct __attribute__((packed,aligned(4)))
{
	uint16_t control;
	uint16_t cap;
	uint32_t status;
} cap_pcie_root_t;

// root port capability registers
typedef struct __attribute__((packed,aligned(4)))
{
	cap_pcie_hdr_t hdr;
	cap_pcie_dls_t device;
	cap_pcie_dls_t link;
	cap_pcie_dls_t slot;
	cap_pcie_root_t root;
} cap_pcie_root_port_t;

// port with slots capability registers
typedef struct __attribute__((packed,aligned(4)))
{
	cap_pcie_hdr_t hdr;
	cap_pcie_dls_t device;
	cap_pcie_dls_t link;
	cap_pcie_dls_t slot;
	uint8_t reserved[sizeof(cap_pcie_root_t)];
	cap_pcie_dls_t device2;
	cap_pcie_dls_t link2;
	cap_pcie_dls_t slot2;
} cap_pcie_port_slots_t;

// devices with links capability registers
typedef struct __attribute__((packed,aligned(4)))
{
	cap_pcie_hdr_t hdr;
	cap_pcie_dls_t device;
	cap_pcie_dls_t link;
	uint8_t reserved1[sizeof(cap_pcie_dls_t)];
	uint8_t reserved2[sizeof(cap_pcie_root_t)];
	cap_pcie_dls_t device2;
	cap_pcie_dls_t link2;
} cap_pcie_dev_links_t;

// common device capability registers
typedef struct __attribute__((packed,aligned(4)))
{
	cap_pcie_hdr_t hdr;
	cap_pcie_dls_t device;
	uint8_t reserved1[sizeof(cap_pcie_dls_t)];
	uint8_t reserved2[sizeof(cap_pcie_dls_t)];
	cap_pcie_root_t root;
} cap_pcie_root_complex_t;

// common device capability registers
typedef struct __attribute__((packed,aligned(4)))
{
	cap_pcie_hdr_t hdr;
	cap_pcie_dls_t device;
	uint8_t reserved1[sizeof(cap_pcie_dls_t)];
	uint8_t reserved2[sizeof(cap_pcie_dls_t)];
	uint8_t reserved3[sizeof(cap_pcie_root_t)];
	cap_pcie_dls_t device2;
} cap_pcie_dev_t;

typedef union
{
	cap_pcie_hdr_t hdr;
	cap_pcie_dev_t dev;
	cap_pcie_dev_links_t dev_w_links;
	cap_pcie_port_slots_t ports_w_slots;
	cap_pcie_root_port_t root_port;
	cap_pcie_root_complex_t root_complex;
} cap_pcie_t;



#endif	/* _CAP_PCIE_PRIV_H_ */

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/services/pci/modules/capabilities/private/cap_pcie_priv.h $ $Rev: 804842 $")
#endif
