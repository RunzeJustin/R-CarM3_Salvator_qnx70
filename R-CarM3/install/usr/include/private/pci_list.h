#ifndef _PCI_LIST_H_
#define _PCI_LIST_H_
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

#include "private/pci_lock.h"
/*
 ===============================================================================
 List processing types and macros
 ===============================================================================
*/
typedef struct node_s
{
    struct node_s *next;
    struct node_s **prev;
} node_t;

typedef struct list_hdr_s
{
	node_t *head;
	node_t *tail;
	uint_t count;
	pthread_mutex_t lock;
} list_hdr_t;

/*
 * NOTE
 *
 * all list add and delete operations operate on a locked list. List traversal
 * operations do not. Locking for list traversal should be done around the
 * traversal if list adds or deletions are possible during that operation
 */
#define LINK_BEG(_queue, _object, _type) \
		do { \
			LOCK(&(_queue).lock); \
	        if((((node_t *)(_object))->next = (_queue).head)) \
	        { \
	        	(_queue).head->prev = &((node_t *)(_object))->next; \
	        } \
	        else \
	        { \
	            (_queue).tail = (node_t *)(_object); \
	        } \
	        ((node_t *)(_object))->prev = &(_queue).head; \
	        (_queue).head = (node_t *)(_object); \
			++(_queue).count; \
			UNLOCK(&(_queue).lock); \
		} while (0)

#define LINK_END(_queue, _object, _type) \
		do { \
			LOCK(&(_queue).lock); \
	        if ((_queue).tail) \
	        { \
	            ((node_t *)(_object))->prev = &(_queue).tail->next; \
	            (_queue).tail->next = (node_t *)(_object); \
	        } \
	        else \
	        { \
	            ((node_t *)(_object))->prev = &(_queue).head; \
	            (_queue).head = (node_t *)(_object); \
	        } \
	        (_queue).tail = (node_t *)(_object); \
	        ((node_t *)(_object))->next = NULL; \
			++(_queue).count; \
			UNLOCK(&(_queue).lock); \
		} while (0)

#define LINK_REM(_queue, _object, _type) \
		do { \
			LOCK(&(_queue).lock); \
			if (!(((_object)->next == NULL) && ((_object)->prev == NULL))) \
			{ \
		        if((*(((node_t *)(_object))->prev) = ((node_t*)(_object))->next)) \
				{ \
		            ((node_t *)(_object))->next->prev = ((node_t *)(_object))->prev; \
		        } \
		        if ((_queue).tail == (node_t *)(_object)) \
				{ \
		            if ((_queue).head == NULL) \
		            { \
		                (_queue).tail = NULL; \
		            } else \
		            { \
		                (_queue).tail = (node_t *)((node_t *)(_object))->prev; \
		            } \
		        } \
				(_object)->next = NULL; \
				(_object)->prev = NULL; \
				--(_queue).count; \
			} \
			UNLOCK(&(_queue).lock); \
		} while (0)

#define LIST_INIT(_queue, _lock_attr) \
		do { \
			(_queue).count = 0; \
			(_queue).head = (_queue).tail = NULL; \
			pthread_mutex_init(&(_queue).lock, (_lock_attr)); \
		} while(0)


#define LIST_FIRST(_q)		(_q).head
#define LIST_LAST(_q)		(_q).tail
#define LIST_COUNT(_q)		(_q).count
#define LIST_ADD(_q, _n)	LINK_END((_q), &(_n)->hdr, typeof(_n))
#define LIST_DEL(_q, _n)	LINK_REM((_q), &(_n)->hdr, typeof(_n))
#define LIST_NEXT(_n)		((_n)->hdr.next)
#define LIST_PREV(_n)		((node_t *)((((uintptr_t)((_n)->hdr.prev))) - offsetof(node_t, next)))



#endif	/* _PCI_LIST_H_ */

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/lib/pci/private/pci_list.h $ $Rev: 798837 $")
#endif
