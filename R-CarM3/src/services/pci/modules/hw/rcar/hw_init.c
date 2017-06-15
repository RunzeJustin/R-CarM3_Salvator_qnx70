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

#include <stddef.h>
#include <stdint.h>
#include <sys/mman.h>
#include <hw/inout.h>
#include <pthread.h>
#include <unistd.h>

#include <pci/pci.h>
#include <pci/pci_ccode.h>
#include <pci/cap_pcie.h>

#include "private/hwmod_api.h"
#include "private/pci_slog.h"
#include "hw_rcar.h"

#ifdef DUMP_INFO
static void dump_pcie_icfg_reg(const uint_t controller);
#endif

uint32_t rcar_pcie_rmw32(uint32_t val, uint32_t mask, uint32_t setting)
{
    val &= ~mask;
    val |= setting;
    return val;
}

/*
 ===============================================================================
 hw_init

 Do all required per controller hardware initialization

 */
static pci_err_t hw_init(const uint_t controller)
{
    pci_err_t r = PCI_ERR_OK;
    pcie_ctrl_info_t *pcie_ctrl_info = get_pcie_ctrl_info(controller);
    rcar_pcie_icfg_reg_t * const icfg_reg = get_pcie_icfg_reg_p(controller);

    if (pcie_ctrl_info != NULL)
    {
        /* Begin initialization */
        icfg_reg->PCIEMSR = 1; // PCI Express Root Port

        if (r == PCI_ERR_OK) r = init_outbound_atus(controller);
        if (r == PCI_ERR_OK) r = init_inbound_atus(controller);

        if (r != PCI_ERR_OK)
        {
            slog_error(0, "%s: unable to init ATUs", __func__);
            return PCI_ERR_EINVAL;
        }

        if(!(icfg_reg->PCIEPHYSR & 0x1))
        {
            slog_error(0, "%s: PHYRDY not ready", __func__);
            return PCI_ERR_EINVAL;
        }

        /* PCIe control registers */
        icfg_reg->IDSETR1 = RCAR_PCIE_CLASS_BRIDGE_PCI;

        /* Configuration Registers. */
        icfg_reg->EXPCAP[EXPCAP_CAP_IDX] = rcar_pcie_rmw32(icfg_reg->EXPCAP[EXPCAP_CAP_IDX], RCAR_PCIE_CAP_ID_MASK|RCAR_PCIE_PORT_TYPE_MASK, RCAR_PCIE_CAP_ID|RCAR_PCIE_PORT_TYPE_ROOT);
        icfg_reg->PCICONF[PCICONF_HEADER_TYPE_IDX] = rcar_pcie_rmw32(icfg_reg->PCICONF[PCICONF_HEADER_TYPE_IDX], RCAR_PCIE_HEADER_TYPE_MASK, RCAR_PCIE_HEADER_TYPE_01);
        icfg_reg->EXPCAP[EXPCAP_LINK_IDX] = rcar_pcie_rmw32(icfg_reg->EXPCAP[EXPCAP_LINK_IDX], RCAR_PCIE_SUPPORT_SPEED_MASK|RCAR_PCIE_DLLACTRPCAP_MASK, RCAR_PCIE_SUPPORT_SPEED|RCAR_PCIE_DLLACTRPCAP);
        icfg_reg->EXPCAP[EXPCAP_TARGET_SPEED_IDX] = rcar_pcie_rmw32(icfg_reg->EXPCAP[EXPCAP_TARGET_SPEED_IDX], RCAR_PCIE_TARGET_SPEED_MASK, RCAR_PCIE_TARGET_SPEED);
        icfg_reg->VCCAP[VCCAP_CAP_OFFSET_IDX] = rcar_pcie_rmw32(icfg_reg->VCCAP[VCCAP_CAP_OFFSET_IDX], RCAR_PCIE_CAP_OFFSET_MASK, RCAR_PCIE_CAP_OFFSET);

        icfg_reg->EXPCAP[EXPCAP_CAP_EN_IDX] = rcar_pcie_rmw32(icfg_reg->EXPCAP[EXPCAP_CAP_EN_IDX], RCAR_PCIE_CAP_MASK, RCAR_PCIE_SERRCEE|RCAR_PCIE_SERRNFEE|RCAR_PCIE_SERRFEE|RCAR_PCIE_PMEINTE|RCAR_PCIE_CRSVISE);

        /* Write out the physical slot number = 0 */
        icfg_reg->EXPCAP[EXPCAP_SLOT_NUM_IDX] = rcar_pcie_rmw32(icfg_reg->EXPCAP[EXPCAP_SLOT_NUM_IDX], RCAR_PCIE_SLOT_NUM_MASK, RCAR_PCIE_SLOT_NUM);

        /* Set the completion timer timeout to the maximum 50ms. */
        icfg_reg->TLCTLR = rcar_pcie_rmw32(icfg_reg->TLCTLR, RCAR_PCIE_COMP_TIMEOUT_MASK, RCAR_PCIE_COMP_TIMEOUT);

        /* Clear Interrupt Line */
        icfg_reg->PCICONF[PCICONF_INT_IDX] = rcar_pcie_rmw32(icfg_reg->PCICONF[PCICONF_INT_IDX], RCAR_PCIE_INT_MASK, RCAR_PCIE_INT_CLEAR);

        /* Disable MSI */
        icfg_reg->PCIEMSITXR = 0;
        icfg_reg->PCIEMSIALR = 0;
        icfg_reg->PCIEMSIAUR = 0;
        icfg_reg->PCIEMSIIER = 0;

        /* Finish initialization - establish a PCI Express link */
        icfg_reg->PCIETCTLR = RCAR_PCIE_CFINIT;
    }

    return r;
}

