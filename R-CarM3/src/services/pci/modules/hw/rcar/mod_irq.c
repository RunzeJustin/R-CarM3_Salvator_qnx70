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
#include <pthread.h>
#include <stdlib.h>
#include <sys/rsrcdbmgr.h>

#include <pci/pci.h>

#include "private/hwmod_api.h"
#include "hw_lib.h"
#include "private/pci_slog.h"
#include "hw_rcar.h"


static uint64_t get_msi_base_addr(const uint_t controller);
static void enable_msi(const uint_t controller);
static void disable_msi(const uint_t controller);
static void enable_intpins(const uint_t controller, const uint_t intpin);
static void disable_intpins(const uint_t controller, const uint_t intpin);

/*
 ===============================================================================
 hw_alloc_irq

 HW dependent IRQ allocation
 An <*nirq> value of 0 always succeeds

 For _pci_irqType_e_PIN interrupts ...

     The IRQ is determined by in a chipset specific fashion.

     For pin interrupts, we typically expect only 1 interrupt per BDF but we
     allow this HW dependent module function to determine that hence why a
     pointer to <nirq> is provided

 For _pci_irqType_e_MSG interrupts ...

     The IRQ is not fixed and will be chosen from a list of available IRQ's
     managed in the resource data base (rsrcdb). For MSI interrupts, if the
     request is for > 1 IRQ, the IRQ's must be allocated such that the
     corresponding vector is aligned to the number of requested IRQ's.

     If _pci_irqAttr_e_CONTIG is set in <irq_attr>, the caller is requesting
     that the allocated IRQ's be associated with a contiguous range of
     corresponding vectors. The IRQ's do not need to be numerically contiguous
     however the vectors associated with the IRQ's MUST be numerically
     contiguous. This is a requirement of MSI but not MSI-X
*/
__attribute__ ((visibility ("internal")))
pci_err_t hw_alloc_irq(pci_bdf_t bdf, _pci_irqType_e irq_type, _pci_irqAttr_e irq_attr, uint_t *nirq, _pci_irqmap_t *irq_map)
{
    pci_err_t r = PCI_ERR_EINVAL;
    const int_t controller = get_pcie_ctrl_num(bdf);

    if (controller >= 0)
    {
        switch(irq_type)
        {
            case _pci_irqType_e_PIN:
            {
                if (*nirq == 0) r = PCI_ERR_OK;
                else
                {
                    /*
                     * for PIN interrupts there can only be one. Also, we don't need to
                     * worry about the vector (for now) since the vectors are currently
                     * hard mapped to an IRQ (via syspage) and we don't store them in
                     * the rsrcdb. There is therefore nothing to free
                    */
                    uint8_t intpin;

                    r = _pci_device_cfg_rd8(bdf, 0x3d, &intpin);
                    if (r == PCI_ERR_OK)
                    {
                        *nirq = 0;  // assume no interrupt for device
                        if ((intpin >= 1) && (intpin <= 4))
                        {
                            char intpin_c = (intpin - 1) + 'A';

                            /* look for a config file override first */
                            pci_irq_t irq = extcfg_intpin_to_irq(bdf, intpin_c);
                            if (irq < 0)
                            {
                                /*
                                 * no override, try the bridge routing algorithm for subordinate bus devices
                                 * since its possible that the user has plugged in a device which contains a
                                 * PCIe-to-PCI bridge with subordinate PCI devices and that <bdf> corresponds
                                 * to one of those PCI devices and it doesn't support MSI (PCIe devices must
                                 * support MSI/MSI-X and unless blacklisted, we would not be in this code).
                                 *
                                 * We only need to call the bridge_intpin_route() function for devices
                                 * downstream of root ports.
                                 *
                                 * Instead of bothering to figure out the BDF of the first downstream bridge from
                                 * the root ports in order to terminate the INTPIN route mapping, just pass
                                 * PCI_BDF_NONE for the 'root_bdf' since considering the root ports will not
                                 * effect the result of bridge_intpin_route() anyway
                                 */
                                pcie_ctrl_info_t *pcie_ctrl_info = get_pcie_ctrl_info(controller);

                                if (PCI_BUS(bdf) > pcie_ctrl_info->link.num)
                                {
                                    intpin_c = bridge_intpin_route(PCI_BDF_NONE, bdf, intpin_c);
                                }
                                if ((intpin_c >= 'A') && (intpin_c <= 'D'))
                                {
                                    irq = pcie_ctrl_info->interrupts.pin[intpin_c - 'A'];
                                }
                            }
                            /* if an IRQ was found, return it */
                            if (irq < 0) r = PCI_ERR_IRQ_NOT_AVAIL;
                            else
                            {
                                *nirq = 1;
                                irq_map[0].irq = irq;
                                irq_map[0].vector.pin.val = (intpin_c - 'A') + 1;
                                /* enable the INTPIN interrupt */
                                enable_intpins(controller, irq_map[0].vector.pin.val);
                            }
                        }
                    }
                }
                break;
            }
            case _pci_irqType_e_MSG:
            {
                if (*nirq == 0) r = PCI_ERR_OK;
                else
                {
                    const uint64_t msi_base_addr = get_msi_base_addr(controller);
                    if (msi_base_addr == -(uint64_t)1) r = PCI_ERR_IRQ_NOT_AVAIL;
                    else
                    {
                        /* we want to specify the range so need at least 2 entries */
                        pci_irq_t *irq_list = calloc(max(*nirq, 2), sizeof(*irq_list));
                        if (irq_list == NULL) r = PCI_ERR_ENOMEM;
                        else
                        {
                            pcie_ctrl_info_t *pcie_ctrl_info = get_pcie_ctrl_info(controller);

                            /* reserve the vector(s) from the range associated with this controller */
                            irq_list[0] = pcie_ctrl_info->interrupts.msi_vec_first;
                            irq_list[1] = pcie_ctrl_info->interrupts.msi_vec_last;

                            r = rsrcdb_irq_resv(irq_type, irq_attr, *nirq, irq_list, RSRCDBMGR_FLAG_RANGE);
                            if (r == PCI_ERR_OK)
                            {
                                /* create the MSI vectors (addr/data pairs) from the irq_list */
                                uint_t i;
                                for (i=0; i<*nirq; i++)
                                {
                                    /* vectors are normalized to between 0 and 255 */
                                    const uint_t msi_vector = irq_list[i] & 0xFF;

                                    irq_map[i].vector.msg.data = msi_vector;
                                    irq_map[i].vector.msg.addr = msi_base_addr;
                                    irq_map[i].irq = find_vector(irq_list[i], irq_type);

                                    if (irq_map[i].irq < 0)
                                    {
                                        rsrcdb_irq_free(irq_type, *nirq, irq_list);
                                        r = PCI_ERR_IRQ_NOT_AVAIL;
                                        break;
                                    }
                                    else
                                    {
                                        /*
                                         * enable the MSI if required. This is not
                                         * the MSI/MSI-X capability register and need
                                         * only be done if your platform requires it
                                         */
                                    }
                                }
                                enable_msi(controller);
                            }
                            free(irq_list);
                        }
                    }
                }
                break;
            }
            default:
                break;
        }
    }
    slog_info(2, "%s(B%u:D%u:F%u, %d, 0x%x, %u, %p=%d,...) %s", __FUNCTION__,
                PCI_BUS(bdf), PCI_DEV(bdf), PCI_FUNC(bdf),
                irq_type, irq_attr, *nirq, irq_map, (irq_map != NULL) ? irq_map[0].irq : -1, pci_strerror(r));
    return r;
}

