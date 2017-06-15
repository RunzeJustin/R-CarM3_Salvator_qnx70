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

 #include "hw_rcar.h"

static iATU_config_t ctrl_0_oATU_config[] =
{
    {
        .id = iATU_OUTBOUND_id_e_IO,
        .mask = RCAR_PCIE0_MEM_MASK_1MB,
        .aspace.cpu_phys.ba =
        {
            .addr = RCAR_PCIE0_MEM_0,
            .size = RCAR_PCIE0_MEM_SIZE_1MB,
            .type = pci_asType_e_IO,
            .attr = pci_asAttr_e_ALIGN_IO_MIN | pci_asAttr_e_32BIT | pci_asAttr_e_CONTIG | pci_asAttr_e_ENABLED,
        },
        .aspace.pci_phys.ba =
        {
            .addr = RCAR_PCIE0_MEM_0,
            .size = RCAR_PCIE0_MEM_SIZE_1MB,
            .type = pci_asType_e_IO,
            .attr = pci_asAttr_e_ALIGN_IO_MIN | pci_asAttr_e_16BIT | pci_asAttr_e_CONTIG | pci_asAttr_e_ENABLED,
        },
    },
    {
        .id = iATU_OUTBOUND_id_e_MEM_1,
        .mask = RCAR_PCIE0_MEM_MASK_2MB,
        .aspace.cpu_phys.ba =
        {
            .addr = RCAR_PCIE0_MEM_1,
            .size = RCAR_PCIE0_MEM_SIZE_2MB,
            .type = pci_asType_e_MEM,
            .attr = (1u << 12) | pci_asAttr_e_32BIT | pci_asAttr_e_CONTIG | pci_asAttr_e_ENABLED,
        },
        .aspace.pci_phys.ba =
        {
            .addr = RCAR_PCIE0_MEM_1,
            .size = RCAR_PCIE0_MEM_SIZE_2MB,
            .type = pci_asType_e_MEM,
            .attr = (1u << 12) | pci_asAttr_e_32BIT | pci_asAttr_e_CONTIG | pci_asAttr_e_ENABLED,
        },
    },
    {
        .id = iATU_OUTBOUND_id_e_MEM,
        .mask = RCAR_PCIE0_MEM_MASK_128MB,
        .aspace.cpu_phys.ba =
        {
            .addr = RCAR_PCIE0_MEM_2,
            .size = RCAR_PCIE0_MEM_SIZE_128MB,
            .type = pci_asType_e_MEM,
            .attr = (1u << 12) | pci_asAttr_e_32BIT | pci_asAttr_e_CONTIG | pci_asAttr_e_ENABLED,
        },
        .aspace.pci_phys.ba =
        {
            .addr = RCAR_PCIE0_MEM_2,
            .size = RCAR_PCIE0_MEM_SIZE_128MB,
            .type = pci_asType_e_MEM,
            .attr = (1u << 12) | pci_asAttr_e_32BIT | pci_asAttr_e_CONTIG | pci_asAttr_e_ENABLED,
        },
    },
    {
        .id = iATU_OUTBOUND_id_e_MEM_PF,
        .mask = RCAR_PCIE0_MEM_MASK_128MB,
        .aspace.cpu_phys.ba =
        {
            .addr = RCAR_PCIE0_MEM_3,
            .size = RCAR_PCIE0_MEM_SIZE_128MB,
            .type = pci_asType_e_MEM,
            .attr = (1u << 12) | pci_asAttr_e_32BIT | pci_asAttr_e_CONTIG | pci_asAttr_e_ENABLED | pci_asAttr_e_PREFETCH,
        },
        .aspace.pci_phys.ba =
        {
            .addr = RCAR_PCIE0_MEM_3,
            .size = RCAR_PCIE0_MEM_SIZE_128MB,
            .type = pci_asType_e_MEM,
            .attr = (1u << 12) | pci_asAttr_e_32BIT | pci_asAttr_e_CONTIG | pci_asAttr_e_ENABLED | pci_asAttr_e_PREFETCH,
        },
    },
};

