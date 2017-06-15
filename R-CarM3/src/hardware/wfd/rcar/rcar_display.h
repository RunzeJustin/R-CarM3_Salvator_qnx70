/*
 * $QNXLicenseC:
 * Copyright 2014-2016, QNX Software Systems. 
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

#ifndef __RCARDU_H__
#define __RCARDU_H__

#include <graphics/display.h>
#include <graphics/disputil.h>
#include <pthread.h>
#include <sys/cache.h>
#include <sys/queue.h>
#include <sys/slog.h>
#include <sys/slogcodes.h>
#include <WF/wfd.h>
#include <WF/wfdext.h>
#include <KHR/khronos_utils.h>

#include "macros.h"
#include "rcardu.h"

/* local header file includes bottom of file */

/* DEBUG & verbose control 
 * undefine TC_DEBUG_ON to remove all verbose at compile time
 */
//#define RCARDU_DEBUG_ON
#if defined(RCARDU_DEBUG_ON)
	#define TRACE do { slogf(_SLOGC_GRAPHICS_DISPLAY, _SLOG_DEBUG2, "TRACE: %s()", __FUNCTION__); } while (0)
	#define SLOG_DEBUG(x, ...)      slogf(_SLOGC_GRAPHICS_DISPLAY, _SLOG_DEBUG1,  "[rcardu]DEBG: " x, ##__VA_ARGS__)
	/* For fine debug message, trace, etc */
	#define SLOG_DEBUG2(x, ...)      slogf(_SLOGC_GRAPHICS_DISPLAY, _SLOG_DEBUG2,  "[rcardu]DBG2: " x, ##__VA_ARGS__)
	#define DEBUG_CMD(x)  x
#else
	#define TRACE
	#define SLOG_DEBUG(x, ...)
	#define SLOG_DEBUG2(x, ...)
	#define DEBUG_CMD(x)
#endif

#define SLOG_INFO(x, ...) 		slogf(_SLOGC_GRAPHICS_DISPLAY, _SLOG_INFO,    "[rcardu]INFO: " x, ##__VA_ARGS__)
#define SLOG_WARNING(x, ...) 	slogf(_SLOGC_GRAPHICS_DISPLAY, _SLOG_WARNING, "[rcardu]WARN: " x, ##__VA_ARGS__)
#define SLOG_ERROR(x, ...) 		slogf(_SLOGC_GRAPHICS_DISPLAY, _SLOG_ERROR,   "[rcardu]ERR : " x, ##__VA_ARGS__)

#define DEVICE_MAGIC \
(((unsigned)'r'<<24)|((unsigned)'c'<<16)|((unsigned)'r'<<8)|(unsigned)'d')

#define SOURCE_MAGIC \
(((unsigned)'r'<<24)|((unsigned)'c'<<16)|((unsigned)'r'<<8)|(unsigned)'s')

#define LOCK_DEVICE()        pthread_mutex_lock(&dev->mutex)
#define UNLOCK_DEVICE()      pthread_mutex_unlock(&dev->mutex)

#ifdef NDEBUG
#define LOG_ERROR_LOCKED(err) \
{ \
    if (dev->error == WFD_ERROR_NONE) { \
        dev->error = err; \
    } \
    pthread_mutex_unlock(&dev->mutex);  \
}
#else /* NDEBUG */
#define LOG_ERROR_LOCKED(err) \
{ \
    fprintf(stderr, "WFD error [0x%04x] @%s:%d\n", err, __FILE__, __LINE__); \
    if (dev->error == WFD_ERROR_NONE) { \
        dev->error = err; \
    } \
    pthread_mutex_unlock(&dev->mutex);  \
}
#endif /* NDEBUG */

#define LOG_ERROR(err) \
{ \
     pthread_mutex_lock(&dev->mutex);  \
     LOG_ERROR_LOCKED(err); \
}

#define UP_SCALE_MAX	16	/* Currently, no scale support */
#define DOWN_SCALE_MAX	16
#define NO_SCALE		1

#define RCARDU_POSIX_TYPED_MEM_PATH "/memory/below4G"

