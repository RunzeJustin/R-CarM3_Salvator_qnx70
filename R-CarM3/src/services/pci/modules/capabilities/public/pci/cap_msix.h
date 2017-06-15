#ifndef _CAP_MSIX_H_
#define _CAP_MSIX_H_
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

#include <stdint.h>
#include <inttypes.h>
#include <assert.h>

#include <pci/pci.h>

/*
 ===============================================================================

 This file contains public interfaces (functions and types) for accessing MSIX
 specific capability data.

 The API's are only available after a successful call to pci_device_read_cap()

 ===============================================================================
*/
#define CAPID_MSIX	0x11

/*
 ===============================================================================
 MSIX capability specific types

*/

/* MSI-X Pending Bit Array (PBA) entry structure */
typedef uint64_t	cap_msix_pba_t;

typedef struct
{
	uint_t (*get_nirq)(pci_cap_t cap);
	pci_err_t (*set_nirq)(pci_cap_t cap, pci_devhdl_t hdl, uint_t nirq);
	pci_err_t (*set_irq_entry)(pci_cap_t cap, pci_devhdl_t hdl, uint_t irq_entry, int_t disposition);
	cap_msix_pba_t *(*get_pba_ptr)(pci_cap_t cap, pci_devhdl_t hdl);
	pci_err_t (*mask_irq_entry)(pci_cap_t cap, pci_devhdl_t hdl, uint_t irq_entry);
	pci_err_t (*unmask_irq_entry)(pci_cap_t cap, pci_devhdl_t hdl, uint_t irq_entry);
	pci_err_t (*mask_irq_all)(pci_cap_t cap, pci_devhdl_t hdl);
	pci_err_t (*unmask_irq_all)(pci_cap_t cap, pci_devhdl_t hdl);
	/* ISR callable */
	uint_t (*get_nirq_isr)(pci_cap_t cap);
	pci_err_t (*mask_irq_entry_isr)(pci_cap_t cap, pci_devhdl_t hdl, uint_t irq_entry);
	pci_err_t (*unmask_irq_entry_isr)(pci_cap_t cap, pci_devhdl_t hdl, uint_t irq_entry);
	pci_err_t (*mask_irq_all_isr)(pci_cap_t cap, pci_devhdl_t hdl);
	pci_err_t (*unmask_irq_all_isr)(pci_cap_t cap, pci_devhdl_t hdl);

} cap_msix_api_t;

#define MSIX_API_VERSION		{.major = 1, .minor = 0}

#define MSIX_API_CALL(_api_, _cap_, args...) \
		do { \
			assert((_cap_) != NULL);	/* bogus 'pci_cap_t' */ \
			assert((_cap_)->api_p != NULL);		/* 'pci_cap_t' not read */ \
			assert(((cap_msix_api_t *)((_cap_)->api_p))->_api_ != NULL);	/* module does not implement API */ \
			const pci_version_t api_version = MSIX_API_VERSION; \
			(_cap_)->api_version = api_version; \
			return ((cap_msix_api_t *)((_cap_)->api_p))->_api_(_cap_, ## args); \
		} while(0)

/*
 ===============================================================================
 MSIX capability specific API's

 MSI-X API's that may be called from an interrupt handler, have an '_isr' suffix

*/

/* the following API's allow driver software to configure the MSI-X capabilities */

/* configure the desired number of interrupts to use (takes effect on enable) */
static inline pci_err_t cap_msix_set_nirq(pci_devhdl_t hdl, pci_cap_t cap, uint_t nirq)
{
	MSIX_API_CALL(set_nirq, cap, hdl, nirq);
}

/* retrieve the number of interrupts the device supports */
static inline uint_t cap_msix_get_nirq(pci_cap_t cap)
{
	MSIX_API_CALL(get_nirq, cap);
}
static inline uint_t cap_msix_get_nirq_isr(pci_cap_t cap)
{
	MSIX_API_CALL(get_nirq_isr, cap);
}

/* set the disposition of the interrupt entry (takes effect on enable) */
static inline pci_err_t cap_msix_set_irq_entry(pci_devhdl_t hdl, pci_cap_t cap, uint_t irq_entry, int_t disposition)
{
	MSIX_API_CALL(set_irq_entry, cap, hdl, irq_entry, disposition);
}

/* obtain a read-only pointer to the 'cap_msix_pba_t' array (PBA) */
static inline cap_msix_pba_t *cap_msix_get_pba_ptr(pci_devhdl_t hdl, pci_cap_t cap)
{
	MSIX_API_CALL(get_pba_ptr, cap, hdl);
}

/* mask/unmask a specific interrupt entry */
static inline pci_err_t cap_msix_mask_irq_entry(pci_devhdl_t hdl, pci_cap_t cap, uint_t irq_entry)
{
	MSIX_API_CALL(mask_irq_entry, cap, hdl, irq_entry);
}
static inline pci_err_t cap_msix_mask_irq_entry_isr(pci_devhdl_t hdl, pci_cap_t cap, uint_t irq_entry)
{
	MSIX_API_CALL(mask_irq_entry_isr, cap, hdl, irq_entry);
}
static inline pci_err_t cap_msix_unmask_irq_entry(pci_devhdl_t hdl, pci_cap_t cap, uint_t irq_entry)
{
	MSIX_API_CALL(unmask_irq_entry, cap, hdl, irq_entry);
}
static inline pci_err_t cap_msix_unmask_irq_entry_isr(pci_devhdl_t hdl, pci_cap_t cap, uint_t irq_entry)
{
	MSIX_API_CALL(unmask_irq_entry_isr, cap, hdl, irq_entry);
}

/* mask/unmask all interrupts for a device. These do not affect individual entry masks */
static inline pci_err_t cap_msix_mask_irq_all(pci_devhdl_t hdl, pci_cap_t cap)
{
	MSIX_API_CALL(mask_irq_all, cap, hdl);
}
static inline pci_err_t cap_msix_mask_irq_all_isr(pci_devhdl_t hdl, pci_cap_t cap)
{
	MSIX_API_CALL(mask_irq_all_isr, cap, hdl);
}
static inline pci_err_t cap_msix_unmask_irq_all(pci_devhdl_t hdl, pci_cap_t cap)
{
	MSIX_API_CALL(unmask_irq_all, cap, hdl);
}
static inline pci_err_t cap_msix_unmask_irq_all_isr(pci_devhdl_t hdl, pci_cap_t cap)
{
	MSIX_API_CALL(unmask_irq_all_isr, cap, hdl);
}




#endif	/* _CAP_MSIX_H_ */

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/services/pci/modules/capabilities/public/pci/cap_msix.h $ $Rev: 798837 $")
#endif
