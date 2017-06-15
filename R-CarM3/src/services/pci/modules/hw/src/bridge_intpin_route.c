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
 bridge_intpin_route

 Trace back through the bridge hierarchy starting with <device_bdf> until we
 either get to <root_bdf> or a root bridge (a bridge with no parent). Starting
 with <device_bdf>, and at each bridge, we use the interrupt mapping documented
 in Table 9-1, pg 118 of the "PCI-to-PCI Bridge Architecture Specification,
 Revision 1.2, June 9, 2003".

 A <root_bdf> value of PCI_BDF_NONE will trace back to the root port of
 <device_bdf>. The <root_bdf> bridge, if within the hierarchy of <device_bdf>,
 is the last bridge considered in the routing algorithm

 <_intpin_c> must be a value of ascii 'A' thru 'D' inclusive. The same will be
 returned

*/
__attribute__ ((visibility ("internal")))
char bridge_intpin_route(const pci_bdf_t root_bdf, const pci_bdf_t device_bdf, const char intpin_c)
{
	slog_info(2, "%s(): Determine B%u:D%u:F%u INTPIN ...", __FUNCTION__,
					PCI_BUS(device_bdf), PCI_DEV(device_bdf), PCI_FUNC(device_bdf));

	static const struct {char pin[4];} irt_map[] =
	{
		[0] = {.pin = {[0] = 'A', [1] = 'B', [2] = 'C', [3] = 'D'}},
		[1] = {.pin = {[0] = 'B', [1] = 'C', [2] = 'D', [3] = 'A'}},
		[2] = {.pin = {[0] = 'C', [1] = 'D', [2] = 'A', [3] = 'B'}},
		[3] = {.pin = {[0] = 'D', [1] = 'A', [2] = 'B', [3] = 'C'}},
	};

	/* process <device_bdf> */
	char _intpin_c = irt_map[PCI_DEV(device_bdf) % 4].pin[intpin_c - 'A'];

	slog_info(2, "%s(): B%u:D%u:F%u map INTPIN %c --> INTPIN %c", __FUNCTION__,
				PCI_BUS(device_bdf), PCI_DEV(device_bdf), PCI_FUNC(device_bdf),
				intpin_c, _intpin_c);

	/* now process the bridges above <device_bdf> */
	bridge_t *bridge = find_parent_bridge(NULL, device_bdf);
	pci_bdf_t terminating_bdf = device_bdf;

	while (bridge != NULL)
	{
		/* for the bridges, we need to consider the Function # as well as the device # */
		const uint_t dev = PCI_DEV(bridge->bdf) + PCI_FUNC(bridge->bdf);

		slog_info(2, "%s(): B%u:D%u:F%u map INTPIN %c --> INTPIN %c", __FUNCTION__,
					PCI_BUS(bridge->bdf), PCI_DEV(bridge->bdf), PCI_FUNC(bridge->bdf), _intpin_c,
					irt_map[dev % 4].pin[_intpin_c - 'A']);

		_intpin_c = irt_map[dev % 4].pin[_intpin_c - 'A'];

		/* grab this for log in case we reached the root */
		terminating_bdf = bridge->bdf;

		if (root_bdf == terminating_bdf) break;
		else bridge = bridge->parent;
	}
	assert((_intpin_c >= 'A') && (_intpin_c <= 'D'));

	slog_info(2, "%s(): terminated at B%u:D%u:F%u, B%u:D%u:F%u INTPIN %c --> %c", __FUNCTION__,
					PCI_BUS(terminating_bdf), PCI_DEV(terminating_bdf), PCI_FUNC(terminating_bdf),
					PCI_BUS(device_bdf), PCI_DEV(device_bdf), PCI_FUNC(device_bdf), intpin_c, _intpin_c);

	return _intpin_c;
}


#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/services/pci/modules/hw/src/bridge_intpin_route.c $ $Rev: 798837 $")
#endif