#define LUT_MIN_GAMMA (0.01f)
#define LUT_DEF_GAMMA (1.00f)
#define LUT_MAX_GAMMA (7.99f)

enum {
    WFD_DEVICE_CHANGES_PORT                     = (1 << 0),
    WFD_DEVICE_CHANGES_PIPELINE                 = (1 << 1),
};

enum
{ WFD_PORT_CHANGES_MODE                         = (1 << 0),
  WFD_PORT_CHANGES_BACKGROUND_COLOR             = (1 << 1),
  WFD_PORT_CHANGES_FLIP                         = (1 << 2),
  WFD_PORT_CHANGES_MIRROR                       = (1 << 3),
  WFD_PORT_CHANGES_ROTATION                     = (1 << 4),
  WFD_PORT_CHANGES_GAMMA                        = (1 << 5),
  WFD_PORT_CHANGES_POWER_MODE                   = (1 << 6),
  WFD_PORT_CHANGES_PARTIAL_REFRESH              = (1 << 7),
  WFD_PORT_CHANGES_PARTIAL_REFRESH_RECT         = (1 << 8),
  WFD_PORT_CHANGES_PROTECTION_ENABLE            = (1 << 9),
  WFD_PORT_CHANGES_PIPELINE                     = (1 << 10),
};

enum {
    WFD_PIPELINE_CHANGES_BIND                   = (1 << 0),
    WFD_PIPELINE_CHANGES_SOURCE                 = (1 << 1),
    WFD_PIPELINE_CHANGES_RECT                   = (1 << 2),
    WFD_PIPELINE_CHANGES_SOURCE_RECTANGLE       = (1 << 3),
    WFD_PIPELINE_CHANGES_DESTINATION_RECTANGLE  = (1 << 4),
};

#define RCARDU_MAX_NUMBER_EVENTS 10

enum
{
    RCARDU_LAYER1_ID = 1,
    RCARDU_LAYER2_ID = 2,
    RCARDU_LAYER3_ID = 3,
    RCARDU_LAYER4_ID = 4,
    RCARDU_LAYER5_ID = 5,
    RCARDU_LAYER6_ID = 6,
    RCARDU_LAYER7_ID = 7,
    RCARDU_LAYER8_ID = 8,
    RCARDU_LAYER9_ID = 9,
    RCARDU_LAYER10_ID = 10,
    RCARDU_LAYER11_ID = 11,
    RCARDU_LAYER12_ID = 12,
    RCARDU_LAYER13_ID = 13,
    RCARDU_LAYER14_ID = 14,
    RCARDU_LAYER15_ID = 15,
    RCARDU_LAYER16_ID = 16,
    RCARDU_LAYER17_ID = 17,
    RCARDU_LAYER18_ID = 18,
    RCARDU_LAYER19_ID = 19,
    RCARDU_LAYER20_ID = 20,
};

#define RCARDU_MAX_PORT_MODES 5

enum {
    RCARDU_LAYER1 = 0,
    RCARDU_LAYER2 = 1,
    RCARDU_LAYER3 = 2,
    RCARDU_LAYER4 = 3,
    RCARDU_LAYER5 = 4,
    RCARDU_LAYER6 = 5,
    RCARDU_LAYER7 = 6,
    RCARDU_LAYER8 = 7,
    RCARDU_LAYER9 = 8,
    RCARDU_LAYER10 = 9,
    RCARDU_LAYER11 = 10,
    RCARDU_LAYER12 = 11,
    RCARDU_LAYER13 = 12,
    RCARDU_LAYER14 = 13,
    RCARDU_LAYER15 = 14,
    RCARDU_LAYER16 = 15,
	RCARDU_LAYER17 = 16,
	RCARDU_LAYER18 = 17,
	RCARDU_LAYER19 = 18,
	RCARDU_LAYER20 = 19,
    RCARDU_MAX_NUMBER_PIPELINES
};

#define RCARDU_MAX_NUMBER_PIPELINES_PER_PORT 5

enum
{
    RCARDU_WFD_PORT1_ID = 1,
    RCARDU_WFD_PORT2_ID = 2,
    RCARDU_WFD_PORT3_ID = 3,
    RCARDU_WFD_PORT4_ID = 4,	
};

