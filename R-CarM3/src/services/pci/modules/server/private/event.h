#ifndef _EVENT_H_
#define _EVENT_H_
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

#include "private/pci_server_xmsg.h"
#include "private/pci_list.h"



/*
 ===============================================================================
 event_t

 This structure contains the event delivery information for the registered
 client
*/
typedef struct
{
	node_t hdr;
	int rcvid;				// used for MsgDeleiverEvent() to 'pid'
	pid_t pid;				// pid of the registering process
	struct sigevent event;	// provided by the registering 'pid'

} event_t;

/*
 ===============================================================================
 event_handler_queue_t

 This structure contains a list of 'event_t' entries for each of the applicable
 event types.
 A process can only register for notification of an event once however multiple
 processes can register for the same event. The 'event_t' for each process
 which registers for a specific event is kept in a linked 'list' per event
 queue.

*/
typedef struct
{
	volatile uint_t event_cnt;	// incremented by ISR to indicate that this event type needs processing
	list_hdr_t list;		// a list of 'event_t'
} event_handler_queue_t;


/*
 ===============================================================================
 event_connect_entry_t

 When an event is registered, an 'event_connect_entry_t' is queued to the
 'connect_entry_t.events' so that the event can be easily released

*/
typedef struct
{
	node_t hdr;
	event_t *event;		// the event_t that was registered
	list_hdr_t *list;	// the event list that the 'event' is on
} event_connect_entry_t;

/*
 ===============================================================================
 event_info_t

 This structure is the main event information structure attached to any device
 (ex. 'bridge_t' or 'bdf_entry_t') that will process events. Exactly one of
 these structures will exist per device interrupt. Since most bridges have only
 a single interrupt (whether pin or message based) there is typically only one
 of these structures per bridge or device.

 The fields of this structure are as follows.

 The 'thread_t' field is the ID of the event handler thread which dispatches the
 sigevents to the registered processes based on what events the ISR has
 determined to have occurred.
 The 'irq' and 'iid' fields are used for attaching the ISR

 The remaining fields are event specific and can be extended as the server event
 handler module is extended. If a particular device does not handle a specific
 event type, then the entry will be NULL. For example, a PCIe root port (logical
 bridge) that does not have a slot implemented will have its 'slot_event_info'
 field set to NULL. It may however still implement the 'pm_event_info' for
 power management events

*/
struct slot_event_info_s;
struct link_event_info_s;
typedef struct
{
	pci_devhdl_t hdl;	// handle to the attached bridge

	struct slot_event_info_s *slot_event_info;	// per slot event queues (if slots implemented)
	struct link_event_info_s *link_event_info;	// event queues related to PCIe link events

	/* extend as required */

} event_info_t;




#endif	/* _EVENT_H_ */

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/services/pci/modules/server/private/event.h $ $Rev: 798837 $")
#endif
