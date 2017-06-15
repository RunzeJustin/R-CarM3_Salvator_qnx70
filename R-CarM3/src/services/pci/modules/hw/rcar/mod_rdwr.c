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
#include <string.h>
#include <sys/mman.h>
#include <assert.h>

#include <pci/pci.h>
#include <pci/pci_ccode.h>

#include <pci/cap_pcie.h>

#include "private/hwmod_api.h"
#include "hw_lib.h"
#include "private/pci_debug.h"
#include "private/pci_slog.h"
#include "hw_rcar.h"

static inline bool_t bdf_doesnt_exist(const pci_bdf_t bdf)
{
    extern uint32_t _bdf_doesnt_exist[];
    const uint_t idx = bdf / 32;
    const uint_t bit = (PCI_BDF(0, PCI_DEV(bdf), PCI_FUNC(bdf))) % 32;
    return (((_bdf_doesnt_exist[idx] >> bit) & 1) == 1);
}

/*
 ===============================================================================
 hw_rd

 This function implements the entry point to the PCI configuration space read
 operations.

*/
__attribute__ ((visibility ("internal")))
pci_err_t hw_rd(pci_bdf_t bdf, uint_t offset, uint_t size, uint8_t *buf)
{
    pci_err_t r = PCI_ERR_ENODEV;
    const int_t controller = get_pcie_ctrl_num(bdf);
    assert(controller >= 0);
    pcie_ctrl_info_t *pcie_ctrl_info = get_pcie_ctrl_info(controller);

    assert(pcie_ctrl_info != NULL);

    if (pcie_ctrl_info == NULL) r = PCI_ERR_ENODEV;
    else if (bdf_doesnt_exist(bdf))
    {
        /* mimic UR completion for a non-existent device */
        memset(buf, 0xFF, size);
        r = PCI_ERR_OK;
    }
    else
    {
        rcar_pcie_icfg_reg_t * const icfg_reg = get_pcie_icfg_reg_p(controller);
        void *cfgmem_base = 0;

        if(bdf == PCI_BDF(0, 0, 0))
        {
            cfgmem_base = (void*)&icfg_reg->PCICONF[offset >> 2];
            r = hw_mem_rd(cfgmem_base, bdf, offset, size, buf);

            if (r == PCI_ERR_OK)
            {
                if ((offset == 0x1c) && (size == 1)) // fix up I/O base
                {
                    uint8_t io_base = buf[0];
                    io_base &= ~0xF;
                    io_base |= 1; // 32 bit
                    buf[0] = io_base;
                }
                else if ((offset == 0x1d) && (size == 1)) // fix up I/O limit
                {
                    uint8_t io_limit = buf[0];
                    io_limit &= ~0xF;
                    io_limit |= 1; // 32 bit
                    buf[0] = io_limit;
                }
                else if ((offset == 0x24) && (size == 2)) // fix up prefetchable MEM base
                {
                    uint16_t pfmem_base = ENDIAN_LE16(*((uint16_t *)&buf[0]));
                    pfmem_base &= ~0xF;
                    pfmem_base |= 1; // 64 bit
                    *((uint16_t *)&buf[0]) = ENDIAN_LE16(pfmem_base);
                }
                else if ((offset == 0x26) && (size == 2)) // fix up prefetchable MEM limit
                {
                    uint16_t pfmem_limit = ENDIAN_LE16(*((uint16_t *)&buf[0]));
                    pfmem_limit &= ~0xF;
                    pfmem_limit |= 1; // 64 bit
                    *((uint16_t *)&buf[0]) = ENDIAN_LE16(pfmem_limit);
                }
            }
        }
        else if (PCI_BUS(bdf) > 0)
        {
            /* Endpoint device read */
            if(!pcie_ctrl_info->link.trained && !check_link_trained(controller)) r = PCI_ERR_PCIe_LINK_NOT_ACTIVE;
            else
            {
                icfg_reg->PCIEERRFR = RCAR_PCIEERRFR_CLEAR;
                icfg_reg->PCIECAR = (PCI_BUS(bdf) << 24) | (PCI_DEV(bdf) << 19) | (PCI_FUNC(bdf) << 16) | (offset & ~3);
                icfg_reg->PCIECCTLR = RCAR_PCIE_CONFIG_SEND_ENABLE | ((PCI_BUS(bdf) == 1) ? RCAR_PCIE_TYPE_0 : RCAR_PCIE_TYPE_1);
                if(icfg_reg->PCIEERRFR & RCAR_PCIE_UNSUPORTED_REQUEST) r = PCI_ERR_EINVAL;
                else
                {
                    cfgmem_base = (void*)&icfg_reg->PCIECDR;
                    r = hw_mem_rd(cfgmem_base, bdf, offset, size, buf);

                    icfg_reg->PCIECCTLR = RCAR_PCIE_CONFIG_SEND_DISABLE;
                }
            }
        }
    }
    if (r == PCI_ERR_ENODEV)
    {
        /* mimic UR completion for a non-existent device */
        memset(buf, 0xFF, size);
        r = PCI_ERR_OK;
    }

    slog_debug(4, "%s(B%u:D%u:F%u, 0x%x, %u, %p) MMIO access %s", __FUNCTION__,
                    PCI_BUS(bdf), PCI_DEV(bdf), PCI_FUNC(bdf), offset, size, buf, pci_strerror(r));

    return r;
}

