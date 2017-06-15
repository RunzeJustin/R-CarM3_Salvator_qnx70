#ifndef _CAP_PMI_H_
#define _CAP_PMI_H_
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

 This file contains public interfaces (functions and types) for accessing PMI
 specific capability data.

 The API's are only available after a successful call to pci_device_read_cap()

 ===============================================================================
*/
#define CAPID_PMI		0x1

/*
 ===============================================================================
 Power Management Interface PCI capability specific types
*/

/*
 * cap_pmi_state_e
 *
 * This type identifies the state to enter or retrieve info for in the PMI API's
 */
typedef enum
{
cap_pmi_state_e_first = 0,

	cap_pmi_state_e_D0 = cap_pmi_state_e_first,
	cap_pmi_state_e_D1,
	cap_pmi_state_e_D2,
	cap_pmi_state_e_D3hot,
	cap_pmi_state_e_D3cold,

cap_pmi_state_e_last = cap_pmi_state_e_D3cold,

	cap_pmi_state_e_ANY = (cap_pmi_state_e_D0 | cap_pmi_state_e_D1 |
						   cap_pmi_state_e_D2 | cap_pmi_state_e_D3hot |
						   cap_pmi_state_e_D3cold),

	cap_pmi_state_e_UNKNOWN = ~cap_pmi_state_e_ANY,

} cap_pmi_state_e;

/*
 * cap_pmi_power_t
 *
 * power consumption/dissipation values are real numbers in Watts
 */
typedef double	cap_pmi_power_t;

/*
 * cap_pmi_current_t
 *
 * current values are real numbers in milli-amps
 */
typedef double	cap_pmi_current_t;

typedef struct
{
	int_t (*version)(pci_cap_t cap);
	bool_t (*init_required)(pci_cap_t cap);
	bool_t (*state_supported)(pci_cap_t cap, cap_pmi_state_e state);
	bool_t (*pme_supported)(pci_cap_t cap, cap_pmi_state_e state);
	pci_err_t (*pme_enable)(pci_cap_t cap, pci_devhdl_t hdl);
	pci_err_t (*pme_disable)(pci_cap_t cap, pci_devhdl_t hdl);
	bool_t (*pme_isenabled)(pci_cap_t cap);
	cap_pmi_state_e (*state_get)(pci_cap_t cap);
	pci_err_t (*state_set)(pci_cap_t cap, pci_devhdl_t hdl, cap_pmi_state_e state);
	bool_t (*pme_clock_required)(pci_cap_t cap);
	cap_pmi_power_t (*power_consumption)(pci_cap_t cap, cap_pmi_state_e state);
	cap_pmi_power_t (*power_dissipation)(pci_cap_t cap, cap_pmi_state_e state);
	pci_err_t (*read_data)(pci_cap_t cap, uint_t data_sel, uint_t *data_val, uint_t *data_scale);
	cap_pmi_current_t (*V3_3aux_current)(pci_cap_t cap);

} cap_pmi_api_t;

#define PMI_API_VERSION		{.major = 1, .minor = 0}

#define PMI_API_CALL(_api_, _cap_, args...) \
		do { \
			assert((_cap_) != NULL);	/* bogus 'pci_cap_t' */ \
			assert((_cap_)->api_p != NULL);		/* 'pci_cap_t' not read */ \
			assert(((cap_pmi_api_t *)((_cap_)->api_p))->_api_ != NULL);	/* module does not implement API */ \
			const pci_version_t api_version = PMI_API_VERSION; \
			(_cap_)->api_version = api_version; \
			return ((cap_pmi_api_t *)((_cap_)->api_p))->_api_(_cap_, ## args); \
		} while(0)

/*
 ===============================================================================
 Power Management Interface PCI capability specific API's
*/

/* returns the PMI capability version. A value < 0 indicates an error */
static inline int_t cap_pmi_version(pci_cap_t cap)
{
	PMI_API_CALL(version, cap);
}
/* returns whether or not device specific initialization is required when transitioning to the D0 state
 * This function tests the state of bit 5 of the PMI capabilities register and is not related to bit 3
 * of the PMCSR
 */
static inline bool_t cap_pmi_init_required(pci_cap_t cap)
{
	PMI_API_CALL(init_required, cap);
}
/* returns whether <state> is supported */
static inline bool_t cap_pmi_state_supported(pci_cap_t cap, cap_pmi_state_e state)
{
	PMI_API_CALL(state_supported, cap, state);
}
/* returns whether a PME can be generated in the specified <state> */
static inline bool_t cap_pmi_pme_supported(pci_cap_t cap, cap_pmi_state_e state)
{
	PMI_API_CALL(pme_supported, cap, state);
}
/* enable/disable PME generation */
static inline pci_err_t cap_pmi_pme_enable(pci_devhdl_t hdl, pci_cap_t cap)
{
	PMI_API_CALL(pme_enable, cap, hdl);
}
static inline pci_err_t cap_pmi_pme_disable(pci_devhdl_t hdl, pci_cap_t cap)
{
	PMI_API_CALL(pme_disable, cap, hdl);
}
static inline bool_t cap_pmi_pme_isenabled(pci_cap_t cap)
{
	PMI_API_CALL(pme_isenabled, cap);
}
/* set/get the current PMI state */
static inline cap_pmi_state_e cap_pmi_state_get(pci_cap_t cap)
{
	PMI_API_CALL(state_get, cap);
}
static inline pci_err_t cap_pmi_state_set(pci_devhdl_t hdl, pci_cap_t cap, cap_pmi_state_e state)
{
	PMI_API_CALL(state_set, cap, hdl, state);
}
/* returns whether PME generation requires a PCI clock */
static inline bool_t cap_pmi_pme_clock_required(pci_cap_t cap)
{
	PMI_API_CALL(pme_clock_required, cap);
}

/* PMI data API's */
/*
 * return the power consumed/dissipated for the specified state.
 * Use cap_pmi_state_e_ANY for common logic power consumption
*/
static inline cap_pmi_power_t cap_pmi_power_consumption(pci_cap_t cap, cap_pmi_state_e state)
{
	PMI_API_CALL(power_consumption, cap, state);
}
static inline cap_pmi_power_t cap_pmi_power_dissipation(pci_cap_t cap, cap_pmi_state_e state)
{
	PMI_API_CALL(power_dissipation, cap, state);
}

/* a raw data API that will return the raw data and scale values for index <data_sel> */
static inline pci_err_t cap_pmi_read_data(pci_cap_t cap, uint_t data_sel, uint_t *data_val, uint_t *data_scale)
{
	PMI_API_CALL(read_data, cap, data_sel, data_val, data_scale);
}

static inline cap_pmi_current_t cap_pmi_V3_3aux_current(pci_cap_t cap)
{
	PMI_API_CALL(V3_3aux_current, cap);
}


#endif	/* _CAP_PMI_H_ */

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/services/pci/modules/capabilities/public/pci/cap_pmi.h $ $Rev: 798837 $")
#endif
