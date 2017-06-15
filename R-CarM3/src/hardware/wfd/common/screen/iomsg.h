/*
 * $QNXLicenseC:
 * Copyright 2008, QNX Software Systems. 
 *
 * Licensed under the Apache License, Version 2.0 (the "License"). You  
 * may not reproduce, modify or distribute this software except in  
 * compliance with the License. You may obtain a copy of the License  
 * at: http://www.apache.org/licenses/LICENSE-2.0  
 *  
 * Unless required by applicable law or agreed to in writing, software  
 * distributed under the License is distributed on an "AS IS" basis,  
 * WITHOUT WARRANTIES OF ANY KIND, either express or implied. 
 * 
 * This file may contain contributions from others, either as  
 * contributors under the License or as licensors under other terms.   
 * Please review this entire file for other proprietary rights or license  
 * notices, as well as the QNX Development Suite License Guide at  
 * http://licensing.qnx.com/license-guide/ for other information. 
 * $ 
 */

#ifndef _SCREEN_IOMSG_H_INCLUDED
#define _SCREEN_IOMSG_H_INCLUDED

#include <screen/screen.h>

#ifndef _WIN32
#include <sys/iomsg.h>
#include <_pack64.h>
#else
    #ifdef _MSC_VER
    typedef int pid_t;
    #endif
#endif

#ifndef _IOMGR_WINDOW
#define _IOMGR_WINDOW              (0x16)
#endif

#define WIN_MAX_MSG_SIZE           (2048)

#define WIN_MAX_GROUP_LEN          (64)

 /* The max number of devices supported */
#define WIN_MAX_DEVICE_COUNT        (8)

/*
 * This version number is used to detect version mismatches between the
 * screen API and screen. At every release, the number should be incremented.
 */
#define WIN_VERSION                 0x00000002

#define WIN_PATH_NAME               "/dev/screen"

#define WIN_IOMSG_SIZEOF(u_type)    (sizeof(io_msg_t) + sizeof(u_type))

/*
 * These are the commands accepted by screen.
 * For binary compatibility, commands should not be removed after being
 * given a command id. At a minimum, the command can be removed from this
 * enumeration as long as the command id isn't re-used for another command.
 */

enum {
	/*
	 * Connects to a client and creates an event queue. OpenKODE requires a
	 * seperate event queue for each thread that creates windows. Consequently,
	 * more connections can be created internally when a window is created from
	 * a different thread. All threads from the same process have access to the
	 * window in this case.
	 */
	WIN_CONNECT                         = 0x0001,

	/*
	 * Destroys an event queue and any window associated with a given
	 * connection. Internally, connections will be close whenever an
	 * application terminates abruptly. This may end up close multiple
	 * connections if several threads of the same process have their own event
	 * queue.
	 */
	WIN_DISCONNECT                      = 0x0002,

	/*
	 * Since WIN_FLUSH (blits=0 and wait_idle=1) causes an update to the
	 * screen, it cannot be used by an application that wants to synchronize
	 * with vsync. This request does just that. Keep the client reply blocked
	 * until the next vsync.
	 */
	WIN_WAIT_VSYNC                      = 0x0004,

	/*
	 * Blocks the calling thread until the requested event occurs. The event
	 * can either be the queuing of a front buffer, dequeuing of a back buffer,
	 * or the update of the frame buffer following a post. The message can be
	 * used by the window owner as a way of timing render times, for example,
	 * or by processes sharing buffer contents.
	 */
	WIN_WAIT_POST                       = 0x0005,

	/*
	 * Create a window. The window has no render buffers, and therefore cannot
	 * be visible until it has at least one buffer and one post was issued.
	 * A window can also become visible by associating it with another window
	 * that has buffers with one valid post. Separating window creation from
	 * full realization allows 1) to share buffers between windows, and 2) the
	 * application to control how many buffers are needed for the window.
	 */
	WIN_CREATE_WINDOW                   = 0x0006,

	WIN_SET_PROPERTY                    = 0x0007,
	WIN_GET_PROPERTY                    = 0x0008,
	WIN_REALIZE                         = 0x0009,
	WIN_DESTROY                         = 0x000A,
	WIN_GET_EVENT                       = 0x000B,
	WIN_SET_EVENT                       = 0x000C,
	WIN_POST                            = 0x000D,
	WIN_REF                             = 0x000E,
	WIN_CREATE_GROUP                    = 0x000F,
	WIN_JOIN_GROUP                      = 0x0010,
	WIN_LEAVE_GROUP                     = 0x0011,
	WIN_UNREALIZE                       = 0x0012,

