#ifndef _BKWD_PCI_H_
#define _BKWD_PCI_H_
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

/*
 * Select structures and macros required from the old PCI server hw/pci.h
 */

struct pci_dev_info
{
	uint16_t		DeviceId;
	uint16_t		VendorId;
	uint16_t		SubsystemId;
	uint16_t		SubsystemVendorId;
	uint8_t			BusNumber;
	uint8_t			DevFunc;
	uint8_t			Revision;
	uint8_t			Rsvd[5];
	uint32_t		Class;
	uint32_t		Irq;

	uint64_t		CpuIoTranslation;	/* pci_addr = cpu_addr - translation */
	uint64_t		CpuMemTranslation;	/* pci_addr = cpu_addr - translation */
	uint64_t		CpuIsaTranslation;	/* pci_addr = cpu_addr - translation */
	uint64_t		CpuBmstrTranslation;/* pci_addr = cpu_addr + translation */

	uint64_t		PciBaseAddress[6];
	uint64_t		CpuBaseAddress[6];
	uint32_t		BaseAddressSize[6];
	uint64_t		PciRom;
	uint64_t		CpuRom;
	uint32_t		RomSize;
	uint32_t		Rsvd1;
	uint64_t		BusIoStart;
	uint64_t		BusIoEnd;
	uint64_t		BusMemStart;
	uint64_t		BusMemEnd;
	uint8_t			msi;
	uint8_t			Rsvd2 [3];
	uint32_t		Rsvd3;
};


#define	PCI_DEVNO(address)				((address) >> 3)
#define	PCI_FUNCNO(address)				((address) & 7)
#define	PCI_DEVFUNC(dev,func)			((dev<<3)|(func))
#define PCI_CLASS(class)				(((class) & 0xff0000) >> 16)
#define PCI_SUBCLASS(class)				(((class) & 0xff00) >> 8)
#define PCI_INTERFACE(class)			((class) & 0xff)

#define PCI_SHARE			0x00000001
#define PCI_PERSIST			0x00000002
#define	PCI_SEARCH_VEND		0x00000010
#define	PCI_SEARCH_VENDEV	0x00000020
#define	PCI_SEARCH_CLASS	0x00000040
#define	PCI_SEARCH_BUSDEV	0x00000080
#define	PCI_SEARCH_MASK		0x000000f0

#define PCI_INIT_ROM		0x00020000
#define PCI_INIT_BASE0		0x00040000

#define	PCI_MASTER_ENABLE	0x01000000
#define	PCI_USE_MSI			0x20000000	/* default */
#define	PCI_USE_MSIX		0x40000000

#define	PCI_MSI				1
#define	PCI_MSIX			2


#endif	/* _BKWD_PCI_H_ */

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/services/pci/modules/bkwd_compat/bkwd_pci.h $ $Rev: 798837 $")
#endif
