#ifndef _PCI_LIB_H_
#define _PCI_LIB_H_
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


extern int get_pci_server_coid(int *err);
extern const char *pci_getenv(const char * const envvar);

/*
 ===============================================================================
 pci_runtime_flags_e

 This type allows runtime features to be selected and the variable
 'pci_runtime_flags' may contain a bitwise inclusive OR of the following values
*/
typedef enum
{
	pci_runtime_flags_e_SERVER = (1u << 0),				// server
	pci_runtime_flags_e_BKWD_COMPAT_MODULE = (1u << 1),	// backwards compatibility module
	pci_runtime_flags_e_OVERRIDE_WR_MASK = (1u << 2),	// will override per BDF register write masks
														// (see _bdf.c) in the server

} pci_runtime_flags_e;

extern pci_runtime_flags_e pci_runtime_flags;

/*
 ===============================================================================
 envvar_e

 Allow client programs to obtain the name of the HW module in use by the PCI
 server as this is the only mandatory module that is required.
 Note that only the module name and not the full path will be returned
 therefore the client is required to have access via LD_LIBRARY_PATH or some
 other means
*/
typedef enum
{
envvar_e_first = 0,

	envvar_e_HW_MODULE = envvar_e_first,

envvar_e_last = envvar_e_HW_MODULE,
} envvar_e;


/*
 ===============================================================================
 pci_max_bus

 This global variable is set to the highest bus number on which a device was
 found
*/
extern int_t pci_max_bus;

/*
 ===============================================================================
 ilog2

 return the log2 of n. 'n' is expected to be a power of 2
*/
static __inline__ unsigned int ilog2(uint64_t n)
{
	uint_t i = 0;
	while (((uint64_t)1 << i) < n) {
		++i;
	}
	return i;
}

/*
 ===============================================================================
 version_cmp

 like 'strcmp()' returns an integer less than, equal to, or greater than zero
 if v1 is found, respectively, to be less than, to match, or be greater than v2

*/
static __inline__ int_t version_cmp(const pci_version_t v1, const pci_version_t v2)
{
	const uint32_t ver1 = (v1.major << 16) | v1.minor;
	const uint32_t ver2 = (v2.major << 16) | v2.minor;

	return ver1 - ver2;
}

/*
 ===============================================================================
 PCI_DEVHDL_* macros

 The 'pci_devhdl_t' is untyped to driver software however encoded within it is
 	 1. the 'pci_bdf_t' for the device
 	 2. the connection id to the PCI server
 	 3. an attachment id which uniquely identifies the attach

 The following macros will encode and decode the various bits of information
 into/from a 'pci_devhdl_t'

 IMPORTANT:

 Use these macros on a zeroed pci_devhdl_t so that the BDF ARI bit is encoded
 properly

*/
#if __PTR_BITS__ == 64
typedef struct __attribute__((packed, aligned(8)))
{
	uint8_t coid;
	uint8_t attach_id;
	uint16_t bdf;
	uint32_t spare;
} _pci_devhdl_t;
#else	/* __PTR_BITS__ */
typedef struct __attribute__((packed, aligned(4)))
{
	uint8_t coid;
	uint8_t attach_id;
	uint16_t bdf;
} _pci_devhdl_t;
#endif	/* __PTR_BITS__ */


#if 1	// make this 0 and clean compile everything if you want to debug using unscrambled devhdl's

#define PCI_DEVHDL_COID_XOR				(uint8_t)'Q'
#define PCI_DEVHDL_ATTACH_ID_XOR		(uint8_t)'S'
#define PCI_DEVHDL_BDF_XOR				(((uint16_t)'S' << 8) | ((uint16_t)'L'))

#else

#define PCI_DEVHDL_COID_XOR				0
#define PCI_DEVHDL_ATTACH_ID_XOR		0
#define PCI_DEVHDL_BDF_XOR				0

#endif

#define PCI_DEVHDL_ARI_BIT				7		// bit 7 of the attach_id
#define PCI_DEVHDL_ATTACH_ID_MASK		((1u << PCI_DEVHDL_ARI_BIT) - 1)