	/*
	 * The dup request is usually used to copy content between windows. If the
	 * destination window doesn't have a stream with its own buffers, the
	 * content becomes shared between windows. In this case, only the original
	 * owner of the stream is allowed to post. When the destination window has
	 * its own stream of buffers, the contents of the front buffer are copied
	 * to the destination window's back buffer. This allows the application to
	 * follow the call with an eglSwapBuffers to make the contents visible.
	 */
	WIN_DUP                             = 0x0013,

	/*
	 * The holes request identifies rectangular regions that should be
	 * considered fully transparency by the composition manager. This can be
	 * used to optimize performance when compositing windows with large areas
	 * with nothing but transparency.
	 */
	WIN_HOLES                           = 0x0014,

	/*
	 * This is a multi-part request that can contain a mix of WIN_SET_PROPERTY
	 * and WIN_POST requests. This enables a number of property changes to a
	 * single window to be synchronized with content updates for that window
	 * (WIN_POST). To synchronize many operations with multiple WIN_POSTs, the
	 * lock mechanism must be used. WIN_TRANSACT can still be used in this case
	 * to reduce the number of messages sent to screen. The total size of
	 * a WIN_TRANSACT message should not exceed WIN_TRANSACT_MAX_SIZE.
	 */
	WIN_TRANSACT                        = 0x0015,

	WIN_CREATE_PIXMAP                   = 0x0016,
	WIN_BLIT                            = 0x0017,
	WIN_FLUSH                           = 0x0018,
	WIN_GRAB_EFFECT                     = 0x0019,

	WIN_SET_EFFECT_PROPERTY             = 0x0020,
	WIN_START_EFFECT                    = 0x0021,
	WIN_STOP_EFFECT                     = 0x0022,
	WIN_GET_MODELIST                    = 0x0023,
	WIN_CREATE_DEVICE                   = 0x0024,
};

/*
 * Two flags that are passed back to the client when buffers are allocated.
 * The interlaced flag indicates whether data will be interpreted as two
 * interlaced frames that are line interleaved. The second indicates whether
 * the buffer is physically contiguous or not.
 */
enum {
	WIN_IMAGE_FLAG_INTERLACED         = (1 << 0),
	WIN_IMAGE_FLAG_PHYS_CONTIG        = (1 << 1),
	WIN_IMAGE_FLAG_ATTACHED           = (1 << 2),
	WIN_IMAGE_FLAG_PHYS_OVERLAY       = (1 << 3),
	WIN_IMAGE_FLAG_PROTECTED          = (1 << 4),
	WIN_IMAGE_FLAG_NOALLOC            = (1 << 5),
	WIN_IMAGE_FLAG_EXPORT_DVADDR      = (1 << 6),
	WIN_IMAGE_FLAG_ROTATION_SUPPORT   = (1 << 7),
	WIN_IMAGE_FLAG_BOTTOM_FIELD_FIRST = (1 << 14),
	WIN_IMAGE_FLAG_UNCACHED_MAPPING   = (1 << 15),
};

enum {
	WIN_INT32                           = 0,
	WIN_INT64                           = 1,
	WIN_FLOAT32                         = 2,
};

union win_property_value {
    _Int8t      sz[1];
    _Int32t     i;
    _Int32t     ai[2];
    _Int64t     ll;
    void       *p;
	screen_display_mode_t mode;
};

struct win_state {
    _Uint8t     type;
    union {
        _Int32t i;
        _Int64t l;
        float   f;
    } u;
};