/*
 ===============================================================================
 hw_free_irq

 Only message based interrupts need to be unreserved
 An <nirq> value of 0 always succeeds

*/
__attribute__ ((visibility ("internal")))
pci_err_t hw_free_irq(pci_bdf_t bdf, _pci_irqType_e irq_type, uint_t nirq, _pci_irqmap_t *irq_map)
{
    pci_err_t r = PCI_ERR_OK;
    const int_t controller = get_pcie_ctrl_num(bdf);

    if (controller < 0) r = PCI_ERR_EINVAL;
    else
    {
        if (irq_type == _pci_irqType_e_MSG)
        {
            if (nirq > 0)
            {

                pcie_ctrl_info_t *pcie_ctrl_info = get_pcie_ctrl_info(controller);
                pci_irq_t *irq_list = calloc(nirq, sizeof(*irq_list));
                uint_t i;

                disable_msi(controller);

                for (i=0; i<nirq; i++)
                {
                    const uint_t msi_vector = irq_map[i].vector.msg.data;

                    irq_list[i] = msi_vector;
                    /* adjust the vector list based on the controller */
                    irq_list[i] += pcie_ctrl_info->interrupts.msi_vec_first;

                    /*
                     * disable the MSI if required. This is not
                     * the MSI/MSI-X capability register and need
                     * only be done if your platform requires it
                     */
                }
                r = rsrcdb_irq_free(irq_type, nirq, irq_list);
                free(irq_list);
            }
        }
        else if (irq_type == _pci_irqType_e_PIN)
        {
            /* disable the INTPIN interrupt */
            disable_intpins(controller, irq_map[0].vector.pin.val);
        }
    }

    slog_info(2, "%s(B%u:D%u:F%u, %d, %u, %p) %s", __FUNCTION__,
                PCI_BUS(bdf), PCI_DEV(bdf), PCI_FUNC(bdf),
                irq_type, nirq, irq_map, pci_strerror(r));
    return r;
}

