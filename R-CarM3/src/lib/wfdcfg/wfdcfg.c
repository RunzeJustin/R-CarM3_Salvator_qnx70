/*
 * $QNXLicenseC:
 * Copyright 2013, QNX Software Systems.
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

#include <wfdqnx/wfdcfg.h>

#include <stddef.h>
#include <assert.h>
#include <errno.h>
#include <stdlib.h>  // NULL
#include <string.h>

/* Internal structure to keep a mode and
 * its associated extension(s).
 * */
struct mode {
	const struct wfdcfg_timing timing;
	const struct wfdcfg_keyval *ext_list;
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

/* Helper function(s) */
static const struct mode*
cast_timing_to_mode(const struct wfdcfg_timing *timing)
{
	char *p = (char*)timing;
	if (p) {
		p -= offsetof(struct mode, timing);
	}
	return (const struct mode*)p;
}

static const struct wfdcfg_keyval*
get_ext_from_list(const struct wfdcfg_keyval *ext_list, const char *key)
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

#ifdef VARIANT_sample
static const struct wfdcfg_keyval device_ext[] = {
	{ "port_ext_example", .i = 1 },
	{ NULL }  // marks end of list
};

static const struct wfdcfg_keyval port_ext[] = {
	{ "device_ext_example", .i = 1 },
	{ NULL }  // marks end of list
};

static const struct mode sample_timings[] = {
	{
		// 800x480 @ 60 Hz
		.timing = {
			.pixel_clock_kHz =  29760,
			.hpixels =  800, .hfp= 24, .hsw= 72, .hbp= 96,  //  992 total
			.vlines  =  480, .vfp=  3, .vsw= 10, .vbp=  7,  //  500 total
			.flags = WFDCFG_INVERT_HSYNC,
		},
		.ext_list = (const struct wfdcfg_keyval[]){
			{ "ext_1_example", .i = 1 },
			{ "ext_2_example", .i = 2 },
			{ NULL }  // marks end of list
		},
	},
	{
		// 1024x768 @ 60 Hz (CVT)
		.timing = {
			.pixel_clock_kHz =  63500,
			.hpixels = 1024, .hfp= 48, .hsw=104, .hbp=152,  // 1328 total
			.vlines  =  768, .vfp=  3, .vsw=  4, .vbp= 23,  //  798 total
			.flags = WFDCFG_INVERT_VSYNC,
		},
		.ext_list = NULL,
	},
	{
		// 1280x1024 @ 60 Hz (CVT)
		.timing = {
			.pixel_clock_kHz = 109000,
			.hpixels = 1280, .hfp= 88, .hsw=128, .hbp=216,  // 1712 total
			.vlines  = 1024, .vfp=  3, .vsw=  7, .vbp= 29,  // 1063 total
			.flags = WFDCFG_INVERT_VSYNC,
		},
		.ext_list = NULL,
	},
	{
		// 1080p @ 60 Hz (1920x1080)
		.timing = {
			.pixel_clock_kHz = 148500,
			.hpixels = 1920, .hfp= 88, .hsw= 44, .hbp=148,  // 2200 total
			.vlines  = 1080, .vfp=  4, .vsw=  5, .vbp= 36,  // 1125 total
			.flags = 0,
		},
		.ext_list = NULL,
	},
	{
		// 720p @ 60 Hz (1280x720)
		.timing = {
			.pixel_clock_kHz =  74250,
			.hpixels = 1280, .hfp=110, .hsw= 40, .hbp=220,  // 1650 total
			.vlines  =  720, .vfp=  5, .vsw=  5, .vbp= 20,  //  750 total
			.flags = 0,
		},
		.ext_list = NULL,
	},
	{
		// marks end of list
		.timing = {.pixel_clock_kHz = 0},
	},
};
#endif

int
wfdcfg_device_create(struct wfdcfg_device **device, int deviceid,
		const struct wfdcfg_keyval *opts)
{
	int err = EOK;
	struct wfdcfg_device *tmp_dev = NULL;
	(void)opts;

	switch(deviceid) {
#ifdef VARIANT_sample
		case 1:
			tmp_dev = malloc(sizeof *tmp_dev);
			if(!tmp_dev) {
				err = ENOMEM;
				goto end;
			}
			*tmp_dev = (struct wfdcfg_device){
				.ext_list = device_ext,
			};
			break;
#endif
		default:
			/* Invalid device id*/
			err = ENOENT;
			goto end;
	}

end:
	if(err) {
		free(tmp_dev);
	} else {
		*device = tmp_dev;
	}

	return err;

}

const struct wfdcfg_keyval*
wfdcfg_device_get_extension(const struct wfdcfg_device *device,
		const char *key)
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
	(void)opts;

	assert(device);

	switch(portid) {
		default:
#ifdef VARIANT_sample
			// allow any nonzero port ID
			tmp_port = malloc(sizeof *tmp_port);
			if(!tmp_port) {
				err = ENOMEM;
				goto end;
			}
			*tmp_port = (struct wfdcfg_port){
				.id = portid,
				.ext_list = port_ext,
				.first_mode = &sample_timings[0],
			};
			break;
#endif
		case 0:
			/* Invalid port id*/
			err = ENOENT;
			goto end;
	}

end:
	if(err) {
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
wfdcfg_mode_list_create(struct wfdcfg_mode_list **mode_list,
		const struct wfdcfg_port* port, const struct wfdcfg_keyval *opts)
{
	int err = 0;
	struct wfdcfg_mode_list *tmp_mode_list = NULL;

	(void)opts;
	assert(port);

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
		*mode_list = tmp_mode_list;
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
wfdcfg_mode_list_get_next(const struct wfdcfg_mode_list *mode_list,
		const struct wfdcfg_timing *prev_timing)
{
	assert(mode_list);

	const struct mode *m = mode_list->port->first_mode;
	if (prev_timing) {
		m = cast_timing_to_mode(prev_timing) + 1;
	}

	if (m && m->timing.pixel_clock_kHz == 0) {
		m = NULL;  // end of list
	}
	return m ? &m->timing : NULL;
}

const struct wfdcfg_keyval*
wfdcfg_mode_get_extension(const struct wfdcfg_timing *timing,
		const char *key)
{
	const struct wfdcfg_keyval *ext = cast_timing_to_mode(timing)->ext_list;
	return get_ext_from_list(ext, key);
}

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/6.6.0/trunk/lib/wfdcfg/wfdcfg.c $ $Rev: 743813 $")
#endif