union win_event_data {
    struct {
        _Int32t          wid;
    } close;
    struct {
        _Uint32t         wid;
        _Int32t          pid;
        _Uint32t         type;
        _Int32t          len;
        _Int8t          *name;
    } create;
    struct {
        _Uint32t         id;
        _Int32t          pname;
    } property;
    struct {
        _Int32t          wid;
    } post;
    struct {
        _Int32t          index;
        struct win_state value;
    } input;
    struct {
        _Int32t          index;
        _Int32t          count;
    } jog;
    struct {
        _Uint32t         wid;
        _Int32t          index;
        _Int32t          select;
        _Uint32t         modifiers;
        _Int32t          x;
        _Int32t          y;
        _Int32t          screen_x;
        _Int32t          screen_y;
        _Int32t          wheel;
        _Int32t          hwheel;
    } pointer;
    struct {
        _Uint32t         wid;
        _Int32t          index;
        _Uint32t         seq_id;
        _Uint32t         flags;
        _Uint32t         modifiers;
        _Uint32t         key_cap;
        _Uint32t         key_scan;
        _Uint32t         key_sym;
    } keyboard;
    struct {
        _Int32t          id;
        _Int32t          attached_state;
        _Int32t          protection_state;
        _Int32t          mirrored;
        _Int32t          mode;
    } display;
    struct {
        _Int32t          data[4];
    } user;
    struct {
        _Uint32t         wid;
        _Uint32t         index;
        _Uint64t         timestamp;
        _Uint32t         seq_id;
        _Uint32t         contact_id;
        _Int32t          screen_x;
        _Int32t          screen_y;
        _Int32t          screen_width;
        _Int32t          screen_height;
        _Int32t          x;
        _Int32t          y;
        _Int32t          width;
        _Int32t          height;
        _Uint32t         orientation;
        _Uint32t         pressure;
        _Uint32t         contact_type;
        _Uint32t         select;
        _Uint32t         modifiers;
    } mtouch;
    struct {
        _Uint32t         id;
        _Uint32t         state;
    } idle_state;
    struct {
        _Int32t          wid;
    } unrealize;
    struct {
        _Int32t          effect;
    } effect_complete;
    struct {
        _Uint32t         wid;
        _Int32t          index;
        _Int32t          select;
        _Uint32t         modifiers;
        _Int16t          x;
        _Int16t          y;
        _Int16t          z;
        _Int16t          rx;
        _Int16t          ry;
        _Int16t          rz;
    } gamepad;
    struct {
        _Uint32t         wid;
        _Int32t          index;
        _Int32t          select;
        _Uint32t         modifiers;
        _Int16t          x;
        _Int16t          y;
        _Int16t          z;
    } joystick;
    struct {
        _Int32t          did;
        _Int32t          attached_state;
    } device;
};

/*
 * WIN_CONNECT
 */
struct win_connect_msg {
    _Uint32t version;
    _Int32t  flags;
};

struct win_connect_reply {
	_Int32t qid;
	_Int32t display_count;
	_Int32t display_ids[4];
	_Int32t device_count;
	_Int32t device_ids[WIN_MAX_DEVICE_COUNT];
};

/*
 * WIN_DISCONNECT
 */

/* (empty) */

/*
 * WIN_TERM
 */

/* (empty) */

/*
 * WIN_WAIT_VSYNC
 */

struct win_vsync_msg {
	_Uint32t disp_id;
};

/*
 * WIN_WAIT_POST
 */

struct win_wait_msg {
	_Uint32t id;
	_Uint32t flags;
};

/*
 * WIN_CREATE
 */

struct win_create_msg {
    _Uint32t type;
};

struct win_create_reply {
    _Uint32t id;
};

/*
 * WIN_SET_PROPERTY
 */
struct win_set_property_msg {
    _Uint32t                 id;
    _Uint32t                 pname;
    _Uint16t                 size;
    _Uint16t                 reserved[3];
    union win_property_value param;
};

/*
 * WIN_GET_PROPERTY
 */
struct win_get_property_msg {
    _Uint32t id;
    _Uint32t pname;
};

struct win_get_property_reply {
    _Uint32t                 pname;
    _Uint16t                 size;
    _Uint16t                 reserved;
    union win_property_value param;
};

/*
 * WIN_REALIZE
 */
struct win_realize_msg {
    _Uint32t      id;
    _Int16t       count;
    _Int16t       attach;
    struct {
        _Int16t   width;
        _Int16t   height;
        _Int32t   stride;
        _Int32t   format;
        _Int32t   flags;
        _Int32t   size;
        _Uint32t  offsets[3];
        void     *pointer;
        void     *handle;
    } buffers[0];
};

struct win_realize_reply {
    _Int16t       count;
    _Int16t       error;
    _Int16t       reserved[2];
    struct {
        _Int16t   width;
        _Int16t   height;
        _Int32t   stride;
        _Int32t   format;
        _Int32t   flags;
        _Int32t   size;
        _Uint32t  offsets[3];
        void     *pointer;
        void     *handle;
        _Uint64t  paddr;
    } buffers[0];
};

