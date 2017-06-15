#ifndef _CAP_PCIE_H_
#define _CAP_PCIE_H_
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

 This file contains public interfaces (functions and types) for accessing PCIe
 specific capability data.

 The API's are only available after a successful call to pci_device_read_cap()

 ===============================================================================
*/
#define CAPID_PCIe		0x10

/*
 ===============================================================================
 PCIe capability specific types
*/
typedef uint16_t	pcie_capid_t;
typedef uint8_t		cap_pcie_portType_t;
typedef uint32_t	cap_pcie_cap_reg_t;
typedef uint16_t	cap_pcie_ctrl_reg_t;
typedef uint16_t	cap_pcie_stat_reg_t;
typedef uint16_t	cap_pcie_root_cap_reg_t;
typedef uint16_t	cap_pcie_root_ctrl_reg_t;
typedef uint32_t	cap_pcie_root_stat_reg_t;

typedef enum
{
	cap_pcie_portType_e_ENDPOINT = 0,
	cap_pcie_portType_e_LEGACY_ENDPOINT = 1,
	cap_pcie_portType_e_ROOT_PORT_of_ROOT_COMPLEX = 4,
	cap_pcie_portType_e_UPSTREAM_SWITCH_PORT = 5,
	cap_pcie_portType_e_DOWNSTREAM_SWITCH_PORT = 6,
	cap_pcie_portType_e_PCIe_TO_PCI_BRIDGE = 7,
	cap_pcie_portType_e_PCI_TO_PCIe_BRIDGE = 8,
	cap_pcie_portType_e_ROOT_COMPLEX_ENDPOINT = 9,
	cap_pcie_portType_e_ROOT_COMPLEX_EVENT_COLLECTOR = 10,

cap_pcie_portType_e_last = cap_pcie_portType_e_ROOT_COMPLEX_EVENT_COLLECTOR,

} cap_pcie_portType_e;

typedef pci_cap_t	pcie_cap_t;


