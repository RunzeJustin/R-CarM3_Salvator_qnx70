/*
 * $QNXLicenseC:
 * Copyright 2015, QNX Software Systems.
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

#include <stddef.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include <WF/wfd.h>

#include <wfdqnx/wfdcfg.h>
#include <wfdqnx/wfdcfg_rcardu.h>

#include <sys/slog.h>
#include <sys/slogcodes.h>

#define SLOG_DEBUG(x, ...)      //slogf(_SLOGC_GRAPHICS_DISPLAY, _SLOG_DEBUG1,  "[wfdcfg]DEBG: " x, ##__VA_ARGS__)
#define SLOG_INFO(x, ...)       slogf(_SLOGC_GRAPHICS_DISPLAY, _SLOG_INFO,    "[wfdcfg]INFO: " x, ##__VA_ARGS__)
#define SLOG_WARNING(x, ...)    slogf(_SLOGC_GRAPHICS_DISPLAY, _SLOG_WARNING, "[wfdcfg]WARN: " x, ##__VA_ARGS__)
#define SLOG_ERROR(x, ...)      slogf(_SLOGC_GRAPHICS_DISPLAY, _SLOG_ERROR,   "[wfdcfg]ERR : " x, ##__VA_ARGS__)

extern int clk_5p49_config(int du_index, int source_clk);

// Internal structure to keep a mode and its associated extension(s).
struct mode {
    const struct wfdcfg_timing timing;
    const struct wfdcfg_keyval *ext;
};

struct wfdcfg_device {
	const struct wfdcfg_keyval *ext_list;
};

struct wfdcfg_port {
	int id;
	const struct wfdcfg_keyval *ext_list;
	const struct mode *first_mode;
};

struct wfdcfg_mode_list {
    const struct wfdcfg_port *port;
    const struct wfdcfg_keyval *ext_list;
};


/* Helper functions */

static const struct mode*
cast_timing_to_mode(const struct wfdcfg_timing *timing)
{
	char *p = (char*)timing;
	if (p) {
		p -= offsetof(struct mode, timing);
	}
	return (const struct mode*)p;
}

static const struct wfdcfg_keyval* get_ext_from_list(const struct wfdcfg_keyval *ext_list, const char *key)
{
	while (ext_list) {
		if (!ext_list->key) {
			ext_list = NULL;
			break;
		} else if (strcmp(ext_list->key, key) == 0) {
		    return ext_list;
		}
		++ext_list;
	}
    return NULL;
}

/* Function extensions */
static int hdmi_ext_init(struct wfdcfg_port *port)
{
    /* We currently have nothing to do here */
    (void)port;
    return EOK;
}

static void hdmi_ext_cleanup(struct wfdcfg_port *port)
{
    /* We currently have nothing to do here */
    (void)port;
}

static int lvds_ext_init(struct wfdcfg_port *port)
{
    /* We currently have nothing to do here */
    (void)port;
    return EOK;
}

static void lvds_ext_cleanup(struct wfdcfg_port *port)
{
    /* We currently have nothing to do here */
    (void)port;
}

static int vga_ext_init(struct wfdcfg_port *port)
{
    /* We currently have nothing to do here */
    (void)port;
    return EOK;
}

static void vga_ext_cleanup(struct wfdcfg_port *port)
{
    /* We currently have nothing to do here */
    (void)port;
}

static const struct wfdcfg_keyval device_ext[] = {
    {   /* marks end of list */
        .key = NULL,
        .i = 0,
        .p = NULL,
    },
};

static const struct wfdcfg_keyval nc_port_ext[] = {
    {
        .key = WFDCFG_EXT_PORT_TYPE,
        .i = WFD_PORT_TYPE_OTHER,
        .p = NULL,
    },
    {   /* marks end of list */
        .key = NULL,
        .i = 0,
        .p = NULL,
    },
};

static const struct wfdcfg_keyval hdmi0_port_ext[] = {
    {
        .key = WFDCFG_EXT_FN_PORT_INIT1,
        .i = 0,
        .p = WFDCFG_FNPTR(&hdmi_ext_init, wfdcfg_ext_fn_port_init1_t*),
    },
    {
        .key = WFDCFG_EXT_FN_PORT_UNINIT1,
        .i = 0,
        .p = WFDCFG_FNPTR(&hdmi_ext_cleanup, wfdcfg_ext_fn_port_uninit1_t*),
    },
    {
        .key = WFDCFG_EXT_PORT_TYPE,
        .i = WFD_PORT_TYPE_HDMI,
        .p = NULL,
    },
    {
        .key = WFDCFG_OPT_PHYSPORT_TYPE,
        .i = RCAR_PHYSPORT_HDMI0,
        .p = NULL,
    },
    {   /* marks end of list */
        .key = NULL,
        .i = 0,
        .p = NULL,
    },
};

