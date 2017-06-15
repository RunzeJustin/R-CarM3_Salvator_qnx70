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
#include <sys/syspage.h>
#include <sys/rsrcdbmgr.h>
#include <sys/rsrcdbmsg.h>
#include <string.h>
#include <hw/sysinfo.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <assert.h>

#include "private/hwmod_api.h"
#include "hw_lib.h"
#include "private/pci_slog.h"
#include "private/pci_debug.h"

/*
 ===============================================================================
  hw_shm

*/
__attribute__ ((visibility ("internal"))) hw_shm_t *hw_shm;


#define HW_SHMSEG_NAME		"/dev/shmem/pci_hw"
#define HW_SHMSEG_SIZE		4096	// 1 page

/*
 ===============================================================================
                               Device Hold Off

 The device hold-off mechanism provides a means of halting access to the
 configuration space of specific devices. This can be used in conjunction with
 event notifications to prevent driver software from accessing the configuration
 space of removed devices or devices on buses that will be reset.

 In order to accomplish this, we make use of an access semaphore which resides
 in the HW specific shared memory object. This semaphore is incremented each
 time that the HW module services an access request (either read or write). It
 is decremented when that request completes regardless of error status.

 When it is required to hold off configuration space accesses (usually the PCI
 server process will initiate this) the HW module API add_device_hold_off() is
 called one or more times with the devices for which access should be halted.
 The initiate_hold_off() function is then called to cause the HW module to
 halt access on those added devices (see details below about synchronization).
 Access to devices not added to the hold off list are allowed to complete
 normally.
 When the hold-off is no longer required, remove_device_hold_off() is called and
 all accesses continue as before. The device hold-off list can then be torn down
 or modified for future hold-offs.

 Note that accesses to non-configuration device space is not controlled by this
 mechanism hence why this mechanism is only part of the solution.
 ===============================================================================
*/

/*
 ===============================================================================
 add_device_hold_off
 remove_device_hold_off

 Add/remove <bdf> to/from the list of devices that will be inaccessible

 A bit map of buses and devices is maintained to allow fast access checks to be
 performed. Multiple 32 bit unsigned integers are used to hold the bus bit map.
 A single 32 bit integer per bus is used to hold the 32 possible devices per
 bus. Hold off is at the device (not function) granularity.
*/
__attribute__ ((visibility ("internal")))
pci_err_t hw_add_device_hold_off(pci_bdf_t bdf)
{
	hw_shm->dev_hold_off[PCI_BUS(bdf)] |= (1u << PCI_DEV(bdf));
	hw_shm->bus_hold_off[PCI_BUS(bdf) / 32] |= (1u << (PCI_BUS(bdf) % 32));

	return PCI_ERR_OK;
}
__attribute__ ((visibility ("internal")))
pci_err_t hw_remove_device_hold_off(pci_bdf_t bdf)
{
	hw_shm->dev_hold_off[PCI_BUS(bdf)] &= ~(1u << PCI_DEV(bdf));
	hw_shm->bus_hold_off[PCI_BUS(bdf) / 32] &= ~(1u << (PCI_BUS(bdf) % 32));

	return PCI_ERR_OK;
}

/*
 ===============================================================================
 initiate_hold_off
 release_hold_off

 Add/release device hold checking.

 There are 2 phases to activate a hold-off. The first sets the request bit field
 HW_HOLDOFF_REQ of the access semaphore 'cfg_access_sem'. This will cause all
 new configuration space accesses after that point to block on a condition
 variable. The count portion of the semaphore is then polled (with sleeps) until
 it becomes 0 indicating that all "in-flight" accesses have completed.

 The second phase sets the HW_HOLDOFF_INUSE bit field of the access semaphore
 and unblocks all accesses pending on the condition variable. All future
 accesses will then be tested against the bus and device bit map and only those
 which hit will block. Accesses to all other devices will be allowed to complete
 normally.

 When the hold-off is no longer required, both the HW_HOLDOFF_REQ and
 HW_HOLDOFF_INUSE bits are cleared and any blocked accesses are unblocked.

 When this routine returns, it is guaranteed that no further accesses will be
 made to devices in the hold-off list

 IMPORTANT:
 These functions are meant to be called as a matched set. They will nest so in
 order to remove all hold-offs, the same number of releases must be called as
 initiations

*/
__attribute__ ((visibility ("internal")))
pci_err_t hw_initiate_hold_off(void)
{
	atomic_add(&hw_shm->cfg_nested_holdoff_count, 1);
	/*
	 * set up a hold-off request. If an active hold-off is already set, we move
	 * back to a request to force all accesses to block as this is the only way
	 * we are guaranteed to see zero "in-progress" accesses and hence return
	 * knowing that no accesses to devices in the hold off list will be made
	 */
	atomic_clr(&hw_shm->cfg_access_sem, HW_HOLDOFF_INUSE);
	atomic_set(&hw_shm->cfg_access_sem, HW_HOLDOFF_REQ);

	assert((hw_shm->cfg_access_sem & HW_HOLDOFF_ACTIVE) == HW_HOLDOFF_REQ);

	/* wait until there are no more accesses in progress */
	while ((hw_shm->cfg_access_sem & ~HW_HOLDOFF_ACTIVE) != 0)
	{
		struct timespec tdelay;
		nsec2timespec(&tdelay, 5000);	// 5 usec arbitrary choice
		nanosleep(&tdelay, NULL);
	}
	/*
	 * set hold-off access checking "in use". This will cause all accesses to be
	 * checked to determine whether or not the access can proceed
	 */
	atomic_set(&hw_shm->cfg_access_sem, HW_HOLDOFF_INUSE);
	/* release currently blocked accesses */
	pthread_cond_broadcast(&hw_shm->hold_off.cond);

	return PCI_ERR_OK;
}
__attribute__ ((visibility ("internal")))
pci_err_t hw_release_hold_off(void)
{
	if (atomic_sub_value(&hw_shm->cfg_nested_holdoff_count, 1) == 1)
	{
		atomic_clr(&hw_shm->cfg_access_sem, HW_HOLDOFF_REQ);
		atomic_clr(&hw_shm->cfg_access_sem, HW_HOLDOFF_INUSE);

		/* release any blocked accesses */
		pthread_cond_broadcast(&hw_shm->hold_off.cond);
	}
	return PCI_ERR_OK;
}