enum
{
    RCARDU_WFD_PORT1 = 0,
    RCARDU_WFD_PORT2 = 1,
    RCARDU_WFD_PORT3 = 2,
    RCARDU_WFD_PORT4 = 3,	
    RCARDU_WFD_MAX_NUMBER_OF_PORTS
};

typedef struct {
	void 	*(*sub_hw_init)(void *device, void *disp);
	void	(*activate_pipeline)(void *device, void *pipe);
	void	(*deactivate_pipeline)(void *device, void *pipe);
	void	(*frame_update)(void *device, void *pipe);
	void	(*sub_hw_fini)(void *device, void *disp);
	void 	(*sub_hw_frame_sync)(void *device);
	uint32_t composited_pbuffer;
	uint8_t *composited_vbuffer;
} compose_hdl;

typedef struct _du_cfg
{
	uint32_t	priv_phy_base;
	uint32_t 	reg_off;
	uintptr_t 		priv_vir_base;
	uintptr_t 		grp_vir_base;
	int				irq;
	int				source_clk;
	int				clk_source_mode;
	const struct sigevent* (*isr)(void *arg, int id);
	int				grp_index;	
	int 			du_index;
	int 			type;
	int				pipe_ids[RCARDU_MAX_NUMBER_PIPELINES_PER_PORT];
	int				use_planes[RCARDU_MAX_NUMBER_PIPELINES_PER_PORT];
	int				pipe_num;
	int				scale_pipe;
	compose_hdl		*hw_compose;
	int				ComposeHwId;
	int				ExScaleHwId;
	void 			*compose_dev;
	void 			*scaling_dev;
	void			(*ext_clk_config)(int channel,int clock);
} du_cfg_t;

typedef struct _graphics_conf {
	int usable;
	int xres;
	int yres;
	int refresh_rate;
	int pipeline;
} graphics_conf_t;

typedef struct _event
{
    int created;

    WFDEventType type;
    WFDint attached_portId;
    WFDboolean attached_state;
    WFDint port_protectionId;
    WFDint pipeId;
    WFDSource bind_source;
    WFDMask bind_mask;
    WFDboolean bind_queue_overflow;
} event_t;

typedef struct
{
    struct
    {
        int magic;
        int version;
    } hdr;
    WFDEGLImage     image;
    disp_surface_t* surf;
    void*           vaddr;
    int             bpp;
    int             pixel_format;
} source_t;

typedef struct _portmode_t
{
    STAILQ_ENTRY(_portmode_t) list_entry;
    const struct wfdcfg_timing *timings;
    /* mode attributes */
    WFDfloat            refresh;
    WFDint              mirror;
    WFDint              rotation_support;
    WFDint              interlaced;
} portmode_t;

typedef struct _port_t
{
    int                         portId;
	void						*enc;
    unsigned int                display_mode;
    unsigned int                bgcolor;
    unsigned int                attached;
    unsigned int                detachable;
    volatile unsigned int       vsync_counter;
    volatile unsigned int       update_vsync;
    int                         isr_pulse;

    WFDPortType                 type;
    unsigned int                native_width;
    unsigned int                native_height;
    float                       physical_width;
    float                       physical_height;
    enum rcar_physport_type     physport_type;
    float                       gamma_range_min;
    float                       gamma_range_max;
    float                       gamma;
    unsigned int                power_mode;
	int							default_pipeline;
	float 						actual_refresh;
	unsigned int				pixel_clk;
	
    portmode_t*                 active_mode;
    STAILQ_HEAD(, _portmode_t)  modelist;
    struct wfdcfg_port*         cfglib_port;
    struct wfdcfg_mode_list*    cfglib_modelist;
    unsigned int                bindablesSize;
    unsigned int                bindables[RCARDU_MAX_NUMBER_PIPELINES_PER_PORT];
    WFDint                      changes;
    WFDboolean                  created;

    /* pipes bound to port */
    unsigned int                bound_pipesSize;
    struct _pipe_t*             bound_pipes[RCARDU_MAX_NUMBER_PIPELINES_PER_PORT];

	du_cfg_t					*du_cfg;
	
	int							irqid;
    struct sigevent             irqevent;
    struct intrspin             irqspin;
	int							irqchan;
	int							irqcoid;
    pthread_mutex_t             vsync_mutex;
    pthread_cond_t              vsync_cv;
	unsigned					want_vsync_pulse;
	volatile uint32_t			vsync_done;
	uint32_t					dl_phy;
	uint8_t						*dl_vrt;
} port_t;

