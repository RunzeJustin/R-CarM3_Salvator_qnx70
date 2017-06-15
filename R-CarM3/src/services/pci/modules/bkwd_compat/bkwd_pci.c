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
#include <stdint.h>
#include <string.h>
#include <sys/resmgr.h>
#include <sys/iofunc.h>
#include <assert.h>

#include <pci/pci.h>
#include <pci/cap_msi.h>

#include "bkwd_pci.h"
#include "private/pci_lib.h"
#include "private/pci_hw.h"
#include "bkwdmod_lib.h"
#include "private/pci_slog.h"
#include "private/pci_debug.h"

#undef	NDEBUG
#ifndef NDEBUG
#define DUMP_DEV_INFO(di)	dump_dev_info((di))
static void dump_dev_info(struct pci_dev_info *dinfo);
#else	/* NDEBUG */
#define DUMP_DEV_INFO(di)
#endif	/* NDEBUG */


/*
 * this flag is a new old PCI server API flag to direct the attach to configure
 * all available MSI's instead of just 1. Only code which has been explicily
 * made aware of this new flag will ever be able to set it. This is currently
 * being done as CE work for a customer
 */
#ifndef PCI_INIT_MSI_ALL
#define PCI_INIT_MSI_ALL	0x8000000
#endif

/*
 ===============================================================================
 config_msi

 The old PCI server will use MSI interrupts if the device supports them
 regardless of whether or not the driver wants to use them so lets mimic this
 if we can

 Return PCI_MSI if successful, otherwise 0
*/
static int_t config_msi(pci_devhdl_t hdl)
{
	pci_bdf_t bdf = PCI_DEVHDL_DECODE_BDF(hdl);
	int idx = pci_device_find_capid(bdf, CAPID_MSI);
	int msi = 0;

	if (idx >= 0)
	{
		pci_cap_t cap = NULL;
		pci_err_t r = pci_device_read_cap(bdf, &cap, idx);

		if (r != PCI_ERR_OK) msi = 0;
		else if ((r = cap_msi_set_nirq(hdl, cap, 1)) != PCI_ERR_OK) msi = 0;
		else if ((r = pci_device_cfg_cap_enable(hdl, pci_reqType_e_MANDATORY, cap)) != PCI_ERR_OK) msi = 0;
		else msi = PCI_MSI;
		slog_debug(0, "%s(), %s", __FUNCTION__, pci_strerror(r));
	}
	slog_info(0, "Request for use of MSI was %s", (msi == PCI_MSI) ? "successful" : "unsuccessful");
	return msi;
}
/*
 ===============================================================================
 config_all_msi

 The old PCI server will use MSI interrupts if the device supports them
 regardless of whether or not the driver wants to use them so lets mimic this
 if we can.
 This particular function will enable all device supported MSI's. Only the
 first is returned and the caller of the attach which set the PCI_INIT_MSI_ALL
 flag needs to know this

 Return PCI_MSI if successful, otherwise 0
*/
static int_t config_all_msi(pci_devhdl_t hdl, int_t *nirq)
{
	pci_bdf_t bdf = PCI_DEVHDL_DECODE_BDF(hdl);
	int idx = pci_device_find_capid(bdf, CAPID_MSI);
	int msi = 0;

	if (idx >= 0)
	{
		pci_cap_t cap = NULL;
		pci_err_t r = pci_device_read_cap(bdf, &cap, idx);
		uint_t num_msi = 0;

		if (r != PCI_ERR_OK) msi = 0;
		else if ((num_msi = cap_msi_get_nirq(cap)) == 0) msi = 0;
		else if ((r = cap_msi_set_nirq(hdl, cap, num_msi)) != PCI_ERR_OK) msi = 0;
		else
		{
			*nirq = num_msi;
			msi = PCI_MSI;
		}
		if (!pci_device_cfg_cap_isenabled(hdl, cap))
		{
			r = pci_device_cfg_cap_enable(hdl, pci_reqType_e_MANDATORY, cap);
			if (r != PCI_ERR_OK) msi = 0;
		}
		slog_debug(0, "%s(), %s", __FUNCTION__, pci_strerror(r));
	}
	slog_info(0, "Request for use of %u MSI's was %s", *nirq, (msi == PCI_MSI) ? "successful" : "unsuccessful");
	return msi;
}

