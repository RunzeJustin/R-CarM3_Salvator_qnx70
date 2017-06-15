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
#include <atomic.h>
#include <pthread.h>

#include <pci/pci.h>

#include "private/hwmod_api.h"
#include "private/pci_slog.h"
#include "hw_lib.h"

/*
 ===============================================================================
 config_access_start
 config_access_end

 These 2 functions mark the start and end of a configuration space access
 respectively.
 The fast path (ie. no hold off is requested or active) is simply an atomic
 increment/decrement with an added mask and test on increment.

 If a hold request has been made or is active on <bdf>, the caller will either
 block until all configuration accesses complete or return PCI_ERR_DEV_NOT_AVAIL
 if <no_hold_off> is true. <no_hold_off> is used for situations in which the
 blocking code cannot be called (ex. if an API is callable from an ISR)

 NOTE
 Only if PCI_ERR_OK is returned will the semaphore count have been incremented


 Hold-off Implementation Note
 ----------------------------
 The idea of "hold-off" is to prevent accesses to a device when it is (or will)
 become unavailable either do to removal or reset or due to a parent bridge
 becoming unavailable.

 There are 2 types of access, configuration space accesses and device memory
 accesses. Configuration space accesses flow through the HW dependent module and
 hence the config_access_start()/config_access_end() routines and so these are
 directly manageable. Memory mapped device accesses occur independent of the
 PCI server and associated modules.

 In order to control both types of access, an event notification mechanism will
 be put in place that will allow driver software to be notified of an event
 related to a device it is managing so that it can gracefully cease accesses to
 both the configuration and device specific memory spaces. This design of this
 event mechanism is TBD

 In addition, in order to ensure that configuration space accesses are halted,
 the config_access_start() and config_access_end() routines exist. They operate
 as follows.

 When an access (either read or write) is initiated, config_access_start() is
 called. A shared semaphore is simultaneously (and atomically) incremented and
 checked to see whether a hold-off is pending (ie. a request is in progress) or
 active. If neither of these conditions exist, the access proceeds and at the
 end of the access the semaphore is atomically decremented. The semaphore holds
 a count of the number of configuration space accesses currently in progress.

 If a hold-off is pending, the caller will be blocked on a condition variable
 until the hold-off either clears or becomes active. Once once of those
 conditions becomes true, the config_access_start() restarts. If the condition
 was cleared, the semaphore is incremented and the access performed. If the
 hold-off became active, then an additional check is performed to see if the
 hold-off applies to the device to be accessed. If not, the access proceeds, if
 so, the caller again blocks on the condition variable until the hold-off is
 released.
 The exception to blocking on the condition variable is when the access occurs
 from an ISR. In this situation, PCI_ERR_DEV_NOT_AVAIL is returned to let the
 caller know of the unavailability of the device. While this situation is rare,
 it is possible for some API's to require configuration space access from an
 ISR. The situation exists for MSI and MSI-X interrupt masking.

 In order to minimize the occurrence of accesses from ISR's, when a hold-off is
 initiated, the initiator will clear the Bus Master Enable and set the Interrupt
 Disable bits in the devices command register. This will block both pin and
 message based interrupts from being transmitted to the processor by the device
 however we still handle the situation (as described above) since pending
 interrupts may exist that were sent prior to the command register update.

 So a hold-off event will cause the driver software to ceases accesses. What the
 driver will then do after it has quiesced itself is to block on the condition
 variable such that when the condition clears it can return to normal operation.
 In the event some driver thread starts an access after the event is received,
 that thread will still be blocked on the condition variable so the net result
 is the same

 The sequence will be as follows. Note that the sequence is described for a
 single device however if multiple devices must be held off, the sequence is
 repeated for all of them

 For the hold-off initiator (the PCI server process)

	 - send event notification to driver to cease accesses
 	 - call add_device_hold_off(bdf)
 	 - clear Bus Master and set Interrupt disable in command register keeping
 	   current values for restore
 	 - repeat for each device to be held off

 	 - call initiate_hold_off()

 	 upon return from initiate_hold_off(), we are assured that no device
 	 accesses are occurring

 	 Perform whatever maintenance is required that necessitated the hold-off

 	 - restore Bus Master and Interrupt disable bits in command register
 	 - call remove_device_hold_off()
 	 - repeat for each device held off

 	 - call release_hold_off()

 In driver software

 	 - the device hold-off event handler synchronizes all threads in order to
 	   prevent further device accesses. This may require disabling the attached
 	   interrupt if any device accesses are done in an ISR
 	 - at least one thread calls <TBD API>() which causes a call to
 	   config_access_start() which blocks the thread on the condition variable.
 	   Other threads could continue to function as long as they do not attempt
 	   device memory accesses. Configuration space accesses will of course block

 	 - at some point later, the blocked thread(s) will be unblocked indicating
 	   that accesses to the device can resume. It is driver specific what
 	   internal synchronization is required in order to restore use of the
 	   device
*/
static inline pci_err_t config_access_start(pci_bdf_t bdf, bool_t no_hold_off)
{
	int32_t sem_val;

//	slog_debug(0, "%s(B%u:D%u:F%u)", __FUNCTION__, PCI_BUS(bdf), PCI_DEV(bdf), PCI_FUNC(bdf));

	while ((sem_val = (atomic_add_value(&hw_shm->cfg_access_sem, 1) & HW_HOLDOFF_ACTIVE)) != 0)
	{
//		slog_debug(0, "%s(B%u:D%u:F%u), sem_val = 0x%x", __FUNCTION__, PCI_BUS(bdf), PCI_DEV(bdf), PCI_FUNC(bdf), sem_val);
		/*
		 * if the hold off is not yet active or its active and our BDF is affected,
		 * block until the hold-off either clears or goes active
		 * We never block the server as its the server that initiates a holdoff
		 * and may need to access a device while its held off
		 */
		if (pci_runtime_flags & pci_runtime_flags_e_SERVER) break;
		else if ((sem_val == HW_HOLDOFF_REQ) ||
				 ((sem_val == HW_HOLDOFF_ACTIVE) &&
				 (((1u << (PCI_BUS(bdf) % 32)) & hw_shm->bus_hold_off[PCI_BUS(bdf) / 32]) != 0) &&
				 (((1u << PCI_DEV(bdf)) & hw_shm->dev_hold_off[PCI_BUS(bdf)]) != 0)))
		{
			atomic_sub(&hw_shm->cfg_access_sem, 1);

			if (no_hold_off) return PCI_ERR_DEV_NOT_AVAIL;
			else
			{
				LOCK(&hw_shm->hold_off.mutex);
				pthread_cond_wait(&hw_shm->hold_off.cond, &hw_shm->hold_off.mutex);
				UNLOCK(&hw_shm->hold_off.mutex);
				/* at this point the hold-off is active or it has been cleared */
			}
		}
		else break;
	}
	return PCI_ERR_OK;
}
static inline void config_access_end(pci_bdf_t bdf)
{
	atomic_sub(&hw_shm->cfg_access_sem, 1);
//	slog_debug(0, "%s(B%u:D%u:F%u)", __FUNCTION__, PCI_BUS(bdf), PCI_DEV(bdf), PCI_FUNC(bdf));
}
static void _config_access_end(void *bdf_p)
{
	pci_bdf_t bdf = *((pci_bdf_t *)bdf_p);
	config_access_end(bdf);
}

