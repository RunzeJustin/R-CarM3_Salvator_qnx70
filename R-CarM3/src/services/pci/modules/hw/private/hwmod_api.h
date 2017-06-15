#ifndef _HWMOD_API_H_
#define _HWMOD_API_H_
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

#include "private/pci_mod.h"
#include "private/pci_list.h"


extern volatile uint_t force_io;


/* names for the PCI server resources managed by the RSRCDB. These can be NULL */
#define PCI_RSRC_NAME_MSGVEC		"PCI MSG Vectors"
#define PCI_RSRC_NAME_MEM_ASPACE	"PCI Mem ASpace"
#define PCI_RSRC_NAME_IO_ASPACE		"PCI IO ASpace"

/*
 ===============================================================================
 _pci_bar_t

 Type to hold a base address register value
*/
typedef uint32_t	pci_bar_t;
/*
 ===============================================================================
 _pci_irqType_e
 _pci_irqAttr_e

 IRQ types and attributes

*/
typedef enum
{
	/* types are specified as one of the following values */
	_pci_irqType_e_MSG,		// message based interrupts (like MSI/MSI-X or others)
	_pci_irqType_e_PIN,		// pin based interrupts

	_pci_irqType_e_NONE = -1,

} _pci_irqType_e;

typedef enum
{
	/* attributes are specified as a bitwise inclusive OR of the following values */
	_pci_irqAttr_e_NONE = 0,

	_pci_irqAttr_e_CONTIG = (1u << 0),		// interrupts must be a contiguous series (ie. 1,2,3,..)

} _pci_irqAttr_e;

/*
 ===============================================================================
 irq_pin_t

 This structure defines the information required by the '_pci_irqType_e_PIN'
 interrupt type
*/
typedef struct
{
	uint32_t val;
} irq_pin_t;

/*
 ===============================================================================
 irq_msg_t

 This structure defines the information required by the '_pci_irqType_e_MSG'
 interrupt type
*/
typedef struct
{
	uint64_t addr;
	uint32_t data;
} irq_msg_t;

/*
 ===============================================================================
 _pci_irqmap_t

 This structure defines the mapping between an internal, hardware dependent
 system vector and IRQ used to InterruptAttach() to
*/

typedef struct
{
	pci_irq_t irq;	// system IRQ to InterruptAttach() to
	union
	{
		irq_pin_t pin;
		irq_msg_t msg;
	} vector;
} _pci_irqmap_t;

/*
 ===============================================================================
 _pci_asmap_t

*/

typedef struct
{
	node_t hdr;		// used for the free list when devices are live inserted/removed
	pci_ba_t ba;
} _pci_asmap_t;

/*
 ===============================================================================
 _pci_accessAttr_e

*/

typedef enum
{
	_pci_accessAttr_e_NORMAL = 0,

	_pci_accessAttr_e_CHECK = (1u << 0),		// just do an access check
	_pci_accessAttr_e_ISR = (1u << 1),		// the access is from an ISR

	/* the access is to normally protected offset 0x34 */
	_pci_accessAttr_e_OFFSET_34 = (1u << 2),

} _pci_accessAttr_e;

/*
 ===============================================================================
 _pci_resetPhase_e

 Currently defined HW reset phases

 Phase 1 - just prior to issuing the reset. This means prior to toggling bit
 	 	   6 in the BCTRL register of a bridge device or of setting the
 	 	   appropriate FLR bit in either the PCIe or AF capability.
 	 	   All required devices have entered hold off and no transactions are
 	 	   pending. The command register bus master bit has been cleared and
 	 	   the interrupt disable bit has been set

 Phase 2 - immediately after issuing the reset (as per above) and after the
  	  	   bus/link has returned active

 Phase 3 - just prior to releasing all applicable device hold offs

 UNSPECIFIED - this is the value used for a reset type other than BUS or
 	 	 	   FUNCTION. The reset message handler will call directly into the
 	 	 	   HW module to perform the reset. At the time of the call, all
 	 	 	   device activity will have been halted (similar to the end of a
 	 	 	   phase 1 call) and upon return, all hold offs will be removed
 	 	 	   (similar to the end of a phase 3 call) however there is only one
 	 	 	   call into the HW dependent module and so the phase is unspecified
 	 	 	   If PCI_ERR_OK is returned, the device will be reconfigured otherwise
 	 	 	   it will not

 CHECK_SUPPORTED - this phase is used to check whether a HW specific reset type
                   is supported. It's main use is for HW dependent modules which
                   do support a range of HW specific reset types. For this phase,
                   PCI_ERR_ENOTSUP should be returned for those types not defined.

*/

typedef enum
{
_pci_resetPhase_first = 0,

	_pci_resetPhase_e_UNSPECIFIED = _pci_resetPhase_first,
	_pci_resetPhase_e_1,
	_pci_resetPhase_e_2,
	_pci_resetPhase_e_3,

	_pci_resetPhase_e_CHECK_SUPPORTED,	/* see reset callout description below */

_pci_resetPhase_last = _pci_resetPhase_e_CHECK_SUPPORTED,

} _pci_resetPhase_e;

