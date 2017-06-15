#ifndef _PCI_SERVER_MOD_H_
#define _PCI_SERVER_MOD_H_
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

#include <dlfcn.h>
#include <sys/resmgr.h>
#include <sys/iofunc.h>

#include <pci/pci.h>

#include "private/pci_server_msg.h"
#include "private/pci_debug.h"
#include "private/pci_list.h"
#include "private/hwmod_api.h"


/*
 @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

 This file contains types and macros common to both the PCI server and server
 modules

 @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
*/

#define STRINGX(s)	STRING(s)
#define STRING(s)	#s

/* all server modules must have this function defined */
#define SERVER_MODULE_INITFN		server_mod_init
#define SERVER_MODULE_INITFN_SYM	STRINGX(SERVER_MODULE_INITFN)

#define SERVER_MODULE_VERSIONFN			server_mod_version
#define SERVER_MODULE_VERSIONFN_SYM		STRINGX(SERVER_MODULE_VERSIONFN)

/* we use typeof() to ensure that 64bit rounding works properly */
#define ROUND_UP(x, a) ((typeof(x))(((x) + ((typeof(x))(a)-1)) & ~((typeof(x))(a)-1)))
#define ROUND_DN(x, a) ((typeof(x))((x) & ~((typeof(x))(a)-1)))

/*
 ===============================================================================
 PCI_SERVER_SYM_LOOKUP

 For modules that need to access functions within the PCI server, this macro
 will do the symbol lookup on the first call to the module wrapper and
 initialize the function pointer <fp> so that subsequent calls go directly to
 the intended function.

 It is important that the module wrapper function have the identical name as the
 server implementation

 See _server_func_p.c in the modules requiring this functionality

*/
#define PCI_SERVER_SYM_LOOKUP(fp) \
		do { \
			if ((fp) == NULL) \
			{ \
				(fp) = dlsym(RTLD_DEFAULT, __FUNCTION__); \
				slog_debug(1, "Lookup of server sym %s %s", __FUNCTION__, (fp) == NULL ? "failed" : "ok"); \
			} \
		} while(0)

/*
 ===============================================================================
 connect_entry_t

 This data structure maintains per connection information. See _connect.c
*/
typedef struct connect_entry_s
{
	node_t hdr;
	pthread_mutex_t lock;

	list_hdr_t attach;	// attach list (attach_entry_t)
	list_hdr_t events;	// registered event list (event_connect_entry_t)

} connect_entry_t;

/*
 ===============================================================================
 cs_entry_t

 This data structure contains chassis and slot specific information. See _cs.c

*/

typedef struct
{
	uint_t ref_cnt;			// current number of references to this entry
	pci_bdf_t bdf;			// the device in the chassis/slot (Func always == 0)
} cs_entry_t;

/*
 ===============================================================================
 bdf_entry_t

 This data structure contains per device information. See _bdf.c

*/

typedef struct
{
	pci_cs_t cs;			// the chassis and slot associated with this device

	pci_attachFlags_t attach_flags;	// contains most restrictive flags of all attachers
	uint_t ref_cnt;			// current number of references to this entry
	uint_t attach_cnt;		// number of attaches to the device
	uint_t owner_attach_cnt;	// number of owners attached to the device (for multi owned devices)
	struct ba_info_s
	{
		bool_t assigned;
		pci_ba_t ba;
	} ba_info[7];					// max of 6 BAR entries and 1 Expansion ROM entry as per PCI spec

	list_hdr_t cap_enabled;			// list of enabled capabilities

	struct
	{
		pthread_mutex_t lock;		// protects manipulation of the 'irq' sub-structure */
		_pci_irqType_e type;		// IRQ type
		uint_t num;					// number of assigned IRQ's
		_pci_irqmap_t *map;			// list of assigned IRQ's to vectors
	} irq;

	uint8_t reg_mask[256];			// this needs to be sized large enough to accommodate the
									// the highest register offset that will be accessed in
									// bdf_reg_mask()
} bdf_entry_t;