/*
 ===============================================================================
 cfg_rd

 This function implements the entry point to the PCI configuration space read
 operations.

 It records the start of the access transaction (which checks for and handles
 hold-off requests), calls the HW specific read entry point hw_rd() (unless only
 access checking is requested) and then ends the transaction

 This function should be entered into the .cfg_rd field of the hwmod_api_t
 structure

*/
//#define TEST_BAD_CCODE_WITH_TYPE_1_HDR

__attribute__ ((visibility ("internal")))
pci_err_t cfg_rd(pci_bdf_t bdf, uint_t offset, uint_t size, uint8_t *buf, _pci_accessAttr_e accessAttr)
{
	pci_err_t r;

#ifdef TEST_BAD_CCODE_WITH_TYPE_1_HDR
	if ((bdf == PCI_BDF(0, 25, 0)) && (offset == 8) && (size == 4))
	{
		*(uint32_t *)buf = PCI_CCODE(0xff, 3, 3) << 8;
		return PCI_ERR_OK;
	}
#endif	/* TEST_BAD_CCODE_WITH_TYPE_1_HDR */

	pthread_cleanup_push(_config_access_end, (void *)&bdf);

	if ((accessAttr & _pci_accessAttr_e_OFFSET_34) != 0)
	{
		slog_info(0, "== off 34 - B%u:D%u:F%u", PCI_BUS(bdf), PCI_DEV(bdf), PCI_FUNC(bdf));
		hw_shm->off_34 = true;
	}

	r = config_access_start(bdf, (accessAttr & _pci_accessAttr_e_ISR) != 0);
	if (r == PCI_ERR_OK)
	{
		if (!(accessAttr & _pci_accessAttr_e_CHECK)) r = hw_rd(bdf, offset, size, buf);
		config_access_end(bdf);
	}
	pthread_cleanup_pop(0);
	return r;
}

/*
 ===============================================================================
 cfg_wr

 This function implements the entry point to the PCI configuration space write
 operations.

 It records the start of the access transaction (which checks for and handles
 hold-off requests), calls the HW specific write entry point hw_wr() (unless only
 access checking is requested) and then ends the transaction

 This function should be entered into the .cfg_wr field of the hwmod_api_t
 structure

*/
__attribute__ ((visibility ("internal")))
pci_err_t cfg_wr(pci_bdf_t bdf, uint_t offset, uint_t size, uint8_t *buf, _pci_accessAttr_e accessAttr)
{
	pci_err_t r;

	pthread_cleanup_push(_config_access_end, (void *)&bdf);

	if ((accessAttr & _pci_accessAttr_e_OFFSET_34) != 0)
	{
		slog_info(0, "== off 34 - B%u:D%u:F%u", PCI_BUS(bdf), PCI_DEV(bdf), PCI_FUNC(bdf));
		hw_shm->off_34 = true;
	}

	r = config_access_start(bdf, (accessAttr & _pci_accessAttr_e_ISR) != 0);
	if (r == PCI_ERR_OK)
	{
		if (!(accessAttr & _pci_accessAttr_e_CHECK)) r = hw_wr(bdf, offset, size, buf);
		config_access_end(bdf);
	}
	pthread_cleanup_pop(0);
	return r;
}

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/services/pci/modules/hw/src/cfg_rdwr.c $ $Rev: 799923 $")
#endif
