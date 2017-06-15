/*
 * $QNXLicenseC:
 * Copyright 2013-2015, QNX Software Systems.
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

#ifndef HDRINCL_WFDCFG_RCARDU
#define HDRINCL_WFDCFG_RCARDU

#include <wfdqnx/wfdcfg.h>

#define DU_CH_0		0
#define DU_CH_1		1
#define DU_CH_2		2
#define DU_CH_3		3

/**
 * Returns port type.  This is a port extension.
 *  .p is NULL.
 *  .i is WFDPortType enum value.
 */
#define WFDCFG_EXT_PORT_TYPE             "port_type"

/**
 * Sets port operation mode. wfdcfg should check port argument to ensure
 * type of output instead of port id. wfdcfg should not assume that particular
 * port id has some particular type. wfdcfg must know detailed configuration
 * of board and limitation.
 *
 *  .p is NULL.
 *  .i is one of the rcar_physport_type enum values.
 */
#define WFDCFG_OPT_PHYSPORT_TYPE         "physport_type"

enum rcar_physport_type {
    RCAR_PHYSPORT_UNKNOWN, // Unknown video port, treat it as generic as possible
    RCAR_PHYSPORT_LVDS,
    RCAR_PHYSPORT_HDMI0, // HDMI
    RCAR_PHYSPORT_HDMI1,
    RCAR_PHYSPORT_DPAD, // Digital RGB output with frame control signals
};

enum rcar_clock_source {
    RCAR_INTERNAL_CLOCK_SOURCE,
    RCAR_FIXED_EXTERNAL_CLOCK_SOURCE,
    RCAR_CONFIGURABLE_EXTERNAL_CLOCK_SOURCE //Some board have Programmable Clock Generator chip as external clock
};

/**
 * Sets clock source mode for particular video mode.
 *
 *  .p is NULL
 *  .i is one of the rcar_clock_source enum values.
 */
#define WFDCFG_EXT_PORTMODE_CLOCK_SOURCE "clock_source"


/**
 * Defines frequency of external clock source.
 *
 *  .p is NULL.
 *  .i is frequency of clock source mode in Hz.
 */
#define WFDCFG_EXT_PORTMODE_EXTERNAL_CLOCK_RATE "external_clock_rate"

/**
 * This option key help wfdcfg_port_create know which DU channel bound to current display
 *  .p is NULL.
 *  .i is bound DU index of current portId.
 */
#define WFDCFG_OPT_BOUND_DU_INDEX        "bound_du_index"

/**
 * External clock configuration hook.
 * This extension function use for configuring external lock.
 * Some boards use external (configurable) source clock, wfdcfg have to
 * provide function pointer to configure this clock source.
 *  .p is a pointer to a function of type wfdcfg_ext_fn_extclk_config_t
 *      (returns EOK on success or another errno code on failure)
 *  .i must be zero
 */
#define WFDCFG_EXT_FN_EXTCLK_CONFIG "extclk_config"
typedef int (wfdcfg_ext_fn_extclk_config_t)(int channel, int clock);

#endif /* HDRINCL_WFDCFG_RCARDU included */

