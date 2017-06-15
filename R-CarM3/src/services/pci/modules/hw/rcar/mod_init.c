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

#include <unistd.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/rsrcdbmgr.h>

#include <pci/pci.h>

#include "private/pci_slog.h"
#include "hw_rcar.h"


#define DUMP_INFO
#ifdef DUMP_INFO
static void dump_pcie_info(void);
#define DUMP_PCIE_INFO()    dump_pcie_info()
#else   /* DUMP_INFO */
#define DUMP_PCIE_INFO()
#endif  /* DUMP_INFO */



/*
 ===============================================================================
 _pcie_info

 The pcie_info structure contains all of the information required by the HW
 module. This structure is initialized in init_controller_info() to deal with
 the number of supported controllers

*/
static pcie_info_t _pcie_info =
{
    .num_controllers = 0,
    .ctrl = NULL,
};

/*
 ===============================================================================
 hwmod_access

 This structure is a table of function pointers for accessing PCI Configuration
 space in a HW dependent manner. If the HW_MODULE_INITFN does not exist, this
 symbol will be searched for. If HW_MODULE_INITFN does exist and returns
 PCI_ERR_OK, this symbol will not be searched for
*/
const hwmod_api_t HW_MODULE_ACCESS =
{
    .struct_size = sizeof(HW_MODULE_ACCESS),
    .mod_version = mod_version,
    .mod_compat = mod_compat,
    .cfg_rd = cfg_rd,           // utilize the common entry point
    .cfg_wr = cfg_wr,           // utilize the common entry point
    .alloc_irq = hw_alloc_irq,
    .free_irq = hw_free_irq,
    .alloc_as = hw_alloc_as,
    .resv_as = hw_resv_as,
    .free_as = hw_free_as,
    .map_as = hw_map_as,

    .add_device_hold_off = hw_add_device_hold_off,
    .remove_device_hold_off = hw_remove_device_hold_off,
    .initiate_hold_off = hw_initiate_hold_off,
    .release_hold_off = hw_release_hold_off,

    .find_csd_assignment = extcfg_find_csd_assignment,
    .check_rbar_override = extcfg_check_rbar_override,

    .reset = hw_reset,
};

/*
 ===============================================================================
 HW_MODULE_INITFN

 This optional function will be called (if it exists) after the HW dependent module
 is opened and prior to the search for the 'hwmod_api_t' function pointer table.

 If this function exists, it accepts as a parameter a pointer to a 'hwmod_api_t'
 pointer into which it can store a pointer to the HW dependent 'hwmod_api_t'
 structure.

 It returns PCI_ERR_OK on success, or one of the defined errors as follows

     PCI_ERR_EAGAIN - this init function should be called again

 This function is called once per process when the HW dependent module is
 loaded. It will setup the _pcie_ctrl_info[] array for each PCIe controller.

 Additionally, if the caller is the PCI server, the rsrcdb will be updated with
 address space and IRQ resource information.

*/
pci_err_t HW_MODULE_INITFN(volatile const hwmod_api_t **hw_p)
{
    /* setup '_pcie_info'. This is needed regardless of pci_runtime_flags_e_SERVER */
    pci_err_t r = init_controller_info(&_pcie_info);
    if (r == PCI_ERR_OK)
    {
        static pthread_once_t lock_once = PTHREAD_ONCE_INIT;
        extern void init_hw_shm(void);

        pthread_once(&lock_once, init_hw_shm);

        if (pci_runtime_flags & pci_runtime_flags_e_SERVER)
        {
            syspage_load_intrinfo();

            /* do hardware initialization */
            r = rcar_pcie_init(&_pcie_info);

            /* atu init done inside hw initialization due to hw flow */
        }
        else    /* non server processes */
        {
            uint_t i;
            for (i=0; i<_pcie_info.num_controllers; i++)
            {
                /* cache the non configuration address space information required by the rd/wr routines */
                if (r == PCI_ERR_OK) r = get_outbound_memspace_atu_info(i, iATU_OUTBOUND_id_e_MEM, &_pcie_info.ctrl[i].outbound.mem.cpu_phys.ba, &_pcie_info.ctrl[i].outbound.mem.pci_phys.ba);
                if (r == PCI_ERR_OK) r = get_outbound_memspace_atu_info(i, iATU_OUTBOUND_id_e_MEM_1, &_pcie_info.ctrl[i].outbound.mem1.cpu_phys.ba, &_pcie_info.ctrl[i].outbound.mem1.pci_phys.ba);
                if (r == PCI_ERR_OK) r = get_outbound_memspace_atu_info(i, iATU_OUTBOUND_id_e_MEM_PF, &_pcie_info.ctrl[i].outbound.pfmem.cpu_phys.ba, &_pcie_info.ctrl[i].outbound.pfmem.pci_phys.ba);
                if (r == PCI_ERR_OK) r = get_outbound_iospace_atu_info(i, &_pcie_info.ctrl[i].outbound.io.cpu_phys.ba, &_pcie_info.ctrl[i].outbound.io.pci_phys.ba);
                if (r == PCI_ERR_OK) r = get_inbound_memspace_atu_info(i, &_pcie_info.ctrl[i].inbound.mem.cpu_phys.ba, &_pcie_info.ctrl[i].inbound.mem.pci_phys.ba);
            }

        }
    }

    DUMP_PCIE_INFO();

    if (r == PCI_ERR_OK) *hw_p = &HW_MODULE_ACCESS;

    return r;
}

