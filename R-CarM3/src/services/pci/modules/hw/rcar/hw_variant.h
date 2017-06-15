#ifndef _HW_VARIANT_H_
#define _HW_VARIANT_H_
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

#include <pci/pci.h>

#include "hw_rcar.h"

/*******************************************************************************
**   Mem base
********************************************************************************/
#define RCAR_PCIE0_BASE                     0xFE000000

#define RCAR_PCIE0_MEM_0                    0xFE100000
#define RCAR_PCIE0_MEM_1                    0xFE200000
#define RCAR_PCIE0_MEM_2                    0x30000000
#define RCAR_PCIE0_MEM_3                    0x38000000

#define RCAR_PCIE0_MEM_SIZE_1MB             0x100000
#define RCAR_PCIE0_MEM_SIZE_2MB             0x200000
#define RCAR_PCIE0_MEM_SIZE_128MB           0x8000000

#define RCAR_PCIE0_MEM_MASK_1MB             0x1fff
#define RCAR_PCIE0_MEM_MASK_2MB             0x3fff
#define RCAR_PCIE0_MEM_MASK_128MB           0xfffff

#define RCAR_CPU_MEM_0                      0x40000000
#define RCAR_CPU_MEM_1                      0x500000000
#define RCAR_CPU_MEM_2                      0x600000000

#define RCAR_CPU_MEM_SIZE_1GB               0x40000000

#define RCAR_CPU_MEM_MASK_1GB               0x3ffffff0

/*******************************************************************************
**   Interrupt source
********************************************************************************/
#define RCAR_PCIE0_INT                      (116+32)
#define RCAR_PCIE0_DMA_INT                  (117+32)
#define RCAR_PCIE0_ERROR_INT                (118+32)

/*******************************************************************************
**   Salvator-X Registers
********************************************************************************/
/* PCIECCTLR */
#define RCAR_PCIE_CONFIG_SEND_ENABLE        (1 << 31)
#define RCAR_PCIE_CONFIG_SEND_DISABLE       (0 << 31)
#define RCAR_PCIE_TYPE_0                    (0 << 8)
#define RCAR_PCIE_TYPE_1                    (1 << 8)

/* PCIEINTXR */
#define RCAR_PCIE_INT_ENABLE_MASK           (0xf << 8)
 #define RCAR_PCIE_INTA_EN                  (1 << 8)
 #define RCAR_PCIE_INTB_EN                  (1 << 9)
 #define RCAR_PCIE_INTC_EN                  (1 << 10)
 #define RCAR_PCIE_INTD_EN                  (1 << 11)

/* PCIETCTLR */
#define  RCAR_PCIE_CFINIT                   1
/* PCIETSTR */
#define  RCAR_PCIE_DATA_LINK_ACTIVE         1
/* PCIEERRFR */
#define RCAR_PCIEERRFR_CLEAR                0x1E303373
#define RCAR_PCIE_UNSUPORTED_REQUEST        (1 << 4)
/* PCIELAMR */
#define  RCAR_PCIE_LAM_PREFETCH             (1 << 3)
#define  RCAR_PCIE_LAM_64BIT                (1 << 2)
#define  RCAR_PCIE_LAR_ENABLE               (1 << 1)
/* PCIEPTCTLR */
#define  RCAR_PCIE_PAR_ENABLE               (1 << 31)
#define  RCAR_PCIE_IO_SPACE                 (1 << 8)

/* PCICONF3 */
#define PCICONF_HEADER_TYPE_IDX             3
 #define RCAR_PCIE_HEADER_TYPE_MASK         (0xff << 16)
 #define RCAR_PCIE_HEADER_TYPE_01           (0x01 << 16)
/* PCICONF6 */
#define PCICONF_BUS_NUM_IDX                 6
 #define RCAR_PCIE_SECONDARY_BUS_MASK       (0xff << 8)
 #define RCAR_PCIE_SECONDARY_BUS_NUM        (1 << 8)
 #define RCAR_PCIE_SUBORDINATE_BUS_MASK     (0xff << 16)
 #define RCAR_PCIE_SUBORDINATE_BUS_NUM      (1 << 16)
/* PCICONF15 */
#define PCICONF_INT_IDX                     15
 #define RCAR_PCIE_INT_MASK                 0xffff
 #define RCAR_PCIE_INT_CLEAR                0

/* EXPCAP0 */
#define EXPCAP_CAP_IDX                      0
 #define RCAR_PCIE_CAP_ID_MASK              0xff
 #define RCAR_PCIE_CAP_ID                   0x10       /* PCI Express */
 #define RCAR_PCIE_PORT_TYPE_MASK           (0xf << 20)
 #define RCAR_PCIE_PORT_TYPE_ROOT           (0x4 << 20)