/*
 * WIN_DESTROY
 */
struct win_destroy_msg {
    _Uint32t id;
};

/*
 * WIN_GET_EVENT
 */
struct win_get_event_msg {
    _Uint64t timeout;
};

struct win_get_event_reply {
    _Int32t                  type;
    _Uint64t                 timestamp;
    union win_event_data     data;
};

/*
 * WIN_SET_EVENT
 */
struct win_set_event_msg {
    _Int32t                  pid;
    _Uint32t                 disp_id;
    _Int32t                  type;
    _Int64t                  timestamp;
    union win_event_data     data;
};

/*
 * WIN_POST
 */
struct win_post_msg {
    _Uint32t wid;
    _Uint32t flags;
    _Uint32t apicnt;
    _Uint32t drawcnt;
    _Uint32t tricnt;
    _Uint32t vtxcnt;
    _Uint32t teximg;
    _Uint32t subdata;
    _Uint32t cpu_time;
    _Uint32t gpu_time;
    _Int16t  idx;
    _Int16t  count;
    _Int16t  dirty_rects[0];
};

struct win_post_reply {
    _Int16t error;
    _Int16t count;
    _Int16t indices[0];
};

/*
 * WIN_REF
 */
struct win_ref_msg {
    _Uint32t wid;
    _Int32t  adjust;
};

/*
 * WIN_CREATE_GROUP, WIN_JOIN_GROUP, WIN_LEAVE_GROUP
 */

struct win_group_msg {
	_Uint32t id;
	_Int8t   name[0];
};

/*
 * WIN_UNREALIZE
 */
struct win_unrealize_msg {
	_Uint32t id;
};

/*
 * WIN_DUP
 */
struct win_dup_msg {
    _Uint32t child;
    _Uint32t parent;
    _Int32t  count;
    _Int32t  rect[4];
};

/*
 * WIN_HOLES
 */
struct win_holes_msg {
	_Uint32t wid;
	_Uint32t count;
	_Uint32t reserved[3];
	_Int16t  rects[0];
};

/*
 * WIN_BLIT
 */

struct win_blit_msg {
    _Uint32t  src_id;
    _Uint32t  src_bidx;
    _Int16t   sx;
    _Int16t   sy;
    _Int16t   swidth;
    _Int16t   sheight;
    _Uint32t  dst_id;
    _Uint32t  dst_bidx;
    _Int16t   dx;
    _Int16t   dy;
    _Int16t   dwidth;
    _Int16t   dheight;
    _Int16t   rotation;
    _Uint8t   global_alpha;
    _Uint8t   transparency;
    _Int8t    quality;
};

/*
 * WIN_FLUSH
 */

struct win_flush_msg {
	_Int32t blits;
	_Int32t wait_idle;
};

/*
 * WIN_GRAB_EFFECT
 */

struct win_grab_effect_msg {
    _Uint32t wid;
    _Uint32t effect;
};

/*
 * WIN_SET_EFFECT_PROPERTY
 */

struct win_set_effect_property_msg {
    _Uint16t    pname;
    union {
        _Int32t i;
        float   f;
    } params[8];
};

/*
 * WIN_START_EFFECT
 */

struct win_start_effect_msg {
    float    duration;
    _Int32t  notify;
    _Uint32t reserved[7];
};

/*
 * WIN_STOP_EFFECT
 */

struct win_stop_effect_msg {
    float    duration;
    _Int32t  notify;
    _Uint32t reserved[7];
};


/*
 * WIN_GET_MODELIST
 */

struct win_getmodelist_msg {
    _Uint32t	id;
    _Uint32t	reserved[7];
};
/*
 * WIN_TRANSACT
 */

/*
 * Each WIN_TRANSACT message has 'count' number of sub-messages. Each of these
 * messages start with the following header. The subtype can either be WIN_POST
 * or WIN_SET_PROPERTY. The data following the reserved field must be
 * interpreted as a win_post_msg or a win_set_property_msg depending on the
 * subtype.
 */
