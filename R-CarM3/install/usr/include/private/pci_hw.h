#ifndef _PCI_HW_H_
#define _PCI_HW_H_
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

#include <gulliver.h>

#include <pci/pci.h>
#include "private/hwmod_api.h"

// define NDEBUG to eliminate assert logic
#include <assert.h>

extern volatile const hwmod_api_t *pci_hw;

/*
 ===============================================================================
 _pci_device_cfg_rd8
 _pci_device_cfg_rd16
 _pci_device_cfg_rd32
 _pci_device_cfg_rd64
 _pci_device_cfg_rd

 4 convenience functions to read fixed size configuration space entries and 1
 to read an arbitrary number of bytes

 The fixed size functions will perform endian correction

 Note:
 	 - since these are internal API's we assert the offset correctness so that
 	   this code is not always present
*/
static inline pci_err_t _pci_device_cfg_rd8(pci_bdf_t bdf, uint_t offset, uint8_t *val)
{
	return pci_hw->cfg_rd(bdf, offset, sizeof(*val), val, _pci_accessAttr_e_NORMAL);
}
static inline pci_err_t _pci_device_cfg_rd16(pci_bdf_t bdf, uint_t offset, uint16_t *val)
{
	assert((offset & 1) == 0);
	uint16_t tmp;
	pci_err_t r = pci_hw->cfg_rd(bdf, offset, sizeof(tmp), (uint8_t *)&tmp, _pci_accessAttr_e_NORMAL);

	if (r == PCI_ERR_OK) *val = ENDIAN_LE16(tmp);
	return r;
}
static inline pci_err_t _pci_device_cfg_rd32(pci_bdf_t bdf, uint_t offset, uint32_t *val)
{
	assert((offset & 3) == 0);
	uint32_t tmp;
	pci_err_t r = pci_hw->cfg_rd(bdf, offset, sizeof(tmp), (uint8_t *)&tmp, _pci_accessAttr_e_NORMAL);

	if (r == PCI_ERR_OK) *val = ENDIAN_LE32(tmp);
	return r;
}
static inline pci_err_t _pci_device_cfg_rd64(pci_bdf_t bdf, uint_t offset, uint64_t *val)
{
	assert((offset & 7) == 0);
	uint64_t tmp;
	pci_err_t r = pci_hw->cfg_rd(bdf, offset, sizeof(tmp), (uint8_t *)&tmp, _pci_accessAttr_e_NORMAL);

	if (r == PCI_ERR_OK) *val = ENDIAN_LE64(tmp);
	return r;
}
static inline pci_err_t _pci_device_cfg_rd(pci_bdf_t bdf, uint_t offset, uint_t size, uint8_t *buf)
{
	return pci_hw->cfg_rd(bdf, offset, size, buf, _pci_accessAttr_e_NORMAL);
}

/*
 ===============================================================================
 _pci_device_cfg_wr8
 _pci_device_cfg_wr16
 _pci_device_cfg_wr32
 _pci_device_cfg_wr64

 4 convenience functions to write fixed size configuration space entries and 1
 to write an arbitrary number of bytes

 The fixed size functions will perform endian correction

 Note:
 	 - since these are internal API's we assert the offset correctness so that
 	   this code is not always present
*/
static inline pci_err_t _pci_device_cfg_wr8(pci_bdf_t bdf, uint_t offset, uint8_t val)
{
	return pci_hw->cfg_wr(bdf, offset, sizeof(val), (uint8_t *)&val, _pci_accessAttr_e_NORMAL);
}
static inline pci_err_t _pci_device_cfg_wr16(pci_bdf_t bdf, uint_t offset, uint16_t val)
{
	assert((offset & 1) == 0);
	uint16_t tmp = ENDIAN_LE16(val);
	return pci_hw->cfg_wr(bdf, offset, sizeof(tmp), (uint8_t *)&tmp, _pci_accessAttr_e_NORMAL);
}
static inline pci_err_t _pci_device_cfg_wr32(pci_bdf_t bdf, uint_t offset, uint32_t val)
{
	assert((offset & 3) == 0);
	uint32_t tmp = ENDIAN_LE32(val);
	return pci_hw->cfg_wr(bdf, offset, sizeof(tmp), (uint8_t *)&tmp, _pci_accessAttr_e_NORMAL);
}
static inline pci_err_t _pci_device_cfg_wr64(pci_bdf_t bdf, uint_t offset, uint64_t val)
{
	assert((offset & 7) == 0);
	uint64_t tmp = ENDIAN_LE64(val);
	return pci_hw->cfg_wr(bdf, offset, sizeof(tmp), (uint8_t *)&tmp, _pci_accessAttr_e_NORMAL);
}
static inline pci_err_t _pci_device_cfg_wr(pci_bdf_t bdf, uint_t offset, uint_t size, uint8_t *buf)
{
	return pci_hw->cfg_wr(bdf, offset, size, buf, _pci_accessAttr_e_NORMAL);
}