static const struct wfdcfg_keyval hdmi1_port_ext[] = {
    {
        .key = WFDCFG_EXT_FN_PORT_INIT1,
        .i = 0,
        .p = WFDCFG_FNPTR(&hdmi_ext_init, wfdcfg_ext_fn_port_init1_t*),
    },
    {
        .key = WFDCFG_EXT_FN_PORT_UNINIT1,
        .i = 0,
        .p = WFDCFG_FNPTR(&hdmi_ext_cleanup, wfdcfg_ext_fn_port_uninit1_t*),
    },
    {
        .key = WFDCFG_EXT_PORT_TYPE,
        .i = WFD_PORT_TYPE_HDMI,
        .p = NULL,
    },
    {
        .key = WFDCFG_OPT_PHYSPORT_TYPE,
        .i = RCAR_PHYSPORT_HDMI1,
        .p = NULL,
    },
    {   /* marks end of list */
        .key = NULL,
        .i = 0,
        .p = NULL,
    },
};

static const struct wfdcfg_keyval lvds_port_ext[] = {
    {
        .key = WFDCFG_EXT_FN_PORT_INIT1,
        .i = 0,
        .p = WFDCFG_FNPTR(&lvds_ext_init, wfdcfg_ext_fn_port_init1_t*),
    },
    {
        .key = WFDCFG_EXT_FN_PORT_UNINIT1,
        .i = 0,
        .p = WFDCFG_FNPTR(&lvds_ext_cleanup, wfdcfg_ext_fn_port_uninit1_t*),
    },
    {
        .key = WFDCFG_EXT_PORT_TYPE,
        .i = WFD_PORT_TYPE_OTHER,
        .p = NULL,
    },
    {
        .key = WFDCFG_OPT_PHYSPORT_TYPE,
        .i = RCAR_PHYSPORT_LVDS,
        .p = NULL,
    },
    {   /* marks end of list */
        .key = NULL,
        .i = 0,
        .p = NULL,
    },
};

static const struct wfdcfg_keyval vga_port_ext[] = {
    {
        .key = WFDCFG_EXT_FN_PORT_INIT1,
        .i = 0,
        .p = WFDCFG_FNPTR(&vga_ext_init, wfdcfg_ext_fn_port_init1_t*),
    },
    {
        .key = WFDCFG_EXT_FN_PORT_UNINIT1,
        .i = 0,
        .p = WFDCFG_FNPTR(&vga_ext_cleanup, wfdcfg_ext_fn_port_uninit1_t*),
    },
    {
        .key = WFDCFG_EXT_PORT_TYPE,
        .i = WFD_PORT_TYPE_COMPONENT_RGB,
        .p = NULL,
    },
    {
        .key = WFDCFG_OPT_PHYSPORT_TYPE,
        .i = RCAR_PHYSPORT_DPAD,
        .p = NULL,
    },
    {   /* marks end of list */
        .key = NULL,
        .i = 0,
        .p = NULL,
    },
};

static const struct wfdcfg_keyval external_fixed_clk_portmode_ext[] = {
    {
        .key = WFDCFG_EXT_PORTMODE_CLOCK_SOURCE,
        .i = RCAR_FIXED_EXTERNAL_CLOCK_SOURCE,
        .p = NULL,
    },
    {
        .key = WFDCFG_EXT_PORTMODE_EXTERNAL_CLOCK_RATE,
        .i = 33000000, //DU1,DU2 external clock rate
        .p = NULL,
    },
    {   /* marks end of list */
        .key = NULL,
        .i = 0,
        .p = NULL,
    },
};

static const struct wfdcfg_keyval external_configurable_clk_portmode_ext[] = {
    {
        .key = WFDCFG_EXT_PORTMODE_CLOCK_SOURCE,
        .i = RCAR_CONFIGURABLE_EXTERNAL_CLOCK_SOURCE,
        .p = NULL,
    },
    {
        .key = WFDCFG_EXT_FN_EXTCLK_CONFIG,
        .i = 0,
        .p = WFDCFG_FNPTR(&clk_5p49_config, wfdcfg_ext_fn_extclk_config_t*),
    },
    {   /* marks end of list */
        .key = NULL,
        .i = 0,
        .p = NULL,
    },
};