typedef struct
{
	pci_err_t (*read_xtnd_capid)(pci_cap_t cap, pcie_capid_t *capid, uint_t idx);
	int_t (*find_xtnd_capid)(pci_cap_t cap, pcie_capid_t capid);
	int_t (*read_xtnd_capid_version)(pcie_cap_t xtnd_cap, pcie_capid_t capid);
	pci_err_t (*read_xtnd_cap)(pci_cap_t cap, pcie_cap_t *xtnd_cap, uint_t idx);
	int_t (*version)(pci_cap_t cap);
	pci_err_t (*read_portType)(pci_cap_t cap, cap_pcie_portType_t *portType);
	bool_t (*is_slot_implemented)(pci_cap_t cap);
	int_t (*read_slot_num)(pci_cap_t cap);
	int_t (*read_int_msgnum)(pci_cap_t cap);
	pci_err_t (*read_dev_cap_reg)(pci_cap_t cap, cap_pcie_cap_reg_t *dev_cap_reg);
	pci_err_t (*read_dev_cap_reg2)(pci_cap_t cap, cap_pcie_cap_reg_t *dev_cap_reg);
	pci_err_t (*read_dev_ctrl_reg)(pci_cap_t cap, cap_pcie_ctrl_reg_t *dev_ctrl_reg);
	pci_err_t (*read_dev_ctrl_reg2)(pci_cap_t cap, cap_pcie_ctrl_reg_t *dev_ctrl_reg);
	pci_err_t (*read_dev_stat_reg)(pci_cap_t cap, cap_pcie_stat_reg_t *dev_stat_reg);
	pci_err_t (*read_dev_stat_reg2)(pci_cap_t cap, cap_pcie_stat_reg_t *dev_stat_reg);
	pci_err_t (*write_dev_ctrl_reg)(pci_cap_t cap, pci_devhdl_t hdl, cap_pcie_ctrl_reg_t *dev_ctrl_reg);
	pci_err_t (*write_dev_ctrl_reg2)(pci_cap_t cap, pci_devhdl_t hdl, cap_pcie_ctrl_reg_t *dev_ctrl_reg);
	pci_err_t (*write_dev_stat_reg)(pci_cap_t cap, pci_devhdl_t hdl, cap_pcie_stat_reg_t *dev_stat_reg);
	pci_err_t (*write_dev_stat_reg2)(pci_cap_t cap, pci_devhdl_t hdl, cap_pcie_stat_reg_t *dev_stat_reg);
	pci_err_t (*read_link_cap_reg)(pci_cap_t cap, cap_pcie_cap_reg_t *link_cap_reg);
	pci_err_t (*read_link_cap_reg2)(pci_cap_t cap, cap_pcie_cap_reg_t *link_cap_reg);
	pci_err_t (*read_link_ctrl_reg)(pci_cap_t cap, cap_pcie_ctrl_reg_t *link_ctrl_reg);
	pci_err_t (*read_link_ctrl_reg2)(pci_cap_t cap, cap_pcie_ctrl_reg_t *link_ctrl_reg);
	pci_err_t (*read_link_stat_reg)(pci_cap_t cap, cap_pcie_stat_reg_t *link_stat_reg);
	pci_err_t (*read_link_stat_reg2)(pci_cap_t cap, cap_pcie_stat_reg_t *link_stat_reg);
	pci_err_t (*write_link_ctrl_reg)(pci_cap_t cap, pci_devhdl_t hdl, cap_pcie_ctrl_reg_t *link_ctrl_reg);
	pci_err_t (*write_link_ctrl_reg2)(pci_cap_t cap, pci_devhdl_t hdl, cap_pcie_ctrl_reg_t *link_ctrl_reg);
	pci_err_t (*write_link_stat_reg)(pci_cap_t cap, pci_devhdl_t hdl, cap_pcie_stat_reg_t *link_stat_reg);
	pci_err_t (*write_link_stat_reg2)(pci_cap_t cap, pci_devhdl_t hdl, cap_pcie_stat_reg_t *link_stat_reg);
	pci_err_t (*read_slot_cap_reg)(pci_cap_t cap, cap_pcie_cap_reg_t *slot_cap_reg);
	pci_err_t (*read_slot_cap_reg2)(pci_cap_t cap, cap_pcie_cap_reg_t *slot_cap_reg);
	pci_err_t (*read_slot_ctrl_reg)(pci_cap_t cap, cap_pcie_ctrl_reg_t *slot_ctrl_reg);
	pci_err_t (*read_slot_ctrl_reg2)(pci_cap_t cap, cap_pcie_ctrl_reg_t *slot_ctrl_reg);
	pci_err_t (*read_slot_stat_reg)(pci_cap_t cap, cap_pcie_stat_reg_t *slot_stat_reg);
	pci_err_t (*read_slot_stat_reg2)(pci_cap_t cap, cap_pcie_stat_reg_t *slot_stat_reg);
	pci_err_t (*write_slot_ctrl_reg)(pci_cap_t cap, pci_devhdl_t hdl, cap_pcie_ctrl_reg_t *slot_ctrl_reg);
	pci_err_t (*write_slot_ctrl_reg2)(pci_cap_t cap, pci_devhdl_t hdl, cap_pcie_ctrl_reg_t *slot_ctrl_reg);
	pci_err_t (*write_slot_stat_reg)(pci_cap_t cap, pci_devhdl_t hdl, cap_pcie_stat_reg_t *slot_stat_reg);
	pci_err_t (*write_slot_stat_reg2)(pci_cap_t cap, pci_devhdl_t hdl, cap_pcie_stat_reg_t *slot_stat_reg);
	pci_err_t (*read_root_cap_reg)(pci_cap_t cap, cap_pcie_root_cap_reg_t *root_cap_reg);
	pci_err_t (*read_root_ctrl_reg)(pci_cap_t cap, cap_pcie_root_ctrl_reg_t *root_ctrl_reg);
	pci_err_t (*read_root_stat_reg)(pci_cap_t cap, cap_pcie_root_stat_reg_t *root_stat_reg);
	pci_err_t (*write_root_ctrl_reg)(pci_cap_t cap, pci_devhdl_t hdl, cap_pcie_root_ctrl_reg_t *root_ctrl_reg);
	pci_err_t (*write_root_stat_reg)(pci_cap_t cap, pci_devhdl_t hdl, cap_pcie_root_stat_reg_t *root_stat_reg);

} cap_pcie_api_t;

#define PCIe_API_VERSION		{.major = 1, .minor = 0}

