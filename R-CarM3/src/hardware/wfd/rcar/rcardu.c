/*
 * $QNXLicenseC:
 * Copyright 2014-2015, QNX Software Systems. 
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

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <malloc.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <dlfcn.h>
#include <hw/inout.h>
#include <hw/sysinfo.h>
#include <stdarg.h>
#include <string.h>
#include <sys/slog.h>
#include <sys/slogcodes.h>
#include <atomic.h>
#include <math.h>
#include <screen/screen.h>
#include <screen/iomsg.h>

#include "rcar_display.h"
#include "vsp_drv.h"
#include "wfd_common.h"

extern du_cfg_t du_cfg_list[];
extern void *exscale_init (void *disp);

SYM_INTERNAL_ONLY int get_chip_info(du_dev_t* dev)
{
    uint32_t *prr_reg;
    const char* infostr_product="";
    const char* infostr_revision="";

    prr_reg = mmap_device_memory(NULL, RCAR_PRR_REGSIZE,
        PROT_READ | PROT_WRITE | PROT_NOCACHE, MAP_SHARED, RCAR_PRR);

    if (prr_reg == MAP_FAILED) {
        return ENOMEM;
    }

    switch ((*prr_reg >> RCAR_PRODUCT_SHIFT) & RCAR_PRODUCT_MASK) {
        case RCAR_PRODUCT_H3:
             dev->chip_type = RCAR_PRODUCT_H3;
             infostr_product = "R-CarH3";
             break;
        case RCAR_PRODUCT_M3W:
             dev->chip_type = RCAR_PRODUCT_M3W;
             infostr_product = "R-CarM3-W";
             break;
        default:
             SLOG_ERROR("Did not detect a supported chip type: prr=%02X", (*prr_reg >> RCAR_PRODUCT_SHIFT) & RCAR_PRODUCT_MASK);
             raise(SIGABRT);
    }

    switch ((*prr_reg >> RCAR_REVISION_SHIFT) & RCAR_REVISION_MASK) {
        case RCAR_REVISION_ES10:
             dev->chip_revision = RCAR_REVISION_ES10;
             infostr_revision = "Revision ES1.0";
             break;
        case RCAR_REVISION_ES11:
             dev->chip_revision = RCAR_REVISION_ES11;
             infostr_revision = "Revision ES1.1";
             break;
        case RCAR_REVISION_ES20:
             dev->chip_revision = RCAR_REVISION_ES20;
             infostr_revision = "Revision ES2.0";
             break;
        case RCAR_REVISION_ES30:
             dev->chip_revision = RCAR_REVISION_ES30;
             infostr_revision = "Revision ES3.0";
             break;
        default:
             SLOG_ERROR("Did not detect a supported revision of chip: prr=%02X", (*prr_reg >> RCAR_REVISION_SHIFT) & RCAR_REVISION_MASK);
             raise(SIGABRT);
    }

    munmap_device_memory(prr_reg, RCAR_PRR_REGSIZE);

    SLOG_INFO("Detected platform: %s, %s", infostr_product, infostr_revision);

    return EOK;
}

int
parse_device_config(du_dev_t *dev)
{
	char filename[256];
	const char *graphics_root = NULL;
    TRACE;

	graphics_root = getenv("GRAPHICS_ROOT");
	if (!graphics_root) {
	    SLOG_ERROR("GRAPHICS_ROOT environment variable not found");
		return 0;
	}
	snprintf(filename, sizeof(filename), "%s/graphics.conf", graphics_root);
	return (get_config_data(dev, filename));
}

static inline struct sigevent* du_isr (void *arg, int id)
{
	port_t *port = arg;
	/* Clear the interrupt */
	if (!(*DSSR & (1<<11))) {
		/* Not our interrupt */
		return NULL;
	}
	*DSRCR = DSRCR_VBCL;
	atomic_add(&port->vsync_counter, 1);
	
	if (port->du_cfg && port->du_cfg->hw_compose->sub_hw_frame_sync && port->du_cfg->compose_dev)
		port->du_cfg->hw_compose->sub_hw_frame_sync (port->du_cfg->compose_dev);
	
	if (port->want_vsync_pulse) {
		port->want_vsync_pulse = 0;
		return &port->irqevent;
	}
	return NULL;
}