/*
 ===============================================================================
 restore_ATUs

 This function is called by the hw_reset() callout to restore the ATU's after
 a reset

*/
__attribute__ ((visibility ("internal")))
void restore_ATUs(const uint_t controller)
{
    assert(controller < _pcie_info.num_controllers);

    _pcie_info.ctrl[controller].init_level = 1;
    init_outbound_atus(controller);
    init_inbound_atus(controller);
}

/*
 ===============================================================================
 check_link_trained

 This function will check to make sure the link is trained and set the
 'pcie_info->link.trained' field accordingly. It is used in hw_rd() to make sure
 we don't fault and also during the initial link training in hw_init.c

 Returns whether or not the link is trained

 Implementation Note
 -------------------


*/
__attribute__ ((visibility ("internal")))
bool_t check_link_trained(const uint_t controller)
{
    assert(controller < _pcie_info.num_controllers);

    uint32_t val;
    rcar_pcie_icfg_reg_t * const icfg_reg = get_pcie_icfg_reg_p(controller);

    if(!icfg_reg->PCIETSTR & RCAR_PCIE_DATA_LINK_ACTIVE)
    {
        _pcie_info.ctrl[controller].link.trained = false;
    }
    else
    {
        _pcie_info.ctrl[controller].link.trained = true;
        val = icfg_reg->MACSR;
        switch(val & RCAR_PCIE_LINK_SPEED_MASK) {
            case RCAR_PCIE_LINK_SPEED_5_0:
                slog_info(1, "%s: PCIe x%d: 2.5GT/s link up", __FUNCTION__, RCAR_PCIE_LINK_WIDTH(val));
                break;
            case RCAR_PCIE_LINK_SPEED_2_5:
                slog_info(1, "%s: PCIe x%d: 5GT/s link up", __FUNCTION__, RCAR_PCIE_LINK_WIDTH(val));
                break;
            default:
                _pcie_info.ctrl[controller].link.trained = false;
        }
    }

    if(!_pcie_info.ctrl[controller].link.trained){
        slog_info(1, "%s: PCIe link down", __FUNCTION__);
    }

    return _pcie_info.ctrl[controller].link.trained;
}

/*
 ===============================================================================
 map_reg

 'once' function to mmap the controller internal registers and initialize
 'pcie_ctrl_info_t.icfg_reg'.

 Its called by get_pcie_icfg_reg_p()

 Implementation Note
 -------------------
 the 'icfg_map[]' array MUST contain an entry for each controller regardless of
 how many controllers are configured for use. The entry index corresponds to
 the controller number, 0 thru NUM_CONTROLLERS - 1.

 The assigned '_pcie_info.ctrl[].ctrl_num' is used to select which icfg_map[]
 entry is used to perform the mapping

*/
static pthread_once_t map_reg_once = PTHREAD_ONCE_INIT;
static const ctrl_map_info_t icfg_map[] = CTRL_ICFG_MAP_INITIALIZER;