/*
 ===============================================================================
 bridge_t

 This structure is used to record bridge specific information and is used to
 create the topology tree

 Locking Notes
 -------------
 Bridge fields not mentioned below have initialize once, read only access and
 hence don't require explicit locking.

 All 'list_hdr_t' types will have their list add and delete operations
 implicitly locked by the LIST_ADD()/LIST_DEL() macros.

 The 'subordinate' list is built during enumeration which is a single threaded
 operation performed only by the PCI server process. The subordinate list can
 potentially also be updated if a 'bridge' device is live inserted or deleted
 however this not currently supported hence there are no locking issues and so
 traversal of this list is not subject to any locking as it is read-only

 The 'as_info.*.free_space' list is manipulated as devices on the subordinate
 bus of the bridge are live inserted and removed (ie. hot plugged or simulated
 hot plugged). These lists therefore must be explicitly protected during
 traversal

 The 'dev' array is built during enumeration which is a single threaded
 operation performed only by the PCI server process. The 'dev' array will be
 manipulated for bridges which support live insertion/deletion (ie. hotplug).
 In order to support the maximum concurrency, the 'dev' field is protected by a
 reader/writer lock. In systems which do not support hotplug, traversal will be
 read-only and there will be no lock contention and hence minimal overhead
 associated with the locking semantics.

*/
typedef struct bridge_s
{
	node_t hdr;			// peer bridges

	pci_hdrType_t hdrType;
	pci_bdf_t bdf;		// this bridges BDF
	pci_ccode_t ccode;	// cache this so we don't need to re-read it

	/* Bridge capability pointer cache for most needed capabilities */
	struct
	{
		pci_cap_t pcie;
		pci_cap_t slotid;
		pci_cap_t shpc;		// PCI Standard Hot Plug Controller
		pci_cap_t spare;	// just in case
	} cap_ptr;

	struct bridge_s *parent;

	list_hdr_t subordinate;	// head of the list of subordinate bridges

	struct dev_info_s
	{
		pthread_rwlock_t lock;
		uint_t num_devs;	// number of devices in 'dev'
		pci_bdf_t *dev;		// array of BDF's for the devices/endpoints on the secondary bus
	} dev_info;

	/*
	 * slot information
	 * for each bridge that has slots, we maintain an array of secondary bus logical
	 * slot numbers (ie. a bridge with 3 slots has logical slots 0, 1, 2). The actual
	 * system wide physical slot number is contained within the logical slot entry.
	 * 'last_slot_num' is the last assigned physical slot number. The next bridge
	 * would start its first physical slot number (in its slot[0] entry) with
	 * 'last_slot_num' + 1
	 */
	struct cs_info_s
	{
		uint_t num_slots;
		uint_t last_slot_num;	// last slot number on the secondary bus segment
		struct
		{
			uint_t num;		// physical slot number
			uint_t device;	// device number for the slot
			struct reservation_s
			{
				uint_t num_buses;				// number of reserved bus numbers

				struct
				{
					_pci_asmap_t configured;	// 0 size indicates no reservation
					uint64_t assigned;			// amount of any reservation currently assigned
				} io;
				struct
				{
					_pci_asmap_t configured;	// 0 size indicates no reservation
					uint64_t assigned;			// amount of any reservation currently assigned
				} mem;
				struct
				{
					_pci_asmap_t configured;	// 0 size indicates no reservation
					uint64_t assigned;			// amount of any reservation currently assigned
				} pfmem;
			} reservation;
		} *slot;
	} cs_info;

	/*
	 * NOTE that these totals cannot be used to set up the bridge base/limit registers
	 * for pre-configured scenarios. The reason is that these values are based on
	 * the size requirements of devices on the secondary and subordinate buses. However
	 * some pre-configured assignments which have multiple BAR's of the same type use
	 * discontiguous address ranges and the overall requirements calculation for a
	 * device does not consider the hole (as it shouldn't) but the base/limit registers
	 * would need to. This is not really a problem since in the case of a pre-configuration,
	 * we aren't going to change the base/limit settings anyway so these values remain
	 * correct for a fresh configuration
	*/
	struct as_info_s				// address space info for secondary and subordinate buses
	{
		struct
		{
			uint64_t size_total;
			uint_t align_max;		// where alignment is 2^align_max
			bool_t _32bit;			// if any device is not 64bit capable, this is set
			list_hdr_t free_aspace;	// head of the list of free address space
			bool_t assigned;		// marks the bridge as having been processed when assigning address space
		} pfmem;
		struct
		{
			uint64_t size_total;
			uint_t align_max;		// where alignment is 2^align_max
			bool_t _32bit;			// if any device is not 64bit capable, this is set
			list_hdr_t free_aspace;	// head of the list of free address space
			bool_t assigned;		// marks the bridge as having been processed when assigning address space
		} mem;
		struct
		{
			uint32_t size_total;
			uint_t align_max;		// where alignment is 2^align_max
			bool_t _16bit;			// if any device is not 32bit capable, this is set
			list_hdr_t free_aspace;	// head of the list of free address space
			bool_t assigned;		// marks the bridge as having been processed when assigning address space
		} io;
	} as_info;

	pthread_mutex_t event_info_lock;
	void *event_info;	// filled in if the pci_server-event_handler.so module is loaded

	/*
	 * the 'cached_params' sections is used for data that may not be accessible by during certain
	 * scenarios. For example, when a device is live removed (without going through the operator
	 * initiated protocol, ie. a surprise removal), the removal code needs certain pieces of information
	 * like the secondary bus number of a subordinate bridge. Because the card has been removed, this
	 * information will not be available and will cause issues with cleanup. Caching these values in the
	 * data structures associated with the devices before they are removed allows the cleanup processing
	 * to complete properly. The idea is that this structure is only set up by code that needs it. In the
	 * case of live removal, thats when the event_handler module is loaded
	 */
	pthread_mutex_t cached_params_lock;
	struct cached_params_s
	{
		struct
		{
			struct
			{
				pci_busnum_t value;
				bool_t valid;
			} pri;
			struct
			{
				pci_busnum_t value;
				bool_t valid;
			} sec;
			struct
			{
				pci_busnum_t value;
				bool_t valid;
			} sub;
		} busnum;

		struct
		{
			struct
			{
				pci_ba_val_t base;
				pci_ba_val_t limit;
				bool_t valid;
			} mem;
			struct
			{
				pci_ba_val_t base;
				pci_ba_val_t limit;
				bool_t valid;
			} pfmem;
			struct
			{
				pci_ba_val_t base;
				pci_ba_val_t limit;
				bool_t valid;
			} io;
		} base_limit;

	} *cached_params;

} bridge_t;

