#ifndef _CAP_MSI_H_
#define _CAP_MSI_H_
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

 This file contains public interfaces (functions and types) for accessing MSI
 specific capability data.

 The API's are only available after a successful call to pci_device_read_cap()

 ===============================================================================
*/
#define CAPID_MSI	0x5

/*
 ===============================================================================
 MSI capability specific types
*/
typedef uint32_t	cap_msi_pend_t;		/* pending interrupts register */
typedef uint32_t	cap_msi_mask_t;		/* interrupt entry mask register */

typedef struct
{
	uint_t (*get_nirq)(pci_cap_t cap);
	pci_err_t (*set_nirq)(pci_cap_t cap, pci_devhdl_t hdl, uint_t nirq);
	pci_err_t (*get_irq_pend)(pci_cap_t cap, pci_devhdl_t hdl, cap_msi_pend_t *pend);
	pci_err_t (*get_irq_mask)(pci_cap_t cap, pci_devhdl_t hdl, cap_msi_mask_t *mask);
	pci_err_t (*mask_irq)(pci_cap_t cap, pci_devhdl_t hdl, cap_msi_mask_t mask_val);
	pci_err_t (*unmask_irq)(pci_cap_t cap, pci_devhdl_t hdl, cap_msi_mask_t unmask_val);
	pci_err_t (*mask_irq_entry)(pci_cap_t cap, pci_devhdl_t hdl, uint_t irq_entry);
	pci_err_t (*unmask_irq_entry)(pci_cap_t cap, pci_devhdl_t hdl, uint_t irq_entry);
	/* ISR callable variants */
	uint_t (*get_nirq_isr)(pci_cap_t cap);
	pci_err_t (*get_irq_pend_isr)(pci_cap_t cap, pci_devhdl_t hdl, cap_msi_pend_t *pend);
	pci_err_t (*get_irq_mask_isr)(pci_cap_t cap, pci_devhdl_t hdl, cap_msi_mask_t *mask);
	pci_err_t (*mask_irq_isr)(pci_cap_t cap, pci_devhdl_t hdl, cap_msi_mask_t mask_val);
	pci_err_t (*unmask_irq_isr)(pci_cap_t cap, pci_devhdl_t hdl, cap_msi_mask_t unmask_val);
	pci_err_t (*mask_irq_entry_isr)(pci_cap_t cap, pci_devhdl_t hdl, uint_t irq_entry);
	pci_err_t (*unmask_irq_entry_isr)(pci_cap_t cap, pci_devhdl_t hdl, uint_t irq_entry);
} cap_msi_api_t;

#define MSI_API_VERSION		{.major = 1, .minor = 0}

#define MSI_API_CALL(_api_, _cap_, args...) \
		do { \
			assert((_cap_) != NULL);	/* bogus 'pci_cap_t' */ \
			assert((_cap_)->api_p != NULL);		/* 'pci_cap_t' not read */ \
			assert(((cap_msi_api_t *)((_cap_)->api_p))->_api_ != NULL);	/* module does not implement API */ \
			const pci_version_t api_version = MSI_API_VERSION; \
			(_cap_)->api_version = api_version; \
			return ((cap_msi_api_t *)((_cap_)->api_p))->_api_(_cap_, ## args); \
		} while(0)


/*
 ===============================================================================
 MSI capability specific API's

 MSI API's that may be called from an interrupt handler, have an '_isr' suffix

*/

/* the following API's allow driver software to configure the MSI capabilities */

/* configure the desired number of interrupts to use (takes effect on enable) */
static inline pci_err_t cap_msi_set_nirq(pci_devhdl_t hdl, pci_cap_t cap, uint_t nirq)
{
	MSI_API_CALL(set_nirq, cap, hdl, nirq);
}

/* retrieve the number of interrupts the device supports */
static inline uint_t cap_msi_get_nirq(pci_cap_t cap)
{
	MSI_API_CALL(get_nirq, cap);
}
static inline uint_t cap_msi_get_nirq_isr(pci_cap_t cap)
{
	MSI_API_CALL(get_nirq_isr, cap);
}

/* retrieve the pending interrupts register value (if supported) */
static inline pci_err_t cap_msi_get_irq_pend(pci_devhdl_t hdl, pci_cap_t cap, cap_msi_pend_t *pend)
{
	MSI_API_CALL(get_irq_pend, cap, hdl, pend);
}
static inline pci_err_t cap_msi_get_irq_pend_isr(pci_devhdl_t hdl, pci_cap_t cap, cap_msi_pend_t *pend)
{
	MSI_API_CALL(get_irq_pend_isr, cap, hdl, pend);
}

/* retrieve the interrupt mask register value (if supported) */
static inline pci_err_t cap_msi_get_irq_mask(pci_devhdl_t hdl, pci_cap_t cap, cap_msi_mask_t *mask)
{
	MSI_API_CALL(get_irq_mask, cap, hdl, mask);
}
static inline pci_err_t cap_msi_get_irq_mask_isr(pci_devhdl_t hdl, pci_cap_t cap, cap_msi_mask_t *mask)
{
	MSI_API_CALL(get_irq_mask_isr, cap, hdl, mask);
}

/* mask/unmask a set of interrupt entries (if supported) */
static inline pci_err_t cap_msi_mask_irq(pci_devhdl_t hdl, pci_cap_t cap, cap_msi_mask_t mask_val)
{
	MSI_API_CALL(mask_irq, cap, hdl, mask_val);
}
static inline pci_err_t cap_msi_mask_irq_isr(pci_devhdl_t hdl, pci_cap_t cap, cap_msi_mask_t mask_val)
{
	MSI_API_CALL(mask_irq_isr, cap, hdl, mask_val);
}
static inline pci_err_t cap_msi_unmask_irq(pci_devhdl_t hdl, pci_cap_t cap, cap_msi_mask_t unmask_val)
{
	MSI_API_CALL(unmask_irq, cap, hdl, unmask_val);
}
static inline pci_err_t cap_msi_unmask_irq_isr(pci_devhdl_t hdl, pci_cap_t cap, cap_msi_mask_t unmask_val)
{
	MSI_API_CALL(unmask_irq_isr, cap, hdl, unmask_val);
}

/* mask/unmask a specific interrupt entry (if supported) */
static inline pci_err_t cap_msi_mask_irq_entry(pci_devhdl_t hdl, pci_cap_t cap, uint_t irq_entry)
{
	MSI_API_CALL(mask_irq_entry, cap, hdl, irq_entry);
}
static inline pci_err_t cap_msi_mask_irq_entry_isr(pci_devhdl_t hdl, pci_cap_t cap, uint_t irq_entry)
{
	MSI_API_CALL(mask_irq_entry_isr, cap, hdl, irq_entry);
}
static inline pci_err_t cap_msi_unmask_irq_entry(pci_devhdl_t hdl, pci_cap_t cap, uint_t irq_entry)
{
	MSI_API_CALL(unmask_irq_entry, cap, hdl, irq_entry);
}
static inline pci_err_t cap_msi_unmask_irq_entry_isr(pci_devhdl_t hdl, pci_cap_t cap, uint_t irq_entry)
{
	MSI_API_CALL(unmask_irq_entry_isr, cap, hdl, irq_entry);
}


#endif	/* _CAP_MSI_H_ */

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/services/pci/modules/capabilities/public/pci/cap_msi.h $ $Rev: 798837 $")
#endif
