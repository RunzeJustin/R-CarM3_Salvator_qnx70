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
#include <stdlib.h>

#include <pci/pci.h>
#include "hw_lib.h"
#include "private/pci_slog.h"
#include "private/pci_server_mod.h"


/*
 ===============================================================================
 find_parent_bridge()

 This is the wrapper function for server implementation of find_parent_bridge()
 Even though it is only used in this file it can't be static or the declaration
 will conflict with the server implementation prototype

*/
__attribute__ ((visibility ("internal")))
bridge_t *find_parent_bridge(bridge_t *bridge, pci_bdf_t bdf)
{
	static bridge_t *(*find_parent_bridge_p)(bridge_t *bridge, pci_bdf_t bdf) = NULL;

	PCI_SERVER_SYM_LOOKUP(find_parent_bridge_p);
	if (find_parent_bridge_p != NULL) return find_parent_bridge_p(bridge, bdf);
	else return NULL;
}


#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/services/pci/modules/hw/src/_server_func_p.c $ $Rev: 798837 $")
#endif