/*
 ===============================================================================
 server_mod_params_t

 This type defines the data that is made available to server modules without
 the server module having to do a dlsym()
*/
typedef struct
{
	pci_version_t server_version;
	pci_version_t lib_version;
	struct
	{
		int argc;
		const char **argv;
	} args;
} server_mod_params_t;

/*
 ===============================================================================
 enum_params_t

 This type defines enumeration controls and parameters
*/
typedef struct
{
	bool_t configure;	// perform configuration as well as enumeration
	bool_t update_as_requirements;	// calculate and adjust address space requirements

	/*
	 * function pointer for retrieving the next function number. This allows
	 * ARI devices to be more efficiently handled using the ARI capability
	 * function list instead of probing for 256 functions however it is also
	 * used for regularly addressed devices as well. See implementations in
	 * enumerate_bus.c
	 */
	uint_t (*next_func)(pci_bdf_t bdf);

	// this allows enumeration of a subset of devices on the bus segment
	uint_t device_first;
	uint_t device_last;

	bool_t ari;		// treat the end point device as ARI capable
	bool_t retrain_link;	// initiate retraining for secondary bus link

	/*
	 * on initial bus enumeration, we need to process any bus number reservations. This
	 * provides a pointer to those reservations
	 */
	const struct busnum_reservation_s *busnum_reservation;

} enum_params_t;

/*
 ===============================================================================
 pci_ocb_t

 This data structure records the connect_entry on a per open() basis

*/
struct connect_entry_s;
typedef struct
{
	iofunc_ocb_t _ocb;
	struct connect_entry_s *connect_entry;
} pci_ocb_t;

/*
 ===============================================================================
 server_init_f

 This type defines the server module initialization entry point function
*/
typedef pci_err_t (*server_init_f)(const server_mod_params_t *server_mod_params);

/*
 ===============================================================================
 server_mod_version_f

 This type defines the server module version entry point function
*/
typedef pci_version_t (*server_mod_version_f)(void);

/*
 ===============================================================================
 extended_msg_handler_f

 Function pointer type for extended message handler function

*/
typedef pci_err_t (*extended_msg_handler_f)(resmgr_context_t *ctp, io_msg_t *msg,
											IOFUNC_OCB_T *_ocb, iofunc_attr_t *attr, uint_t *reply_len);


/*
 ===============================================================================
 server function prototypes explicitly exported for use by server modules to
 help with type checking. Other symbols can (currently) be located however their
 prototype will not be checked for consistency with their use
*/

bdf_entry_t *bdf_ref_entry(pci_bdf_t bdf);
void bdf_unref_entry(bdf_entry_t *bdf_entry);
pci_err_t bdf_del_entry(pci_bdf_t bdf);
pci_err_t bdf_assign_irqs(pci_bdf_t bdf, uint_t nirq, _pci_irqType_e irq_type, _pci_irqmap_t *irq_map);
void bdf_release_irqs(pci_bdf_t bdf);
pci_ba_val_t bdf_get_bar_ba(pci_bdf_t bdf, uint_t bar_idx);

cs_entry_t *cs_ref_entry(pci_cs_t cs);
void cs_unref_entry(cs_entry_t *cs_entry);
pci_err_t cs_del_entry(pci_cs_t cs);

