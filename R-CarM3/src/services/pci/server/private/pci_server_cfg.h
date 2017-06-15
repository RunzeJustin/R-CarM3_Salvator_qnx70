#ifndef _PCI_SERVER_CFG_H_
#define _PCI_SERVER_CFG_H_
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

/*
 @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

 This file contains types and macros related to PCI server configuration

 @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
*/

typedef struct
{
	uint64_t size;
	uint64_t alignment;
} aspace_reservation_info_t;

/*
 ===============================================================================
 server_cfg_t

 This data structure contains all of the runtime configuration parameters that
 can be controlled using a combination of command line parameters and the
 PCI server configuration file (specified with the --config= command line
 option)

*/

typedef struct
{
	const char *namespace_node_name;
	bool_t do_bus_config;		// whether or not the PCI bus bridges and devices will be configured
	bool_t do_link_retraining;	// whether or not PCIe ports will have their links retrained prior to access during enumeration
	uint_t bus_scan_limit;		// highest bus that will be scanned
	/*
	 * in recent UEFI based x86 platforms, we have noticed that even though memory
	 * and/or I/O address spaces are assigned to a device that in some circumstances
	 * the address spaces are not enabled in the CMD register. Although there are other
	 * ways of working around this issue, a command line option (--aspace-enable) has been
	 * added to clean this up. The option can be specified without arguments, with the
	 * keyword "all" (causing all assigned address spaces to be enabled) or with "mem" or "io"
	 * which will only enable the address space specified. Because the option is applied to
	 * all devices, it's not known how useful specifying an individual address space type
	 * will be but nonetheless it is handled. As for the value of this field, it will
	 * contain either pci_asType_e_MEM, pci_asType_e_IO or (pci_asType_e_MEM + pci_asType_e_IO).
	 * This field can also be established with the server config file parameter ASPACE_ENABLE
	 */
	pci_asType_e aspace_enable;
	struct
	{
		bool_t multithreaded;
		uint_t lo_water;
		uint_t hi_water;
		uint_t increment;
		uint_t max;
	} thread_info;
	struct server_module_s
	{
		uint_t nentries;	// number of entries in the 'list' array
		struct server_module_list_s
		{
			const char *name;	// null terminated server module name to load
			char *optstr;		// null terminated (un-tokenized) option string
		} *list;			// array of module name/option string pairs
	} module;
	struct aspace_reservation_s
	{
		uint_t nentries;	// number of entries in the 'list' array
		struct aspace_reservation_list_s
		{
			pci_cs_t cs;		// chassis/slot
			bool_t reserved;	// if not reserved, shared
			aspace_reservation_info_t io;
			aspace_reservation_info_t mem;
			aspace_reservation_info_t pfmem;
		} *list;
	} aspace_reservation;
	struct busnum_reservation_s
	{
		uint_t nentries;	// number of entries in the 'list' array
		struct busnum_reservation_list_s
		{
			pci_cs_t cs;		// chassis/slot
			uint_t num;
		} *list;
	} busnum_reservation;
} server_cfg_t;

/*
 ===============================================================================
 server function prototypes
*/

void parse_config_file(const char const *fname, server_cfg_t *cfg);




#endif	/* _PCI_SERVER_CFG_H_ */

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/services/pci/server/private/pci_server_cfg.h $ $Rev: 811129 $")
#endif
