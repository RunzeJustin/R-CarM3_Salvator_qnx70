#ifndef _HW_CFG_H_
#define _HW_CFG_H_
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

#include <pthread.h>
#include <atomic.h>
#include <sys/mman.h>
#include <assert.h>

#include <pci/pci.h>

#include "private/pci_lib.h"
#include "private/pci_debug.h"
#include "private/hwmod_api.h"


typedef enum
{
pcie_ctrl_type_e_first = 0,

	pcie_ctrl_type_e_NONE = pcie_ctrl_type_e_first,
	pcie_ctrl_type_e_RC,
	pcie_ctrl_type_e_EP,

pcie_ctrl_type_e_last = pcie_ctrl_type_e_EP
} pcie_ctrl_type_e;

typedef struct pin_to_irq_map_s
{
	struct pin_to_irq_map_s *next;
	pci_bdf_t bdf;
	char intpin;		// A, B, C or D
	pci_irq_t irq;
} pin_to_irq_map_t;

typedef struct csd_assignment_s
{
	struct csd_assignment_s *next;
	pci_vid_t vid;
	pci_did_t did;
	uint_t idx;
	pci_csd_t csd;
} csd_assignment_t;

typedef struct aspace_filter_s
{
	struct aspace_filter_s *next;
	_pci_asmap_t asmap;
} aspace_filter_t;

typedef struct rbar_override_s
{
	struct rbar_override_s *next;
	pci_vid_t vid;
	pci_did_t did;
	uint_t idx;
	struct rbar_info_s
	{
		int_t num;	// -1 indicates no entry
		pci_ba_val_t size;
	} bar[6];
} rbar_override_t;

typedef struct
{
	pin_to_irq_map_t *irq_list;
	csd_assignment_t *csd_list;
	aspace_filter_t *aspace_filter_list;
	rbar_override_t *rbar_override_list;
} hw_cfg_t;


extern void force_load_hwcfg_file(void);
extern void parse_config_file(const char const *fname, hw_cfg_t *cfg);


#endif	/* _HW_CFG_H_ */

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/services/pci/modules/hw/src/hw_cfg.h $ $Rev: 798837 $")
#endif