struct win_transact_part {
    _Int16t                                subtype;
    _Int16t                                size;
    _Int16t                                reserved[2];
    union {
        struct win_blit_msg                blit;
        struct win_destroy_msg             destroy;
        struct win_dup_msg                 dup;
        struct win_flush_msg               flush;
        struct win_get_property_msg        get_property;
        struct win_grab_effect_msg         grab_effect;
        struct win_group_msg               group;
        struct win_holes_msg               holes;
        struct win_post_msg                post;
        struct win_realize_msg             realize;
        struct win_set_effect_property_msg set_effect_property;
        struct win_set_property_msg        set_property;
        struct win_start_effect_msg        start_effect;
        struct win_stop_effect_msg         stop_effect;
        struct win_unrealize_msg           unrealize;
        struct win_getmodelist_msg         getmodelist;
    } u;
};

struct win_transact_msg {
    _Uint32t                 count;
    _Int32t                  reserved;
    struct win_transact_part parts[0];
};

/*
 * The following structures are the base for any message request sent to
 * screen. Not all messages require extra arguments, i.e. WIN_TERM.
 * In this case, the information contained in the header is sufficient.
 * Also, not all messages have replies.
 */

typedef struct {
    io_msg_t                     iomsg;
    union {
        struct win_connect_msg   connect;
        struct win_create_msg    create;
        struct win_get_event_msg get_event;
        struct win_ref_msg       ref;
        struct win_set_event_msg set_event;
        struct win_transact_msg  transact;
        struct win_vsync_msg     vsync;
        struct win_wait_msg      wait;
    } u;
} win_iomsg_t;

typedef struct {
    union {
        struct win_connect_reply      connect;
        struct win_create_reply       create;
        struct win_realize_reply      dup;
        struct win_get_event_reply    get_event;
        struct win_get_property_reply get_property;
        struct win_post_reply         post;
        struct win_realize_reply      realize;
    } u;
} win_iomsg_reply_t;

typedef struct {
	/**
	 ** The native image structure contains elements that are required by
	 ** drivers for proper allocation. It also contains fields that describe
	 ** the memory allocated. The following section contains those elements
	 ** which are inputs for drivers.
	 **/

    /*
     * The dimensions of the surface in pixels. These are inputs provided
     * to the allocator, blit function, 2D/3D driver, display controller.
     */
    int width, height;

    /*
     * The pixel format of the buffer/image. <screen/screen.h> contain the list
     * of pixel formats. This is an input required by the allocator.
     */
    int format;

    /*
     * The intended usage for the buffer. <screen/screen.h> contains the
     * possible usage bits. The usage can be a combination of any number
     * of these usage flags. This attribute is in input for the allocator.
     */
    int usage;

    /*
     * This item may contain both inputs and outputs. The interlacing flags
     * can be used by drivers for display controllers that support
     * de-interlacing. The physically contiguous flag is usually an output of
     * the allocation. The flags are also defined in <screen/screen.h>.
     */
    int flags;

	/**
	 ** The following elements contain the results of the allocation. The image
	 ** can only have one mapping. Different APIs will wrap the memory and
	 ** store a handle in the 'handles' structure. The display controller will
	 ** usually be asked to allocate an image first. If the image isn't
	 ** allocated by the driver, the compositing module will be required to
	 ** allocate images used by native windows. Blitting modules will be
	 ** required to allocate images used by native pixmaps if they aren't
	 ** allocated by the display controller driver.
	 **/

	/*
	 * If the image's memory was created via a shared object, then
	 * this is the file descriptor associated with it.
	 */
	int fd;

	/*
	 * The offset into the shared memory object that fd refers to.
	 */
	off64_t offset;

	/*
	 * The number of bytes allocated beyond vaddr/offset. The real size of the
	 * allocation is size + padding. This variable will be used when mapping
	 * the memory in the client's address space. There must be at least size
	 * bytes mapped starting at offset into the fd, or from vaddr.
	 */
	int size;

	/*
	 * The number of bytes allocated that precede vaddr or offset into the fd.
	 * This can be non-zero if the allocator requires a certain alignment for
	 * a given usage and padding is used to get that correct alignment.
	 */
	int padding;

    /*
     * The physical address of the buffer. In most cases, this will be
     * used by the display controller driver if the buffer is displayable.
     * This might also be used by 2D/3D drivers. The physical address is
     * an output of the allocator. The physical address will only be valid
     * when the surface is physically contiguous.
     */
    off64_t paddr;

    /*
     * The size of each row in bytes. This may be larger than width * bpp
     * to meet certain alignment constraints. The stride is an output of
     * the allocator. The second stride should be used when the orientation
     * has changed. When the usage flag includes the rotation bit, the
     * allocator must reserve enough memory for both orientations, i.e.
     * width x height and height x width. The application determines whether
     * it uses one orientation or the other.
     */
    int strides[2];

    /*
     * The virtual address to the buffer in the composition manager's
     * address space. This will most likely only be used by the software
     * compositing module. vaddr can be NULL if composition manager has
     * no need of a virtual address, i.e. everything runs accelerated.
     * The vaddr is an output of the allocator.
     */
    void *vaddr;

    /*
     * The virtual address in the client space. The pointer will be used
     * by applications using software to do their rendering like Flash or
     * Webkit. Although windows from different processes can share the
     * same window buffers, only the owner is allowed to post updates.
     * Otherwise, we would need a client virtual address per window. It
     * is not the responsibility of the allocator to provide the cvaddr.
     * The composition manager is responsible for creating that mapping
     * if required.
     */
    void *cvaddr;

	/*
	 * A device virtual address that can be used when GPUs have their own MMU.
	 * The value here may instead contain a pointer to a driver-specific handle
	 * that identifies the mapping.
	 */
	void *dvaddr;

	/*
	 * Planar formats are backed by one mapping like buffers with packed
	 * formats. The different planes can be located in the buffer at the
	 * following offsets. The index into the planar_offsets array for each
	 * plane depends on the format. The array will be filled with 0's for the
	 * remaining elements if the format has less than three planes.
	 */
	int planar_offsets[3];

	/*
	 * If the allocation is not physically contiguous, the allocator has the
	 * option to provide an array containing the physical addresses of all the
	 * pages allocated for the image. If non-NULL, the value must be at an
	 * array with at least (padding + size) / page size elements.
	 */
	off64_t *pages;

} win_image_t;