/*
 ===============================================================================
 hw_wr

 This function implements the entry point to the PCI configuration space write
 operations


*/
__attribute__ ((visibility ("internal")))
pci_err_t hw_wr(pci_bdf_t bdf, uint_t offset, uint_t size, uint8_t *buf)
{
    pci_err_t r;
    const int_t controller = get_pcie_ctrl_num(bdf);
    assert(controller >= 0);
    pcie_ctrl_info_t *pcie_ctrl_info = get_pcie_ctrl_info(controller);

    assert(pcie_ctrl_info != NULL);

    if (pcie_ctrl_info == NULL) r = PCI_ERR_ENODEV;
    else if (bdf_doesnt_exist(bdf)) r = PCI_ERR_ENODEV;
    else
    {
        rcar_pcie_icfg_reg_t * const icfg_reg = get_pcie_icfg_reg_p(controller);
        void *cfgmem_base = 0;

        if(bdf == PCI_BDF(0, 0, 0)) {
            cfgmem_base = (void*)&icfg_reg->PCICONF[offset >> 2];

            if ((offset == 0x1c) && (size == 1)) // fix up I/O base
            {
                uint8_t io_base = buf[0];
                io_base &= ~0xF;
                io_base |= 1; // 32 bit
                buf[0] = io_base;
            }
            else if ((offset == 0x1d) && (size == 1)) // fix up I/O limit
            {
                uint8_t io_limit = buf[0];
                io_limit &= ~0xF;
                io_limit |= 1; // 32 bit
                buf[0] = io_limit;
            }
            else if ((offset == 0x24) && (size == 2)) // fix up prefetchable MEM base
            {
                uint16_t pfmem_base = ENDIAN_LE16(*((uint16_t *)&buf[0]));
                pfmem_base &= ~0xF;
                pfmem_base |= 1; // 64 bit
                *((uint16_t *)&buf[0]) = ENDIAN_LE16(pfmem_base);
            }
            else if ((offset == 0x26) && (size == 2)) // fix up prefetchable MEM limit
            {
                uint16_t pfmem_limit = ENDIAN_LE16(*((uint16_t *)&buf[0]));
                pfmem_limit &= ~0xF;
                pfmem_limit |= 1; // 64 bit
                *((uint16_t *)&buf[0]) = ENDIAN_LE16(pfmem_limit);
            }
            r = hw_mem_wr(cfgmem_base, bdf, offset, size, buf);

        } else {
            /* Endpoint device write */
            if(!pcie_ctrl_info->link.trained) r = PCI_ERR_PCIe_LINK_NOT_ACTIVE;
            else
            {
                icfg_reg->PCIEERRFR = RCAR_PCIEERRFR_CLEAR;
                icfg_reg->PCIECAR = (PCI_BUS(bdf) << 24) | (PCI_DEV(bdf) << 19) | (PCI_FUNC(bdf) << 16) | ((offset >> 2) << 2);
                icfg_reg->PCIECCTLR = RCAR_PCIE_CONFIG_SEND_ENABLE | ((PCI_BUS(bdf) == 1) ? RCAR_PCIE_TYPE_0 : RCAR_PCIE_TYPE_1);
                if(icfg_reg->PCIEERRFR & RCAR_PCIE_UNSUPORTED_REQUEST) r = PCI_ERR_EINVAL;
                else
                {
                    cfgmem_base = (void*)&icfg_reg->PCIECDR;
                    r = hw_mem_wr(cfgmem_base, bdf, offset, size, buf);

                    icfg_reg->PCIECCTLR = RCAR_PCIE_CONFIG_SEND_DISABLE;
                }
            }
        }
    }

    slog_debug(4, "%s(B%u:D%u:F%u, 0x%x, %u, %p) MMIO access %s", __FUNCTION__,
                    PCI_BUS(bdf), PCI_DEV(bdf), PCI_FUNC(bdf), offset, size, buf, pci_strerror(r));

    return r;
}

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL$ $Rev$")
#endif
