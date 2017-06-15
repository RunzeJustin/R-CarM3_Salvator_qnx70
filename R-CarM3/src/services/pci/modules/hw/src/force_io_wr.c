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

#include <stdlib.h>

#include "hw_lib.h"

/*
 The 'force_io' variable exists to provide control over HW modules which
 implement ECAM. These modules provide a read only mapping to the configuration
 space of devices in order to protect them as all writes are meant to go through
 the PCI server, which has a read-write mapping to device configuration spaces.

 The exception to this is for certain utilities and capability modules. For
 example, the MSI/MSI-X capability modules have API's to mask and unmask
 interrupts and we want these API's to be callable from ISR's so no message send
 to the server is possible.

 So this variable will bypass the read-only ECAM mappings and use the I/O
 mechanism which does not have this restriction. If a HW module does not
 implement ECAM, this variable does not need to be checked since all
 configuration space accesses will use the I/O mechanism.
 The HW dependent module should check for 'force_io' > 0 and perform the write
 using the I/O mechanism. It is implemented as a counter in order to be thread/MP
 safe within the context of the executing driver/utility.
 If a HW module does not support the I/O mechanism it must return an error for
 any write operation for which 'force_io' is > 0 or map the configuration space
 for non server processes as writeable. This is the option of last resort.

 Ideally, this variable should be thread specific however there is not an efficient
 means that is ISR callable to accomplish that. Currently, the only modules that need
 to use this is the MSI and MSI-X capability modules and only for masking and unmasking
 interrupts. All other writes go through the server and this variable is never set in
 the PCI server

 Note that this variable must remain global because capability modules like
 MSI/MSI-X need access to it

*/

volatile uint_t force_io = 0;

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/services/pci/modules/hw/src/force_io_wr.c $ $Rev: 798837 $")
#endif