/*
 * Regions are used everywhere in the composition manager. The typically
 * define a rectangle for a blit or a fill. The same regions are also used
 * to store areas dirtied by updates.
 */
typedef struct win_region {
	/*
	 * A native region corresponds to a 2D rectangle defined by two points:
	 * (x1,y1) and (x2,y2). This defines the area [x1..x2[, [y1..y2[. The same
	 * edge (left egde) is used for both sides of the rectangle, which makes
	 * (x2,y2) non-inclusive. An integer type is used, but scaling may be
	 * applied. For example, screen uses a 24.8 fixed format internally for its
	 * rectangles and regions.
	 */
	int x1, y1, x2, y2;

	/*
	 * Regions also have the ability to be part of a list. This is the pointer
	 * to the next region in the list or NULL if this is the last element.
	 */
	struct win_region *next;

} win_region_t;

typedef struct win_stream {
	/*
	 * Corresponds to the number of buffers allocated for the stream. Since
	 * a buffer cannot be queued multiple times, there cannot be more elements
	 * in the consumer or producer queues than there are buffers. Therefore,
	 * nqels also represents the size of the consumer and producer queues.
	 */
	int8_t nqels;

	/*
	 * The index of the consumer buffer. The actual buffer is at qels[front].
	 * front is set to -1 when there are no consumer buffers. The convention
	 * is that the consumer will read data from the buffers and not modify it.
	 */
	int8_t front;

	/*
	 * The index of the producer buffer. The actual buffer is at qels[back].
	 * When there are more than two buffers in a stream, the producer buffer
	 * becomes more of a suggestion. Any buffer that is in the producer queue
	 * can be used. The convention is that the producer will write to buffers.
	 * Note that it may also require to read from it when performing blending
	 * operations, for example.
	 */
	int8_t back;

	/*
	 * In EGL, the swap interval is the minimum number of vsync periods between
	 * updates. In the composition manager, the swap interval determines when
	 * a stream will be checked for updates in the render thread. Swap
	 * intervals of 0 are special. They require the post to signal the update
	 * thread so that it unblocks immediately.
	 */
	uint8_t swap_interval;

	/*
	 * This callback will be called after an image has been produced, i.e.
	 * put in the consumer queue. If the stream end points are not in the same
	 * process, the callback must cause a reply or message to be sent, causing
	 * the other process to unblock.
	 */
	void (*callback)(struct win_stream *stream);

	/*
	 * A generic pointer that can be used to associate a native stream to
	 * additional data. This would normally be used by the consumer end of the
	 * stream when handling buffers being generated by the producer end,
	 * usually signaled by the producer calling the native stream callback.
	 */
	void *data;

	/*
	 * There should be nqels elements to this array. Each element truly belongs
	 * to a different array: the native buffers, the consumer queue, and the
	 * producer queue. All three arrays were combined in an array of structures
	 * to make the allocation of native streams easier.
	 */
	struct {
		win_image_t *image;

		struct {
			int8_t idx;
			win_region_t *dirty;
		} front;

		struct {
			int8_t idx;
			win_region_t *dirty;
		} back;

	} *qels;

} win_stream_t;