static iATU_config_t ctrl_0_iATU_config[] =
{
    {
        .id = iATU_INBOUND_id_e_MEM_0,
        .mask = RCAR_CPU_MEM_MASK_1GB,
        .aspace.cpu_phys.ba =
        {
            .addr = RCAR_CPU_MEM_0,
            .size = RCAR_CPU_MEM_SIZE_1GB,
            .type = pci_asType_e_MEM,
            .attr = (1u << 12) | pci_asAttr_e_32BIT | pci_asAttr_e_CONTIG | pci_asAttr_e_ENABLED,
        },
        .aspace.pci_phys.ba =
        {
            .addr = RCAR_CPU_MEM_0,
            .size = RCAR_CPU_MEM_SIZE_1GB,
            .type = pci_asType_e_MEM,
            .attr = (1u << 12) | pci_asAttr_e_32BIT | pci_asAttr_e_CONTIG | pci_asAttr_e_ENABLED,
        },
    },
    {
        .id = iATU_INBOUND_id_e_MEM_1,
        .mask = RCAR_CPU_MEM_MASK_1GB,
        .aspace.cpu_phys.ba =
        {
            .addr = RCAR_CPU_MEM_1,
            .size = RCAR_CPU_MEM_SIZE_1GB,
            .type = pci_asType_e_MEM,
            .attr = (1u << 12) | pci_asAttr_e_32BIT | pci_asAttr_e_CONTIG | pci_asAttr_e_ENABLED,
        },
        .aspace.pci_phys.ba =
        {
            .addr = RCAR_CPU_MEM_1,
            .size = RCAR_CPU_MEM_SIZE_1GB,
            .type = pci_asType_e_MEM,
            .attr = (1u << 12) | pci_asAttr_e_32BIT | pci_asAttr_e_CONTIG | pci_asAttr_e_ENABLED,
        },
    },
    {
        .id = iATU_INBOUND_id_e_MEM_2,
        .mask = RCAR_CPU_MEM_MASK_1GB,
        .aspace.cpu_phys.ba =
        {
            .addr = RCAR_CPU_MEM_2,
            .size = RCAR_CPU_MEM_SIZE_1GB,
            .type = pci_asType_e_MEM,
            .attr = (1u << 12) | pci_asAttr_e_32BIT | pci_asAttr_e_CONTIG | pci_asAttr_e_ENABLED,
        },
        .aspace.pci_phys.ba =
        {
            .addr = RCAR_CPU_MEM_2,
            .size = RCAR_CPU_MEM_SIZE_1GB,
            .type = pci_asType_e_MEM,
            .attr = (1u << 12) | pci_asAttr_e_32BIT | pci_asAttr_e_CONTIG | pci_asAttr_e_ENABLED,
        },
    },
};

static ATU_ctrl_config_t ATU_config[] =
{
    [0] =
    {
        .outbound = {.cfg = ctrl_0_oATU_config, .num_entries = NELEMENTS(ctrl_0_oATU_config)},
        .inbound = {.cfg = ctrl_0_iATU_config, .num_entries = NELEMENTS(ctrl_0_iATU_config)},
    },
};

/*
 ===============================================================================
 init_inbound_atus

 This function is called by the PCI server once at HW module load to configure
 the inbound ATU's

*/
__attribute__ ((visibility ("internal")))
pci_err_t init_inbound_atus(const uint_t controller)
{
    pci_err_t r = PCI_ERR_OK;
    assert(controller < NELEMENTS(ATU_config));
    pcie_ctrl_info_t * const pcie_ctrl_info = get_pcie_ctrl_info(controller);

    if (pcie_ctrl_info == NULL) r = PCI_ERR_EINVAL;
    else
    {
        r = init_inbound_mem_atu(controller, ATU_config[controller].inbound.cfg);
        if (r == PCI_ERR_OK) r = get_inbound_memspace_atu_info(controller, &pcie_ctrl_info->inbound.mem.cpu_phys.ba, &pcie_ctrl_info->inbound.mem.pci_phys.ba);
    }

    return r;
}

/*
 ===============================================================================
 init_outbound_atus

 This function is called by the PCI server once at HW module load to configure
 the outbound ATU's

*/
__attribute__ ((visibility ("internal")))
pci_err_t init_outbound_atus(const uint_t controller)
{
    pci_err_t r = PCI_ERR_OK;
    assert(controller < NELEMENTS(ATU_config));
    pcie_ctrl_info_t * const pcie_ctrl_info = get_pcie_ctrl_info(controller);

    if (pcie_ctrl_info == NULL) r = PCI_ERR_EINVAL;
    else
    {
        r = init_outbound_mem_atu(controller, ATU_config[controller].outbound.cfg);
        if (r == PCI_ERR_OK) r = get_outbound_memspace_atu_info(controller, iATU_OUTBOUND_id_e_MEM, &pcie_ctrl_info->outbound.mem.cpu_phys.ba, &pcie_ctrl_info->outbound.mem.pci_phys.ba);
        if (r == PCI_ERR_OK) r = rsrcdb_as_add(&pcie_ctrl_info->outbound.mem.cpu_phys);
        if (r == PCI_ERR_OK) r = get_outbound_memspace_atu_info(controller, iATU_OUTBOUND_id_e_MEM_1, &pcie_ctrl_info->outbound.mem1.cpu_phys.ba, &pcie_ctrl_info->outbound.mem1.pci_phys.ba);
        if (r == PCI_ERR_OK) r = rsrcdb_as_add(&pcie_ctrl_info->outbound.mem1.cpu_phys);
        if (r == PCI_ERR_OK) r = get_outbound_memspace_atu_info(controller, iATU_OUTBOUND_id_e_MEM_PF, &pcie_ctrl_info->outbound.pfmem.cpu_phys.ba, &pcie_ctrl_info->outbound.pfmem.pci_phys.ba);
        if (r == PCI_ERR_OK) r = rsrcdb_as_add(&pcie_ctrl_info->outbound.pfmem.cpu_phys);

        r = init_outbound_io_atu(controller, ATU_config[controller].outbound.cfg);
        if (r == PCI_ERR_OK) r = get_outbound_iospace_atu_info(controller, &pcie_ctrl_info->outbound.io.cpu_phys.ba, &pcie_ctrl_info->outbound.io.pci_phys.ba);
        if (r == PCI_ERR_OK) r = rsrcdb_as_add(&pcie_ctrl_info->outbound.io.cpu_phys);
    }

    return r;
}

__attribute__ ((visibility ("internal")))
ATU_ctrl_config_t * get_atu_config(const uint_t controller)
{
    assert(controller < NELEMENTS(ATU_config));
    return &ATU_config[controller];
}

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL$ $Rev$")
#endif
