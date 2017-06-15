#ifndef _PCI_SERVER_XMSG_H_
#define _PCI_SERVER_XMSG_H_
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

#include <sys/resmgr.h>
#include <sys/iofunc.h>

#include <pci/pci.h>
#include "private/pci_server_msg.h"

/* prototypes */


/*
 ===============================================================================
 server extended message types
*/
typedef enum
{
pci_xmsg_e_first = pci_msg_e_last,

	/* the following are handled by the server hotplug_sim module */
	pci_xmsg_e_DEVICE_INSERT = pci_xmsg_e_first,
	pci_xmsg_e_DEVICE_REMOVE,

	pci_xmsg_e_resv1,
	pci_xmsg_e_resv2,
	pci_xmsg_e_resv3,
	pci_xmsg_e_resv4,
	pci_xmsg_e_resv5,
	pci_xmsg_e_resv6,
	pci_xmsg_e_resv7,
	pci_xmsg_e_resv8,
	pci_xmsg_e_resv9,
	pci_xmsg_e_resv10,
	pci_xmsg_e_resv11,
	pci_xmsg_e_resv12,
	pci_xmsg_e_resv13,
	pci_xmsg_e_resv14,
	pci_xmsg_e_resv15,
	pci_xmsg_e_resv16,

	/* event messages */
pci_xmsg_e_event_first,

	pci_xmsg_e_EVENT_DEVICE_INSERT = pci_xmsg_e_event_first,
	pci_xmsg_e_EVENT_DEVICE_REMOVE,
	pci_xmsg_e_EVENT_PCIe_LINK_STATE,
	pci_xmsg_e_EVENT_PCIe_LINK_BW_CHANGE,

pci_xmsg_e_event_last = pci_xmsg_e_EVENT_PCIe_LINK_BW_CHANGE,


/* the following are handled by the server debug module */
	pci_xmsg_e_DUMP_BDF,

pci_xmsg_e_last,

} pci_xmsg_e;


/*
 -------------------------------------------------------------------------------
 pci_xmsg_e_DEVICE_INSERT message structure
*/
typedef struct
{
	pci_msg_hdr_t hdr;
	union
	{
		struct
		{
			pci_bdf_t bdf;
		} in;
		struct
		{
		} out;
	} io;
} pci_xmsg_dev_insert_t;

/*
 -------------------------------------------------------------------------------
 pci_xmsg_e_DEVICE_REMOVE message structure
*/
typedef struct
{
	pci_msg_hdr_t hdr;
	union
	{
		struct
		{
			pci_bdf_t bdf;
		} in;
		struct
		{
		} out;
	} io;
} pci_xmsg_dev_remove_t;

/*
 -------------------------------------------------------------------------------
 pci_xmsg_e_DUMP_BDF message structure
*/
typedef struct
{
	pci_msg_hdr_t hdr;
	union
	{
		struct
		{
			pci_bdf_t bdf;
		} in;
		struct
		{
		} out;
	} io;
} pci_xmsg_dump_bdf_t;

/*
 -------------------------------------------------------------------------------
 pci_xmsg_e_EVENT_DEVICE_INSERT message structure
*/
typedef struct
{
	pci_msg_hdr_t hdr;
	struct sigevent event;
	union
	{
		struct
		{
			pci_cs_t cs;
			bool_t unregister;	// otherwise register for the event notification
		} in;
		struct
		{
		} out;
	} io;
} pci_xmsg_evt_dev_insert_t;

/*
 -------------------------------------------------------------------------------
 pci_xmsg_e_EVENT_DEVICE_REMOVE message structure
*/
typedef struct
{
	pci_msg_hdr_t hdr;
	struct sigevent event;
	union
	{
		struct
		{
			pci_cs_t cs;
			bool_t unregister;	// otherwise register for the event notification
		} in;
		struct
		{
		} out;
	} io;
} pci_xmsg_evt_dev_remove_t;

/*
 -------------------------------------------------------------------------------
 pci_xmsg_e_EVENT_PCIe_LINK_STATE and pci_xmsg_e_EVENT_PCIe_LINK_BW_CHANGE
 message structure
*/
typedef struct
{
	pci_msg_hdr_t hdr;
	struct sigevent event;
	union
	{
		struct
		{
			pci_bdf_t bdf;
			bool_t unregister;	// otherwise register for the event notification
		} in;
		struct
		{
		} out;
	} io;
} pci_xmsg_evt_link_state_t;

typedef pci_xmsg_evt_link_state_t	pci_xmsg_evt_link_bw_change_t;

/*
 -------------------------------------------------------------------------------
 union for all PCI server extended message types
*/

typedef union
{
	pci_msg_hdr_t			hdr;
	pci_xmsg_dev_insert_t	device_insert;
	pci_xmsg_dev_remove_t	device_remove;

	pci_xmsg_dump_bdf_t		dump_bdf;

	pci_xmsg_evt_dev_insert_t	event_device_insert;
	pci_xmsg_evt_dev_remove_t	event_device_remove;
	pci_xmsg_evt_link_state_t	event_link_state;
	pci_xmsg_evt_link_bw_change_t	event_link_bw_change;

} pci_xmsg_t;



#endif	/* _PCI_SERVER_XMSG_H_ */

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/services/pci/modules/server/private/pci_server_xmsg.h $ $Rev: 811604 $")
#endif