/*
 * The structure of data in the /dev/winmgr/blits file. The file is a
 * re-ordered version of the composition manager's ring buffer where each
 * element has the following fixed-size structure.
 */

typedef struct {
	/*
	 * The window id of the source. The destination is always the frame buffer.
	 * Solid color fills and blits from the checkerboard pattern are encoded
	 * with a wid set to ~0.
	 */
	int32_t wid;

	/*
	 * The source rectangle for the operation. Solid color fills have no source
	 * rectangle, i.e. sw and sh are set to 0. All other operations will have
	 * a fully specified source rectangle. Scaling is applied if the size of
	 * the source rectangle (sw, sh) doesn't correspond to the size of the
	 * destination rectangle (dw, dh).
	 */
	int16_t sx, sy, sw, sh;

	/*
	 * The destination rectangle for the operation. The destination rectangle
	 * is always fully specified.
	 */
	int16_t dx, dy, dw, dh;

	/*
	 * This field contains the last 16 bits of the frame counter. The frame
	 * counter is incremented each time the composition manager's update thread
	 * goes through the scene. This variable can be used to tell apart blits
	 * from two consecutive updates. Note that the update thread might not
	 * issue any blits for a frame, causing gaps in the frame numbers.
	 */
	uint16_t frame;

	/*
	 * The first 8 bits of the flags contain the global alpha value. Bits 8 and
	 * 9 correspond to the transparency mode. Bits 10 and 11 correspond to the
	 * scale quality. Finally, bit 12 is set if the source alpha mode is set to
	 * pre-multiplied alpha.
	 */
	int16_t flags;

} win_encoded_blit_t;

typedef struct {
	_Int16t          type;
	union {
		struct {
			_Int32t  id;
			_Int8t   state;
		} display;
		struct {
			_Int16t x;
			_Int16t y;
			_Int16t rx;
			_Int16t ry;
			_Int32t select;
		} gamepad;
		struct {
			_Int16t x;
			_Int16t y;
			_Int16t z;
			_Int16t select;
		} joystick;
		struct {
			_Uint16t seq_id;
			_Uint16t flags;
			_Uint16t modifiers;
			_Uint16t key_cap;
			_Uint16t key_scan;
			_Uint16t key_sym;
		} keyboard;
		struct {
			_Int16t  x;
			_Int16t  y;
			_Int16t  select;
			_Int16t  wheel;
			_Int16t  hwheel;
		} pointer;
		struct {
			_Uint16t seq_id;
			_Uint16t contact_id;
			_Int16t  x;
			_Int16t  y;
			_Uint8t  width;
			_Uint8t  height;
			_Uint8t  contact_type;
			_Uint8t  select;
		} mtouch;
	} data;
} win_encoded_input_t;

/*
 * All our window ids will have the MSB set. This allows us to detect quickly
 * if the window is ours, or a native window.
 */
#define WIN_MASK              0xF0000000U
#define WIN_TYPE(x)           ((x) & WIN_MASK)
#define WIN_TYPE_WINDOW       0x80000000U
#define WIN_TYPE_PIXMAP       0x40000000U
#define WIN_TYPE_DEVICE       0x30000000U
#define WIN_TYPE_DISPLAY      0x20000000U
#define WIN_TYPE_GROUP        0x10000000U

/*
 * Device specific formats are allowed as long as they add extra bits of
 * information on the upper 16-bit word of a known screen format. As far as
 * the application will be concerned, the format will be the more general
 * purpose screen format.
 */
#define WIN_FORMAT_MASK       0x0000ffffU
#define WIN_FORMAT(x)         ((x) & WIN_FORMAT_MASK)

#ifndef _WIN32
#include <_packpop.h>
#endif

#endif /* _SCREEN_IOMSG_H_INCLUDED */