static inline void set_pci_max_bus(void)
{
	uint_t idx = 0;
	/* trigger an initial scan in case pci_device_find() has not been called yet */
	while (pci_device_find(idx++, PCI_VID_ANY, PCI_DID_ANY, PCI_CCODE_ANY) != PCI_BDF_NONE);

	slog_info(0, "pci_max_bus = %d", pci_max_bus);
}

/*
 ===============================================================================
 pci_bkwd_attach_device

*/
__attribute__ ((visibility ("internal")))
pci_devhdl_t pci_bkwd_attach_device(uint32_t flags, uint_t idx, void *bufptr)
{
	struct pci_dev_info *dinfo = (struct pci_dev_info *)bufptr;
	pci_bdf_t bdf;
	pci_bdf_t search_bdf = PCI_BDF_NONE;
	pci_vid_t vid = PCI_VID_ANY;
	pci_did_t did = PCI_DID_ANY;
	pci_ccode_t ccode = PCI_CCODE_ANY;
	uint_t search_idx = idx;
	pci_devhdl_t hdl = NULL;

	/*
	 * check for the new PCI_INIT_MSI_ALL flag and if set, record that fact and
	 * then strip it so it can't cause problems
	 */
	const bool_t msi_init_all = (flags & PCI_INIT_MSI_ALL) != 0;
	if (msi_init_all) flags &= ~PCI_INIT_MSI_ALL;

	if (flags & PCI_SEARCH_VEND) vid = dinfo->VendorId;
	else if (flags & PCI_SEARCH_CLASS)
	{
		ccode = PCI_CCODE(PCI_CLASS(dinfo->Class), PCI_SUBCLASS(dinfo->Class), PCI_INTERFACE(dinfo->Class));
		slog_debug(0, "%s(0x%x, %u, %p) using classcode %x", __FUNCTION__, flags, idx, bufptr, ccode);
	}
	else if (flags & PCI_SEARCH_BUSDEV)
	{
		search_bdf = PCI_BDF(dinfo->BusNumber, PCI_DEVNO(dinfo->DevFunc), PCI_FUNCNO(dinfo->DevFunc));
		slog_debug(0, "%s(0x%x, %u, %p) using B%u:D%u:F%u", __FUNCTION__, flags, idx, bufptr,
					PCI_BUS(search_bdf), PCI_DEV(search_bdf), PCI_FUNC(search_bdf));
	}
//	if (flags & PCI_SEARCH_VENDEV)
	else	// by default we search for the vendor and device ID
	{
		vid = dinfo->VendorId;
		did = dinfo->DeviceId;
		slog_debug(0, "%s(0x%x, %u, %p) using vid/did %x/%x", __FUNCTION__, flags, idx, bufptr, vid, did);
	}

	while((bdf = pci_device_find(search_idx, vid, did, ccode)) != PCI_BDF_NONE)
	{
		if ((search_bdf == PCI_BDF_NONE) || (bdf == search_bdf))
		{
			pci_attachFlags_t attachFlags = pci_attachFlags_DEFAULT | pci_attachFlags_e_BKWD_COMPAT;
			pci_err_t err = PCI_ERR_OK;

			/*
			 * we always try to attach as the owner first (because cleanup requires an owner). If that
			 * fails, it means the device is already owned so we try and attach as shared. The
			 * 'pci_attachFlags_e_BKWD_COMPAT' flag ensures we will be able to read ba and irq info
			 */
			hdl = pci_device_attach(bdf, attachFlags, &err);
//if (hdl == NULL) slog_warn(0, "%s(): first attach attempt with flags %x failed, %s", __FUNCTION__, attachFlags, pci_strerror(err));
			if (hdl == NULL) hdl = pci_device_attach(bdf, attachFlags = pci_attachFlags_e_SHARED | pci_attachFlags_e_BKWD_COMPAT, &err);
//if (hdl == NULL) slog_error(0, "%s(): second attach attempt with flags %x failed, %s", __FUNCTION__, attachFlags, pci_strerror(err));
			if (hdl != NULL)
			{
				int_t nirq = 1;
				pci_irq_t _irq;
				pci_irq_t *irq = &_irq;
				pci_ba_t ba[7];
				int_t nba = NELEMENTS(ba);
				int_t i;
				const pci_ba_t as_bm_0 =
				{
					.addr = 0, .size = 0x1000,
					.attr = pci_asAttr_e_64BIT | pci_asAttr_e_INBOUND,
					.type = pci_asType_e_MEM,
				};
				pci_ba_t as_xlate;

				/* fill in dinfo */
				pci_device_read_vid(bdf, &dinfo->VendorId);
				pci_device_read_did(bdf, &dinfo->DeviceId);
				pci_device_read_ssid(bdf, &dinfo->SubsystemId);
				pci_device_read_ssvid(bdf, &dinfo->SubsystemVendorId);
				dinfo->BusNumber = PCI_BUS(bdf);
				dinfo->DevFunc = PCI_DEVFUNC(PCI_DEV(bdf), PCI_FUNC(bdf));
				pci_device_read_revid(bdf, &dinfo->Revision);
				if (ccode == PCI_CCODE_ANY) pci_device_read_ccode(bdf, &ccode);
				dinfo->Class = (PCI_CCODE_CLASS(ccode) << 16) | (PCI_CCODE_SUBCLASS(ccode) << 8) | PCI_CCODE_REG_IF(ccode);

				/*
				 * If either PCI_USE_MSI or PCI_USE_MSIX are set, we will try to use MSI's (not MSI-X)
				 * The attempted configuration must be done before we read the IRQ's.
				 */
				dinfo->msi = msi_init_all ? config_all_msi(hdl, &nirq) : config_msi(hdl);
				dinfo->Irq = -1;

				if (msi_init_all && (nirq > 1))
				{
					static pci_irq_t _irqs[32];
					irq = _irqs;
				}
				if ((err = pci_device_read_irq(hdl, &nirq, irq)) == PCI_ERR_OK)
				{
					if (nirq >= 1)
					{
						dinfo->Irq = irq[0];
						/*
						 * some stupid drivers (like graphics) ignore the IRQ values returned in the
						 * attach and go and read 0x3c themselves. In the new server they will not be able
						 * to do that and this chaos will not be able to happen. Anyway, for this (compat)
						 * module we will stuff the IRQ value into 0x3c
						 */
						pci_err_t r = pci_bkwd_write_config(hdl, 0x3c, 1, sizeof(uint8_t), &dinfo->Irq);
						slog_debug(1, "update of intline at 0x3c %s", pci_strerror(r));
					}
				}
				else slog_debug(0, "%s(), pci_device_read_irq() failed, nirq=%u, %s", __FUNCTION__, nirq, pci_strerror(err));

				/* pci_device_read_ba() already provides a translated OUTBOUND address */
				dinfo->CpuMemTranslation = 0;
				dinfo->CpuIoTranslation = 0;

				/* use a 0 address to obtain any inbound (BusMaster) translation */
				if (pci_hw->map_as(bdf, &as_bm_0, &as_xlate) != PCI_ERR_OK)
				{
					slog_debug(0, "%s(), map_as() of INBOUND MEM failed, %s", __FUNCTION__, pci_strerror(err));
					pci_device_detach(hdl);
					return NULL;
				}
				else dinfo->CpuBmstrTranslation = as_xlate.addr;

				dinfo->CpuIsaTranslation = 0;
				dinfo->RomSize = 0;
				dinfo->CpuRom = 0;
				dinfo->PciRom = 0;

				/* check and set the Expansion ROM enable/disable before we read ba */
				if (flags & PCI_INIT_ROM) pci_device_rom_enable(hdl);

				memset(ba, 0, sizeof(ba));
				if ((err = pci_device_read_ba(hdl, &nba, ba, pci_reqType_e_UNSPECIFIED)) != PCI_ERR_OK)
				{
					slog_debug(0, "%s(), pci_device_read_ba() failed, %s", __FUNCTION__, pci_strerror(err));
					pci_device_detach(hdl);
					return NULL;
				}
				for (i=0; i<NELEMENTS(dinfo->CpuBaseAddress); i++)
				{
					dinfo->CpuBaseAddress[i] = 0;
					dinfo->BaseAddressSize[i] = 0;
					dinfo->PciBaseAddress[i] = 0;
				}
				for (i=0; i<nba; i++)
				{
//slog_debug(0, "ba[%u] type/attr %u/%x, addr/size: %"PRIx64"/%"PRIx64"", i, ba[i].type, ba[i].attr, ba[i].addr, ba[i].size);
					if (ba[i].type == pci_asType_e_NONE) continue;
					else if (ba[i].attr & pci_asAttr_e_EXPANSION_ROM)
					{
						dinfo->RomSize = ba[i].size;
						dinfo->CpuRom = ba[i].addr;
						dinfo->PciRom = dinfo->CpuRom - dinfo->CpuMemTranslation;
						if (ba[i].attr & pci_asAttr_e_ENABLED) dinfo->PciRom |= 1;
					}
					else
					{
						assert(i < NELEMENTS(dinfo->CpuBaseAddress));

						/* old pci server gives back the attribute bits so stuff them back in */
						if (ba[i].type == pci_asType_e_IO) ba[i].addr |= 0x1;
						if (ba[i].attr & pci_asAttr_e_PREFETCH) ba[i].addr |= 0x8;
						if ((ba[i].attr & pci_asAttr_e_BIT_MASK) == pci_asAttr_e_64BIT) ba[i].addr |= 0x4;
//slog_debug(0, "*ba[%u] type/attr %u/%x, addr/size: %"PRIx64"/%"PRIx64"/%x", i, ba[i].type, ba[i].attr, ba[i].addr, ba[i].size, (uint32_t)ba[i].size);

						/* unless the PCI_INIT_BASE[0..5] flag is included, leave the entry as zeros */
						if (((PCI_INIT_BASE0 << ba[i].bar_num) & flags) != 0)
						{
							dinfo->CpuBaseAddress[ba[i].bar_num] = ba[i].addr;
							dinfo->BaseAddressSize[ba[i].bar_num] = (uint32_t)ba[i].size;
							dinfo->PciBaseAddress[ba[i].bar_num] = dinfo->CpuBaseAddress[ba[i].bar_num] -
									((ba[i].type == pci_asType_e_IO) ? dinfo->CpuIoTranslation : dinfo->CpuMemTranslation);
//slog_debug(0, "CpuBaseAddress: %"PRIx64", PciBaseAddress: %"PRIx64", BaseAddressSize: %x", dinfo->CpuBaseAddress[i], dinfo->PciBaseAddress[i], dinfo->BaseAddressSize[i]);
						}
					}
				}
				// are these values ok? It seems that they are so far
				dinfo->BusIoStart = 0;
				dinfo->BusIoEnd = 0;
				dinfo->BusMemStart = 0;
				dinfo->BusMemEnd = 0;

				DUMP_DEV_INFO(dinfo);

				/* set the BUS Master Bit */
				if (flags & PCI_MASTER_ENABLE)
				{
					pci_cmd_t cmd;
					if (pci_device_read_cmd(bdf, &cmd) == PCI_ERR_OK)
					{
						pci_cmd_t cmd2;
						pci_err_t r = pci_device_write_cmd(hdl, cmd | (1u << 2), &cmd2);
						if (r == PCI_ERR_OK) slog_debug(1, "CMD changed from %x to %x", cmd, cmd2);
						else slog_error(0, "Failed on write to cmd register (%x -> %x), %s", cmd, cmd2, pci_strerror(r));
					}
				}
			}
			slog_debug(0, "pci_device_attach(B%u:D%u:F%u, %x) @ search_idx %u %s",
						PCI_BUS(bdf), PCI_DEV(bdf), PCI_FUNC(bdf),
						attachFlags, search_idx, pci_strerror(err));
			break;
		}
		++search_idx;
	}
	return hdl;
}