/*
 ===============================================================================
 pci_csd_t

 This type is used by the find_csd_assignment() API to return the chassis, slot
 and device number information for a chassis slot record

 It is encoded into a 32 bit unsigned integer as follows

 	 xxxx xxxx xxxd dddd cccs ssss ssss ssss
 	                    {_____ pci_cs_t ____}

*/
typedef uint32_t	pci_csd_t;

#define PCI_CSD(_c_, _s_, _d_) \
		((pci_csd_t) \
			( \
				(PCI_CS(_c_, _s_) << 0) | \
				(((_d_) & (PCI_BDF_MAX_DEVS - 1)) << (sizeof(pci_cs_t) * 8)) \
			) \
		)

#define PCI_CSD_NONE		((pci_csd_t)-1)
#define PCI_CSD_CS(_csd_)	((pci_cs_t)((pci_csd_t)(_csd_) & ((sizeof(pci_cs_t) * 8) - 1)))
#define PCI_CSD_DEV(_csd_)	((uint_t)(((pci_csd_t)(_csd_) >> (sizeof(pci_cs_t) * 8)) & (PCI_BDF_MAX_DEVS - 1)))

/*
 ===============================================================================
 intrinfo_entry_type_e

 Used with find_vec_entry() to allow for an intrinfo entry to be found based on
 whether its a match with the 'vector_base', the 'cpu_intr_base' or the
 'cascade_vector' field respectively
 The *_RANGE selections match if <vector> is within the first and last of the
 specified range

*/
typedef enum
{
	intrinfo_entry_type_e_BASE,
	intrinfo_entry_type_e_BASE_RANGE,
	intrinfo_entry_type_e_CPU,
	intrinfo_entry_type_e_CPU_RANGE,
	intrinfo_entry_type_e_CASCADE,
} intrinfo_entry_type_e;

