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
#include <string.h>
#include <stdio.h>
#include <assert.h>

#include <pci/pci.h>

#include "hw_rcar.h"

#include "private/hwmod_api.h"
#include "hw_lib.h"
#include "private/pci_slog.h"
#include "private/pci_debug.h"

/*
 ===============================================================================
 hw_alloc_as

 Reserve address space of the specified type, size and attributes as contained
 in <as_map>. The address is unspecified (ie. 0) and so the rsrcdbmgr is free
 to choose any suitable values based on the search (ie attribute) criteria,
 hence the allocation

 This function is only called by the PCI server and only when configuring PCI
 devices (ie. a BIOS or other bootloader does not perform this function). This
 is typically restricted to initial PCI bus enumeration however it can also be
 called in response to changing address space requirements due to the live
 insertion or removal of devices.

 Once the address space requirements of the PCI hierarchy have been calculated
 and summed (enumeration) to the highest point in the hierarchy, typically the
 host bridge(s), this function will be called for each host bridge(s) for each
 valid combination of address space type and attributes required by that segment
 of the hierarchy. This may include some or all of I/O, Memory or Prefetchable
 memory address spaces. In the future, there could be other combinations.

 The information returned by this function MUST BE SUITABLE for use in
 programming the base address registers (BAR's) of each bridge and device in
 the hierarchy and the implementation should pay attention to attributes such
 as prefetchability, 32 vs 64 bit, contiguity, alignment and other attributes
 defined in pci/pci.h. All of these must be satisfied or the request failed
 (at the discretion of the implementation or course).

 The addresses returned by this function ARE NOT NECESSARILY the addresses used
 by driver software to access the device address space as some hardware may
 require or optionally support additional translation mechanisms. As previously
 stated, this function must return address assignments suitable for configuring
 device BAR's. At some point, driver software will request the address space
 assignments from the PCI server with the pci_device_read_ba() call. The
 information returned in a successful pci_device_read_ba() call will be used by
 the driver software to mmap() the device address spaces.

 When the PCI server processes the pci_device_read_ba() request, it will, for
 each address space assigned to the device by this function, call the
 hw->map_as() function so that any translations can be performed. It is up to
 the implementation to decide

     a. whether any translation will be done
     b. whether the hardware to set up those translations is done in this
        function or when the hw->map_as() (ie. hw_map_as()) call is made
     c. whether the translated addresses need to be "managed" in the RSRCDB
        (for example, a static mapping based on offset would probably not
        warrant management by the RSRCDB however this is a system memory map
        issue)

*/
__attribute__ ((visibility ("internal")))
pci_err_t hw_alloc_as(pci_bdf_t bdf, pci_asType_e as_type, pci_asAttr_e as_attr, uint64_t as_size, _pci_asmap_t *as_map)
{
    int_t controller = get_pcie_ctrl_num(bdf);
    if (controller < 0) return PCI_ERR_EINVAL;
    else
    {
        pci_err_t r;
        pcie_ctrl_info_t * const pcie_ctrl_info = get_pcie_ctrl_info(controller);
        const _pci_asmap_t * const asbase_pci_p = (as_type == pci_asType_e_MEM) ?
                                                    ((as_attr & pci_asAttr_e_PREFETCH) ? &pcie_ctrl_info->outbound.pfmem.pci_phys : &pcie_ctrl_info->outbound.mem.pci_phys) :
                                                    &pcie_ctrl_info->outbound.io.pci_phys;

        memset(as_map, 0, sizeof(*as_map));

        if ((as_attr & pci_asAttr_e_BIT_MASK) == pci_asAttr_e_16BIT)
        {
            slog_info(1, "16 bit alignment not supported. Bumping up to 32 bit");
            as_attr &= ~pci_asAttr_e_BIT_MASK;
            as_attr |= pci_asAttr_e_32BIT;
        }

        /* start the search from the 'as_type' specific starting PCI address */
        as_map->ba.addr = asbase_pci_p->ba.addr;
        as_map->ba.size = as_size;
        as_map->ba.type = as_type;
        /*
         * before we do the allocation, we need to check the 'attr' field of the
         * request. If the device is not capable of 64 bit memory accesses this
         * will be reflected in the 'pci_asAttr_e_BIT_MASK' field of the 'attr'.
         * We must therefore make sure that the 'attr' passed to rsrcdb_as_resv()
         * is consistent so that the allocation can succeed.
         * The requested 'attr' must be met by the translated (ie. the 'pci_phys')
         * address, not the untranslated (ie. 'cpu_phys').
         * We therefore replace the requested BIT_MASK with that of the available
         * address space for the allocation and then restore (after verification)
         * prior to return (see below)
         */
        as_map->ba.attr = as_attr;
        if(as_type == pci_asType_e_MEM) {
            as_map->ba.attr &= ~pci_asAttr_e_BIT_MASK;
            as_map->ba.attr |= (asbase_pci_p->ba.attr & pci_asAttr_e_BIT_MASK);
        }

#ifdef NDEBUG

        r = rsrcdb_as_resv(as_map, 0);

#else   /* NDEBUG */

        _pci_asmap_t tmp_map = *as_map;

        r = rsrcdb_as_resv(&tmp_map, 0);
        /* on success, lets assert the reservation was as requested */
        if (r == PCI_ERR_OK)
        {
            assert(tmp_map.ba.size == as_map->ba.size);
            assert(tmp_map.ba.attr == as_map->ba.attr);
            assert(tmp_map.ba.type == as_map->ba.type);
            *as_map = tmp_map;
        }

#endif  /* NDEBUG */

        slog_info(2, "AS allocation req for B%u:D%u:F%u (%"PRIx64"/%"PRIx64", align: %"PRIx64", attr: %x ... %s)",
                    PCI_BUS(bdf), PCI_DEV(bdf), PCI_FUNC(bdf),
                    as_map->ba.addr, as_map->ba.size,
                    (uint64_t)1 << (as_map->ba.attr & pci_asAttr_e_ALIGN),
                    as_map->ba.attr & ~pci_asAttr_e_ALIGN,
                    (r == PCI_ERR_OK) ? "Ok" : "Failed");

        /* if the address space was successfully allocated, make sure its valid */
        if (r == PCI_ERR_OK)
        {
            if (as_map->ba.addr < asbase_pci_p->ba.addr) r = PCI_ERR_ASPACE_INVALID;
            else if ((as_map->ba.addr + as_map->ba.size) > (asbase_pci_p->ba.addr + asbase_pci_p->ba.size)) r = PCI_ERR_ASPACE_INVALID;
            else
            {
                as_map->ba.attr &= ~pci_asAttr_e_BIT_MASK;
                as_map->ba.attr |= (as_attr & pci_asAttr_e_BIT_MASK);
                as_map->ba.attr |= pci_asAttr_e_ENABLED;

                r = PCI_ERR_OK;
            }
            slog_info(1, "%s(B%u:D%u:F%u, 0x%x, 0x%x, 0x%"PRIx64", %p), %s", __FUNCTION__,
                    PCI_BUS(bdf), PCI_DEV(bdf), PCI_FUNC(bdf), as_type, as_attr, as_size, as_map,
                    pci_strerror(r));

        }
        return r;
    }
}