static const struct wfdcfg_keyval internal_clk_portmode_ext[] = {
    {
        .key = WFDCFG_EXT_PORTMODE_CLOCK_SOURCE,
        .i = RCAR_INTERNAL_CLOCK_SOURCE,
        .p = NULL,
    },
    {   /* marks end of list */
        .key = NULL,
        .i = 0,
        .p = NULL,
    },
};

static const struct mode lvds_modes[] = {
    {
        .timing = {  // 1280x800 @ 60 Hz
            .pixel_clock_kHz = 71107,
            .hpixels = 1280, .hbp = 48, .hsw = 32, .hfp = 80,  // 1440 total
            .vlines  =  800, .vbp =  2, .vsw =  6, .vfp = 15,  //  823 total
            .flags = WFDCFG_PREFERRED, // Mark this native resolution
        },
        .ext = external_configurable_clk_portmode_ext,
    },
    {
        // marks end of list
        .timing = {.pixel_clock_kHz = 0},
    },
};

static const struct mode hdmi_modes[] = {
	{
		.timing = {  // 1280x720 @ 60Hz
			.pixel_clock_kHz = 74250,
	        .hpixels = 1280, .hfp = 110, .hsw = 40, .hbp = 220, // 1650 total
	        .vlines  = 720,  .vfp = 5,   .vsw = 5,  .vbp = 20,  // 750 total
			.flags = 0 ,
		},
		.ext = external_configurable_clk_portmode_ext,
	},
	{
		.timing = { // 1920x1080 @ 60Hz
			.pixel_clock_kHz = 148500,
			.hpixels = 1920, .hfp = 88, .hsw = 44, .hbp = 148, // 2200 total
			.vlines  = 1080, .vfp =  4, .vsw =  5, .vbp =  36, // 1125 total
			.flags = WFDCFG_PREFERRED, // Mark this native resolution
		},
		.ext = external_configurable_clk_portmode_ext,
	},
	{
		// marks end of list
		.timing = {.pixel_clock_kHz = 0},
	},
};

static const struct mode vga_modes[] = {
	{
		.timing = { // 1024x768 @ 60Hz
			.pixel_clock_kHz = 64996,
			.hpixels = 1024, .hfp = 24, .hsw = 136, .hbp = 160, // 1344 total
			.vlines  =  768, .vfp =  3, .vsw =   6, .vbp =  29, //  806 total
			.flags = 0,
		},
		.ext = external_configurable_clk_portmode_ext,
	},
	{
		.timing = {  // 1280x720 @ 60 Hz
			.pixel_clock_kHz = 74250,
			.hpixels = 1280, .hfp=110, .hsw= 40, .hbp=220,  // 1650 total
			.vlines  =  720, .vfp=  5, .vsw=  5, .vbp= 20,  //  750 total
			.flags = 0 ,
		},
		.ext = external_configurable_clk_portmode_ext,
	},
	/* Common resolution for VGA monitors */
	{
		.timing = {  // 1366x768 @ 60 (59.789) Hz, standard timing
			.pixel_clock_kHz = 85500,
			.hpixels = 1366, .hfp= 70, .hsw=143, .hbp=213,  // 1792 total
			.vlines  =  768, .vfp=  3, .vsw=  3, .vbp= 24,  //  798 total
			.flags = 0 ,
		},
		.ext = external_configurable_clk_portmode_ext,
	},
	{
		// marks end of list
		.timing = {.pixel_clock_kHz = 0},
		.ext = NULL,
	},
};

int wfdcfg_device_create(struct wfdcfg_device **device, int deviceid, const struct wfdcfg_keyval *opts)
{
	int err = EOK;
	struct wfdcfg_device *tmp_dev = NULL;
	(void)opts;

	switch(deviceid) {
		case 1:
			tmp_dev = malloc(sizeof(*tmp_dev));
			if (!tmp_dev) {
				err = ENOMEM;
				goto end;
			}
			tmp_dev->ext_list = device_ext;
			break;

		default:
			/* Invalid device id*/
			err = ENOENT;
			goto end;
	}

end:
	if (err) {
		free(tmp_dev);
	} else {
		*device = tmp_dev;
	}
	return err;
}

const struct wfdcfg_keyval* wfdcfg_device_get_extension(const struct wfdcfg_device *device, const char *key)
{
	return get_ext_from_list(device->ext_list, key);
}

