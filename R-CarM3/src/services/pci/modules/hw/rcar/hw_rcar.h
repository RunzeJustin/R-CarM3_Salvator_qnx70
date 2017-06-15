#ifndef _HW_RCAR_H_
#define _HW_RCAR_H_
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

#include "private/pci_hw.h"
#include "hw_lib.h"
#include "hw_cfg.h"
#include "hw_variant.h"


/*
 ===============================================================================

                        Implementation Notes

 This module applies to all current R-Car Gen3 variants of Renesas R-Car family.

 ===============================================================================
*/

/*
 ===============================================================================
rcar_pcie_icfg_reg_t

 This structure contains per controller runtime configuration information.

 Because which controllers are initialized can be controlled with the HW config
 file, the 'ctrl_num' field will identify the controller to which this entry
 applies. Controllers are numbered 0 through (NUM_CONTROLLERS - 1)

*/
#define PAD_FROM_TO(_off_from_, _off_to_, _pad_name_) \
        volatile uint32_t _pad_name_[((_off_to_) - (_off_from_)) / sizeof(uint32_t)]

typedef struct __attribute__((packed,aligned(4)))
{
    volatile uint32_t PCIELAR;
    volatile uint32_t PCIELA_RESERVE1;
    volatile uint32_t PCIELAMR;
    volatile uint32_t PCIELA_RESERVE2[5];
} pciela_reg_t;

typedef struct __attribute__((packed,aligned(4)))
{
    volatile uint32_t PCIEPALR;
    volatile uint32_t PCIEPAUR;
    volatile uint32_t PCIEPAMR;
    volatile uint32_t PCIEPTCTLR;
    volatile uint32_t PCIEP_RESERVE[4];
} pciep_reg_t;

typedef struct __attribute__((packed,aligned(4)))
{
    PAD_FROM_TO(0, 0x10, reserved1);
    volatile uint32_t PCIECAR;
    volatile uint32_t reserved2;
    volatile uint32_t PCIECCTLR;
    volatile uint32_t reserved3;
    volatile uint32_t PCIECDR; /* must not be read in EP mode */
    volatile uint32_t reserved4;
    volatile uint32_t PCIEMSR;
    PAD_FROM_TO(0x2C, 0x48, reserved5);
    volatile uint32_t PCIEUNLOCKCR;
    PAD_FROM_TO(0x4C, 0x400, reserved6);
    volatile uint32_t PCIEINTXR;
    PAD_FROM_TO(0x404, 0x7F0, reserved7);
    volatile uint32_t PCIEPHYSR;
    PAD_FROM_TO(0x7F4, 0x840, reserved8);
    volatile uint32_t PCIEMSITXR;
    PAD_FROM_TO(0x844, 0x2000, reserved9);
    volatile uint32_t PCIETCTLR;
    volatile uint32_t PCIETSTR;
    volatile uint32_t PCIEINTR;
    volatile uint32_t PCIEINTER;
    PAD_FROM_TO(0x2010, 0x2020, reserved10);
    volatile uint32_t PCIEERRFR;
    PAD_FROM_TO(0x2024, 0x2044, reserved11);
    volatile uint32_t PCIEMSIFR;
    volatile uint32_t PCIEMSIALR;
    volatile uint32_t PCIEMSIAUR;
    volatile uint32_t PCIEMSIIER;
    PAD_FROM_TO(0x2054, 0x2080, reserved12);
    volatile uint32_t PCIEPRAR[6];
    PAD_FROM_TO(0x2098, 0x2200, reserved13);
    pciela_reg_t PCIELA[6];
    PAD_FROM_TO(0x22C0, 0x3400, reserved14);
    pciep_reg_t PCIEP[4];
    PAD_FROM_TO(0x3480, 0x10000, reserved15);
    volatile uint32_t PCICONF[16];
    volatile uint32_t PMCAP[2];
    PAD_FROM_TO(0x10048, 0x10070, reserved16);
    volatile uint32_t EXPCAP[15];
    PAD_FROM_TO(0x100AC, 0x10100, reserved17);
    volatile uint32_t VCCAP[7];
    PAD_FROM_TO(0x1011C, 0x11000, reserved18);
    volatile uint32_t IDSETR0;
    volatile uint32_t IDSETR1;
    PAD_FROM_TO(0x11008, 0x11048, reserved19);
    volatile uint32_t TLCTLR;
    PAD_FROM_TO(0x1104C, 0x11054, reserved20);
    volatile uint32_t MACSR;
    volatile uint32_t MACCTLR;
} rcar_pcie_icfg_reg_t;

typedef struct
{
    _pci_asmap_t cpu_phys;  // address space of the bus from CPU perspective
    _pci_asmap_t pci_phys;  // address space of the bus from PCI perspective (translated)
} aspace_t;