/*
 ===============================================================================
 rcar_pcie_init

 This function is only called by the PCI server when it starts up

 The PCIe controllers can be configured to operate as either EP or RC devices.
 Obviously if the PCI server is running, it needs to be a Root Complex (RC)
 so this function will ensure that it is indeed configured that way.

 In addition, some of the normally read only PCI configuration header is
 read/write and uninitialized. Specifically, the classcode of the device
 defaults to 0.

 This implementation will initialize the classcode to PCI_CCODE_BRIDGE_PCItoPCI
 so that it is handled as a bridge by the PCI server

*/
__attribute__ ((visibility ("internal")))
pci_err_t rcar_pcie_init(pcie_info_t * const pcie_info)
{
    assert(pci_runtime_flags & pci_runtime_flags_e_SERVER);

    pci_err_t r = PCI_ERR_EINVAL;
    uint_t controller;

    for (controller=0; controller<pcie_info->num_controllers; controller++)
    {
        slog_info(1, "++++ %s() [PCIe %u] start ++++", __FUNCTION__, controller);

        /* do required HW initialization */
        r = hw_init(controller);

        /* do any other initializations */
        slog_info(1, "++++ %s() [PCIe %u] end, %s ++++", __FUNCTION__, controller, pci_strerror(r));
    }
    return r;
}

