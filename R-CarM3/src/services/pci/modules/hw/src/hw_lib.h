#ifndef _HW_LIB_H_
#define _HW_LIB_H_
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

#include <pthread.h>
#include <atomic.h>
#include <sys/mman.h>
#include <assert.h>

#include <pci/pci.h>

#include "private/pci_lib.h"
#include "private/pci_debug.h"
#include "private/hwmod_api.h"


#if !defined(ROUND_UP) && !defined(ROUND_DN)
/* we use typeof() to ensure that 64bit rounding works properly */
#define ROUND_UP(x, a) ((typeof(x))(((x) + ((typeof(x))(a)-1)) & ~((typeof(x))(a)-1)))
#define ROUND_DN(x, a) ((typeof(x))((x) & ~((typeof(x))(a)-1)))
#endif	/* !defined(ROUND_UP) && !defined(ROUND_DN) */


/*
 ===============================================================================
 hw_shm_t

 A single shared memory page is created that is used by the HW dependent module
 to maintain shared process data. This includes the configuration space access
 lock as well as data structures for allowing device accesses to be blocked in
 the case of the server dealing with device removals of power state changes.

 A single variable 'hw_shm' exists in the context of all processes using the
 PCI server

 This structure overlays a shared memory object at offset 0 and is therefore
 page aligned

 Based on a typical 32 byte cache line, we force the 'cfg_reg_lock' and the
 'cfg_access_sem' into separate lines. The reason is to try and avoid the
 invalidation of one because of the other in an SMP environment
*/
typedef struct
{
	// 32 byte alignment (implied)
	uint32_t cfg_reg_lock;
	uint32_t cfg_reg_lock_contention_cnt_lo;
	uint32_t cfg_reg_lock_contention_cnt_hi;
	uint32_t reserved1[5];

	// 32 byte alignment
	uint32_t cfg_access_sem;
	uint32_t cfg_nested_holdoff_count;	// the number of times hw_initiate_hold_off() has been called
	uint32_t reserved2[6];

	// 32 byte alignment
	uint32_t bus_hold_off[PCI_MAX_BUSES / (sizeof(uint32_t) * 8)];
	uint32_t dev_hold_off[PCI_MAX_BUSES];

	struct
	{
		pthread_mutex_t mutex;
		pthread_cond_t cond;
	} hold_off;

	bool_t off_34;
	uint64_t last;

	/* from 'last' to the end of the shared memory segment can be used by HW modules */

} hw_shm_t;

#define HW_HOLDOFF_REQ		(1u << 31)
#define HW_HOLDOFF_INUSE	(1u << 30)
#define HW_HOLDOFF_ACTIVE	(HW_HOLDOFF_REQ | HW_HOLDOFF_INUSE)

extern hw_shm_t *hw_shm;

/*
 ===============================================================================
 bdf_hw_map_t

 This structure is used to map a BDF to a pointer which will allow memory
 mapped accesses to the configuration region. A value of BDF_HW_MAP_IO indicates
 that there is no memory mapped access to the configuration space. A value of
 BDF_HW_MAP_UNKNOWN indicates that it has not yet been determined whether the
 region is accessible by memory mapped accesses

 If a particular hardware implementation does not (or will not) support memory
 mapped configuration space accesses, an array of this type is not required
*/
#define BDF_HW_MAP_UNKNOWN	NULL	// allows 'bdf_hw_array' to be a .bss variable
#define BDF_HW_MAP_MMAP
#define BDF_HW_MAP_IO		((void *)-1)

typedef struct
{
	void *cfgmem_base;
} bdf_hw_map_t;


/* convert the 'bdf' to a 'bd' for use as an index into 'bdf_hw_array' */
#define PCI_BD_IDX(_bdf_)		((((pci_bdf_t)(_bdf_)) & ~PCI_BDF_RSVD_MASK) >> (PCI_IS_ARI(_bdf_) ? 8 : 3))


/* prototypes for required HW dependent module functions */

extern pci_err_t hw_rd(pci_bdf_t bdf, uint_t offset, uint_t size, uint8_t *buf);
extern pci_err_t hw_wr(pci_bdf_t bdf, uint_t offset, uint_t size, uint8_t *buf);
extern pci_err_t hw_alloc_irq(pci_bdf_t bdf, _pci_irqType_e irq_type, _pci_irqAttr_e irq_attr, uint_t *nirq, _pci_irqmap_t *irq_map);
extern pci_err_t hw_free_irq(pci_bdf_t bdf, _pci_irqType_e irq_type, uint_t nirq, _pci_irqmap_t *irq_map);
extern pci_err_t hw_alloc_as(pci_bdf_t bdf, pci_asType_e as_type, pci_asAttr_e as_attr, uint64_t as_size, _pci_asmap_t *as_map);
extern pci_err_t hw_resv_as(pci_bdf_t bdf, _pci_asmap_t *as_map);
extern pci_err_t hw_free_as(pci_bdf_t bdf, _pci_asmap_t *as_map);
extern pci_err_t hw_map_as(pci_bdf_t bdf, const pci_ba_t *as, pci_ba_t *as_xlate);
extern pci_err_t hw_add_device_hold_off(pci_bdf_t bdf);
extern pci_err_t hw_remove_device_hold_off(pci_bdf_t bdf);
extern pci_err_t hw_initiate_hold_off(void);
extern pci_err_t hw_release_hold_off(void);
extern pci_err_t hw_reset(const pci_resetType_e reset_type, const _pci_resetPhase_e phase, const pci_bdf_t bdf, uintptr_t extra);

