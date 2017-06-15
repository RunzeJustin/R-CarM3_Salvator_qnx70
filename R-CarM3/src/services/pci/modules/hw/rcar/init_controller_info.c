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
#include <stddef.h>
#include <stdint.h>
#include <sys/syspage.h>

#include <pci/pci.h>

#include "private/pci_slog.h"
#include "hw_rcar.h"


/*
 ===============================================================================
 initialize_irqs

*/
static const struct
{
    pci_irq_t vector;
} pcie_intmap[NUM_CONTROLLERS] =
{
    [0 ... NUM_CONTROLLERS - 1] = {.vector = -1},
    [0] = {.vector = PCIe1_INT},
};

static void initialize_irqs(const uint_t controller, pcie_ctrl_info_t *ctrl_info)
{
    assert((pci_runtime_flags & pci_runtime_flags_e_SERVER) != 0);

    /* get the 'vector_base' for INT interrupts */
    if (pcie_intmap[controller].vector != -1)
    {
        const uint_t vector = pcie_intmap[controller].vector;
        struct intrinfo_entry *entry = find_vec_entry(NULL, vector, intrinfo_entry_type_e_CASCADE);

        if (entry != NULL)
        {
            assert(entry->num_vectors >= 5);
            assert(entry->cascade_vector == vector);

            ctrl_info->interrupts.pin[0] = entry->vector_base + 0;  /* INTPIN A assertion */
            ctrl_info->interrupts.pin[1] = entry->vector_base + 2;  /* INTPIN B assertion */
            ctrl_info->interrupts.pin[2] = entry->vector_base + 4;  /* INTPIN C assertion */
            ctrl_info->interrupts.pin[3] = entry->vector_base + 6;  /* INTPIN D assertion */

            slog_debug(3, "PCIe %u, INTPIN A to D on IRQ's %d to %d (0x%x to 0x%x)", controller,
                            ctrl_info->interrupts.pin[0], ctrl_info->interrupts.pin[3],
                            ctrl_info->interrupts.pin[0], ctrl_info->interrupts.pin[3]);

            /* the 29th entry is the second level cascade vector for the MSI's */
            entry = find_vec_entry(NULL, entry->vector_base + 29, intrinfo_entry_type_e_CASCADE);
            if ((entry != NULL) && (entry->flags & INTR_FLAG_MSI))
            {
                /*
                 * startup has assigned the 'cpu_intr_base' value for each controller such
                 * that there is 2 groups of 256 vectors organized as 512 contiguous vectors
                 * These are the values that will have been entered into the resource database
                 * and hence are the values we need to search for when allocating so that the
                 * proper IRQ is obtained for the device depending on which controller it is
                 * downstream of
                 */
                ctrl_info->interrupts.msi_vec_first = entry->cpu_intr_base;
                ctrl_info->interrupts.msi_vec_last = entry->cpu_intr_base + entry->num_vectors - 1;

                slog_debug(3, "PCIe %u, MSI IRQ's from %d to %d (0x%x to 0x%x)", controller,
                            entry->vector_base + (ctrl_info->interrupts.msi_vec_first - entry->cpu_intr_base),
                            entry->vector_base + (ctrl_info->interrupts.msi_vec_last - entry->cpu_intr_base),
                            entry->vector_base + (ctrl_info->interrupts.msi_vec_first - entry->cpu_intr_base),
                            entry->vector_base + (ctrl_info->interrupts.msi_vec_last - entry->cpu_intr_base));
            }
        }
    }
}

/*
 ===============================================================================
 init_controller_info

 Initialize <pcie_info>

 **Important**
 This function is called from both the PCI server and normal processes so it
 needs to behave accordingly

 Returns an appropriate error code

*/
__attribute__ ((visibility ("internal")))
pci_err_t init_controller_info(pcie_info_t *pcie_info)
{
    assert(pcie_info != NULL);

    pcie_info->num_controllers = NUM_CONTROLLERS;

    /*
     * now, allocate an array of 'pcie_ctrl_info_t' structures based on the number
     * of controllers that need to be supported
     */
    pcie_info->ctrl = calloc(pcie_info->num_controllers, sizeof(*pcie_info->ctrl));
    if (pcie_info->ctrl == NULL) return PCI_ERR_ENOMEM;
    else
    {
        static const uint_t num_lanes[NUM_CONTROLLERS] =
        {
            [0 ... NUM_CONTROLLERS - 1] = 0,
            [0] = PCIe1_NUM_LANES,
        };
        uint_t controller;

        for (controller=0; controller<pcie_info->num_controllers; ++controller)
        {
            pcie_ctrl_info_t *ctrl_info = &pcie_info->ctrl[controller];

            ctrl_info->ctrl_num = controller;
            ctrl_info->type = pcie_ctrl_type_e_RC;
            ctrl_info->link.num = controller * DEFAULT_BUS_STRIDE;
            ctrl_info->link.lanes = num_lanes[controller];
            ctrl_info->interrupts.msi_base = (uint64_t)-1;

            ctrl_info->interrupts.pin[0] = pcie_intmap[controller].vector + 0;
            ctrl_info->interrupts.pin[1] = pcie_intmap[controller].vector + 1;
            ctrl_info->interrupts.pin[2] = pcie_intmap[controller].vector + 2;
            ctrl_info->interrupts.pin[3] = pcie_intmap[controller].vector + 3;

            /* only the server needs the IRQ info initialized */
            if ((pci_runtime_flags & pci_runtime_flags_e_SERVER) != 0)
            {
                initialize_irqs(controller, ctrl_info);
            }
        }
    }
    return PCI_ERR_OK;
}

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL$ $Rev$")
#endif