/*
 ===============================================================================
 init_hw_shm

 This function is called once (per process) to initialize the 'hw_shm' pointer.
 If required (ie. this is the first software to execute a configuration space
 access since boot), the shared memory segment is created and initialized

*/
__attribute__ ((visibility ("internal")))
void init_hw_shm(void)
{
	int shm_fd = shm_open(HW_SHMSEG_NAME, O_RDWR | O_CREAT | O_EXCL, 0666);

	if (shm_fd >= 0)
	{
		/* this is the first caller. Initialize the hw_shm_t structure */
		void *p;

		if (ftruncate(shm_fd, HW_SHMSEG_SIZE) < 0)
		{
			slog_error(0, "%s(): Unable truncate %s, errno: %d", __FUNCTION__, HW_SHMSEG_NAME, errno);
			close(shm_fd);
		}
		else if ((p = mmap(NULL, HW_SHMSEG_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0)) == MAP_FAILED)
		{
			slog_error(0, "%s(): Unable mmap %s, errno: %d", __FUNCTION__, HW_SHMSEG_NAME, errno);
			close(shm_fd);
		}
		else
		{
			/* shared memory is initialized to 0's */
			hw_shm_t *tmp_hw_shm = (hw_shm_t *)p;
			pthread_condattr_t cond_attr;

			pthread_condattr_init(&cond_attr);
			pthread_condattr_setpshared(&cond_attr, PTHREAD_PROCESS_SHARED);
			pthread_cond_init(&tmp_hw_shm->hold_off.cond, &cond_attr);

			pthread_mutex_init(&tmp_hw_shm->hold_off.mutex, NULL);

			/* unmap and close before we call init_hw_shm() */
			munmap(p, HW_SHMSEG_SIZE);
			close(shm_fd);

			init_hw_shm();	// go through the normal path to initialize 'hw_shm'
		}
	}
	else if (errno == EEXIST)
	{
		/* some other process already created and populated pci_hw. Try a regular open */
		struct stat s;

		/* TODO - there is a (potential - see practicality below) race here where one
		 * thread of one process creates the shared memory region but it has not yet
		 * either been truncated to size or populated and one thread of another process
		 * ends up in this code block. The size from fstat() could be wrong for the mmap()
		 * (if the ftruncate() had not completed)
		 *
		 * In reality, it is not a practical situation since under normal circumstances,
		 * the PCI server is started (which will create and fully intialize the shared
		 * memory object) and then drivers which use the PCI server are started. A barrier
		 * of some sort, to block until the initialization is complete, may be required.
		 *
		 * Will monitor until it is determined that there is a real problem
		 */
		shm_fd = shm_open(HW_SHMSEG_NAME, O_RDWR, 0666);

		if (shm_fd < 0) slog_error(0, "%s(): Unable to open %s, errno: %d", __FUNCTION__, HW_SHMSEG_NAME, errno);
		else if (fstat(shm_fd, &s) < 0) slog_error(0, "%s(): Unable to stat %s, errno: %d", __FUNCTION__, HW_SHMSEG_NAME, errno);
		else
		{
			void *p = mmap(NULL, s.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);

			if (p == MAP_FAILED) slog_error(0, "%s(): Unable to mmap %s, errno: %d", __FUNCTION__, HW_SHMSEG_NAME, errno);
			else hw_shm = (hw_shm_t *)p;
		}
		/* don't need the 'shm_fd' regardless of success */
		if (shm_fd >= 0) close(shm_fd);
	}
	else slog_error(0, "%s(): Unable to create %s, errno: %d", __FUNCTION__, HW_SHMSEG_NAME, errno);
}

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/services/pci/modules/hw/src/hw_shm.c $ $Rev: 798837 $")
#endif
