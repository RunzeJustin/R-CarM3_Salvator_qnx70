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


/*
 ===============================================================================
 rsrcdb_irq_resv

 This function will reserve <nirq> of the system interrupt vectors and return
 the results in the caller provided <irq_list>.

 If pci_asAttr_e_CONTIG is specified in <irq_attr> then this function will
 ensure that <nirq> vectors were allocated contiguously

 The caller can have some control over the resource database allocation by
 setting <req_flags> to one of the defined RSRCDBMGR_FLAG_* flags. The behaviour
 for each flag will be as follows

 1.  RSRCDBMGR_FLAG_RANGE

 	 When the reservation is performed, RSRCDBMGR_FLAG_RANGE is always set
 	 however if the caller of this function sets this flag it instructs this
 	 routine to use the first 2 entries in <irq_list> as the 'start' and 'end'
 	 values respectively in the search. It is the callers responsibility to pass
 	 sensible values for these 2 parameters and 2 values are required even if
 	 <nirq> is 1. The 'end' value (ie. irq_list[1]) can be -1 if the caller only
 	 wants to specify the 'start' value to search from

*/
__attribute__ ((visibility ("internal")))
pci_err_t rsrcdb_irq_resv(_pci_irqType_e irq_type, _pci_irqAttr_e irq_attr, uint_t nirq, pci_irq_t *irq_list, uint_t req_flags)
{
	pci_err_t r = PCI_ERR_EINVAL;

	switch(irq_type)
	{
		case _pci_irqType_e_MSG:
		{
			rsrc_request_t req = {0};

			req.flags = req_flags;

			req.length = nirq;
			req.flags |= (RSRCDBMGR_IRQ | RSRCDBMGR_FLAG_RANGE);
			if ((req_flags & RSRCDBMGR_FLAG_RANGE) != 0)
			{
				req.start = irq_list[0];
				req.end = irq_list[1];
			}
			else
			{
				req.start = 0;
				req.end = -1;
			}
			req.name = PCI_RSRC_NAME_MSGVEC;
			if (req.name != NULL) req.flags |= RSRCDBMGR_FLAG_NAME;
			if (irq_attr & _pci_irqAttr_e_CONTIG)
			{
				req.align = nirq;
				req.flags |= RSRCDBMGR_FLAG_ALIGN;
			}

			/* reserve the vector(s) */
			if (rsrcdbmgr_attach(&req, 1) == -1) r = PCI_ERR_IRQ_NOT_AVAIL;
			else
			{
				uint_t i;

				r = PCI_ERR_OK;
				for (i=0; i<nirq; i++)
				{
					if ((irq_attr & pci_asAttr_e_CONTIG) && (i > 0) && (abs(irq_list[i] - irq_list[i - 1]) > 1))
					{
						slog_error(0, "%s() request for contiguous interrupt vectors could not be satisfied", __FUNCTION__);
						r = PCI_ERR_IRQ_NOT_AVAIL;
						break;
					}
					irq_list[i] = req.start + i;
				}
			}
			break;
		}
		default:
			break;
	}
	slog_debug(3, "%s(%d, 0x%x, %u, %p->[0]=%d,...) returned %s", __FUNCTION__,
				irq_type, irq_attr, nirq, irq_list, irq_list[0], pci_strerror(r));
	return r;
}

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/services/pci/modules/hw/src/rsrcdb/rsrcdb_irq_resv.c $ $Rev: 798837 $")
#endif
