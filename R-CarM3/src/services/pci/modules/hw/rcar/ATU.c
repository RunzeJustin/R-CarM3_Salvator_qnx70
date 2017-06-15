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
#include <pthread.h>
#include <sys/mman.h>
#include <sys/rsrcdbmgr.h>

#include <pci/pci.h>
#include <pci/pci_ccode.h>

#include "private/hwmod_api.h"
#include "private/pci_lib.h"
#include "hw_lib.h"
#include "hw_rcar.h"
#include "private/pci_slog.h"
#include "private/pci_debug.h"

/*
 ===============================================================================
 Notes on ATU usage

 ATU assignments are variant specific and defined within the ATU_config.c and/or
 hw_variant.h file for the specific hardware variant. Various other types and
 macros are also defined within those variant files

*/

pci_err_t config_outbound_atus(const uint_t controller, iATU_config_t * atu_config)
{
    uint32_t mask = atu_config->mask;

    if(atu_config->id >= NUM_OUTBOUND_ATUS)
    {
        slog_error(0, "%s Invalid outbound ATUs", __func__);
        return PCI_ERR_EINVAL;
    }

    rcar_pcie_icfg_reg_t * const icfg_reg = get_pcie_icfg_reg_p(controller);

    icfg_reg->PCIEP[atu_config->id].PCIEPTCTLR = 0x00000000;
    icfg_reg->PCIEP[atu_config->id].PCIEPAMR = mask << 7;
    icfg_reg->PCIEP[atu_config->id].PCIEPAUR = upper_32_bits(atu_config->aspace.cpu_phys.ba.addr);
    icfg_reg->PCIEP[atu_config->id].PCIEPALR = lower_32_bits(atu_config->aspace.cpu_phys.ba.addr) & ~0x7F;

    mask = RCAR_PCIE_PAR_ENABLE;
    if (atu_config->aspace.cpu_phys.ba.type == pci_asType_e_IO) {
        mask |= RCAR_PCIE_IO_SPACE;
        slog_info(1, "%s IO BASE 0x%"PRIx64", mask = 0x%lx resnum = %d", __FUNCTION__, atu_config->aspace.cpu_phys.ba.addr, atu_config->mask, atu_config->id);
    } else {
        slog_info(1, "%s MEM BASE 0x%"PRIx64", mask = 0x%lx resnum = %d", __FUNCTION__, atu_config->aspace.cpu_phys.ba.addr, atu_config->mask, atu_config->id);
    }

    icfg_reg->PCIEP[atu_config->id].PCIEPTCTLR = mask;

    return PCI_ERR_OK;
}

pci_err_t config_inbound_atus(const uint_t controller, iATU_config_t *atu_config)
{
    uint32_t index = atu_config->id << 1;

    if(index >= NUM_INBOUND_ATUS)
    {
        slog_error(0, "%s Invalid inbound ATUs", __func__);
        return PCI_ERR_EINVAL;
    }

    rcar_pcie_icfg_reg_t * const icfg_reg = get_pcie_icfg_reg_p(controller);

    slog_info(1, "%s: RAM Address 0x%"PRIx64"", __FUNCTION__, atu_config->aspace.cpu_phys.ba.addr);
    icfg_reg->PCIEPRAR[index] = lower_32_bits(atu_config->aspace.cpu_phys.ba.addr);
    icfg_reg->PCIELA[index].PCIELAR = lower_32_bits(atu_config->aspace.cpu_phys.ba.addr);
    icfg_reg->PCIELA[index].PCIELAMR = lower_32_bits(atu_config->mask) | RCAR_PCIE_LAM_64BIT | RCAR_PCIE_LAR_ENABLE | RCAR_PCIE_LAM_PREFETCH;

    icfg_reg->PCIEPRAR[index+1] = upper_32_bits(atu_config->aspace.cpu_phys.ba.addr);
    icfg_reg->PCIELA[index+1].PCIELAR = upper_32_bits(atu_config->aspace.cpu_phys.ba.addr);
    icfg_reg->PCIELA[index+1].PCIELAMR = 0;

    return PCI_ERR_OK;
}