/*
 ===============================================================================
 pci_bkwd_detach_device

*/
__attribute__ ((visibility ("internal")))
pci_err_t pci_bkwd_detach_device(pci_devhdl_t hdl)
{
	pci_err_t r = pci_device_detach(hdl);
	slog_debug(0, "%s(%p), %s", __FUNCTION__, hdl, pci_strerror(r));
	return r;
}

/*
 ===============================================================================
 pci_bkwd_device_find

*/
__attribute__ ((visibility ("internal")))
pci_bdf_t pci_bkwd_device_find(uint_t idx, pci_vid_t vid, pci_did_t did, pci_ccode_t ccode)
{
	pci_bdf_t bdf = pci_device_find(idx, vid, did, ccode);

	slog_debug(0, "%s(%u, %x, %x, %x) returns B%u:D%u:F%u", __FUNCTION__,
				idx, vid, did, ccode,
				(bdf == PCI_BDF_NONE) ? 255 : PCI_BUS(bdf),
				(bdf == PCI_BDF_NONE) ? 255 : PCI_DEV(bdf),
				(bdf == PCI_BDF_NONE) ? 255 : PCI_FUNC(bdf));

	return bdf;
}

/*
 ===============================================================================
 pci_bkwd_read_config_bus

 See comments in pci_device_cfg_rd*() for the rationale on the negation of offset
*/
__attribute__ ((visibility ("internal")))
pci_err_t pci_bkwd_read_config_bus(pci_bdf_t bdf, uint_t offset, uint_t cnt, size_t size, void *bufptr)
{
	pci_err_t r = PCI_ERR_OK;
	const uint_t offset_in = offset;	// save for debug display
	const void *bufptr_in = bufptr;		// save for debug display
	uint_t i;

	for (i=0; i<cnt; i++)
	{
		if (size == 1) r = pci_device_cfg_rd8(bdf, -offset, (uint8_t *)bufptr);
		else if (size == 2) r = pci_device_cfg_rd16(bdf, -offset, (uint16_t *)bufptr);
		else if (size == 4) r = pci_device_cfg_rd32(bdf, -offset, (uint32_t *)bufptr);
		else if (size == 8) r = pci_device_cfg_rd64(bdf, -offset, (uint64_t *)bufptr);

		if (r != PCI_ERR_OK) break;
		else
		{
			offset += size;
			bufptr = (void *)((uintptr_t)bufptr + size);
		}
	}
	slog_debug(1, "%s(B%u:D%u:F%u, 0x%x, %u, %zu, %p) %s", __FUNCTION__,
					PCI_BUS(bdf), PCI_DEV(bdf), PCI_FUNC(bdf),
					offset_in, cnt, size, bufptr_in, pci_strerror(r));

	return r;
}

