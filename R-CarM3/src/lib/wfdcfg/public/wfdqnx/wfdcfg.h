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

/**
*   @file wfdcfg.h
*   @brief Definitions, constants and functions for the Wfdcfg library
*
*   This file defines definitions, constants and functions that are supported by
*   the Wfdcfg library.
*/
/**
*   @mainpage Wfdcfg library overview
*
*   The Wfdcfg library provides the modes and attributes of your display
*   hardware to your display driver and to Screen.
*
*   Your display driver (WFD driver) is the primary user of the Wfdcfg library,
*   but the composition manager component in Screen also uses the modes and
*   attributes from the Wfdcfg library.
*/

#ifndef HDRINCL_WFDCFG
#define HDRINCL_WFDCFG

#ifndef __PLATFORM_H_INCLUDED
#include <sys/platform.h>
#endif

/**
* @brief Macro used to validate the type of a function pointer
*        at compile time
*/
#if defined __GNUC__
#define WFDCFG_FNPTR(FN,TYP) __builtin_choose_expr(   \
  __builtin_types_compatible_p(__typeof__(FN), TYP),  \
  (FN), (void)0 /*cause an error*/ )
#else
#define WFDCFG_FNPTR(FN,TYP) (FN)
#endif

/**
* @brief Opaque data type representing a device
* @details This device is an abstraction of a display controller that supports
*          one or more ports and zero or more pipelines. The device may be
*          associated with device-extensions.
*/
struct wfdcfg_device;

/**
* @brief Opaque data type representing a display port
* @details This port is usually associated with an ID and a list of modes
*          that include timing and optional associated mode-extensions.
*/
struct wfdcfg_port;

/**
* @brief Opaque data type representing a list of video modes
* @details A mode list is usually associated with a port and its port-extensions.
*/
struct wfdcfg_mode_list;

/**
* @brief Structure that describes the video timing parameters for the
*        display driver settings
*/
struct wfdcfg_timing {
	_Uint32t pixel_clock_kHz;   /**< Frequency (in kHz) that pixels are transmitted at */
	_Uint32t hpixels;           /**< Width (in pixels) of the display */
	_Uint32t vlines;            /**< Height (in lines) of the display */
	_Uint16t hsw;               /**< Width (in pixels) of horizontal sync pulse*/
	_Uint16t vsw;               /**< Width (in lines) of vertical sync pulse */
	_Uint16t hfp;               /**< Horizontal front porch (in pixels) */
	_Uint16t vfp;               /**< Vertical front porch (in lines) */
	_Uint16t hbp;               /**< Horizontal back porch (in pixels) */
	_Uint16t vbp;               /**< Vertical back porch (in lines) */
	_Uint32t flags;             /**< Bitmask of @c #wfdcfg_flags values */
};

/**
* @brief Flags used in the @c flags field of @c #wfdcfg_timing
*/
enum wfdcfg_flags {
/**
* @brief Helper macro to set bit assignment for each enumerator of @c #wfdcfg_flags
*/
#	define _b(x) ((_Uint32t)1 << (x))

	WFDCFG_INVERT_HSYNC =_b(0),        /**< Use an active-high horizontal sync pulse*/
	WFDCFG_INVERT_VSYNC =_b(1),        /**< Use an active-high vertical sync pulse */
	WFDCFG_INVERT_DATA_EN =_b(2),      /**< Invert the "data enable" signal */
	WFDCFG_INVERT_CLOCK =_b(3),        /**< Invert the pixel clock signal */
	WFDCFG_INVERT_DATA =_b(4),         /**< Invert data */
	WFDCFG_INVERT_HV_SYNC_RF =_b(5),   /**< Drive HSYNC and VSYNC on the opposite edge of the pixel clock */

	WFDCFG_INTERLACE =_b(8),           /**< Use interlacing */
	WFDCFG_DOUBLESCAN =_b(9),          /**< Enable scanline doubling */
	WFDCFG_BOTTOM_FIRST =_b(11),       /**< Bottom field first for interlacing */

	WFDCFG_PREFERRED =_b(31),          /**< Use as default mode */

#	undef _b
};