uint64_t calc_as_size(bdf_entry_t *bdf_entry, pci_asType_e type, pci_asAttr_e attr);
uint_t find_as_align_max(bdf_entry_t *bdf_entry, pci_asType_e type, pci_asAttr_e attr);
bool_t find_as_16bit_restrict(bdf_entry_t *bdf_entry, pci_asType_e type, pci_asAttr_e attr);
bool_t find_as_32bit_restrict(bdf_entry_t *bdf_entry, pci_asType_e type, pci_asAttr_e attr);

void update_as_requirements(pci_bdf_t bdf, bdf_entry_t *bdf_entry, bridge_t *bridge, int add_or_sub);

pci_err_t msg_handler_register(extended_msg_handler_f handler, uint_t flags);

int_t enumerate_bus(uint_t bus, const uint_t bus_limit, enum_params_t *enum_params);
uint_t enum_next_func(pci_bdf_t bdf);
uint_t enum_next_ari_func(pci_bdf_t bdf);
bool_t is_ari_enabled(bridge_t *bridge);

void bridge_free(bridge_t *bridge);
bridge_t *find_parent_bridge(bridge_t *bridge, pci_bdf_t bdf);
bridge_t *find_bridge_bdf(bridge_t *bridge, pci_bdf_t bdf);
bridge_t *find_bridge_slot(bridge_t *bridge, pci_cs_t cs);
bridge_t *find_bridge_sec_busnum(bridge_t *bridge, uint_t sec_busnum);

static inline int_t find_slot_idx(bridge_t *bridge, pci_cs_t device_cs)
{
	uint_t i;
	for (i=0; i<bridge->cs_info.num_slots; i++) {
		if (PCI_SLOT(device_cs) == bridge->cs_info.slot[i].num) return i;
	}
	return -1;
}

/* message handler. Server modules can use this function to utilize server API's without requiring a message pass */
pci_err_t _msg_handler(resmgr_context_t *ctp, pci_msg_t *msg, pci_ocb_t *ocb, iofunc_attr_t *attr, uint_t *reply_len);

void pci_ocb_free(iofunc_ocb_t *ocb);
iofunc_ocb_t *pci_ocb_calloc(resmgr_context_t *ctp, iofunc_attr_t *attr);

pci_err_t mark_bus_reserved(uint_t bus_start, uint_t bus_end);
pci_err_t mark_bus_unreserved(uint_t bus_start, uint_t bus_end);
uint_t calc_reserved_buses(bridge_t *bridge);


void cached_param_set_pri_busnum(bridge_t *bridge, const pci_busnum_t busnum, const bool_t valid);
int_t cached_param_get_pri_busnum(bridge_t *bridge);
void cached_param_set_sec_busnum(bridge_t *bridge, const pci_busnum_t busnum, const bool_t valid);
int_t cached_param_get_sec_busnum(bridge_t *bridge);
void cached_param_set_sub_busnum(bridge_t *bridge, const pci_busnum_t busnum, const bool_t valid);
int_t cached_param_get_sub_busnum(bridge_t *bridge);

void cached_param_set_mem_base_limit(bridge_t *bridge, const pci_ba_val_t base, const pci_ba_val_t limit, const bool_t valid);
bool_t cached_param_get_mem_base(bridge_t *bridge, pci_ba_val_t *base);
bool_t cached_param_get_mem_limit(bridge_t *bridge, pci_ba_val_t *limit);
void cached_param_set_pfmem_base_limit(bridge_t *bridge, const pci_ba_val_t base, const pci_ba_val_t limit, const bool_t valid);
bool_t cached_param_get_pfmem_base(bridge_t *bridge, pci_ba_val_t *base);
bool_t cached_param_get_pfmem_limit(bridge_t *bridge, pci_ba_val_t *limit);
void cached_param_set_io_base_limit(bridge_t *bridge, const pci_ba_val_t base, const pci_ba_val_t limit, const bool_t valid);
bool_t cached_param_get_io_base(bridge_t *bridge, pci_ba_val_t *base);
bool_t cached_param_get_io_limit(bridge_t *bridge, pci_ba_val_t *limit);

pci_err_t force_detachment(const pci_bdf_t bdf);
pci_err_t do_bus_reset(bridge_t *bridge, const pci_bdf_t bdf);
pci_err_t do_function_reset(const pci_bdf_t bdf, const int_t pcie_capid_idx, const int_t af_capid_idx);
pci_err_t do_hw_specific_reset(const pci_bdf_t bdf, const pci_resetType_e reset_type);

#endif	/* _PCI_SERVER_MOD_H_ */

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/services/pci/server/private/pci_server_mod.h $ $Rev: 800885 $")
#endif