void
wfdcfg_device_destroy(struct wfdcfg_device *device)
{
	free(device);
}

int
wfdcfg_port_create(struct wfdcfg_port **port,
		const struct wfdcfg_device *device, int portid,
		const struct wfdcfg_keyval *opts)
{
	int err = EOK;
	struct wfdcfg_port *tmp_port = NULL;
    const struct wfdcfg_keyval* key = NULL;

	if (!device) {
	    return ENODEV;
	}
	if (!portid){
	    return ENOENT;
	}

    tmp_port = malloc(sizeof *tmp_port);
    if (!tmp_port) {
        err = ENOMEM;
        goto end;
    }
    tmp_port->id = portid;
    tmp_port->ext_list = nc_port_ext;

    if (opts) { // Check if caller provides us port map
        key = (struct wfdcfg_keyval*)get_ext_from_list(opts, WFDCFG_OPT_BOUND_DU_INDEX);
    }
    if (key) {
	    switch (key->i){ // Set port modes, extension list base on bound DU index
	    case DU_CH_0:
            tmp_port->ext_list = lvds_port_ext;
            tmp_port->first_mode = &lvds_modes[0];
            SLOG_INFO("Display %d -> DU%d -> LVDS", portid, (int)key->i);
            break;
	    case DU_CH_1:
        	tmp_port->ext_list = hdmi0_port_ext;
            tmp_port->first_mode = &hdmi_modes[0];
        	SLOG_INFO("Display %d -> DU%d -> HDMI0", portid, (int)key->i);
            break;
	    case DU_CH_2:
			tmp_port->ext_list = vga_port_ext;
			tmp_port->first_mode = &vga_modes[0];
			SLOG_INFO("Display %d -> DU%d -> VGA", portid, (int)key->i);
            break;
        default:
            SLOG_ERROR("Display %d -> Invalid bound DU%d. This port is not usable or not exist", portid,(int)key->i);
            err = ENOENT;
            break;
        }
    }
    else
    {
    	// TODO: No port map, we can set port modes, extension list base on portID
        SLOG_ERROR("No port map for display %d", portid);
        err = ENOENT;
    }

end:
	if (err) {
	    free(tmp_port);
	} else {
	    *port = tmp_port;
	}

	return err;
}

const struct wfdcfg_keyval*
wfdcfg_port_get_extension(const struct wfdcfg_port *port, const char *key)
{
	return get_ext_from_list(port->ext_list, key);
}

void
wfdcfg_port_destroy(struct wfdcfg_port *port)
{
	free(port);
}

int
wfdcfg_mode_list_create(struct wfdcfg_mode_list **list,
		const struct wfdcfg_port* port, const struct wfdcfg_keyval *opts)
{
	int err = EOK;
	struct wfdcfg_mode_list *tmp_mode_list = NULL;

	(void)opts;

	if (!port) {
	    return ENODEV;
	}

    tmp_mode_list = malloc(sizeof *tmp_mode_list);
    if (!tmp_mode_list) {
        err = ENOMEM;
        goto out;
    }
    *tmp_mode_list = (struct wfdcfg_mode_list){
        .port = port,
        .ext_list = NULL,
    };

out:
    if (err) {
        free(tmp_mode_list);
    } else {
        *list = tmp_mode_list;
    }
    return err;
}

const struct wfdcfg_keyval*
wfdcfg_mode_list_get_extension(const struct wfdcfg_mode_list *mode_list,
		const char *key)
{
	return get_ext_from_list(mode_list->ext_list, key);
}

void
wfdcfg_mode_list_destroy(struct wfdcfg_mode_list *mode_list)
{
	free(mode_list);
}

const struct wfdcfg_timing*
wfdcfg_mode_list_get_next(const struct wfdcfg_mode_list *list,
		const struct wfdcfg_timing *prev_timing)
{
	if (!list) {
	    return NULL;
	}

	const struct mode *m = list->port->first_mode;
	if (prev_timing) {

		m = cast_timing_to_mode(prev_timing) + 1;
	}

	if (m->timing.pixel_clock_kHz == 0) {
		// end of list (this is not an error)
		m = NULL;
	}
	return (m ? &m->timing : NULL);
}

const struct wfdcfg_keyval*
wfdcfg_mode_get_extension(const struct wfdcfg_timing *timing,
		const char *key)
{
	const struct wfdcfg_keyval *ext = cast_timing_to_mode(timing)->ext;
	return get_ext_from_list(ext, key);
}