/**
* @brief Power modes for display
* @details Some power modes may not be possible for your specific display hardware.
*          Recovery time to @c WFDCFG_POWER_MODE_ON decreases from
*          @c WFDCFG_POWER_MODE_OFF to @c WFDCFG_POWER_MODE_SUSPEND to
*          @c WFDCFG_POWER_MODE_LIMITED_USE and the power consumption increases.
*/
enum wfdcfg_power_mode {
	WFDCFG_POWER_MODE_OFF            = 0x7680, /**< No power - frames lost */
	WFDCFG_POWER_MODE_SUSPEND        = 0x7681, /**< Faster recovery than @c #WFDCFG_POWER_MODE_OFF */
	WFDCFG_POWER_MODE_LIMITED_USE    = 0x7682, /**< Frames maintained in hardware */
	WFDCFG_POWER_MODE_ON             = 0x7683, /**< Fully operational */
};

/**
 * @brief Array(s) of this structure are used to allow for extensions
 *
 * @details Extensions can exist on each of the following:
 *          - device: access these via @c wfdcfg_device_get_extension().
 *          - port: access these via @c wfdcfg_port_get_extension().
 *          - mode: access these via @c wfdcfg_mode_get_extension().
 *          - mode_list: access these via @c wfdcfg_mode_list_get_extension().
 *
 *          Several Wfdcfg library functions take this structure as an optional
 *          argument; certain drivers pass data to the Wfdcfg library using this
 *          interface.
 *
 *          The @c i and @c p members of @c this structure depend on the key
 *          (when part of an array, an element with @c key==NULL marks the end of
 *          that array). Unused @c i and/or @c p fields should be set to 0 or NULL.
 */
struct wfdcfg_keyval {
	const char *key;   /**< Identifier of extension */
	long i;            /**< Data associated with extension */
	void *p;           /**< Data associated with extension */
};

/* Common extensions */
/**
 * @brief A port extension that describes the physical size (in millimetres) of
 *         the display port
 * @details Members of @c #wfdcfg_keyval structure used for
 *          this extension are used as follows:
 *          - @c p: pointer to an array of float (float size[2]={width, height};)
 *          - @c i: set to 0
 */
#define WFDCFG_EXT_PHYS_SIZE_MM "phys_size_mm"

/* Common function extensions */
/**
 *  @brief A port extension function that's called to set the power mode on a
 *         port
 *  @details Members of @c #wfdcfg_keyval structure used for
 *           this extension are used as follows:
 *           - @c p: pointer to the function
 *           - @c i: set to 0
 */
#define WFDCFG_EXT_FN_SET_POWER_MODE "set_power_mode"

/**
 *  @brief A port extension function that's called to set the power mode on a
 *         port
 *
 *  @param port A handle to the display port on which the power mode is to be set
 *  @param power_mode The power mode (of type @c #wfdcfg_power_mode) that the
 *                    display port is to be set to
 *
 *  @return A code from @c errno.h; possible codes include:
 *          - @c EINVAL when a invalid power mode is passed
 *          - @c EOK on success
 */
typedef int (wfdcfg_ext_fn_set_power_mode_t)(struct wfdcfg_port *port, enum wfdcfg_power_mode power_mode);

/**
 *  @brief Create a wfdcfg device
 *
 *  @details There must be at least one device created. Otherwise, the OpenWF
 *           display driver will report an error.
 *
 *  @param device A handle to the device that is to be created
 *  @param deviceid The identification number of the OpenWF device
 *  @param opts An array of optional parameters that is terminated by
 *              @c .key=NULL
 *
 *  @return 0 if device was successfully created; @c *device is set to an
 *          opaque pointer. A code from errno.h if device failed to be created;
 *          @c *device remains unchanged. Possible error codes include:
 *          - @c ENOMEM: Unable to allocate a device
 *          - @c ENOENT: Invalid/Unknown device ID
 */
int
wfdcfg_device_create(struct wfdcfg_device **device, int deviceid,
	const struct wfdcfg_keyval *opts);

