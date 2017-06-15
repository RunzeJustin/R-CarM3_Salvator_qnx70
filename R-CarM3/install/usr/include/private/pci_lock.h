#ifndef _PCI_LOCK_H_
#define _PCI_LOCK_H_
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

#include <pthread.h>

#ifndef NDEBUG
//#define LOCK_DEBUGGING			// comment in and clean compile everything for lock debugging
#endif	/* NDEBUG */


#ifdef LOCK_DEBUGGING

#include <assert.h>
#include "private/pci_debug.h"

static inline int rw_lock(pthread_rwlock_t *lock, int write)
{
	slog_debug(0, "lock %p %s lock", lock, (write == 0) ? "read" : "write");
	int r = (write == 0) ? pthread_rwlock_rdlock(lock) : pthread_rwlock_wrlock(lock);
	if (r != EOK)
	{
		slog_debug(0, "lock %p %s lock failure", lock, (write == 0) ? "read" : "write");
		assert(false);
	}
	return r;
}
static inline int rw_unlock(pthread_rwlock_t *lock, int write)
{
	slog_debug(0, "lock %p %s unlock", lock, (write == 0) ? "read" : "write");
	int r = pthread_rwlock_unlock(lock);
	if (r != EOK)
	{
		slog_debug(0, "lock %p %s unlock failure", lock, (write == 0) ? "read" : "write");
		assert(false);
	}
	return r;
}
static inline int mutex_lock(pthread_mutex_t *lock)
{
	int r = pthread_mutex_lock(lock);
	if (r != EOK)
	{
		slog_debug(0, "lock %p mutex lock failure", lock);
		assert(false);
	}
	return r;
}
static inline int mutex_unlock(pthread_mutex_t *lock)
{
	int r = pthread_mutex_unlock(lock);
	if (r != EOK)
	{
		slog_debug(0, "lock %p mutex unlock failure", lock);
		assert(false);
	}
	return r;
}
#define RD_LOCK(_l_)	rw_lock((_l_), 0)
#define WR_LOCK(_l_)	rw_lock((_l_), 1)
#define RD_UNLOCK(_l_)	rw_unlock((_l_), 0)
#define WR_UNLOCK(_l_)	rw_unlock((_l_), 1)

#define LOCK(_l_)		mutex_lock((_l_))
#define UNLOCK(_l_)		mutex_unlock((_l_))

#else	/* lock debugging */

#define RD_LOCK(_l_)	pthread_rwlock_rdlock((_l_))
#define WR_LOCK(_l_)	pthread_rwlock_wrlock((_l_))
#define RD_UNLOCK(_l_)	pthread_rwlock_unlock((_l_))
#define WR_UNLOCK(_l_)	pthread_rwlock_unlock((_l_))
#define LOCK(_l_)		pthread_mutex_lock((_l_))
#define UNLOCK(_l_)		pthread_mutex_unlock((_l_))

#endif	/* lock debugging */

#endif	/* _PCI_LOCK_H_ */

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/lib/pci/private/pci_lock.h $ $Rev: 798837 $")
#endif