/*
 ===============================================================================
 hw_resv_as

 Reserve address space of the specified address, type, size and attributes as
 contained in <as_map>. The address is specified (ie. !0) and so the rsrcdbmgr
 will reserve the specified space if available or fail the request hence the
 reservation.

 This function is only called by the PCI server and only when not configuring
 PCI devices (ie. a BIOS or other bootloader has already performed this
 function). This is typically restricted to initial PCI bus enumeration however
 it can also be called in response to changing address space requirements due to
 the live insertion or removal of devices.

 Other than the request being for a reservation instead of an allocation, the
 same statements regarding translation apply to this function as for
 hw_alloc_as(). Refer to the comments for hw_alloc_as() for details

*/
__attribute__ ((visibility ("internal")))
pci_err_t hw_resv_as(pci_bdf_t bdf, _pci_asmap_t *as_map)
{
    pci_err_t r;

#ifdef NDEBUG

    r = rsrcdb_as_resv(as_map, 0);

#else   /* NDEBUG */

    _pci_asmap_t tmp_map = *as_map;

    r = rsrcdb_as_resv(&tmp_map, 0);
    /* on success, let assert the reservation was as requested */
    if (r == PCI_ERR_OK)
    {
        assert(tmp_map.ba.addr == as_map->ba.addr);
        assert(tmp_map.ba.size == as_map->ba.size);
        assert(tmp_map.ba.attr == as_map->ba.attr);
        assert(tmp_map.ba.type == as_map->ba.type);
        *as_map = tmp_map;
    }

#endif  /* NDEBUG */

    slog_info(2, "AS reservation req for B%u:D%u:F%u (%"PRIx64"/%"PRIx64", align: %"PRIx64", attr: %x ... %s)",
                PCI_BUS(bdf), PCI_DEV(bdf), PCI_FUNC(bdf),
                as_map->ba.addr, as_map->ba.size,
                (uint64_t)1 << (as_map->ba.attr & pci_asAttr_e_ALIGN),
                as_map->ba.attr & ~pci_asAttr_e_ALIGN,
                (r == PCI_ERR_OK) ? "Ok" : "Failed");

    return r;
}