typedef struct
{
	int					cur_idx;
	uint32_t		obuffer_phys[2];
	uint8_t				*obuffer[2];
    void                *dev;
	int					last_dst_width;
	int					last_dst_height;
} scale_t;

typedef struct _pipe_t
{
    int                 pipeId;
    int                 layer;
    int                 plane;
    struct _port_t*     port;
    struct _port_t*     bound_port;
    WFDEGLImage         src;
    WFDTransition       src_transition;
    WFDEGLImage         bound_src;
    WFDint              src_rect[4];
    WFDint              dst_rect[4];
    WFDRect             region;
    WFDboolean          shareable;
    WFDboolean          direct_refresh;
    WFDRotationSupport  rotation_support;
    WFDint              rotation;

    WFDint              width, height;
    WFDint              changes;
    WFDboolean          enabled;
    WFDboolean          created;
	
    uint32_t            source_color;
    uint32_t            mask;
    int                 global_alpha_active;
    int                 global_alpha;
    int                 transparency;
    int                 chroma_enabled;
    int                 filter;
    int                 interlaced;
	scale_t				scale;
} pipe_t;

typedef struct {
	int32_t cx;
	int32_t cy;
} SIZE;

typedef struct {
	/* Configurable options from .conf file */	
	struct timing_cfg *timing;
	unsigned		xres;
	unsigned		yres;
	unsigned		portId;
	unsigned		portNum;
	unsigned		portType;
} channel_t;

typedef struct _du_dev_t
{
    struct
    {
        int magic;
        int version;
    } hdr;

    WFDErrorCode        error;
    WFDint              changes;
    pthread_mutex_t		mutex;

    int                 chip_type;
    int                 chip_revision;
	uintptr_t			du_vir_reg_base;

    int                 free_context;
    struct wfdcfg_device* cfglib_device;

    unsigned            portsSize;
    port_t              ports[RCARDU_WFD_MAX_NUMBER_OF_PORTS];
    unsigned            pipesSize;
    pipe_t              pipes[RCARDU_MAX_NUMBER_PIPELINES];
    unsigned            eventSize;
    event_t             events[RCARDU_MAX_NUMBER_EVENTS];

    int  				max_width;
    int  				max_height;
} du_dev_t;

/* Internal function prototypes */

const struct sigevent* rcardu_du0_isr(void *arg, int id);
const struct sigevent* rcardu_du1_isr(void *arg, int id);
const struct sigevent* rcardu_du2_isr(void *arg, int id);
const struct sigevent* rcardu_du3_isr(void *arg, int id);

int parse_device_config(du_dev_t *dev);
int rcardu_init(du_dev_t *dev);
port_t * find_port_from_channel (du_dev_t *dev, int chan);
void rcardu_fini(du_dev_t *dev);
void port_init(du_dev_t *dev, port_t *port);
void du0_du1_plane_setting (du_dev_t *dev, port_t *port);
int lvds_init(void *arg);
int hdmi_init(void *arg);
int hdmi_fini(void *arg);
int vga_init(void *arg);
void bind_display_to_port (du_dev_t *dev);
int get_plane_from_pipeId (port_t *port, int pipeId);
int timing_lookup (port_t *port);
int get_config_data(du_dev_t *dev, const char *filename);
int rcardu_wait_vsync(du_dev_t *dev, port_t *port);
void scale_pipe_init(du_dev_t* dev, pipe_t* pipe);
void scale_pipe_quit(du_dev_t *dev, pipe_t* pipe);

int wfdCommitPortUpdates(du_dev_t *dev, port_t *port);

int wfdCommitPipelineUpdates(du_dev_t *dev, pipe_t *pipe);

int get_chip_info(du_dev_t* dev);

#endif