/*
 ===============================================================================
 init_inbound_mem_atu

 The <atu_config>->aspace field is filled in by this function and doesn't
 supply any information on entry

 All inbound memory accesses are routed to system RAM unless default addresses
 are provided

*/
__attribute__ ((visibility ("internal")))
pci_err_t init_inbound_mem_atu(const uint_t controller, iATU_config_t *atu_config)
{
    pcie_ctrl_info_t * const pcie_ctrl_info = get_pcie_ctrl_info(controller);
    pci_err_t r = PCI_ERR_OK;

    if(pcie_ctrl_info == NULL) r = PCI_ERR_EIO;
    else
    {
        r = config_inbound_atus(controller, &atu_config[iATU_INBOUND_id_e_MEM_0]);
        if( r == PCI_ERR_OK) r = config_inbound_atus(controller, &atu_config[iATU_INBOUND_id_e_MEM_1]);
        if( r == PCI_ERR_OK) r = config_inbound_atus(controller, &atu_config[iATU_INBOUND_id_e_MEM_2]);
    }

    return r;
}

/*
 ===============================================================================
 init_outbound_mem_atu

 The <atu_config>->aspace field is filled in by this function and doesn't
 supply any information on entry

*/
__attribute__ ((visibility ("internal")))
pci_err_t init_outbound_mem_atu(const uint_t controller, iATU_config_t *atu_config)
{
    pcie_ctrl_info_t * const pcie_ctrl_info = get_pcie_ctrl_info(controller);
    pci_err_t r = PCI_ERR_OK;

    if(pcie_ctrl_info == NULL) r = PCI_ERR_EIO;
    else
    {
        r = config_outbound_atus(controller, &atu_config[iATU_OUTBOUND_id_e_MEM]);
        if( r == PCI_ERR_OK) r = config_outbound_atus(controller, &atu_config[iATU_OUTBOUND_id_e_MEM_1]);
        if( r == PCI_ERR_OK) r = config_outbound_atus(controller, &atu_config[iATU_OUTBOUND_id_e_MEM_PF]);
    }

    return r;
}

/*
 ===============================================================================
 init_outbound_io_atu

 The <atu_config>->aspace field is filled in by this function and doesn't
 supply any information on entry

*/
__attribute__ ((visibility ("internal")))
pci_err_t init_outbound_io_atu(const uint_t controller, iATU_config_t *atu_config)
{
    pcie_ctrl_info_t * const pcie_ctrl_info = get_pcie_ctrl_info(controller);
    pci_err_t r = PCI_ERR_OK;

    if (pcie_ctrl_info == NULL) r = PCI_ERR_EIO;
    else
    {
        r = config_outbound_atus(controller, &atu_config[iATU_OUTBOUND_id_e_IO]);
    }

    return r;
}

/*
 ===============================================================================
 get_outbound_memspace_atu_info

 Get the address space information for the iATU's that are used for translating
 outbound PCI memory space accesses.

 This function is called by all processes which load the HW dependent module in
 order to initialize the 'pcie_info' structure with information required to
 support the module API's

 NOTE:
 The caller should check the pci_asAttr_e_ENABLED flag in order to determine
 whether or not the ATU is actually in use

*/
__attribute__ ((visibility ("internal")))
pci_err_t get_outbound_memspace_atu_info(const uint_t controller, int iATU_id, pci_ba_t *asinfo, pci_ba_t *asinfo_xlate)
{
    rcar_pcie_icfg_reg_t * const icfg_reg = get_pcie_icfg_reg_p(controller);
    if (icfg_reg == NULL) return PCI_ERR_EIO;
    else
    {
        ATU_ctrl_config_t * atu_config = get_atu_config(controller);
        asinfo->addr = (uint64_t)icfg_reg->PCIEP[iATU_id].PCIEPALR | ((uint64_t)icfg_reg->PCIEP[iATU_id].PCIEPAUR << 32);
        asinfo->size = atu_config->outbound.cfg[iATU_id].aspace.cpu_phys.ba.size;
        asinfo->type = atu_config->outbound.cfg[iATU_id].aspace.cpu_phys.ba.type;
        asinfo->attr = atu_config->outbound.cfg[iATU_id].aspace.cpu_phys.ba.attr;

        asinfo_xlate->addr = asinfo->addr;
        asinfo_xlate->size = asinfo->size;
        asinfo_xlate->type = asinfo->type;
        asinfo_xlate->attr = asinfo->attr;

        return PCI_ERR_OK;
    }
}