/*
 ===============================================================================
 hw_free_as

 Un-reserve previously allocated or reserved address space.

 This function is only called by the PCI server. It will typically only be
 called when the PCI server exists however it could also be called in response
 to changing address space requirements due to the live insertion or removal of
 devices

*/
__attribute__ ((visibility ("internal")))
pci_err_t hw_free_as(pci_bdf_t bdf, _pci_asmap_t *as_map)
{
    pci_err_t r = rsrcdb_as_free(as_map);

    slog_info(2, "AS free req for B%u:D%u:F%u (%"PRIx64"/%"PRIx64", align: %"PRIx64", attr: %x ... %s)",
                    PCI_BUS(bdf), PCI_DEV(bdf), PCI_FUNC(bdf),
                    as_map->ba.addr, as_map->ba.size,
                    (uint64_t)1 << (as_map->ba.attr & pci_asAttr_e_ALIGN),
                    as_map->ba.attr & ~pci_asAttr_e_ALIGN,
                    (r == PCI_ERR_OK) ? "Ok" : "Failed");
    return r;
}

/*
 ===============================================================================
 hw_map_as

 Translate the address space <as> (in a hardware dependent way) and return the
 translated address in <as_xlate>

 For hardware that does not support, or chooses not to implement, address
 translation, this function can simply copy <as> to <as_xlate>.

 The function is called in 2 contexts, the PCI server and driver software.

 PCI server context
 ------------------
 This function is called by the PCI server when processing pci_device_read_ba()
 requests from driver software in order to obtain the address space information
 for the specified device.
 It will be called for each address space allocated or reserved by hw_alloc_as()
 and hw_resv_as() respectively and provides the HW dependent module the
 opportunity to translate the device PCI address spaces into addresses which can
 be mmap()'d by driver software in order to access those PCI device address
 spaces. These accesses to PCI device address space as obtained by the
 pci_device_read_ba() call are OUTBOUND address space translations and this
 mapping function should differentiate between INBOUND (driver context below)
 and OUTBOUND address space translations. The flow is as shown below. Note that
 the translation performed is for physical (hardware) addresses and is post
 processor MMU. The OUTBOUND address translation provided by this function in
 this case is the PCI translated paddr

 CPU address space                           PCI address space
 ------------------                          ------------------
 PCI translated paddr --> translation HW --> PCI device target paddr (BAR's)

 Driver Context
 --------------
 This function is also called in driver context. This means that it can be
 potentially called from multiple programs depending on device attachment
 restrictions.
 For devices that support direct transfers from device to processor address
 spaces (typically memory), the driver software is required to program the
 physical address of the transfer destination. For hardware that supports or
 requires these INBOUND addresses to be translated, this function provides the
 opportunity for driver software to obtain the translated address to use in
 programming the device hardware. The flow is as shown below. Note that the
 translation performed is for physical (hardware) addresses and is pre processor
 MMU. The INBOUND address translation provided by this function in this case is
 the CPU translated paddr

 CPU address space                       PCI address space
 ------------------                      ------------------
 CPU target paddr <-- translation HW <-- CPU translated paddr


 ** IMPORTANT **
 The <as>->ba.addr will always be treated as a physical address.

*/
__attribute__ ((visibility ("internal")))
pci_err_t hw_map_as(pci_bdf_t bdf, const pci_ba_t *as, pci_ba_t *as_xlate)
{
    int_t controller = get_pcie_ctrl_num(bdf);
    if (controller < 0) return PCI_ERR_EINVAL;
    else
    {
        pcie_ctrl_info_t * const pcie_ctrl_info = get_pcie_ctrl_info(controller);
        pci_err_t r = PCI_ERR_OK;

        *as_xlate = *as;    // start with a copy

        if (as->attr & pci_asAttr_e_OUTBOUND)
        {
            /*
             * caller wants the translated CPU address to use to access a specific PCI address.
             * Verify that the address is valid using the assigned PCI address
             */
            const _pci_asmap_t * const asbase_pci_p = (as->type == pci_asType_e_MEM) ?
                                                        ((as->attr & pci_asAttr_e_PREFETCH) ? &pcie_ctrl_info->outbound.pfmem.pci_phys : &pcie_ctrl_info->outbound.mem.pci_phys) :
                                                        &pcie_ctrl_info->outbound.io.pci_phys;
            const _pci_asmap_t * const asbase_cpu_p = (as->type == pci_asType_e_MEM) ?
                                                        ((as->attr & pci_asAttr_e_PREFETCH) ? &pcie_ctrl_info->outbound.pfmem.cpu_phys : &pcie_ctrl_info->outbound.mem.cpu_phys) :
                                                        &pcie_ctrl_info->outbound.io.cpu_phys;

            /* validate that the incoming address to be translated is valid */
            if (as->addr < asbase_pci_p->ba.addr) r = PCI_ERR_ASPACE_INVALID;
            else if ((as->addr + as->size) > (asbase_pci_p->ba.addr + asbase_pci_p->ba.size)) r = PCI_ERR_ASPACE_INVALID;
            else
            {
                /* given the incoming PCI address to be translated, calculate the offset from base */
                const int64_t offset = as->addr - asbase_pci_p->ba.addr;
                as_xlate->addr = asbase_cpu_p->ba.addr + offset;

                slog_info(2, "%s(B%u:D%u:F%u): [OUTBOUND %s translation] %"PRIx64" --> %"PRIx64"",
                            __FUNCTION__, PCI_BUS(bdf), PCI_DEV(bdf), PCI_FUNC(bdf),
                            (as->type == pci_asType_e_MEM) ? "MEM" : "I/O", as->addr, as_xlate->addr);
            }
        }
        else if (as->attr & pci_asAttr_e_INBOUND)
        {
            /* nothing to translate */
        }
        else r = PCI_ERR_EINVAL;

        if (r == PCI_ERR_OK)
        {
            /*
             * if the call is coming from the server (via the msg_rd_ba() call)
             * then don't change the BIT_MASK attributes because they have already
             * been established in the bdf_entry and reflect the BAR configuration
             * For user calls, we will set the mask to reflect the translated
             * address
             */
            if ((pci_runtime_flags & pci_runtime_flags_e_SERVER) == 0)
            {
                /* finish the translation */
                as_xlate->attr &= ~pci_asAttr_e_BIT_MASK;

                /* set attributes */
                if (as_xlate->type == pci_asType_e_MEM)
                {
                    if (as_xlate->addr > UINT32_MAX) as_xlate->attr |= pci_asAttr_e_64BIT;
                    else as_xlate->attr |= pci_asAttr_e_32BIT;
                }
                else if (as_xlate->type == pci_asType_e_IO)
                {
                    if (as_xlate->addr > UINT16_MAX) as_xlate->attr |= pci_asAttr_e_32BIT;
                    else as_xlate->attr |= pci_asAttr_e_16BIT;
                }
            }
        }
        return r;
    }
}

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL$ $Rev$")
#endif
