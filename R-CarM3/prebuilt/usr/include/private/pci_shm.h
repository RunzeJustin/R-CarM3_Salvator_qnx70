#ifndef _PCI_SHM_H_
#define _PCI_SHM_H_
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
 ===============================================================================
 PCI_SHMSEG_NAME

 The name of the PCI device list shared memory region
*/
#define PCI_SHMSEG_NAME		"/pci_db"		// will be /dev/shm/pci_db

/*
 ===============================================================================
 pci_shm_devinfo_t

 This type is used by the PCI server and libpci in order to manage the known
 device list maintained in shared memory
*/
typedef struct __attribute__((packed,aligned(4)))
{
	pci_bdf_t bdf;
	pci_vid_t vid;
	pci_did_t did;
	pci_ccode_t ccode;
} pci_shm_devinfo_t;

/*
 ===============================================================================
 pci_shm_devlist_funcs_t

 This table contains the access functions for managing the PCI device list in
 shared memory. The access functions are called using the API's below and not
 directly

*/
typedef struct
{
	void (*init)(void);		// internal 'once' function
	const pci_shm_devinfo_t *(*next)(const pci_shm_devinfo_t **dev);
	pci_err_t (*add)(const pci_shm_devinfo_t const *devinfo);
	pci_err_t (*del)(pci_bdf_t bdf);
} pci_shm_devlist_funcs_t;

extern pci_shm_devlist_funcs_t *pci_shm_funcs;

/*
 ===============================================================================
 pci_shm_devlist_next
 pci_shm_devlist_add
 pci_shm_devlist_del

 The following API's are provided for managing the PCI device list in shared
 memory. They are internal API's only

 pci_shm_devlist_next()
 	 This function is called to obtain the first/next entry in the list. When
 	 called with <*dev> == NULL, the first entry is returned. Subsequent entries
 	 are returned by passing in the address of the returned 'pci_shm_devinfo_t'.
 	 When NULL is returned there are no more entries

 pci_shm_devlist_add()
 	 This function is called primarily by the PCI server. It will add a new
 	 entry for <bdf> into the device list (including optional VID, DID and CCODE).
 	 If the entry already exists, PCI_ERR_EEXIST will be returned.
 	 On success PCI_ERR_OK is returned

 pci_shm_devlist_del()
  	 This function is called only by the PCI server. It will delete an existing
  	 entry identified by <bdf> from the device list. If the entry does not
  	 exists, PCI_ERR_ENODEV will be returned. On success PCI_ERR_OK is returned

*/
static inline const pci_shm_devinfo_t *pci_shm_devlist_next(const pci_shm_devinfo_t **dev)
{
	return pci_shm_funcs->next(dev);
}

static inline pci_err_t pci_shm_devlist_add(const pci_bdf_t bdf, const pci_vid_t vid, const pci_did_t did, const pci_ccode_t ccode)
{
	pci_shm_devinfo_t devinfo = {.bdf = bdf, .vid = vid, .did = did, .ccode = ccode};

	assert(bdf != PCI_BDF_NONE);

	/* grab any information not provided */
	if (vid == PCI_VID_ANY) pci_device_read_vid(bdf, &devinfo.vid);
	if (did == PCI_DID_ANY) pci_device_read_did(bdf, &devinfo.did);
	if (ccode == PCI_CCODE_ANY) pci_device_read_ccode(bdf, &devinfo.ccode);

	return pci_shm_funcs->add(&devinfo);
}

static inline pci_err_t pci_shm_devlist_del(pci_bdf_t bdf)
{
	return pci_shm_funcs->del(bdf);
}



#endif	/* _PCI_SHM_H_ */

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/lib/pci/private/pci_shm.h $ $Rev: 798837 $")
#endif