#define PCIe_API_CALL(_api_, _cap_, args...) \
		do { \
			assert((_cap_) != NULL);	/* bogus 'pci_cap_t' */ \
			assert((_cap_)->api_p != NULL);		/* 'pci_cap_t' not read */ \
			assert(((cap_pcie_api_t *)((_cap_)->api_p))->_api_ != NULL);	/* module does not implement API */ \
			const pci_version_t api_version = PCIe_API_VERSION; \
			(_cap_)->api_version = api_version; \
			return ((cap_pcie_api_t *)((_cap_)->api_p))->_api_(_cap_, ## args); \
		} while(0)

/*
 ===============================================================================
 PCIe capability specific API's

 For all write functions, a pointer to the value to be written is required and
 the value of the register AFTER the write completes will be returned in that
 parameter

*/

/*
 the following Extended capability API's allow the PCIe extended capabilities
 to be obtained. They are analogues of the pci_device_*_capid() functions. The
 PCIe capability must be read and enabled with pci_device_read_cap() and
 pci_device_cfg_cap_enable()

 The 'pci_cap_t' returned from the pci_device_read_cap() can then be used as a
 parameter to cap_pcie_read_xtnd_capid() and cap_pcie_find_xtnd_capid() to
 locate a desired extended capability ID. Once found, the extended capability
 can be obtained with cap_pcie_read_xtnd_cap() and the 'pcie_cap_t' used in API
 calls to the extended capabilities modules (capx_*())

*/
static inline pci_err_t cap_pcie_read_xtnd_capid(pci_cap_t cap, pcie_capid_t *capid, uint_t idx)
{
	PCIe_API_CALL(read_xtnd_capid, cap, capid, idx);
}

static inline int_t cap_pcie_find_xtnd_capid(pci_cap_t cap, pcie_capid_t capid)
{
	PCIe_API_CALL(find_xtnd_capid, cap, capid);
}

static inline int_t cap_pcie_read_xtnd_capid_version(pcie_cap_t xtnd_cap, pcie_capid_t capid)
{
	PCIe_API_CALL(read_xtnd_capid_version, xtnd_cap, capid);
}

static inline pci_err_t cap_pcie_read_xtnd_cap(pci_cap_t cap, pcie_cap_t *xtnd_cap, uint_t idx)
{
	PCIe_API_CALL(read_xtnd_cap, cap, xtnd_cap, idx);
}

/* the following API's obtain information from the PCIe capabilities register (offset 0x2) */

/* return the PCIe capability version. A value of < 0 indicates an error */
static inline int_t cap_pcie_version(pci_cap_t cap)
{
	PCIe_API_CALL(version, cap);
}

static inline pci_err_t cap_pcie_read_portType(pci_cap_t cap, cap_pcie_portType_t *portType)
{
	PCIe_API_CALL(read_portType, cap, portType);
}

static inline bool_t cap_pcie_is_slot_implemented(pci_cap_t cap)
{
	PCIe_API_CALL(is_slot_implemented, cap);
}
/* return the port slot number. A value of < 0 indicates an error */
static inline int_t cap_pcie_read_slot_num(pci_cap_t cap)
{
	PCIe_API_CALL(read_slot_num, cap);
}
/* return the interrupt number for status register changes. A return value < 0 indicates an error */
static inline int_t cap_pcie_read_int_msgnum(pci_cap_t cap)
{
	PCIe_API_CALL(read_int_msgnum, cap);
}

/*
 * the following API's access the PCIe device capabilities, control and status
 * registers at offsets 0x4/0x24, 0x8/0x28 and 0xA/0x2A respectively
*/
static inline pci_err_t cap_pcie_read_dev_cap_reg(pci_cap_t cap, cap_pcie_cap_reg_t *dev_cap_reg)
{
	PCIe_API_CALL(read_dev_cap_reg, cap, dev_cap_reg);
}

static inline pci_err_t cap_pcie_read_dev_cap_reg2(pci_cap_t cap, cap_pcie_cap_reg_t *dev_cap_reg)
{
	PCIe_API_CALL(read_dev_cap_reg2, cap, dev_cap_reg);
}

static inline pci_err_t cap_pcie_read_dev_ctrl_reg(pci_cap_t cap, cap_pcie_ctrl_reg_t *dev_ctrl_reg)
{
	PCIe_API_CALL(read_dev_ctrl_reg, cap, dev_ctrl_reg);
}

static inline pci_err_t cap_pcie_read_dev_ctrl_reg2(pci_cap_t cap, cap_pcie_ctrl_reg_t *dev_ctrl_reg)
{
	PCIe_API_CALL(read_dev_ctrl_reg2, cap, dev_ctrl_reg);
}

static inline pci_err_t cap_pcie_read_dev_stat_reg(pci_cap_t cap, cap_pcie_stat_reg_t *dev_stat_reg)
{
	PCIe_API_CALL(read_dev_stat_reg, cap, dev_stat_reg);
}

static inline pci_err_t cap_pcie_read_dev_stat_reg2(pci_cap_t cap, cap_pcie_stat_reg_t *dev_stat_reg)
{
	PCIe_API_CALL(read_dev_stat_reg2, cap, dev_stat_reg);
}

static inline pci_err_t cap_pcie_write_dev_ctrl_reg(pci_devhdl_t hdl, pci_cap_t cap, cap_pcie_ctrl_reg_t *dev_ctrl_reg)
{
	PCIe_API_CALL(write_dev_ctrl_reg, cap, hdl, dev_ctrl_reg);
}

static inline pci_err_t cap_pcie_write_dev_ctrl_reg2(pci_devhdl_t hdl, pci_cap_t cap, cap_pcie_ctrl_reg_t *dev_ctrl_reg)
{
	PCIe_API_CALL(write_dev_ctrl_reg2, cap, hdl, dev_ctrl_reg);
}

static inline pci_err_t cap_pcie_write_dev_stat_reg(pci_devhdl_t hdl, pci_cap_t cap, cap_pcie_stat_reg_t *dev_stat_reg)
{
	PCIe_API_CALL(write_dev_stat_reg, cap, hdl, dev_stat_reg);
}

static inline pci_err_t cap_pcie_write_dev_stat_reg2(pci_devhdl_t hdl, pci_cap_t cap, cap_pcie_stat_reg_t *dev_stat_reg)
{
	PCIe_API_CALL(write_dev_stat_reg2, cap, hdl, dev_stat_reg);
}

/*
 * the following API's access the PCIe link capabilities, control and status
 * registers at offsets 0xC/0x2C, 0x10/0x30 and 0x12/0x32 respectively
*/
static inline pci_err_t cap_pcie_read_link_cap_reg(pci_cap_t cap, cap_pcie_cap_reg_t *link_cap_reg)
{
	PCIe_API_CALL(read_link_cap_reg, cap, link_cap_reg);
}

static inline pci_err_t cap_pcie_read_link_cap_reg2(pci_cap_t cap, cap_pcie_cap_reg_t *link_cap_reg)
{
	PCIe_API_CALL(read_link_cap_reg2, cap, link_cap_reg);
}

static inline pci_err_t cap_pcie_read_link_ctrl_reg(pci_cap_t cap, cap_pcie_ctrl_reg_t *link_ctrl_reg)
{
	PCIe_API_CALL(read_link_ctrl_reg, cap, link_ctrl_reg);
}

static inline pci_err_t cap_pcie_read_link_ctrl_reg2(pci_cap_t cap, cap_pcie_ctrl_reg_t *link_ctrl_reg)
{
	PCIe_API_CALL(read_link_ctrl_reg2, cap, link_ctrl_reg);
}

static inline pci_err_t cap_pcie_read_link_stat_reg(pci_cap_t cap, cap_pcie_stat_reg_t *link_stat_reg)
{
	PCIe_API_CALL(read_link_stat_reg, cap, link_stat_reg);
}

static inline pci_err_t cap_pcie_read_link_stat_reg2(pci_cap_t cap, cap_pcie_stat_reg_t *link_stat_reg)
{
	PCIe_API_CALL(read_link_stat_reg2, cap, link_stat_reg);
}

static inline pci_err_t cap_pcie_write_link_ctrl_reg(pci_devhdl_t hdl, pci_cap_t cap, cap_pcie_ctrl_reg_t *link_ctrl_reg)
{
	PCIe_API_CALL(write_link_ctrl_reg, cap, hdl, link_ctrl_reg);
}

static inline pci_err_t cap_pcie_write_link_ctrl_reg2(pci_devhdl_t hdl, pci_cap_t cap, cap_pcie_ctrl_reg_t *link_ctrl_reg)
{
	PCIe_API_CALL(write_link_ctrl_reg2, cap, hdl, link_ctrl_reg);
}

static inline pci_err_t cap_pcie_write_link_stat_reg(pci_devhdl_t hdl, pci_cap_t cap, cap_pcie_stat_reg_t *link_stat_reg)
{
	PCIe_API_CALL(write_link_stat_reg, cap, hdl, link_stat_reg);
}

static inline pci_err_t cap_pcie_write_link_stat_reg2(pci_devhdl_t hdl, pci_cap_t cap, cap_pcie_stat_reg_t *link_stat_reg)
{
	PCIe_API_CALL(write_link_stat_reg2, cap, hdl, link_stat_reg);
}

/*
 * the following API's access the PCIe slot capabilities, control and status
 * registers at offsets 0x14/0x34, 0x18/0x38 and 0x1A/0x3A respectively
*/
static inline pci_err_t cap_pcie_read_slot_cap_reg(pci_cap_t cap, cap_pcie_cap_reg_t *slot_cap_reg)
{
	PCIe_API_CALL(read_slot_cap_reg, cap, slot_cap_reg);
}

static inline pci_err_t cap_pcie_read_slot_cap_reg2(pci_cap_t cap, cap_pcie_cap_reg_t *slot_cap_reg)
{
	PCIe_API_CALL(read_slot_cap_reg2, cap, slot_cap_reg);
}

static inline pci_err_t cap_pcie_read_slot_ctrl_reg(pci_cap_t cap, cap_pcie_ctrl_reg_t *slot_ctrl_reg)
{
	PCIe_API_CALL(read_slot_ctrl_reg, cap, slot_ctrl_reg);
}

static inline pci_err_t cap_pcie_read_slot_ctrl_reg2(pci_cap_t cap, cap_pcie_ctrl_reg_t *slot_ctrl_reg)
{
	PCIe_API_CALL(read_slot_ctrl_reg2, cap, slot_ctrl_reg);
}

static inline pci_err_t cap_pcie_read_slot_stat_reg(pci_cap_t cap, cap_pcie_stat_reg_t *slot_stat_reg)
{
	PCIe_API_CALL(read_slot_stat_reg, cap, slot_stat_reg);
}

static inline pci_err_t cap_pcie_read_slot_stat_reg2(pci_cap_t cap, cap_pcie_stat_reg_t *slot_stat_reg)
{
	PCIe_API_CALL(read_slot_stat_reg2, cap, slot_stat_reg);
}

static inline pci_err_t cap_pcie_write_slot_ctrl_reg(pci_devhdl_t hdl, pci_cap_t cap, cap_pcie_ctrl_reg_t *slot_ctrl_reg)
{
	PCIe_API_CALL(write_slot_ctrl_reg, cap, hdl, slot_ctrl_reg);
}

static inline pci_err_t cap_pcie_write_slot_ctrl_reg2(pci_devhdl_t hdl, pci_cap_t cap, cap_pcie_ctrl_reg_t *slot_ctrl_reg)
{
	PCIe_API_CALL(write_slot_ctrl_reg2, cap, hdl, slot_ctrl_reg);
}

static inline pci_err_t cap_pcie_write_slot_stat_reg(pci_devhdl_t hdl, pci_cap_t cap, cap_pcie_stat_reg_t *slot_stat_reg)
{
	PCIe_API_CALL(write_slot_stat_reg, cap, hdl, slot_stat_reg);
}

static inline pci_err_t cap_pcie_write_slot_stat_reg2(pci_devhdl_t hdl, pci_cap_t cap, cap_pcie_stat_reg_t *slot_stat_reg)
{
	PCIe_API_CALL(write_slot_stat_reg2, cap, hdl, slot_stat_reg);
}

/*
 * the following API's access the PCIe root capabilities, control and status
 * registers at offsets 0x1C, 0x1E and 0x20 respectively
*/
static inline pci_err_t cap_pcie_read_root_cap_reg(pci_cap_t cap, cap_pcie_root_cap_reg_t *root_cap_reg)
{
	PCIe_API_CALL(read_root_cap_reg, cap, root_cap_reg);
}

static inline pci_err_t cap_pcie_read_root_ctrl_reg(pci_cap_t cap, cap_pcie_root_ctrl_reg_t *root_ctrl_reg)
{
	PCIe_API_CALL(read_root_ctrl_reg, cap, root_ctrl_reg);
}

static inline pci_err_t cap_pcie_read_root_stat_reg(pci_cap_t cap, cap_pcie_root_stat_reg_t *root_stat_reg)
{
	PCIe_API_CALL(read_root_stat_reg, cap, root_stat_reg);
}

static inline pci_err_t cap_pcie_write_root_ctrl_reg(pci_devhdl_t hdl, pci_cap_t cap, cap_pcie_root_ctrl_reg_t *root_ctrl_reg)
{
	PCIe_API_CALL(write_root_ctrl_reg, cap, hdl, root_ctrl_reg);
}

static inline pci_err_t cap_pcie_write_root_stat_reg(pci_devhdl_t hdl, pci_cap_t cap, cap_pcie_root_stat_reg_t *root_stat_reg)
{
	PCIe_API_CALL(write_root_stat_reg, cap, hdl, root_stat_reg);
}




#endif	/* _CAP_PCIE_H_ */

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/services/pci/modules/capabilities/public/pci/cap_pcie.h $ $Rev: 798837 $")
#endif