/*
 ===============================================================================
 enable_msi
 disable_msi

 These only need implementing if required

*/
static void enable_msi(const uint_t controller)
{
    assert(controller < NUM_CONTROLLERS);

}
static void disable_msi(const uint_t controller)
{
    assert(controller < NUM_CONTROLLERS);

}

/*
 ===============================================================================
 enable_int1_intpin
 disable_int1_intpin

 These only need implementing if required

*/
static void enable_intpins(const uint_t controller, const uint_t intpin)
{
    assert(controller < NUM_CONTROLLERS);
    assert((intpin >= 1) && (intpin <= 4));

    rcar_pcie_icfg_reg_t * const icfg_reg = get_pcie_icfg_reg_p(controller);
    icfg_reg->PCIEINTXR |= (1u << (intpin + 7));
}
static void disable_intpins(const uint_t controller, const uint_t intpin)
{
    assert(controller < NUM_CONTROLLERS);
    assert((intpin >= 1) && (intpin <= 4));

    rcar_pcie_icfg_reg_t * const icfg_reg = get_pcie_icfg_reg_p(controller);
    icfg_reg->PCIEINTXR &= ~(1u << (intpin + 7));
}

/*
 ===============================================================================
 get_msi_base_addr

 retrieve the base address to use for MSI's. A value of 0 or -1 will indicate
 that they were not initialized and therefore MSI's will not be possible.
 In this case -1 will be returned

*/
static uint64_t get_msi_base_addr(const uint_t controller)
{
    pcie_ctrl_info_t * const pcie_ctrl_info = get_pcie_ctrl_info(controller);

    if (pcie_ctrl_info->interrupts.msi_base == -(uint64_t)1)
    {
        pci_ba_t asinfo, dont_care;
        pci_err_t r = get_inbound_msi_atu_info(controller, &asinfo, &dont_care);

        if (r == PCI_ERR_OK)
        {
            assert((asinfo.attr & pci_asAttr_e_BIT_MASK) == pci_asAttr_e_32BIT);

            if ((asinfo.addr != 0) && (asinfo.addr != -(uint64_t)1))
            {
                pcie_ctrl_info->interrupts.msi_base = asinfo.addr;
            }
        }
    }
    return pcie_ctrl_info->interrupts.msi_base;
}

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL$ $Rev$")
#endif