#define PCI_DEVHDL_ENCODE_BDF(_devhdl_, _bdf_) \
		do { \
			((_pci_devhdl_t *)&(_devhdl_))->bdf = (((_bdf_) & 0xFFFFu) ^ PCI_DEVHDL_BDF_XOR); \
			if (PCI_IS_ARI(_bdf_)) ((_pci_devhdl_t *)&(_devhdl_))->attach_id |=  (1u << PCI_DEVHDL_ARI_BIT); \
		} while(0)

#define PCI_DEVHDL_DECODE_BDF(_devhdl_) \
		((pci_bdf_t) \
			((((((_pci_devhdl_t *)&(_devhdl_))->bdf) ^ PCI_DEVHDL_BDF_XOR) & 0xFFFFu) | \
			(((((_pci_devhdl_t *)&(_devhdl_))->attach_id) & (1u << PCI_DEVHDL_ARI_BIT)) ? (1u << _PCI_ARI_BIT) : 0)) \
		)

#define PCI_DEVHDL_ENCODE_ATTACH_ID(_devhdl_, _id_) \
		((_pci_devhdl_t *)&(_devhdl_))->attach_id |= (((_id_) ^ PCI_DEVHDL_ATTACH_ID_XOR) & PCI_DEVHDL_ATTACH_ID_MASK)

#define PCI_DEVHDL_DECODE_ATTACH_ID(_devhdl_) \
		(((((_pci_devhdl_t *)&(_devhdl_))->attach_id) & PCI_DEVHDL_ATTACH_ID_MASK) ^ PCI_DEVHDL_ATTACH_ID_XOR)

#define PCI_DEVHDL_ENCODE_COID(_devhdl_, _coid_)	((_pci_devhdl_t *)&(_devhdl_))->coid = ((_coid_) ^ PCI_DEVHDL_COID_XOR)
#define PCI_DEVHDL_DECODE_COID(_devhdl_)			((((_pci_devhdl_t *)&(_devhdl_))->coid) ^ PCI_DEVHDL_COID_XOR)





/*
 ===============================================================================
 PCI_BDF_RSVD_MASK

 macro to define the reserved 'pci_bdf_t' bits
*/
#define PCI_BDF_RSVD_MASK				((pci_bdf_t)~((1u << _PCI_ARI_BIT) - 1))

/*
 ===============================================================================
 PCI_BD

 this macro is used to convert a BDF to a BD (ie. set function to 0). This is
 often required when we need to get information from function 0 but need to
 otherwise preserve the BDF

*/
#define PCI_BD(_bdf_) \
		(PCI_IS_ARI(_bdf_) ? PCI_BDF_ARI(PCI_BUS(_bdf_), 0) : PCI_BDF(PCI_BUS(_bdf_), PCI_DEV(_bdf_), 0))

/*
 ===============================================================================
 pci_attachFlags_e_BKWD_COMPAT

 This flag is internal use only

 The new PCI server does not allow attachments that are not the owner to
 retrieve the base address information however the old 'pci' server uses the
 PCI_SHARE flag and still expects to get the base addresses. We'll steal the
 MSb of the 'pci_attachFlags_t' type and use it to allow the bases address
 information to be retrieved if its set.
*/
#define pci_attachFlags_e_BKWD_COMPAT	((pci_attachFlags_t)(1u << ((sizeof(pci_attachFlags_t) * 8) - 1)))

/*
 ===============================================================================
 pci_attachFlags_e_UTIL

 This flag is internal use only

 The new PCI server does not allow attachments that are not the owner to
 retrieve the base address and other sensitive information however this
 information is desirable for utilities for display, debug and control purposes.

*/
#define pci_attachFlags_e_UTIL	((pci_attachFlags_t)(1u << ((sizeof(pci_attachFlags_t) * 8) - 2)))




#endif	/* _PCI_LIB_H_ */

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/lib/pci/private/pci_lib.h $ $Rev: 808442 $")
#endif
