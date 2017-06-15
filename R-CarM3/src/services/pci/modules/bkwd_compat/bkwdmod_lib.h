#ifndef _BKWDMOD_LIB_H_
#define _BKWDMOD_LIB_H_
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

/* prototypes */

extern pci_devhdl_t pci_bkwd_attach_device(uint32_t flags, uint_t idx, void *bufptr);
extern pci_err_t pci_bkwd_detach_device(pci_devhdl_t hdl);
extern pci_bdf_t pci_bkwd_device_find(uint_t idx, pci_vid_t vid, pci_did_t did, pci_ccode_t ccode);
extern pci_err_t pci_bkwd_read_config_bus(pci_bdf_t bdf, uint_t offset, uint_t cnt, size_t size, void *bufptr);
extern pci_err_t pci_bkwd_read_config(pci_devhdl_t hdl, uint_t offset, uint_t cnt, size_t size, void *bufptr);
extern pci_err_t pci_bkwd_write_config_bus(pci_bdf_t bdf, uint_t offset, uint_t cnt, size_t size, const void *bufptr);
extern pci_err_t pci_bkwd_write_config(pci_devhdl_t hdl, uint_t offset, uint_t cnt, size_t size, const void *bufptr);
extern void pci_bkwd_rescan_bus(void);
extern void pci_bkwd_pci_present(uint_t *lastbus, uint_t *version, uint_t *hardware);



#endif	/* _BKWDMOD_LIB_H_ */

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/services/pci/modules/bkwd_compat/bkwdmod_lib.h $ $Rev: 798837 $")
#endif