/*
 ===============================================================================
 hwmod_api_t

 This is the structure which provides the access API's for the HW dependent
 modules of the PCI server

 In order to allow future extensions and retain backwards compatibility, this
 structure can only ever be extended. Fields can never be removed or reordered.
 The 'struct_size' field ensures this compatibility with the PCI library as
 follows.
 	 - If this structure is extended and used with an existing libpci.so, the
 	   new fields will not be known to libpci.so and hence not used but the
 	   interface will continue to function properly since libpci.so idea of
 	   sizeof(hwmod_api_t) will be <= 'struct_size' of the new structure
 	 - If this structure is extended and a new libpci.so delivered but that
 	   library is used with an older HW dependent module, libpci.so's idea of
 	   sizeof(hwmod_api_t) will be > 'struct_size' of the old HW dependent
 	   module and hence will be rejected

 So, if the API's for a new HW dependent module are extended, that new module
 can be used with an existing libpci.so, however none of the new features of
 that HW dependent module will be utilized. If the API's for a new HW dependent
 module are extended and a new libpci.so provided which utilizes those new
 API's, the new libpci.so can only be used with a new HW dependent module, which
 is exactly what one would expect.

*/
typedef struct
{
	uint_t struct_size;
	pci_version_t (*mod_version)(void);
	bool_t (*mod_compat)(version_typecheck_e check_type, pci_version_t version);

	pci_err_t (*cfg_rd)(pci_bdf_t bdf, uint_t offset, uint_t size, uint8_t *buf, _pci_accessAttr_e accessAttr);
	pci_err_t (*cfg_wr)(pci_bdf_t bdf, uint_t offset, uint_t size, uint8_t *buf, _pci_accessAttr_e accessAttr);
	pci_err_t (*alloc_irq)(pci_bdf_t bdf, _pci_irqType_e irq_type, _pci_irqAttr_e irq_attr, uint_t *nirq, _pci_irqmap_t *irq_map);
	pci_err_t (*free_irq)(pci_bdf_t bdf, _pci_irqType_e irq_type, uint_t nirq, _pci_irqmap_t *irq_map);
	pci_err_t (*alloc_as)(pci_bdf_t bdf, pci_asType_e as_type, pci_asAttr_e as_attr, uint64_t as_size, _pci_asmap_t *as_map);
	pci_err_t (*resv_as)(pci_bdf_t bdf, _pci_asmap_t *as_map);
	pci_err_t (*free_as)(pci_bdf_t bdf, _pci_asmap_t *as_map);
	pci_err_t (*map_as)(pci_bdf_t bdf, const pci_ba_t *as, pci_ba_t *as_xlate);

	pci_err_t (*add_device_hold_off)(pci_bdf_t bdf);
	pci_err_t (*remove_device_hold_off)(pci_bdf_t bdf);
	pci_err_t (*initiate_hold_off)(void);
	pci_err_t (*release_hold_off)(void);

	pci_csd_t (*find_csd_assignment)(pci_csd_t csd, pci_vid_t vid, pci_did_t did, uint_t idx);

	/*
	 * reset callout processing (see also msg_device_reset() comments)
	 *
	 * If this callout is implemented, it will be called on every phase of reset processing
	 * so that the HW dependent module can manage any HW specific requirements. The 'pci_err_t'
	 * returned from this function will effect the reset processing sequence as follows
	 *
	 * - if PCI_ERR_OK is returned, reset processing will continue
	 * - if PCI_ERR_ENOTSUP is returned from any phase other than _pci_resetPhase_e_CHECK_SUPPORTED,
	 *   reset processing will continue as if PCI_ERR_OK had been returned
	 * - if any error other than those mentioned above is returned from any phase other than
	 *   _pci_resetPhase_e_CHECK_SUPPORTED, reset processing will be terminated
	 *
	 * Therefore, if a HW dependent module does not support any HW module specific reset types and
	 * is not required to perform and HW specific processing during any phase of reset processing,
	 * PCI_ERR_OK should be returned for every phase except _pci_resetPhase_e_CHECK_SUPPORTED and
	 * PCI_ERR_ENOTSUP should be returned for _pci_resetPhase_e_CHECK_SUPPORTED. Or better yet,
	 * leave this callout entry NULL
	 *
	 * If HW specific reset processing is required, then the result of that processing should be
	 * returned. Any return other than PCI_ERR_OK and PCI_ERR_ENOTSUP will terminate the reset
	 * processing. PCI_ERR_ENOTSUP should be returned for _pci_resetPhase_e_CHECK_SUPPORTED.
	 *
	 * If a HW dependent module does support HW specific reset types then the result of that
	 * processing should be returned. When called for a HW specific reset type, the first call
	 * will be with phase _pci_resetPhase_e_CHECK_SUPPORTED to allow this callout to identify
	 * whether or not the specific reset type is supported. If it is (PCI_ERR_OK returned) then
	 * all remaining calls will be with phase _pci_resetPhase_e_UNSPECIFIED
	 *
	 */
	pci_err_t (*reset)(const pci_resetType_e reset_type, const _pci_resetPhase_e phase, const pci_bdf_t bdf, uintptr_t extra);

	/* void (*reserved1)(void); taken for check_rbar_override() */
	pci_err_t (*check_rbar_override)(const pci_bdf_t bdf, const uint_t bar_num, pci_ba_val_t *size, uintptr_t extra);

	/*
	 * pad out 'hwmod_api_t' so we can avoid an immediate version bump on specific HW modules
	 * if a new API is not required for a specific module.
	 * This allows the sizeof(hwmod_api_t) > struct_size check to pass in the libc loading of
	 * a HW dependent module as long as an API specific existence check (offsetof() and NULL)
	 * is done (as with the 'reset' API above)
	 */
	void (*reserved2)(void);
	void (*reserved3)(void);
	void (*reserved4)(void);
	void (*reserved5)(void);
	void (*reserved6)(void);
	void (*reserved7)(void);

} hwmod_api_t;

extern pci_err_t cfg_rd(pci_bdf_t bdf, uint_t offset, uint_t size, uint8_t *buf, _pci_accessAttr_e accessAttr);
extern pci_err_t cfg_wr(pci_bdf_t bdf, uint_t offset, uint_t size, uint8_t *buf, _pci_accessAttr_e accessAttr);

/*
 ===============================================================================
 The HW dependent module provides an initialized function pointer table of type
 'hwmod_api_t' and named HW_MODULE_ACCESS. The symbol can be found by the
 PCI library using the search string HW_MODULE_ACCESS_SYM.

 Additionally, this module may also provide an initialization function named
 HW_MODULE_INITFN which can be found by the PCI library using the search
 string HW_MODULE_INITFN_SYM.

 The PCI server modload_hw() function will lookup HW_MODULE_INITFN_SYM first
 and if found call that function. It expects the 'hwmod_api_t **' parameter
 passed in to be initialized on successful return.

 If HW_MODULE_INITFN_SYM does not exist (it is optional) or it fails for
 some reason, then the symbol HW_MODULE_ACCESS_SYM is searched for in order
 to gain access to the HW dependent module API's.

 IMPORTANT: These symbol names can never change if backward compatibility is to
 	 	 	be maintained
*/
#define STRINGX(s)	STRING(s)
#define STRING(s)	#s

#define HW_MODULE_ACCESS		hwmod_access
#define HW_MODULE_INITFN		hwmod_init

#define HW_MODULE_ACCESS_SYM	STRINGX(HW_MODULE_ACCESS)
#define HW_MODULE_INITFN_SYM	STRINGX(HW_MODULE_INITFN)



#endif	/* _HWMOD_API_H_ */

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/services/pci/modules/hw/private/hwmod_api.h $ $Rev: 799923 $")
#endif
