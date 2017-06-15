#ifndef _PCI_EVT_H_
#define _PCI_EVT_H_
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

#include <pci/pci_event.h>

#include "private/pci_server_xmsg.h"
#include "private/pci_list.h"


extern int get_pci_server_evt_coid(int *err);
extern int get_pci_server_evt_chid(int *err);

/*
 ===============================================================================
 evt_info_t

 This structure contains all the information required to call the appropriate
 handler with the appropriate arguments when the registered event associated
 with this structure (via the 'event_info_idx') occurs.
 There is one of these structures per successfully registered event

*/
typedef struct
{
	node_t hdr;

	int_t idx;			// back reference to the 'event_info_idx' array
	pthread_t tid;		// ID for the event handler thread
	pci_xmsg_e type;
	union
	{
		struct
		{
			pci_event_device_insert_f func;
			pci_cs_t cs;
		} insert;
		struct
		{
			pci_event_device_remove_f func;
			pci_cs_t cs;
			pci_bdf_t bdf;
		} remove;
		struct
		{
			pci_event_link_state_f func;
			pci_bdf_t bdf;
			pci_cap_t pcie_cap;		/* for the bridge controlling the link */
		} link;
	} handler;

} evt_info_t;


/*
 ===============================================================================
 PCI_EVTHDL_* macros

 The 'pci_evthdl_t' is untyped to driver software however encoded within it is
 	 1. the connection id to the PCI server
 	 2. an event id which uniquely identifies the event registration

 The following macros will encode and decode the various bits of information
 into/from a 'pci_evthdl_t'

 IMPORTANT:

 Use these macros on a zeroed pci_evthdl_t

*/
#if __PTR_BITS__ == 64
typedef struct __attribute__((packed, aligned(8)))
{
	uint8_t coid;
	uint8_t event_id;
	uint16_t reserved;
	uint32_t spare;
} _pci_evthdl_t;
#else	/* __PTR_BITS__ */
typedef struct __attribute__((packed, aligned(4)))
{
	uint8_t coid;
	uint8_t event_id;
	uint16_t reserved;
} _pci_evthdl_t;
#endif	/* __PTR_BITS__ */

#if 1	// make this 0 and clean compile everything if you want to debug using unscrambled evthdl's

#define PCI_EVTHDL_COID_XOR				(uint8_t)'Q'
#define PCI_EVTHDL_EVENT_ID_XOR			(uint8_t)'S'
#define PCI_EVTHDL_RESV_XOR				(((uint16_t)'S' << 8) | ((uint16_t)'L'))

#else

#define PCI_EVTHDL_COID_XOR				0
#define PCI_EVTHDL_EVENT_ID_XOR			0
#define PCI_EVTHDL_RESV_XOR				0

#endif

#define PCI_EVTHDL_ENCODE_EVENT_ID(_evthdl_, _id_)	((_pci_evthdl_t *)&(_evthdl_))->event_id = ((_id_) ^ PCI_EVTHDL_EVENT_ID_XOR)
#define PCI_EVTHDL_DECODE_EVENT_ID(_evthdl_)		((((_pci_evthdl_t *)&(_evthdl_))->event_id) ^ PCI_EVTHDL_EVENT_ID_XOR)

#define PCI_EVTHDL_ENCODE_COID(_evthdl_, _coid_)	((_pci_evthdl_t *)&(_evthdl_))->coid = ((_coid_) ^ PCI_EVTHDL_COID_XOR)
#define PCI_EVTHDL_DECODE_COID(_evthdl_)			((((_pci_evthdl_t *)&(_evthdl_))->coid) ^ PCI_EVTHDL_COID_XOR)


/*
 * SIGEV_PULSE_INIT always assigned to sival_ptr. This was accommodated in
 * 64bit by defining 2 separate macros, 1 for sival_ptr and 1 for sival_int.
 * We now use the new explicit *_INT_INIT macro however it does not exist in
 * pre 7.0 so in order to keep a common source base, do the mapping
 */
#ifndef SIGEV_PULSE_INT_INIT
#define SIGEV_PULSE_INT_INIT	SIGEV_PULSE_INIT
#endif	/* SIGEV_PULSE_INT_INIT */

#define PCI_SIGEV_PULSE_INT_INIT(_arg1_, _arg2_, _arg3_, _arg4_, _evt_idx_) \
			SIGEV_PULSE_INT_INIT(_arg1_, _arg2_, _arg3_, _arg4_, _evt_idx_)

/*
 ===============================================================================


 ===============================================================================
*/

evt_info_t *event_info_alloc(void);
pci_err_t event_info_free(int event_idx);
evt_info_t *event_info_find(int event_idx);



#endif	/* _PCI_EVT_H_ */

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/lib/pci/private/pci_evt.h $ $Rev: 805556 $")
#endif
