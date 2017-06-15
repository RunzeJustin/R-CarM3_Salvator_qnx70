#ifndef _PCI_SERVER_MSG_H_
#define _PCI_SERVER_MSG_H_
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

#include <sys/iofunc.h>
#include <pci/pci.h>

#include "private/cap_priv.h"
#include "private/pci_cap.h"
#include "private/pci_lib.h"

/*
 ===============================================================================
 server message types
*/
typedef enum
{
pci_msg_e_first = 20,	// this must be higher than IOM_PCI_IRQ_ROUTING_OPTIONS of the old PCI server

	pci_msg_e_DEVICE_ATTACH = pci_msg_e_first,
	pci_msg_e_DEVICE_DETACH,
	pci_msg_e_CFG_WRITE,
	pci_msg_e_READ_IRQ,
	pci_msg_e_READ_BA,
	pci_msg_e_CAP_ENABLE,
	pci_msg_e_CAP_DISABLE,
	pci_msg_e_CS_DEVICE,	// which device resides in the given chasssis/slot
	pci_msg_e_DEVICE_CS,	// which chassis/slot does the given device reside in
	pci_msg_e_DEVICE_RECONFIG,
	pci_msg_e_DEVICE_RESET,
	pci_msg_e_ENVVAR,

	pci_msg_e_resv2,
	pci_msg_e_resv3,
	pci_msg_e_resv4,
	pci_msg_e_resv5,
	pci_msg_e_resv6,
	pci_msg_e_resv7,
	pci_msg_e_resv8,

pci_msg_e_last,

} pci_msg_e;

/*
 -------------------------------------------------------------------------------
 common message header
*/
typedef struct
{
	io_msg_t io_msg;
	struct pci_msg_reply_hdr_s
	{
		pci_err_t status;
		uint_t length;
	} rep_hdr;
} pci_msg_hdr_t;

/*
 -------------------------------------------------------------------------------
 pci_msg_e_DEVICE_ATTACH message structure
*/
typedef struct
{
	pci_msg_hdr_t hdr;
	union
	{
		struct
		{
			pci_attachFlags_t flags;
			pci_bdf_t bdf;
		} in;
		struct
		{
			pci_devhdl_t hdl;
		} out;
	} io;
} pci_msg_dev_attach_t;

/*
 -------------------------------------------------------------------------------
 pci_msg_e_DEVICE_DETACH message structure
*/
typedef struct
{
	pci_msg_hdr_t hdr;
	union
	{
		struct
		{
			pci_devhdl_t hdl;
		} in;
		struct
		{
		} out;
	} io;
} pci_msg_dev_detach_t;

/*
 -------------------------------------------------------------------------------
 pci_msg_e_CFG_WRITE message structure
 
 A note about buf[0] __attribute__((aligned(8)))
 
 When data is passed to the PCI server for write, the bytes to be written are
 provided in a separate iov_t. The iov_t's are received as a contiguous area of
 memory however in order that buf[0] can actually represent the start of the iov_t
 which contains the bytes to be written, we have to make sure that buf[0] sits at
 the end of the 'pci_msg_cfg_wr_t' structure (ie. we have to use up any structure
 padding). Aligning to the largest object within the structure accomplishes this
*/
typedef struct
{
	pci_msg_hdr_t hdr;
	union
	{
		struct {
			pci_devhdl_t hdl;
			uint16_t offset;
			uint16_t size;
			uint8_t buf[0] __attribute__((aligned(8)));
		} in;
	} io;
} pci_msg_cfg_wr_t;

/*
 -------------------------------------------------------------------------------
 pci_msg_e_READ_BA message structure
*/
typedef struct
{
	pci_msg_hdr_t hdr;
	union
	{
		struct
		{
			pci_devhdl_t hdl;
		} in;
		struct
		{
			pci_ba_t ba[7];		// max of 6 BAR's and 1 Expansion ROM so max of 7 'pci_ba_t' structures
		} out;
	} io;
} pci_msg_rd_ba_t;

/*
 -------------------------------------------------------------------------------
 pci_msg_e_READ_IRQ message structure

 See 'pci_msg_cfg_wr_t' for rationale for
 pci_irq_t irq[0] __attribute__((aligned(8)));

*/
typedef struct
{
	pci_msg_hdr_t hdr;
	union
	{
		struct
		{
			pci_devhdl_t hdl;
			int_t nirq;
		} in;
		struct
		{
			int_t nirq;
			pci_irq_t irq[0] __attribute__((aligned(8)));		// 'nirq' IRQ's
		} out;
	} io;
} pci_msg_rd_irq_t;