#ifdef DUMP_INFO
static void dump_pcie_icfg_reg(const uint_t controller)
{
    rcar_pcie_icfg_reg_t * const icfg_reg = get_pcie_icfg_reg_p(controller);
    uint_t i = 0;

    slog_debug(0, "%s: PCIECAR=0x%08x", __func__, icfg_reg->PCIECAR);
    slog_debug(0, "%s: PCIECCTLR=0x%08x", __func__, icfg_reg->PCIECCTLR);
    slog_debug(0, "%s: PCIEMSR=0x%08x", __func__, icfg_reg->PCIEMSR);
    slog_debug(0, "%s: PCIEUNLOCKCR=0x%08x", __func__, icfg_reg->PCIEUNLOCKCR);
    slog_debug(0, "%s: PCIEINTXR=0x%08x", __func__, icfg_reg->PCIEINTXR);
    slog_debug(0, "%s: PCIEPHYSR=0x%08x", __func__, icfg_reg->PCIEPHYSR);
    slog_debug(0, "%s: PCIEMSITXR=0x%08x", __func__, icfg_reg->PCIEMSITXR);
    slog_debug(0, "%s: PCIETCTLR=0x%08x", __func__, icfg_reg->PCIETCTLR);
    slog_debug(0, "%s: PCIETSTR=0x%08x", __func__, icfg_reg->PCIETSTR);
    slog_debug(0, "%s: PCIEINTR=0x%08x", __func__, icfg_reg->PCIEINTR);
    slog_debug(0, "%s: PCIEINTER=0x%08x", __func__, icfg_reg->PCIEINTER);
    slog_debug(0, "%s: PCIEERRFR=0x%08x", __func__, icfg_reg->PCIEERRFR);
    slog_debug(0, "%s: PCIEMSIFR=0x%08x", __func__, icfg_reg->PCIEMSIFR);
    slog_debug(0, "%s: PCIEMSIALR=0x%08x", __func__, icfg_reg->PCIEMSIALR);
    slog_debug(0, "%s: PCIEMSIAUR=0x%08x", __func__, icfg_reg->PCIEMSIAUR);
    slog_debug(0, "%s: PCIEMSIIER=0x%08x", __func__, icfg_reg->PCIEMSIIER);

    for(i=0; i<6; i++)
    {
        slog_debug(0, "%s: PCIEPRAR[%d]=0x%08x", __func__, i, icfg_reg->PCIEPRAR[i]);
    }
    for(i=0; i<6; i++)
    {
        slog_debug(0, "%s: PCIELA[%d].PCIELAR=0x%08x", __func__, i, icfg_reg->PCIELA[i].PCIELAR);
        slog_debug(0, "%s: PCIELA[%d].PCIELAMR=0x%08x", __func__, i, icfg_reg->PCIELA[i].PCIELAMR);
    }
    for(i=0; i<4; i++)
    {
        slog_debug(0, "%s: PCIEP[%d].PCIEPALR=0x%08x", __func__, i, icfg_reg->PCIEP[i].PCIEPALR);
        slog_debug(0, "%s: PCIEP[%d].PCIEPAUR=0x%08x", __func__, i, icfg_reg->PCIEP[i].PCIEPAUR);
        slog_debug(0, "%s: PCIEP[%d].PCIEPAMR=0x%08x", __func__, i, icfg_reg->PCIEP[i].PCIEPAMR);
        slog_debug(0, "%s: PCIEP[%d].PCIEPTCTLR=0x%08x", __func__, i, icfg_reg->PCIEP[i].PCIEPTCTLR);
    }
    for(i=0; i<16; i++)
    {
        slog_debug(0, "%s: PCICONF[%d]=0x%08x", __func__, i, icfg_reg->PCICONF[i]);
    }
    for(i=0; i<2; i++)
    {
        slog_debug(0, "%s: PMCAP[%d]=0x%08x", __func__, i, icfg_reg->PMCAP[i]);
    }
    for(i=0; i<15; i++)
    {
        slog_debug(0, "%s: EXPCAP[%d]=0x%08x", __func__, i, icfg_reg->EXPCAP[i]);
    }
    for(i=0; i<7; i++)
    {
        slog_debug(0, "%s: VCCAP[%d]=0x%08x", __func__, i, icfg_reg->VCCAP[i]);
    }
    slog_debug(0, "%s: IDSETR0=0x%08x", __func__, icfg_reg->IDSETR0);
    slog_debug(0, "%s: IDSETR1=0x%08x", __func__, icfg_reg->IDSETR1);
    slog_debug(0, "%s: TLCTLR=0x%08x", __func__, icfg_reg->TLCTLR);
    slog_debug(0, "%s: MACSR=0x%08x", __func__, icfg_reg->MACSR);
    slog_debug(0, "%s: MACCTLR=0x%08x", __func__, icfg_reg->MACCTLR);
}
#endif

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL$ $Rev$")
#endif