const struct sigevent* rcardu_du0_isr(void *arg, int id)
{
	return du_isr(arg,id);
}

const struct sigevent* rcardu_du1_isr(void *arg, int id)
{
	return du_isr(arg,id);
}

const struct sigevent* rcardu_du2_isr(void *arg, int id)
{
	return du_isr(arg,id);
}

const struct sigevent* rcardu_du3_isr(void *arg, int id)
{
	return du_isr(arg,id);
}

int
rcardu_init(du_dev_t *dev)
{
	int i;
	port_t *port = dev->ports;
	pthread_mutexattr_t attr;
	pthread_mutexattr_init (&attr);
	
    TRACE;

    ThreadCtl(_NTO_TCTL_IO, 0);

	dev->du_vir_reg_base = mmap_device_io (DU_REGSIZE, DU_BASE);
	if (dev->du_vir_reg_base==-1)
	{
	    SLOG_ERROR("Unable mapping. Initialization has been failed");
		return -1;
	}

	for (i=0;i<dev->portsSize;i++)
	{
		port[i].du_cfg->grp_vir_base = dev->du_vir_reg_base + GROUP_GAP * port[i].du_cfg->grp_index;
		port[i].du_cfg->priv_vir_base = dev->du_vir_reg_base + port[i].du_cfg->reg_off;
		
		port[i].irqchan = ChannelCreate(0);
		port[i].irqcoid = ConnectAttach(0, 0, port[i].irqchan, _NTO_SIDE_CHANNEL, 0);

		port[i].irqid = InterruptAttach(port[i].du_cfg->irq,
			port[i].du_cfg->isr, (void *)&port[i], sizeof (port[i]), _NTO_INTR_FLAGS_TRK_MSK |_NTO_INTR_FLAGS_END | _NTO_INTR_FLAGS_PROCESS);
		if (port[i].irqid != -1)
			SIGEV_PULSE_INIT(&port[i].irqevent, port[i].irqcoid, SIGEV_PULSE_PRIO_INHERIT, R_CAR_DU_VSYNC_PULSE, 0);
		else
		{
			ConnectDetach(port[i].irqcoid);
			ChannelDestroy(port[i].irqchan);
		}

		/*initialize mutexes*/
		pthread_mutex_init(&port[i].vsync_mutex,&attr);
	}
	SLOG_DEBUG("Initialization done.");
    return 0;
}

port_t * find_port_from_channel (du_dev_t *dev, int index)
{
	int i;
	port_t *port = dev->ports;
	
	for (i=0;i<dev->portsSize;i++)
	{
		if (port[i].du_cfg)
		{
			if (port[i].du_cfg->du_index == index)
			{
				return &port[i];
			}
		}
	}
	return NULL;
}

void rcardu_fini(du_dev_t *dev)
{
	int i;
    TRACE;
	port_t *port = dev->ports;
	for (i=0;i<dev->portsSize;i++)
	{
		*DSYSR = DSYSR_DRES;
		switch (port->physport_type){
			case RCAR_PHYSPORT_LVDS:
				//TODO: Implement lvds_fini
				break;
			case RCAR_PHYSPORT_HDMI0:
				hdmi_fini(port);
				break;
			case RCAR_PHYSPORT_HDMI1:
				hdmi_fini(port);
				break;
			case RCAR_PHYSPORT_DPAD:
				//TODO: Implement dpad_fini
				break;
			default:
				SLOG_ERROR("rcardu_fini: Unsupported physical port type: %d",port->physport_type);
				break;
		}
        InterruptDetach(port->irqid);
        ConnectDetach(port->irqcoid);
        ChannelDestroy(port->irqchan);
		
		if (port->du_cfg->hw_compose->composited_vbuffer)
			munmap(port->du_cfg->hw_compose->composited_vbuffer,
					port->active_mode->timings->hpixels*port->active_mode->timings->vlines*4);
					
		if (port->du_cfg->hw_compose->sub_hw_fini)
			port->du_cfg->hw_compose->sub_hw_fini(dev,(void *)port);
		
		pthread_mutex_destroy(&port->vsync_mutex);
		port++;
	}
	munmap_device_io((uintptr_t)dev->du_vir_reg_base, DU_REGSIZE);
}