static void map_reg(void)
{
    assert(_pcie_info.num_controllers <= NUM_CONTROLLERS);

    uint_t i;
    for (i=0; i<_pcie_info.num_controllers; i++)
    {
        /* map internal control registers */
        if((_pcie_info.ctrl[i].icfg_reg = (rcar_pcie_icfg_reg_t *)mmap_device_io (icfg_map[i].size, icfg_map[i].base)) == MAP_FAILED){
            slog_error(0, "Failed to map PCIe Controller %d Base resiters!!!\n", i);
            _pcie_info.ctrl[i].icfg_reg = NULL;
        }
    }
}

/*
 ===============================================================================
 get_pcie_icfg_reg_p

 This function should be used by any code within the HW dependent module that
 needs a pointer to the internal configuration registers.

*/
__attribute__ ((visibility ("internal")))
rcar_pcie_icfg_reg_t *get_pcie_icfg_reg_p(const uint_t controller)
{
    assert(controller < _pcie_info.num_controllers);

    if (_pcie_info.ctrl[0].icfg_reg == NULL)
    {
        pthread_once(&map_reg_once, map_reg);
    }
    return _pcie_info.ctrl[controller].icfg_reg;
}


/*
 ===============================================================================
 get_pcie_ctrl_info

 Return the 'pcie_ctrl_info_t' for <controller>

*/
__attribute__ ((visibility ("internal")))
pcie_ctrl_info_t *get_pcie_ctrl_info(const uint_t controller)
{
    assert(controller < _pcie_info.num_controllers);
    return &_pcie_info.ctrl[controller];
}

/*
 ===============================================================================
 get_pcie_ctrl_num

 Return the controller associated with <bdf> or -1 on error

*/
__attribute__ ((visibility ("internal")))
int_t get_pcie_ctrl_num(const pci_bdf_t bdf)
{
    if (bdf == PCI_BDF_NONE) return -1;
    else
    {
        const uint_t bus = PCI_BUS(bdf);
        int_t controller = -1;
        uint_t i;
        for (i=0; i<_pcie_info.num_controllers; i++)
        {
            if (bus >= _pcie_info.ctrl[i].link.num) controller = i;
        }
        return controller;
    }
}

/*
 ===============================================================================
 get_pcie_num_ctrls

 Return the number of configured controllers

*/
__attribute__ ((visibility ("internal")))
uint_t get_pcie_num_ctrls(void)
{
    return _pcie_info.num_controllers;
}