/*
 ===============================================================================
 The following are ISR callable versions of the above functions. These are used
 by API's which need to be called from an ISR. Because there is no way to know
 you are being called from an ISR, the API's will be explicitely named with an
 '_isr' suffix and utilize the following to convey that information to the HW
 dependent modules
 ===============================================================================
*/

/*
 ===============================================================================
 _pci_device_cfg_rd8_isr
 _pci_device_cfg_rd16_isr
 _pci_device_cfg_rd32_isr
 _pci_device_cfg_rd64_isr
 _pci_device_cfg_rd_isr

 4 convenience functions to read fixed size configuration space entries and 1
 to read an arbitrary number of bytes

 The fixed size functions will perform endian correction

 Note:
 	 - since these are internal API's we assert the offset correctness so that
 	   this code is not always present
*/
static inline pci_err_t _pci_device_cfg_rd8_isr(pci_bdf_t bdf, uint_t offset, uint8_t *val)
{
	return pci_hw->cfg_rd(bdf, offset, sizeof(*val), val, _pci_accessAttr_e_ISR);
}
static inline pci_err_t _pci_device_cfg_rd16_isr(pci_bdf_t bdf, uint_t offset, uint16_t *val)
{
	assert((offset & 1) == 0);
	uint16_t tmp;
	pci_err_t r = pci_hw->cfg_rd(bdf, offset, sizeof(tmp), (uint8_t *)&tmp, _pci_accessAttr_e_ISR);

	if (r == PCI_ERR_OK) *val = ENDIAN_LE16(tmp);
	return r;
}
static inline pci_err_t _pci_device_cfg_rd32_isr(pci_bdf_t bdf, uint_t offset, uint32_t *val)
{
	assert((offset & 3) == 0);
	uint32_t tmp;
	pci_err_t r = pci_hw->cfg_rd(bdf, offset, sizeof(tmp), (uint8_t *)&tmp, _pci_accessAttr_e_ISR);

	if (r == PCI_ERR_OK) *val = ENDIAN_LE32(tmp);
	return r;
}
static inline pci_err_t _pci_device_cfg_rd64_isr(pci_bdf_t bdf, uint_t offset, uint64_t *val)
{
	assert((offset & 7) == 0);
	uint64_t tmp;
	pci_err_t r = pci_hw->cfg_rd(bdf, offset, sizeof(tmp), (uint8_t *)&tmp, _pci_accessAttr_e_ISR);

	if (r == PCI_ERR_OK) *val = ENDIAN_LE64(tmp);
	return r;
}
static inline pci_err_t _pci_device_cfg_rd_isr(pci_bdf_t bdf, uint_t offset, uint_t size, uint8_t *buf)
{
	return pci_hw->cfg_rd(bdf, offset, size, buf, _pci_accessAttr_e_ISR);
}

/*
 ===============================================================================
 _pci_device_cfg_wr8_isr
 _pci_device_cfg_wr16_isr
 _pci_device_cfg_wr32_isr
 _pci_device_cfg_wr64_isr

 4 convenience functions to write fixed size configuration space entries and 1
 to write an arbitrary number of bytes

 The fixed size functions will perform endian correction

 Note:
 	 - since these are internal API's we assert the offset correctness so that
 	   this code is not always present
*/
static inline pci_err_t _pci_device_cfg_wr8_isr(pci_bdf_t bdf, uint_t offset, uint8_t val)
{
	return pci_hw->cfg_wr(bdf, offset, sizeof(val), (uint8_t *)&val, _pci_accessAttr_e_ISR);
}
static inline pci_err_t _pci_device_cfg_wr16_isr(pci_bdf_t bdf, uint_t offset, uint16_t val)
{
	assert((offset & 1) == 0);
	uint16_t tmp = ENDIAN_LE16(val);
	return pci_hw->cfg_wr(bdf, offset, sizeof(tmp), (uint8_t *)&tmp, _pci_accessAttr_e_ISR);
}
static inline pci_err_t _pci_device_cfg_wr32_isr(pci_bdf_t bdf, uint_t offset, uint32_t val)
{
	assert((offset & 3) == 0);
	uint32_t tmp = ENDIAN_LE32(val);
	return pci_hw->cfg_wr(bdf, offset, sizeof(tmp), (uint8_t *)&tmp, _pci_accessAttr_e_ISR);
}
static inline pci_err_t _pci_device_cfg_wr64_isr(pci_bdf_t bdf, uint_t offset, uint64_t val)
{
	assert((offset & 7) == 0);
	uint64_t tmp = ENDIAN_LE64(val);
	return pci_hw->cfg_wr(bdf, offset, sizeof(tmp), (uint8_t *)&tmp, _pci_accessAttr_e_ISR);
}
static inline pci_err_t _pci_device_cfg_wr_isr(pci_bdf_t bdf, uint_t offset, uint_t size, uint8_t *buf)
{
	return pci_hw->cfg_wr(bdf, offset, size, buf, _pci_accessAttr_e_ISR);
}



#endif	/* _PCI_HW_H_ */

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/lib/pci/private/pci_hw.h $ $Rev: 798837 $")
#endif