/**
 *  @brief Retrieve an extension identified by a key (string) from a device
 *
 *  @details The extension is valid between the time you create and destroy
 *           the device.
 *
 *  @param device A handle to the device whose extension(s) you are retrieving
 *  @param key Identifier of extension to retrieve
 *
 *  @return Pointer to @c #wfdcfg_keyval if the extension was found;
 *          NULL if the extension was not found. It's
 *          considered acceptable for a device to have no extensions.
 */
const struct wfdcfg_keyval*
wfdcfg_device_get_extension(const struct wfdcfg_device *device,
                            const char *key);

/**
 *  @brief Destroy a wfdcfg device
 *
 *  @details Memory allocated that was allocated by create @c wfdcfg_device_create()
 *           is released. The device's extension pointers are not valid after
 *           calling this function.
 *
 *  @param device A handle to the device to be destroyed; if NULL, no action
 *                will be taken
 *
 *  @return Nothing.
 */
void
wfdcfg_device_destroy(struct wfdcfg_device *device);

/**
 * @brief Create a wfdcfg port
 *
 * @details There must be at least one port created. Otherwise, the OpenWF
 *          display driver will report an error.
 *
 *  @param port A handle to the port that is to be created
 *  @param device A handle to the device that is associated with the port to be
 *                created
 *  @param portid The identification number of the OpenWFD port
 *  @param opts An array of optional parameters that is terminated by
 *              @c .key=NULL
 *
 *  @return 0 if port was successfully created; @c *port is set to an
 *          opaque pointer.  A code from errno.h if device failed to be created;
 *          @c *port remains unchanged. Possible error codes include:
 *          - @c ENOMEM: Unable to allocate a port
 *          - @c ENOENT: Invalid/Unknown port ID
 */
int
wfdcfg_port_create(struct wfdcfg_port **port,
	const struct wfdcfg_device *device, int portid,
	const struct wfdcfg_keyval *opts);

/**
 *  @brief Retrieve an extension identified by a key (string) from a port.
 *
 *  @details The extension is valid between the time you create and destroy
 *           the port.
 *
 *  @param port A handle to the port whose extension(s) you are retrieving
 *  @param key Identifier of the extension to retrieve
 *
 *  @return Pointer to @c #wfdcfg_keyval if the extension was found;
 *          NULL if the extension was not found. It's
 *          considered acceptable for a port to have no extensions.
 */
const struct wfdcfg_keyval*
wfdcfg_port_get_extension(const struct wfdcfg_port *port, const char *key);

/**
 *  @brief Destroy a @c wfdcfg port.
 *
 *  @details Memory allocated that was allocated by create wfdcfg_port_create()
 *           is released. The port's extension pointer is not valid after
 *           calling this function.
 *
 *  @param port A handle to the device to be destroyed; if NULL, no action
 *              will be taken
 *
 *  @return Nothing.
 */
void
wfdcfg_port_destroy(struct wfdcfg_port *port);

/**
 * @brief Create a list of video modes associated with specified port
 *
 * @details Once created, use @c wfdcfg_mode_list_get_next() to retrieve
 *          mode (timing) entries in the list.
 *
 *  @param list A handle to the list to be created
 *  @param port The port associated with the list to be created
 *  @param opts An array of optional parameters that is terminated by
 *              @c .key=NULL
 *
 *  @return 0 if port was successfully created; @c *list is set to an
 *          opaque pointer. A code from errno.h if device failed to be created;
 *          @c *list remains unchanged. Possible error codes include:
 *          - @c ENOMEM: Unable to allocate a list
 *          - @c ENOENT: Invalid port
 */
int
wfdcfg_mode_list_create(struct wfdcfg_mode_list **list,
	const struct wfdcfg_port *port, const struct wfdcfg_keyval *opts);

/**
 *  @brief Retrieve an extension identified by a key (string) from the specified
 *         list of video modes
 *
 *         The extension is valid between the time you create and destroy
 *         the list.
 *
 *  @param list A handle to the list whose extension(s) you are retrieving
 *  @param key Identifier of the extension to retrieve
 *
 *  @return Pointer to @c #wfdcfg_keyval if the extension was found;
 *          NULL if the extension was not found. It's
 *          considered acceptable for a list to have no extensions.
 */
