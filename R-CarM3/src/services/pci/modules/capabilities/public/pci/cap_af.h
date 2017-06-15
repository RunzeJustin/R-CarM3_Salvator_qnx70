#ifndef _CAP_AF_H_
#define _CAP_AF_H_
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

#include <stdlib.h>
#include <dlfcn.h>
#include <assert.h>

#include <pci/pci.h>

/*
 ===============================================================================

 This file contains public interfaces (functions and types) for accessing PCI
 Advanced Features (AF) specific capability data.

 The API's are only available after a successful call to pci_device_read_cap()

 ===============================================================================
*/
#define CAPID_AF		0x13

typedef uint8_t	cap_af_cap_reg_t;
typedef uint8_t	cap_af_stat_reg_t;
typedef uint8_t	cap_af_ctrl_reg_t;

/*
 ===============================================================================
 AF PCI capability specific types
*/
typedef struct
{
	pci_err_t (*read_cap_reg)(pci_cap_t cap, cap_af_cap_reg_t *cap_reg);
	pci_err_t (*read_stat_reg)(pci_cap_t cap, cap_af_stat_reg_t *stat_reg);
	pci_err_t (*read_ctrl_reg)(pci_cap_t cap, cap_af_ctrl_reg_t *ctrl_reg);
	pci_err_t (*write_stat_reg)(pci_cap_t cap, pci_devhdl_t hdl, cap_af_stat_reg_t *stat_reg);
	pci_err_t (*write_ctrl_reg)(pci_cap_t cap, pci_devhdl_t hdl, cap_af_ctrl_reg_t *ctrl_reg);
} cap_af_api_t;

#define AF_API_VERSION		{.major = 1, .minor = 0}

#define AF_API_CALL(_api_, _cap_, args...) \
		do { \
			assert((_cap_) != NULL);	/* bogus 'pci_cap_t' */ \
			assert((_cap_)->api_p != NULL);		/* 'pci_cap_t' not read */ \
			assert(((cap_af_api_t *)((_cap_)->api_p))->_api_ != NULL);	/* module does not implement API */ \
			const pci_version_t api_version = AF_API_VERSION; \
			(_cap_)->api_version = api_version; \
			return ((cap_af_api_t *)((_cap_)->api_p))->_api_(_cap_, ## args); \
		} while(0)

/*
 ===============================================================================
 Advanced Features PCI capability specific API's
*/

/* get the AF capability register */
static inline pci_err_t cap_af_read_cap_reg(pci_cap_t cap, cap_af_cap_reg_t *cap_reg)
{
	AF_API_CALL(read_cap_reg, cap, cap_reg);
}

/* read/write the AF status register */
static inline pci_err_t cap_af_read_stat_reg(pci_cap_t cap, cap_af_stat_reg_t *stat_reg)
{
	AF_API_CALL(read_stat_reg, cap, stat_reg);
}

static inline pci_err_t cap_af_write_stat_reg(pci_devhdl_t hdl, pci_cap_t cap, cap_af_stat_reg_t *stat_reg)
{
	AF_API_CALL(write_stat_reg, cap, hdl, stat_reg);
}

/* read/write AF control register */
static inline pci_err_t cap_af_read_ctrl_reg(pci_cap_t cap, cap_af_ctrl_reg_t *ctrl_reg)
{
	AF_API_CALL(read_ctrl_reg, cap, ctrl_reg);
}

static inline pci_err_t cap_af_write_ctrl_reg(pci_devhdl_t hdl, pci_cap_t cap, cap_af_ctrl_reg_t *ctrl_reg)
{
	AF_API_CALL(write_ctrl_reg, cap, hdl, ctrl_reg);
}



#endif	/* _CAP_AF_H_ */

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/services/pci/modules/capabilities/public/pci/cap_af.h $ $Rev: 798837 $")
#endif
