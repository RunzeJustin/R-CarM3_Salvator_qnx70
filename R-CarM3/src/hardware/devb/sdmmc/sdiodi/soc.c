/*
 * $QNXLicenseC:
 * Copyright 2007, 2008, QNX Software Systems.
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

#include <errno.h>
#include <malloc.h>
#include <string.h>

#include <hw/sysinfo.h>

#include <internal.h>

// Module:  scan hwi for devices

#ifdef SDIO_SOC_SUPPORT

extern unsigned hwi_find_bus(const char *name, unsigned unit);

unsigned sdio_soc_hwi( int idx )
{
	return( hwi_find_bus( HWI_ITEM_BUS_SDIO, idx ) );
}

int sdio_soc_config( sdio_hc_t *hc, int idx, unsigned offset )
{
	sdio_hc_cfg_t	*cfg;
	hwi_tag			*tag;
	char			*name;
	char			*opts;

	cfg			= &hc->cfg;
	opts		= NULL;
	cfg->idx	= idx;

	while( ( offset = hwi_next_tag( offset, 1 ) ) != HWI_NULL_OFF ) {
		tag		= hwi_off2tag( offset );
		name	= __hwi_find_string( ((hwi_tag *)tag)->prefix.name );

		if( !strcmp( name, HWI_TAG_NAME_busattr ) ) {
			cfg->bus_width = tag->busattr.width;
		}
		else if( !strcmp( name, HWI_TAG_NAME_location ) ) {
			cfg->base_addr[cfg->base_addrs]			= tag->location.base;
			cfg->base_addr_size[cfg->base_addrs]	= tag->location.len;
			cfg->base_addrs++;
		}
		else if( !strcmp( name, HWI_TAG_NAME_irq ) ) {
			cfg->irq[cfg->irqs++] = tag->irq.vector;
		}
		else if( !strcmp( name, HWI_TAG_NAME_inputclk ) ) {
			cfg->clk = tag->inputclk.clk;
		}
		else if( !strcmp( name, HWI_TAG_NAME_dll ) ) {
			strlcpy( cfg->name, __hwi_find_string( tag->dll.name ), sizeof( cfg->name ) );
		}
		else if( !strcmp( name, HWI_TAG_NAME_optstr ) ) {
			opts = strdup( __hwi_find_string( tag->optstr.string ) );
		}
		else if( !strcmp( name, HWI_TAG_NAME_dma ) ) {
			cfg->dma_chnl[cfg->dma_chnls++] = tag->dma.chnl;
		}
	}

	if( opts ) {
		sdio_options( hc, opts );
		free( opts );
	}			

	if( cfg->name[0] == '\0' ) {
		sdio_slogf( _SLOGC_SDIODI, _SLOG_ERROR, 1, 1, "%s:  skipping SOC %d, base 0x%"PRIx64"", __FUNCTION__, idx, cfg->base_addr[0] );
		return( ENODEV );
	}

	return( EOK );
}

int sdio_soc_scan( )
{
	sdio_hc_t	*hc;
	unsigned	item;
	int			occurence;

	item		= HWI_NULL_OFF; 
	occurence	= 0;
	hc			= TAILQ_FIRST( &sdio_ctrl.hlist );
	while( ( item = sdio_soc_hwi( occurence ) ) != HWI_NULL_OFF ) {
		if( hc == NULL && ( hc = sdio_hc_alloc( ) ) == NULL ) {
			return( ENOMEM );
		}
		if( sdio_soc_config( hc, occurence, item ) != EOK ) {
			sdio_hc_free( hc );
		}
		hc = NULL;
		occurence++;
	}

		// if we don't find any SOCs in HWI, allocate a
		// dumby entry so we will try the default product
	if( TAILQ_FIRST( &sdio_ctrl.hlist ) == NULL ) {
		if( sdio_hc_alloc( ) == NULL ) {
			return( ENOMEM );
		}
	}

	return( EOK );
}

int sdio_soc_device( sdio_hc_t *hc )
{
	sdio_hc_cfg_t	*cfg;
	unsigned		item;
	int				status;

	cfg		= &hc->cfg;
	status	= EOK;

	if( cfg->name[0] == '\0' && cfg->base_addrs ) {
		return( ENODEV );
	}

	if( cfg->idx != -1 && ( item = sdio_soc_hwi( cfg->idx ) ) != HWI_NULL_OFF ) {
		status = sdio_soc_config( hc, cfg->idx, item );
	}

	return( status );
}

#endif

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/hardware/devb/sdmmc/sdiodi/soc.c $ $Rev: 805883 $")
#endif