#ifdef DUMP_INFO
static void dump_pcie_info(void)
{
    uint_t i;

    for (i=0; i<_pcie_info.num_controllers; i++)
    {
        slog_debug(0, "===== PCIe %u INFO =====", i);
        slog_debug(0, "outbound mem1.cpu_phys: 0x%.16lx -> 0x%.16lx, attr:0x%x",
                        _pcie_info.ctrl[i].outbound.mem1.cpu_phys.ba.addr,
                        _pcie_info.ctrl[i].outbound.mem1.cpu_phys.ba.addr +  _pcie_info.ctrl[i].outbound.mem1.cpu_phys.ba.size - 1,
                        _pcie_info.ctrl[i].outbound.mem1.cpu_phys.ba.attr);
        slog_debug(0, "outbound mem1.pci_phys: 0x%.16lx -> 0x%.16lx, attr:0x%x",
                        _pcie_info.ctrl[i].outbound.mem1.pci_phys.ba.addr,
                        _pcie_info.ctrl[i].outbound.mem1.pci_phys.ba.addr +  _pcie_info.ctrl[i].outbound.mem1.pci_phys.ba.size - 1,
                        _pcie_info.ctrl[i].outbound.mem1.pci_phys.ba.attr);

        slog_debug(0, "outbound pfmem.cpu_phys: 0x%.16lx -> 0x%.16lx, attr:0x%x",
                        _pcie_info.ctrl[i].outbound.pfmem.cpu_phys.ba.addr,
                        _pcie_info.ctrl[i].outbound.pfmem.cpu_phys.ba.addr +  _pcie_info.ctrl[i].outbound.pfmem.cpu_phys.ba.size - 1,
                        _pcie_info.ctrl[i].outbound.pfmem.cpu_phys.ba.attr);
        slog_debug(0, "outbound pfmem.pci_phys: 0x%.16lx -> 0x%.16lx, attr:0x%x",
                        _pcie_info.ctrl[i].outbound.pfmem.pci_phys.ba.addr,
                        _pcie_info.ctrl[i].outbound.pfmem.pci_phys.ba.addr +  _pcie_info.ctrl[i].outbound.pfmem.pci_phys.ba.size - 1,
                        _pcie_info.ctrl[i].outbound.pfmem.pci_phys.ba.attr);

        slog_debug(0, "outbound mem.cpu_phys: 0x%.16lx -> 0x%.16lx, attr:0x%x",
                        _pcie_info.ctrl[i].outbound.mem.cpu_phys.ba.addr,
                        _pcie_info.ctrl[i].outbound.mem.cpu_phys.ba.addr +  _pcie_info.ctrl[i].outbound.mem.cpu_phys.ba.size - 1,
                        _pcie_info.ctrl[i].outbound.mem.cpu_phys.ba.attr);
        slog_debug(0, "outbound mem.pci_phys: 0x%.16lx -> 0x%.16lx, attr:0x%x",
                        _pcie_info.ctrl[i].outbound.mem.pci_phys.ba.addr,
                        _pcie_info.ctrl[i].outbound.mem.pci_phys.ba.addr +  _pcie_info.ctrl[i].outbound.mem.pci_phys.ba.size - 1,
                        _pcie_info.ctrl[i].outbound.mem.pci_phys.ba.attr);

        slog_debug(0, "outbound io.cpu_phys: 0x%.16lx -> 0x%.16lx, attr:0x%x",
                        _pcie_info.ctrl[i].outbound.io.cpu_phys.ba.addr,
                        _pcie_info.ctrl[i].outbound.io.cpu_phys.ba.addr +  _pcie_info.ctrl[i].outbound.io.cpu_phys.ba.size - 1,
                        _pcie_info.ctrl[i].outbound.io.cpu_phys.ba.attr);
        slog_debug(0, "outbound io.pci_phys: 0x%.16lx -> 0x%.16lx, attr:0x%x",
                        _pcie_info.ctrl[i].outbound.io.pci_phys.ba.addr,
                        _pcie_info.ctrl[i].outbound.io.pci_phys.ba.addr +  _pcie_info.ctrl[i].outbound.io.pci_phys.ba.size - 1,
                        _pcie_info.ctrl[i].outbound.io.pci_phys.ba.attr);

        slog_debug(0, "inbound mem.cpu_phys: 0x%.16lx -> 0x%.16lx, attr:0x%x",
                        _pcie_info.ctrl[i].inbound.mem.cpu_phys.ba.addr,
                        _pcie_info.ctrl[i].inbound.mem.cpu_phys.ba.addr +  _pcie_info.ctrl[i].inbound.mem.cpu_phys.ba.size - 1,
                        _pcie_info.ctrl[i].inbound.mem.cpu_phys.ba.attr);
        slog_debug(0, "inbound mem.pci_phys: 0x%.16lx -> 0x%.16lx, attr:0x%x",
                        _pcie_info.ctrl[i].inbound.mem.pci_phys.ba.addr,
                        _pcie_info.ctrl[i].inbound.mem.pci_phys.ba.addr +  _pcie_info.ctrl[i].inbound.mem.pci_phys.ba.size - 1,
                        _pcie_info.ctrl[i].inbound.mem.pci_phys.ba.attr);

        /*
         * because the link will never look trained at this point for non server processes
         * and INT[A-D] will always show 0 don't alarm anyone looking at the slogs
         */
        if (pci_runtime_flags & pci_runtime_flags_e_SERVER)
        {
            slog_debug(0, "icfg_reg @ %p, link.trained: %s, interrupts.pin[A-D] = %d, %d, %d, %d", _pcie_info.ctrl[i].icfg_reg,
                            _pcie_info.ctrl[i].link.trained ? "YES" : "NO", _pcie_info.ctrl[i].interrupts.pin[0],
                            _pcie_info.ctrl[i].interrupts.pin[1], _pcie_info.ctrl[i].interrupts.pin[2], _pcie_info.ctrl[i].interrupts.pin[3]);
        }
    }
}
#endif  /* DUMP_INFO */

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL$ $Rev$")
#endif