/*
 ===============================================================================
 pci_bkwd_read_config

*/
__attribute__ ((visibility ("internal")))
pci_err_t pci_bkwd_read_config(pci_devhdl_t hdl, uint_t offset, uint_t cnt, size_t size, void *bufptr)
{
	pci_bdf_t bdf = PCI_DEVHDL_DECODE_BDF(hdl);
	pci_err_t r = pci_bkwd_read_config_bus(bdf, offset, cnt, size, bufptr);

	slog_debug(1, "%s(%p, 0x%x, %u, %zu, %p) %s", __FUNCTION__, hdl, offset, cnt, size, bufptr, pci_strerror(r));

	return r;
}

/*
 ===============================================================================
 pci_bkwd_write_config

 See comments in pci_device_cfg_wr*() for the rationale on the negation of offset
*/
__attribute__ ((visibility ("internal")))
pci_err_t pci_bkwd_write_config(pci_devhdl_t hdl, uint_t offset, uint_t cnt, size_t size, const void *bufptr)
{
	pci_err_t r = PCI_ERR_OK;
	const uint_t offset_in = offset;	// save for debug display
	const void *bufptr_in = bufptr;		// save for debug display

	if (hdl == NULL) return PCI_ERR_EINVAL;
	else
	{
		uint_t i;
		for (i=0; i<cnt; i++)
		{
			if (size == 1) r = pci_device_cfg_wr8(hdl, -offset, *((uint8_t *)bufptr), NULL);
			else if (size == 2) r = pci_device_cfg_wr16(hdl, -offset, *((uint16_t *)bufptr), NULL);
			else if (size == 4) r = pci_device_cfg_wr32(hdl, -offset, *((uint32_t *)bufptr), NULL);
			else if (size == 8) r = pci_device_cfg_wr64(hdl, -offset, *((uint64_t *)bufptr), NULL);
			else
			{
				r = PCI_ERR_EINVAL;
				break;
			}
			offset += size;
			bufptr = (void *)((uintptr_t)bufptr + size);
		}
	}
	slog_debug(1, "%s(%p, 0x%x, %u, %zu, %p) %s", __FUNCTION__, hdl, offset_in, cnt, size, bufptr_in, pci_strerror(r));

	return r;
}