const struct wfdcfg_keyval*
wfdcfg_mode_list_get_extension(const struct wfdcfg_mode_list *list,
	const char *key);

/**
 *  @brief Destroy a list of video modes
 *
 *         Memory allocated that was allocated by create @c wfdcfg_mode_list_create()
 *         is released. The list's extension pointer is not valid after
 *         calling this function.
 *
 *  @param list A handle to the list to be destroyed; if NULL, no action
 *              will be taken.
 *
 *  @return Nothing.
 */
void
wfdcfg_mode_list_destroy(struct wfdcfg_mode_list *list);

/**
 *  @brief Retrieve a mode (timing) from the specified list of video modes
 *
 *  @details Timing is mandatory component of a mode. The mode may also include
 *           extensions. If @c prev_timing is NULL, this function
 *           returns the first mode (timing) in the specified list. Otherwise,
 *           this function returns the next mode (timing) in the list after
 *           @c prev_timing.
 *
 *  @param list A handle to the list to retrieve the mode from
 *  @param prev_timing A handle to the mode (timing) in the list that precedes
 *                     the one to be retrieved
 *
 *  @return A pointer to the mode (@c #wfdcfg_timing) in the list that follows
 *          the argument @c prev_timing; a return value of NULL indicates that
 *          the end of the list has been reached.
 */
const struct wfdcfg_timing*
wfdcfg_mode_list_get_next(const struct wfdcfg_mode_list *list,
	const struct wfdcfg_timing *prev_timing);

/**
 *  @brief Retrieve an extension from the specified mode
 *
 *  @param mode A handle to the mode (timing) whose extension(s) you are
 *              retrieving
 *  @param key Identifier of extension to retrieve
 *
 *  @return Pointer to @c #wfdcfg_keyval if the extension was found;
 *          NULL if the extension was not found. It's
 *          considered acceptable for a list to have no extensions.
 */
const struct wfdcfg_keyval*
wfdcfg_mode_get_extension(const struct wfdcfg_timing *mode,
	const char* key);


/**
 * Port initialisation hook.
 * A WFD driver may choose to implement extension functions for port mode setting.
 * A driver that does implement it will call the extension function when
 * a port is created
 *  .p is a pointer to a function of type wfdcfg_ext_fn_port_init1_t
 *      (which returns EOK on success or another errno code on failure)
 *  .i must be zero
 */
#define WFDCFG_EXT_FN_PORT_INIT1 "port_init1"
typedef int (wfdcfg_ext_fn_port_init1_t)(struct wfdcfg_port*);

/**
 * Port deinitialisation hook.
 * A WFD driver may choose to implement extension functions for port mode setting.
 * A driver that does implement it will call the extension function when
 * a port is destroyed.
 *  .p is a pointer to a function of type wfdcfg_ext_fn_port_uninit1_t
 *  .i must be zero
 */
#define WFDCFG_EXT_FN_PORT_UNINIT1 "port_uninit1"
typedef void (wfdcfg_ext_fn_port_uninit1_t)(struct wfdcfg_port*);

/**
 * Port modesetting hook.
 * A WFD driver may choose to implement extension functions for port mode setting.
 * A driver that does implement it will call the extension function when
 * programming a mode.
 *  .p is a pointer to a function of type wfdcfg_ext_fn_port_set_mode_t
 *      (which takes a pointer to the mode being programmed, and
 *      returns EOK on success or another errno code on failure)
 *  .i must be zero
 */
#define WFDCFG_EXT_FN_PORT_SET_MODE2 "port_set_mode2"
typedef int (wfdcfg_ext_fn_port_set_mode2_t)(struct wfdcfg_port*,
		const struct wfdcfg_timing*);


#endif // HDRINCL_WFDCFG

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/6.6.0/trunk/lib/wfdcfg/public/wfdqnx/wfdcfg.h $ $Rev: 778064 $")
#endif