/*
 ===============================================================================
 pcie_ctrl_info_t

 This structure contains per controller runtime configuration information.

 Because which controllers are initialized can be controlled with the HW config
 file, the 'ctrl_num' field will identify the controller to which this entry
 applies. Controllers are numbered 0 through (NUM_CONTROLLERS - 1)

*/
typedef struct
{
    int_t ctrl_num;         /* 0 to NUM_CONTROLLERS - 1, -1 indicates entry is not used */
    pcie_ctrl_type_e type;  /* controller type */
    uint64_t reg_offset;    /* register offset to the specific controller */
    struct
    {
        aspace_t io;
        aspace_t mem1;
        aspace_t mem;
        aspace_t pfmem;
    } outbound;
    struct
    {
        aspace_t mem;
    } inbound;

    struct
    {
        uint64_t inbound_msi_match_addr;    // non RAM address post ATU, initialized when ATU configured
        uint64_t msi_base;      // holds the address portion of MSI vector. Initialized in mod_irq.c
        uint_t msi_vec_first;   // data portion of the first MSI vector assigned to this controller
        uint_t msi_vec_last;    // data portion of the last MSI vector assigned to this controller
        pci_irq_t pin[4];       // vectors assigned to pins INTA - INTD
    } interrupts;

    struct
    {
        bool_t trained; // see comments in mod_rdwr.c
        uint_t num;     // the (primary side) link (or bus number) for this Root Port
        uint_t lanes;   // the number of lanes
    } link;

    int_t init_level;       // this is used to control what gets (re)initialized

    rcar_pcie_icfg_reg_t *icfg_reg; // mmap()'d address of the internal configuration registers

} pcie_ctrl_info_t;


/*
 ===============================================================================
 pcie_info_t

 This structure contains runtime configuration information for all configured
 PCIe controllers

 The 'ctrl' substructure is allocated to allow future HW config file control.
 See init_controller_info.c

*/
typedef struct
{
    uint_t num_controllers;
    pcie_ctrl_info_t *ctrl;

} pcie_info_t;

/*
 ===============================================================================
 iATU_id_e

*/
typedef enum
{
iATU_id_e_first = 0,

    iATU_INBOUND_id_e_MEM_0 = iATU_id_e_first,
    iATU_INBOUND_id_e_MEM_1,
    iATU_INBOUND_id_e_MEM_2,

    iATU_OUTBOUND_id_e_IO = iATU_id_e_first,
    iATU_OUTBOUND_id_e_MEM_1,
    iATU_OUTBOUND_id_e_MEM,
    iATU_OUTBOUND_id_e_MEM_PF,

    iATU_id_e_last,
} iATU_id_e;

/*
 ===============================================================================
 iATU_config_t

*/
typedef struct
{
    iATU_id_e id;
    aspace_t aspace;
    uint64_t mask;

} iATU_config_t;

typedef struct
{
    struct
    {
        const uint_t num_entries;
        iATU_config_t * const cfg;
    } outbound;
    struct
    {
        const uint_t num_entries;
        iATU_config_t * const cfg;
    } inbound;
} ATU_ctrl_config_t;

/* internal function prototypes */

extern pci_err_t init_controller_info(pcie_info_t *pcie_info);
extern pci_err_t rcar_pcie_init(pcie_info_t * const pcie_info);

extern pci_err_t hw_mem_rd(void *cfgmem_base, const pci_bdf_t bdf, const uint_t offset, const uint_t size, uint8_t *buf);
extern pci_err_t hw_mem_wr(void *cfgmem_base, const pci_bdf_t bdf, const uint_t offset, const uint_t size, uint8_t *buf);

extern pci_err_t init_inbound_atus(const uint_t controller);
extern pci_err_t init_outbound_atus(const uint_t controller);

extern pci_err_t init_inbound_mem_atu(const uint_t controller, iATU_config_t *atu_config);
extern pci_err_t init_outbound_mem_atu(const uint_t controller, iATU_config_t *atu_config);
extern pci_err_t init_outbound_io_atu(const uint_t controller, iATU_config_t *atu_config);

extern pci_err_t get_outbound_memspace_atu_info(const uint_t controller, int iATU_id, pci_ba_t *asinfo, pci_ba_t *asinfo_xlate);
extern pci_err_t get_outbound_iospace_atu_info(const uint_t controller, pci_ba_t *asinfo, pci_ba_t *asinfo_xlate);
extern pci_err_t get_inbound_memspace_atu_info(const uint_t controller, pci_ba_t *asinfo, pci_ba_t *asinfo_xlate);
extern pci_err_t get_inbound_msi_atu_info(const uint_t controller, pci_ba_t *asinfo, pci_ba_t *asinfo_xlate);
extern ATU_ctrl_config_t *get_atu_config(const uint_t controller);

extern rcar_pcie_icfg_reg_t *get_pcie_icfg_reg_p(const uint_t controller);
extern pcie_ctrl_info_t *get_pcie_ctrl_info(const uint_t controller);
extern int_t get_pcie_ctrl_num(const pci_bdf_t bdf);
extern uint_t get_pcie_num_ctrls(void);
extern bool_t check_link_trained(const uint_t controller);

#define upper_32_bits(n) ((uint32_t)(((n) >> 16) >> 16))
#define lower_32_bits(n) ((uint32_t)(n))

#endif  /* _HW_RCAR_H_ */

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL$ $Rev$")
#endif