/*
 ===============================================================================
 pci_bkwd_write_config_bus

*/
__attribute__ ((visibility ("internal")))
pci_err_t pci_bkwd_write_config_bus(pci_bdf_t bdf, uint_t offset, uint_t cnt, size_t size, const void *bufptr)
{
	pci_err_t r;
	pci_devhdl_t hdl = pci_device_attach(bdf, pci_attachFlags_e_SHARED | pci_attachFlags_e_BKWD_COMPAT, &r);
	if (hdl != NULL)
	{
		r = pci_bkwd_write_config(hdl, offset, cnt, size, bufptr);
		pci_device_detach(hdl);
	}

	slog_debug(1, "%s(B%u:D%u:F%u, 0x%x, %u, %zu, %p) %s", __FUNCTION__,
				PCI_BUS(bdf), PCI_DEV(bdf), PCI_FUNC(bdf),
				offset, cnt, size, bufptr, pci_strerror(r));

	return r;
}

/*
 ===============================================================================
 pci_bkwd_rescan_bus

 There is nothing to do here. The PCI shared memory cache always contains an
 up-to-date copy of existing devices
*/
__attribute__ ((visibility ("internal")))
void pci_bkwd_rescan_bus(void)
{
	slog_debug(0, "%s()", __FUNCTION__);
}