/* EXPCAP3 */
#define EXPCAP_LINK_IDX                     3
 #define RCAR_PCIE_DLLACTRPCAP_MASK         (1<<20)
 #define RCAR_PCIE_DLLACTRPCAP              1
 #define RCAR_PCIE_SUPPORT_SPEED_MASK       0xf
 #define RCAR_PCIE_SUPPORT_SPEED            0x2
/* EXPCAP5 */
#define EXPCAP_SLOT_NUM_IDX                 5
 #define RCAR_PCIE_SLOT_NUM_MASK            (0x1fff << 19)
 #define RCAR_PCIE_SLOT_NUM                 (0x1fff << 19)
/* EXPCAP7 */
#define EXPCAP_CAP_EN_IDX                   7
 #define RCAR_PCIE_CAP_MASK                 0x1f
 #define RCAR_PCIE_SERRCEE                  (1<<0)
 #define RCAR_PCIE_SERRNFEE                 (1<<1)
 #define RCAR_PCIE_SERRFEE                  (1<<2)
 #define RCAR_PCIE_PMEINTE                  (1<<3)
 #define RCAR_PCIE_CRSVISE                  (1<<4)
/* EXPCAP12 */
#define EXPCAP_TARGET_SPEED_IDX             12
 #define RCAR_PCIE_TARGET_SPEED_MASK        0xf
 #define RCAR_PCIE_TARGET_SPEED             0x2

/* VCCAP0 */
#define VCCAP_CAP_OFFSET_IDX                0
 #define RCAR_PCIE_CAP_OFFSET_MASK          (0xfff << 20)
 #define RCAR_PCIE_CAP_OFFSET               0

/* IDSETR1 */
#define RCAR_PCIE_CLASS_BRIDGE_PCI          (0x0604 << 16)
/* TLCTLR */
#define RCAR_PCIE_COMP_TIMEOUT_MASK         (0x3f << 8)
#define RCAR_PCIE_COMP_TIMEOUT              50

/* MACSR */
#define RCAR_PCIE_LINK_SPEED_MASK           (0xf << 16)
#define RCAR_PCIE_LINK_SPEED_2_5            (1 << 16)
#define RCAR_PCIE_LINK_SPEED_5_0            (2 << 16)
#define RCAR_PCIE_LINK_WIDTH(x)             (((x) >> 20) & 0x3)


typedef struct
{
    uint64_t base;
    uint64_t size;
} ctrl_map_info_t;

#define CTRL_ICFG_MAP_INITIALIZER \
        { \
            [0] = {.base = RCAR_PCIE0_BASE, .size = sizeof(rcar_pcie_icfg_reg_t)}, \
        }

/*
 ===============================================================================
 NUM_CONTROLLERS
 NUM_INBOUND_ATUS
 NUM_OUTBOUND_ATUS

 The maximum number of controllers and inbound/outbound ATU's on this variant.
 The number of controllers in use can be overridden in a HW config file however
 this is the most supported by this module

 Similarly, the number of ATU's represents the actual number of hardware
 resources.

*/
#define NUM_CONTROLLERS         1

#define NUM_INBOUND_ATUS        6
#define NUM_OUTBOUND_ATUS       4

/*
 ===============================================================================
 DEFAULT_BUS_STRIDE

 for multi-controller implementations, this is the default bus stride. For
 example, for 3 controllers, controller 0 would be on bus 0, controller 1 on
 bus 8 and controller 2 on bus 16. These values can be overridden with a HW
 config file if the module supports that

*/
#define DEFAULT_BUS_STRIDE      8

/*
 ===============================================================================
 PCIe1_SLOT_NUM
 PCIe2_SLOT_NUM

 Prior to enumeration, we have the ability to program the assigned slot numbers
 and these macros provides the board specific values. If a board does not have a
 slot for a given controller, use a value of 0 for the slot number and the slot
 implemented bit of the device capabilities register will be cleared

 Note that slot numbering should start at 1

*/
#define PCIe1_SLOT_NUM          0

/*
 ===============================================================================
 PCIe1_NUM_LANES
 PCIe2_NUM_LANES

*/
#define PCIe1_NUM_LANES         1

/*
 ===============================================================================
 PCIe1_APERTURE_BASE
 PCIe2_APERTURE_BASE

 These 2 macros define the CPU side base address of each of the 128 MB aperture
 for each controller. The ATU's are actually programmed with an aperture offset
 which is added to these base addresses in order to hit the specific address
 space window

*/
#define PCIe1_APERTURE_BASE     RCAR_PCIE0_MEM_2


/*
 ===============================================================================
 PCIe1_INT
 PCIe2_INT

 The 2 PCIe interrupt controllers have the following assignments

*/
#define PCIe1_INT               RCAR_PCIE0_INT


#endif	/* _HW_VARIANT_H_ */

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL$ $Rev$")
#endif
