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
#include <pthread.h>
#include <stdlib.h>
#include <sys/rsrcdbmgr.h>

#include <pci/pci.h>
#include "private/hwmod_api.h"
#include "hw_lib.h"
#include "private/pci_slog.h"
#include "private/pci_debug.h"

/*
 ===============================================================================
 rsrcdb_irq_free

 This function will release back to the rsrcdb, <nirq> vectors pointed to by
 <irq_list>.

 Only message based interrupts need to be unreserved
*/
__attribute__ ((visibility ("internal")))
pci_err_t rsrcdb_irq_free(_pci_irqType_e irq_type, uint_t nirq, pci_irq_t *irq_list)
{
	pci_err_t r = PCI_ERR_OK;

	if (irq_type == _pci_irqType_e_MSG)
	{
		uint_t i;
		uint_t first_vector = UINT_MAX;
		uint_t last_vector = 0;
		rsrc_request_t req = {0};

		/* find the first and last vectors */
		for (i=0; i<nirq; i++)
		{
			if (irq_list[i] < first_vector) first_vector = irq_list[i];
			if (irq_list[i] > last_vector) last_vector = irq_list[i];
		}

		/* release them all */
		req.length = nirq;
		req.flags = RSRCDBMGR_IRQ | RSRCDBMGR_FLAG_RANGE;
		req.start = first_vector;
		req.end = last_vector;
		req.name = PCI_RSRC_NAME_MSGVEC;
		if (req.name != NULL) req.flags |= RSRCDBMGR_FLAG_NAME;

		/* unreserve the vector(s) */
		if (rsrcdbmgr_detach(&req, 1) == -1) r = PCI_ERR_IRQ_CFG;
	}
	return r;
}

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/services/pci/modules/hw/src/rsrcdb/rsrcdb_irq_free.c $ $Rev: 798837 $")
#endif