/*
 ===============================================================================
 pci_bkwd_pci_present

 We are not using the BIOS facilities at all so the following will be returned

 	 hardware = 1 (PCI config mechanism 1 only, no special cycles)
 	 version = 2 (indicating no version 3 features. Also what is currently returned)
 	 lastbus - will be obtained from the global variable 'pci_last_bus' which is
 	 	 	 	 recorded in libpci
*/
__attribute__ ((visibility ("internal")))
void pci_bkwd_pci_present(uint_t *lastbus, uint_t *version, uint_t *hardware)
{
	static pthread_once_t once = PTHREAD_ONCE_INIT;

	pthread_once(&once, set_pci_max_bus);

	if (hardware != NULL) *hardware = 1;
	if (version != NULL) *version = 2 << 8;	// 2.0
	if (lastbus != NULL) *lastbus = pci_max_bus;

	slog_debug(0, "%s(%p=%u, %p=%u, %p=%u)", __FUNCTION__,
				lastbus, lastbus ? *lastbus : 0,
				version, version ? *version : 0,
				hardware, hardware ? *hardware : 0);
}

#ifndef NDEBUG
static void dump_dev_info(struct pci_dev_info *dinfo)
{
	uint_t i;

	slog_debug(0, "++++ struct pci_dev_info ++++");
	slog_debug(0, "\tVendorId: 0x%x", dinfo->VendorId);
	slog_debug(0, "\tDeviceId: 0x%x", dinfo->DeviceId);
	slog_debug(0, "\tDevFunc: 0x%x", dinfo->DevFunc);
	slog_debug(0, "\tBus: %u", dinfo->BusNumber);
	slog_debug(0, "\tClasscode: 0x%x", dinfo->Class);
	slog_debug(0, "\tRevision: %u", dinfo->Revision);
	slog_debug(0, "\tSubsystemId: 0x%x", dinfo->SubsystemId);
	slog_debug(0, "\tSubsystemVendorId: 0x%x", dinfo->SubsystemVendorId);
	slog_debug(0, "\tBusIoStart: 0x%"PRIx64"", dinfo->BusIoStart);
	slog_debug(0, "\tBusIoEnd: 0x%"PRIx64"", dinfo->BusIoEnd);
	slog_debug(0, "\tBusMemStart: 0x%"PRIx64"", dinfo->BusMemStart);
	slog_debug(0, "\tBusMemEnd: 0x%"PRIx64"", dinfo->BusMemEnd);
	slog_debug(0, "\tCpuIoTranslation: 0x%"PRIx64"", dinfo->CpuIoTranslation);
	slog_debug(0, "\tCpuIsaTranslation: 0x%"PRIx64"", dinfo->CpuIsaTranslation);
	slog_debug(0, "\tCpuMemTranslation: 0x%"PRIx64"", dinfo->CpuMemTranslation);
	slog_debug(0, "\tCpuBmstrTranslation: 0x%"PRIx64"", dinfo->CpuBmstrTranslation);
	slog_debug(0, "\tIrq: %d", dinfo->Irq);
	slog_debug(0, "\tMSI: %u", dinfo->msi);
	slog_debug(0, "\tCpuRom: 0x%"PRIx64"", dinfo->CpuRom);
	slog_debug(0, "\tPciRom: 0x%"PRIx64"", dinfo->PciRom);
	slog_debug(0, "\tRomSize: 0x%x", dinfo->RomSize);
	for(i=0; i<NELEMENTS(dinfo->BaseAddressSize); i++)
	{
		slog_debug(0, "\tBaseAddressSize[%u]: 0x%x", i, dinfo->BaseAddressSize[i]);
		slog_debug(0, "\tCpuBaseAddress[%u]: 0x%"PRIx64"", i, dinfo->CpuBaseAddress[i]);
		slog_debug(0, "\tPciBaseAddress[%u]: 0x%"PRIx64"", i, dinfo->PciBaseAddress[i]);
	}
	slog_debug(0, "++++ end struct pci_dev_info ++++\n");
}
#endif	/* NDEBUG */

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/services/pci/modules/bkwd_compat/bkwd_pci.c $ $Rev: 811119 $")
#endif