uint32_t gen3_du_pll_setting (port_t *port, uint32_t ideal_dclk, uint32_t extclk, uint32_t *dpll_n, uint32_t *dpll_m, uint32_t *dpll_fdpll)
{
	uint32_t dpllclk;
	uint32_t best_clk = 0;
	uint32_t diff, last_diff;
	uint32_t n, m, fdpll;
	int match_flag = 0;
	int clk_diff_set = 1;
	
	for (n = 39; n < 120; n++) {
		for (m = 0; m < 4; m++) {
			for (fdpll = 1; fdpll < 32; fdpll++) {
				dpllclk = extclk * (n + 1) / (m + 1)
						 / (fdpll + 1) / 2;
				if (dpllclk >= 400000000)
					continue;	
				
				diff = abs((int32_t)dpllclk - (int32_t)ideal_dclk);
				if (clk_diff_set ||
					((diff == 0) || (last_diff > diff))) {
					last_diff = diff;
					*dpll_n = n;
					*dpll_m = m;
					*dpll_fdpll = fdpll;
					best_clk = dpllclk;
					
					if (clk_diff_set)
						clk_diff_set = 0;

					if (diff == 0) {
						match_flag = 1;
						break;
					}
				}
			}
			if (match_flag)
				break;
		}
		if (match_flag)
			break;
	}
	
	/* Actual value of DPLL output would be double */
	return best_clk*2;
}

void output_route_setting (du_dev_t *dev, port_t *port)
{
	/* Hardcode for superposition setting */
	*DORCR = 	DORCR_PG2T 		| 	/* Display timing 1 -> pin controller 1 */
				DORCR_DK2S 		| 	/* Dotclock generator 1 -> display timing 1 */
				DORCR_PG2D_DS2 	| 	/* Data from superposition processor 1 are input to pin controller 1 */
				DORCR_DPRS; 		/* Use DS0PR or DS1PR for superposition */
				
	*DPTSR = 	DPTSR_PnDK(2) 	| 	/* Plane 3 uses Dotclock generator 1/3 */
				DPTSR_PnTS(2);	 	/* Plane 3 uses Display timing generator 1/3 */
}