/*
 ===============================================================================
 get_outbound_iospace_atu_info

 Get the address space information for the iATU's that are used for translating
 outbound PCI I/O space accesses.

 This function is called by all processes which load the HW dependent module in
 order to initialize the 'pcie_info' structure with information required to
 support the module API's

 NOTE:
 The caller should check the pci_asAttr_e_ENABLED flag in order to determine
 whether or not the ATU is actually in use

*/
__attribute__ ((visibility ("internal")))
pci_err_t get_outbound_iospace_atu_info(const uint_t controller, pci_ba_t *asinfo, pci_ba_t *asinfo_xlate)
{
    rcar_pcie_icfg_reg_t * const icfg_reg = get_pcie_icfg_reg_p(controller);
    if (icfg_reg == NULL) return PCI_ERR_EIO;
    else
    {
        ATU_ctrl_config_t * atu_config = get_atu_config(controller);
        asinfo->addr = (uint64_t)icfg_reg->PCIEP[iATU_OUTBOUND_id_e_IO].PCIEPALR | ((uint64_t)icfg_reg->PCIEP[iATU_OUTBOUND_id_e_IO].PCIEPAUR << 32);
        asinfo->size = atu_config->outbound.cfg[iATU_OUTBOUND_id_e_IO].aspace.cpu_phys.ba.size;
        asinfo->type = atu_config->outbound.cfg[iATU_OUTBOUND_id_e_IO].aspace.cpu_phys.ba.type;
        asinfo->attr = atu_config->outbound.cfg[iATU_OUTBOUND_id_e_IO].aspace.cpu_phys.ba.attr;

        asinfo_xlate->addr = asinfo->addr;
        asinfo_xlate->size = asinfo->size;
        asinfo_xlate->type = asinfo->type;
        asinfo_xlate->attr = asinfo->attr;

        return PCI_ERR_OK;
    }
}

/*
 ===============================================================================
 get_inbound_memspace_atu_info

 Get the address space information for the iATU's that are used for translating
 inbound PCI memory space accesses.

 This function is called by all processes which load the HW dependent module in
 order to initialize the 'pcie_info' structure with information required to
 support the module API's

 NOTE:
 The caller should check the pci_asAttr_e_ENABLED flag in order to determine
 whether or not the ATU is actually in use

*/
__attribute__ ((visibility ("internal")))
pci_err_t get_inbound_memspace_atu_info(const uint_t controller, pci_ba_t *asinfo, pci_ba_t *asinfo_xlate)
{
    rcar_pcie_icfg_reg_t * const icfg_reg = get_pcie_icfg_reg_p(controller);
    if (icfg_reg == NULL) return PCI_ERR_EIO;
    else
    {
        ATU_ctrl_config_t * atu_config = get_atu_config(controller);
        asinfo->addr = (uint64_t)icfg_reg->PCIELA[iATU_INBOUND_id_e_MEM_0].PCIELAR | ((uint64_t)icfg_reg->PCIELA[iATU_INBOUND_id_e_MEM_0+1].PCIELAR << 32);
        uint_t max_ram_index = iATU_INBOUND_id_e_MEM_2 << 1;
        asinfo->size = ((uint64_t)icfg_reg->PCIELA[max_ram_index].PCIELAR | ((uint64_t)icfg_reg->PCIELA[max_ram_index+1].PCIELAR << 32)) +
            atu_config->inbound.cfg[iATU_INBOUND_id_e_MEM_2].aspace.cpu_phys.ba.size - asinfo->addr;
        asinfo->type = atu_config->inbound.cfg[iATU_INBOUND_id_e_MEM_0].aspace.cpu_phys.ba.type;
        asinfo->attr = atu_config->inbound.cfg[iATU_INBOUND_id_e_MEM_0].aspace.cpu_phys.ba.attr;

        asinfo_xlate->addr = asinfo->addr;
        asinfo_xlate->size = asinfo->size;
        asinfo_xlate->type = asinfo->type;
        asinfo_xlate->attr = asinfo->attr;

        return PCI_ERR_OK;
    }
}

/*
 ===============================================================================
 get_inbound_msi_atu_info

 Get the address space information for the iATU's that are used for translating
 inbound PCI memory space accesses used for MSI

 NOTE:
 The caller should check the pci_asAttr_e_ENABLED flag in order to determine
 whether or not the ATU is actually in use

*/
__attribute__ ((visibility ("internal")))
pci_err_t get_inbound_msi_atu_info(const uint_t controller, pci_ba_t *asinfo, pci_ba_t *asinfo_xlate)
{
    rcar_pcie_icfg_reg_t * const icfg_reg = get_pcie_icfg_reg_p(controller);
    if (icfg_reg == NULL) return PCI_ERR_EIO;
    else
    {
        /* TODO: future implemention */

        return PCI_ERR_OK;
    }
}

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL$ $Rev$")
#endif