/*
 -------------------------------------------------------------------------------
 pci_msg_e_CAP_ENABLE message structure
*/
typedef struct
{
	pci_msg_hdr_t hdr;
	union
	{
		struct
		{
			cap_mod_params_t cap_mod_params;
			pci_cap_priv_t cap;
		} in;
		struct
		{
			pci_cap_priv_t cap;
		} out;
	} io;
} pci_msg_cap_enable_t;

/*
 -------------------------------------------------------------------------------
 pci_msg_e_CAP_DISABLE message structure
*/
typedef struct
{
	pci_msg_hdr_t hdr;
	union
	{
		struct
		{
			cap_mod_params_t cap_mod_params;
			_capid_t capid;
		} in;
		struct
		{
		} out;
	} io;
} pci_msg_cap_disable_t;

/*
 -------------------------------------------------------------------------------
 pci_msg_e_CS_DEVICE message structure
*/
typedef struct
{
	pci_msg_hdr_t hdr;
	union
	{
		struct
		{
			pci_cs_t cs;
		} in;
		struct
		{
			pci_bdf_t bdf;
		} out;
	} io;
} pci_msg_cs_dev_t;

/*
 -------------------------------------------------------------------------------
 pci_msg_e_DEVICE_CS message structure
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
			pci_cs_t cs;
		} out;
	} io;
} pci_msg_dev_cs_t;

/*
 -------------------------------------------------------------------------------
 pci_msg_e_DEVICE_RECONFIG message structure
*/
typedef struct
{
	pci_msg_hdr_t hdr;
	union
	{
		struct
		{
			pci_devhdl_t hdl;
		} in;
		struct
		{
		} out;
	} io;
} pci_msg_dev_reconfig_t;

/*
 -------------------------------------------------------------------------------
 pci_msg_e_DEVICE_RESET message structure
*/
typedef struct
{
	pci_msg_hdr_t hdr;
	union
	{
		struct
		{
			pci_devhdl_t hdl;
			pci_resetType_e reset_type;
			/*
			 * if the reset type is pci_resetType_e_FUNCTION, the caller (via
			 * the pci_device_reset() implementation) is going to verify whether
			 * or not FLR is supported. If so, it will provide the capability
			 * index in the message so that the server can just read the capability
			 * without having to search for it. An index value of -1 indicates the
			 * capability is not supported by the device
			 */
			int_t pcie_capid_idx;
			int_t af_capid_idx;
		} in;
		struct
		{
		} out;
	} io;
} pci_msg_dev_reset_t;

/*
 -------------------------------------------------------------------------------
 pci_msg_e_ENVVAR message structure
*/

typedef struct
{
	pci_msg_hdr_t hdr;
	union
	{
		struct
		{
			uint_t len;		/* size of the envvar return buffer */
			envvar_e envvar;	/* the envvar to return */
		} in;
		struct
		{
			uint_t len;		/* the actual length of the envvar */
			char envvar_val[0] __attribute__((aligned(4)));	/* the envvar string */
		} out;
	} io;
} pci_msg_envvar_t;

/*
 -------------------------------------------------------------------------------
 union for all message types
*/

typedef union
{
	pci_msg_hdr_t			hdr;
	pci_msg_dev_attach_t	dev_attach;
	pci_msg_dev_detach_t	dev_detach;
	pci_msg_cfg_wr_t		cfg_wr;
	pci_msg_rd_irq_t		rd_irq;
	pci_msg_rd_ba_t			rd_ba;
	pci_msg_cap_enable_t	cap_enable;
	pci_msg_cap_disable_t	cap_disable;
	pci_msg_cs_dev_t		cs_dev;
	pci_msg_dev_cs_t		dev_cs;
	pci_msg_dev_reconfig_t	dev_reconfig;
	pci_msg_dev_reset_t		dev_reset;
	pci_msg_envvar_t		envvar;
} pci_msg_t;




#endif	/* _PCI_SERVER_MSG_H_ */

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/services/pci/server/private/pci_server_msg.h $ $Rev: 808445 $")
#endif