extern pci_err_t syspage_get_ram_info(_pci_asmap_t *as_ram);
extern void syspage_load_asinfo(void);
extern void syspage_load_intrinfo(void);
extern int find_vector(uint_t vector, _pci_irqType_e irq_type);
extern struct intrinfo_entry *find_vec_entry(struct intrinfo_entry *start, const uint_t vector, const intrinfo_entry_type_e type);

extern void mmap_cfg_reg_lock(void);

extern pci_err_t rsrcdb_as_add(const _pci_asmap_t * const as_map);
extern pci_err_t rsrcdb_as_del(const _pci_asmap_t * const as_map);
extern pci_err_t rsrcdb_as_resv(_pci_asmap_t *as_map, uint_t req_flags);
extern pci_err_t rsrcdb_as_free(_pci_asmap_t *as_map);
extern pci_err_t rsrcdb_irq_resv(_pci_irqType_e irq_type, _pci_irqAttr_e irq_attr, uint_t nirq, pci_irq_t *irq_list, uint_t req_flags);
extern pci_err_t rsrcdb_irq_free(_pci_irqType_e irq_type, uint_t nirq, pci_irq_t *irq_list);

extern pci_irq_t bdf_irq(pci_bdf_t bdf);
extern char bridge_intpin_route(const pci_bdf_t root_bdf, const pci_bdf_t device_bdf, const char intpin_c);

/* function to call to use HW config file interrupt mapping overrides */
extern pci_irq_t extcfg_intpin_to_irq(pci_bdf_t bdf, const char pci_intpin_c);
extern pci_csd_t extcfg_find_csd_assignment(pci_csd_t csd, pci_vid_t vid, pci_did_t did, uint_t idx);
extern bool_t extcfg_check_aspace_filter(const _pci_asmap_t *asmap, _pci_asmap_t *asmap_adjusted);
extern pci_err_t extcfg_check_rbar_override(const pci_bdf_t bdf, const uint_t bar_num, pci_ba_val_t *size, uintptr_t extra);

/*
 ===============================================================================
 config_reg_lock
 config_reg_unlock

 Because the PCI configuration space accesses are performed by accessing 2
 separate registers (an address register and a data register) we must make sure
 that their use is performed atomically. The old PCI server required all reads
 and writes to be handled by only the PCI server process and it runs single
 threaded in order to ensure these accesses are atomic.

 The new PCI server only handles write operations and so read operations are
 performed in the context of the reading process. Additionally, even if this
 were not the case, we desire to run the PCI server multi-threaded, hence the
 need for a global (system wide) lock.

 Note
 There may be a need to have the calling process sleep after 'n' attempts to
 acquire the lock so that long hold offs for say bus segment reconfiguration do
 not impact overall system performance. This is TBD and a counter has been added
 alongside the lock to collect the number of lock contentions that occur

 TODO
 It's unclear whether or not pthread_cleanup_push()/pthread_cleanup_pop() are
 required for this lock as there is (typically) only a handful of instructions
 which execute between the lock and the unlock so the opportunity for
 termination is low, though not zero. If the "backoff" mechanism to sleep after
 'n' unsuccessful lock attempts is added in (this is TBD and will be based on
 whether we see lock contention counts become significant), the window will be
 much larger and will likely necessitate the cleanup handler.

 This is why config_reg_unlock() has a dummy parameter, so its prototype is
 correct for the pthread_cleanup_push()

*/
static inline void config_reg_unlock(void *dummy)
{
	atomic_sub(&hw_shm->cfg_reg_lock, 1);
}

static inline void config_reg_lock(void)
{
//	int_t loop_cnt = 1000;	// 1000 ???
//	pthread_cleanup_push(config_reg_unlock, NULL);

	while (atomic_add_value(&hw_shm->cfg_reg_lock, 1) != 0)
	{
		atomic_sub(&hw_shm->cfg_reg_lock, 1);
		/*
		 * record the number of times the lock is contended. We perform an atomic 64bit
		 * increment by checking for roll over of the least significant 32bits. This will
		 * help us decide if we really need to backoff and sleep
		*/
		if (atomic_add_value(&hw_shm->cfg_reg_lock_contention_cnt_lo, 1) == UINT32_MAX) {
			atomic_add(&hw_shm->cfg_reg_lock_contention_cnt_hi, 1);
		}
#if 0
		if (--loop_cnt < 0)
		{
			struct timespec tdelay;
			nsec2timespec(&tdelay, 5000);	// 5 usec ???
			nanosleep(&tdelay, NULL);
			loop_cnt = 1000;	// 1000 ???, escalate ???
		}
#endif
	}
//	pthread_cleanup_pop(0);
}



#endif	/* _HW_LIB_H_ */

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/services/pci/modules/hw/src/hw_lib.h $ $Rev: 799923 $")
#endif