void du_timing_setting (du_dev_t *dev, port_t *port)
{
	int hsw,hbp,hfp,hpixels,vsw,vbp,vfp,vlines,hc,vc;
	float refresh_rate, dclk;
	int escr_divisor = 1;
	int escr_clock_bit;

    uint32_t dpll_n, dpll_m, ideal_dclk, best_clock, dpllcr=0;
    uint32_t dpll_fdpll;
	
	/* Get DU timing setting */
	hsw = port->active_mode->timings->hsw;
	hbp = port->active_mode->timings->hbp;
	hfp = port->active_mode->timings->hfp;
	vsw = port->active_mode->timings->vsw;
	vbp = port->active_mode->timings->vbp;
	vfp = port->active_mode->timings->vfp;
	hpixels = port->active_mode->timings->hpixels;
	vlines = port->active_mode->timings->vlines;
    hc = hsw + hbp + hfp + hpixels - 1;
    vc = vsw + vbp + vfp + vlines - 1;
	refresh_rate = port->active_mode->refresh;

    dclk = (hc+1)*(vc+1)*refresh_rate;
    ideal_dclk = port->active_mode->timings->pixel_clock_kHz*1000;

	escr_clock_bit = (port->du_cfg->clk_source_mode == RCAR_INTERNAL_CLOCK_SOURCE)?1:0;

    SLOG_INFO("DU%d %dx%d @ %.2f %d, %s %d",
            port->du_cfg->du_index,hpixels,vlines,refresh_rate,ideal_dclk,
            (port->du_cfg->clk_source_mode == RCAR_FIXED_EXTERNAL_CLOCK_SOURCE)?"EXTERNAL":
            (port->du_cfg->clk_source_mode == RCAR_CONFIGURABLE_EXTERNAL_CLOCK_SOURCE)?"EXTERNAL.CFG":
            (port->du_cfg->clk_source_mode == RCAR_INTERNAL_CLOCK_SOURCE)?"INTERNAL":"UNKNOWN",
            port->du_cfg->source_clk);

	/* Select Input Dot Clock Source */
	*DIDSR = 		DIDSR_CODE |
					DIDSR_PDCS_CLK(1, 0) |
					DIDSR_PDCS_CLK(0, 0);

	if (port->du_cfg->clk_source_mode == RCAR_FIXED_EXTERNAL_CLOCK_SOURCE){
	    escr_divisor = 2; //escr_divisor result would be 2 in case using external clock
			/* Get best clock and calculate DPLL */
		best_clock = gen3_du_pll_setting (port, ideal_dclk, port->du_cfg->source_clk, &dpll_n, &dpll_m, &dpll_fdpll);
		port->pixel_clk = best_clock/escr_divisor;
		SLOG_DEBUG("best_clock from gen3_du_pll_setting:%d ", best_clock);
			
			dpllcr =  DPLLCR_CODE | DPLLCR_M(dpll_m) |
			DPLLCR_FDPLL(dpll_fdpll) | DPLLCR_CLKE | 
			DPLLCR_N(dpll_n) | DPLLCR_STBY;

		if (port->du_cfg->du_index == DU_CH_1)
				dpllcr |= (DPLLCR_PLCS1 | DPLLCR_INCS_DPLL01_DOTCLKIN13);
		if (port->du_cfg->du_index == DU_CH_2) {
				dpllcr |= (DPLLCR_PLCS0 | DPLLCR_INCS_DPLL01_DOTCLKIN02);
				dpllcr |=  (0x1 << 21); /* workaround for WS1.0/1.1 */
			}
			*DPLLC2R = DPLLC2R_CODE;
			*DPLLCR = dpllcr;			
		SLOG_DEBUG("dpllcr setting value: dpll_n %d, dpll_m %d, dpll_fdpll %d",dpll_n, dpll_m, dpll_fdpll);
	} else if (port->du_cfg->clk_source_mode == RCAR_CONFIGURABLE_EXTERNAL_CLOCK_SOURCE){
	    escr_divisor = 1; //escr_divisor result would be 1 in case using programmable external clock
		port->du_cfg->source_clk = ideal_dclk; // ext_clk_config should configure input clock exactly by ideal_dclk
			
		if (port->du_cfg->ext_clk_config) {
		    /* Execute external clock configure function for this port */
		    ((wfdcfg_ext_fn_extclk_config_t*)port->du_cfg->ext_clk_config)(port->du_cfg->du_index,port->du_cfg->source_clk);
		}	
		port->pixel_clk = port->du_cfg->source_clk/escr_divisor;
	} else if (port->du_cfg->clk_source_mode == RCAR_INTERNAL_CLOCK_SOURCE){
	    escr_divisor    = (uint32_t)(round((float)port->du_cfg->source_clk/dclk));
	    port->pixel_clk = port->du_cfg->source_clk/escr_divisor;
		    }

	/* DU timing setting */
	*HDSR     		= hsw + hbp - 19;
	*HDER     		= hsw + hbp - 19 + hpixels;
	*VDSR     		= vbp - 2;
	*VDER     		= vbp - 2 + vlines;
	*HSWR     		= hsw - 1;
	*VSPR     		= vbp + vfp + vlines - 1;
	*HCR            = hc;
	*VCR            = vc;
	*DESR			= hsw + hbp;
	*DEWR			= hpixels;
	*ESCR  			= (escr_divisor - 1) | (escr_clock_bit<<20);

    port->actual_refresh = (float)port->pixel_clk/(float)((hc+1)*(vc+1));
	if (port->pixel_clk != ideal_dclk){
	    SLOG_WARNING("DU%d dot clock is not optimal. Actual rate: %.2f (dclk=%d)",
	            port->du_cfg->du_index,port->actual_refresh, port->pixel_clk);
	}

}
void port_init(du_dev_t *dev, port_t *port)
{
	const struct wfdcfg_keyval* keyval = NULL;

	/* Retrieve some port settings from extension  */

	keyval = wfdcfg_mode_get_extension(port->active_mode->timings, WFDCFG_EXT_PORTMODE_CLOCK_SOURCE);

    if (keyval != NULL) {
    	port->du_cfg->clk_source_mode = keyval->i;
    } else {
    	/* Use internal clock by default */
    	port->du_cfg->clk_source_mode = RCAR_INTERNAL_CLOCK_SOURCE;
    	SLOG_WARNING("Clock source mode not provided. Use internal clock by default.");
    }

    switch (port->du_cfg->clk_source_mode){
		case RCAR_INTERNAL_CLOCK_SOURCE:
	    	port->du_cfg->source_clk = RCAR_DU_INTERNAL_CLOCK;

	    	/* Internal clock source workaround in R-Car H3(ES1.0) */
	    	if ((dev->chip_type == RCAR_PRODUCT_H3) && (dev->chip_revision == RCAR_REVISION_ES10))
	    	{
	            port->du_cfg->source_clk = RCAR_DU_INTERNAL_CLOCK_WS10;
	    	}
			break;
		case RCAR_FIXED_EXTERNAL_CLOCK_SOURCE:
			keyval = wfdcfg_mode_get_extension(port->active_mode->timings, WFDCFG_EXT_PORTMODE_EXTERNAL_CLOCK_RATE);
		    if (keyval != NULL) {
		    	port->du_cfg->source_clk = keyval->i;
		    } else {
				SLOG_ERROR("wfdcfg library doesn't provide external clock value for external clock source");
				return;
		    }
			break;
		case RCAR_CONFIGURABLE_EXTERNAL_CLOCK_SOURCE:
			keyval = wfdcfg_mode_get_extension(port->active_mode->timings, WFDCFG_EXT_FN_EXTCLK_CONFIG);
			if (keyval != NULL) {
				port->du_cfg->ext_clk_config = keyval->p;
			} else {
				SLOG_ERROR("wfdcfg library doesn't provide configure function for external clock chip");
				return;
			}
			break;
		default:
			SLOG_ERROR("Unsupported clock mode: %d",port->du_cfg->clk_source_mode);
			break;
    }

	/* Reset DU */
	*DSYSR		= DSYSR_DRES;
	*GRP_DSYSR	= DSYSR_DRES;
	
	/* Initialize display attribute */
	*DOOR    = DOOR_RGB(0, 0, 0xFC); /* blue as off screen */
	*CDER    = CDER_RGB(0, 0, 0);
	*BPOR    = BPOR_RGB(0, 0, 0);
	
	*DSMR    = 	DSMR_VSPM(0) | /* The VSYNC signal is output to the VSYNC pin */
				DSMR_ODPM(0) | /* The ODDF signal is output to the ODDF pin */
				DSMR_DIPM_DE | /* The DE signal is output to the DISP pin */
				DSMR_CSPM(1) | /* The HSYNC signal is output to the CSYNC pin */ 	
				DSMR_VSL(1) | /* The polarity of VSYNC is inverted */
				DSMR_HSL(1) | /* The polarity of HSYNC is inverted */
				DSMR_DDIS(0) | /* The DISP signal is output */
				DSMR_CDEL(0) |
				DSMR_CDEM_CDE; /* The CDE signal is output as is*/
	
	/* Interrupt setting */
	*DSRCR   	= DSRCR_MASK; 	/* Clear all interrupt */
	*DIER 		= DIER_VBE;  	/* Enable Vertical blanking interrupts */

	/* Display Unit System Control Register */
	*DEFR 	= DEFR_CODE | DEFR_DEFE;
	*DEFR5	= DEFR5_CODE | DEFR5_DEFE5;
	*DEFR6	= DEFR6_CODE | DEFR6_ODPM_DISP(DU_CH_2); /* The DISP signal is output to the ODDF pin, which is a DU0 and DU2 */	
	*DEFR10 = DEF10R_CODE | DEF10R_DEFE10; 	/* VSPD0/1 data format is RGB */
								/* DOC0/1 ch1 data format is RGB */
								/* Data to Split View is output from DU0 and DU1 */
								/* Extensional functions are enabled*/
								/* Set DPAD if DU is in group 1 (DU2/DU3) */
	if (port->du_cfg->grp_index == DU_GRP_1)
		*DEFR8 = DEFR8_CODE | DEFR8_DRGBS_DU(DU_CH_2);	/* DPAD is connected to DU2 */

	output_route_setting (dev,port);
	du_timing_setting (dev,port);

	switch (port->physport_type){
		case RCAR_PHYSPORT_LVDS:
			lvds_init(port);
			break;
		case RCAR_PHYSPORT_HDMI0:
			hdmi_init(port);
			break;
		case RCAR_PHYSPORT_HDMI1:
			hdmi_init(port);
			break;
		case RCAR_PHYSPORT_DPAD:
			// We currently have nothing to init with DPAD type
			break;
		default:
			SLOG_ERROR("Unsupported physical port type: %d. Port may not initialized.",port->physport_type);
			return;
			break;
	}

	/* Initialize master window */
	*(P1DDCR4 + (port->du_cfg->use_planes[0]-1)*PLANE_GAP) = PnDDCR4_CODE | PnDDCR4_EDF_ARGB8888;
	*(P1DSXR + (port->du_cfg->use_planes[0]-1)*PLANE_GAP)  = port->active_mode->timings->hpixels;
	*(P1DSYR + (port->du_cfg->use_planes[0]-1)*PLANE_GAP)  = port->active_mode->timings->vlines;
	*(P1DPXR + (port->du_cfg->use_planes[0]-1)*PLANE_GAP)  = 0;
	*(P1DPYR + (port->du_cfg->use_planes[0]-1)*PLANE_GAP)  = 0;
	
	*(P1MR + (port->du_cfg->use_planes[0]-1)*PLANE_GAP)		= PnMR_DDDF_16BPP | PnMR_SPIM_NOTP;

	/* Hardcode to activate Planes */
	*DS0PR = DU_PLANE_1;	
	*DS1PR = DU_PLANE_3;
	
	/* Initialize composition hardware */
	if (port->du_cfg->hw_compose->sub_hw_init)
	{
 		port->du_cfg->compose_dev = port->du_cfg->hw_compose->sub_hw_init (dev, port);
		if (!port->du_cfg->compose_dev)
		{
			SLOG_ERROR("Composition core initialization error\n");
			return;
		}
	}	
	
	/* If Scale is enabled, init Scale device */
	if (port->du_cfg->ExScaleHwId != DEVICE_NONE)
	{
		port->du_cfg->scaling_dev = exscale_init (port);
		if (!port->du_cfg->scaling_dev)
		{
		    SLOG_ERROR("Scaling core initialization error\n");
			return;
		}
	}

	/* Enable display */
	*DSYSR		= DSYSR_DEN;
	*GRP_DSYSR	= DSYSR_DEN;
}
