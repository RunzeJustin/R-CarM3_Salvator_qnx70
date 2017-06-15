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

// Module Description:   PCI interface

#include <errno.h>
#include <string.h>
#include <hw/pci.h>

#include <internal.h>

#ifdef SDIO_PCI_SUPPORT
int sdio_pci_init( )
{
	sdio_ctrl.phdl		= pci_attach( 0 );
	return( EOK );
}

int sdio_pci_dinit( )
{
	if( sdio_ctrl.phdl != -1 ) {
		pci_detach( sdio_ctrl.phdl );
	}
	return( EOK );
}

int sdio_pci_detach_device( sdio_hc_t *hc )
{
	if( hc->phdl ) {
		pci_detach_device( hc->phdl );
	}
	return( EOK );
}

int sdio_pci_read_config8( unsigned busnum, unsigned devfunc, unsigned offset )
{
	_uint8		data;

	if( pci_read_config_bus( busnum, devfunc, offset, 1, 1, &data ) == PCI_SUCCESS ) {
		return( data );
	}
	return( 0xffffffff );
}

int sdio_pci_read_config32( unsigned busnum, unsigned devfunc, unsigned offset )
{
	_uint32		data;

	if( pci_read_config_bus( busnum, devfunc, offset, 1, 4, &data ) == PCI_SUCCESS ) {
		return( data );
	}
	return( 0xffffffff );
}

int sdio_pci_config( sdio_hc_t *hc, sdio_product_t *prod, struct pci_dev_info *pdev )
{
	sdio_hc_cfg_t		*cfg;
	int					idx;

	cfg							= &hc->cfg;
	cfg->vid					= pdev->VendorId;
	cfg->did					= pdev->DeviceId;
	cfg->class					= pdev->Class;
	cfg->bus					= pdev->BusNumber;
	cfg->devfunc				= pdev->DevFunc;
	cfg->io_xlat				= pdev->CpuIoTranslation;
	cfg->mem_xlat				= pdev->CpuMemTranslation;
	cfg->bmstr_xlat				= pdev->CpuBmstrTranslation;

	cfg->irqs		= 1;
	if( !cfg->irq[0] ) {
		cfg->irq[0] = pdev->Irq;
	}

	for( idx = 0; idx < 6; idx++ ) {
		if( ( cfg->base_addr_size[cfg->base_addrs] = pdev->BaseAddressSize[idx] ) ) {
			cfg->base_addr[cfg->base_addrs]	=	PCI_IS_IO( pdev->CpuBaseAddress[idx] ) ?
												PCI_IO_ADDR( pdev->CpuBaseAddress[idx] ) :
												PCI_MEM_ADDR( pdev->CpuBaseAddress[idx] );
			cfg->base_addrs++;
		}
	}
	return( EOK );
}

int sdio_pci_device( sdio_hc_t *hc )
{
	sdio_hc_cfg_t		*cfg;
	sdio_product_t		*prod;
	struct pci_dev_info	pdev;
	void				*phdl;
	uint32_t			aflgs;

	cfg		= &hc->cfg;

	if( sdio_ctrl.phdl == -1 ) {
		return( ENOTSUP );
	}

	if( ( prod = sdio_hc_lookup( cfg->vid, cfg->did, cfg->class, "" ) ) == NULL ) {
		return( ENODEV );
	}

	memset( &pdev, 0, sizeof( pdev ) );
	pdev.VendorId			= cfg->vid;
	pdev.DeviceId			= cfg->did;
	pdev.BusNumber			= cfg->bus;
	pdev.DevFunc			= cfg->devfunc;

	/*
	 * If it's called from sdio_pci_scan(), we do search by PCI_SEARCH_BUSDEV
	 * otherwise, search by PCI_SEARCH_VENDEV
	 */
	aflgs		= ( prod->aflags ? prod->aflags : PCI_INIT_ALL | PCI_MASTER_ENABLE );
	aflgs		|= ( ( cfg->idx == -1 ) ? PCI_SEARCH_BUSDEV : PCI_SEARCH_VENDEV );

	if( ( phdl = pci_attach_device( NULL, aflgs, ( cfg->idx == -1 ) ? 0 : cfg->idx, &pdev ) ) == NULL ) {
		return( ENODEV );
	}

	hc->phdl		= phdl;
	sdio_pci_config( hc, prod, &pdev );

	return( EOK );
}

int sdio_pci_scan( )
{
	sdio_hc_t			*hc;
	sdio_hc_cfg_t		*cfg;
	sdio_product_t		*prod;
	unsigned			bus;
	unsigned			dev;
	unsigned			func;
	unsigned			nfuncs;
	unsigned			devfunc;
	uint8_t				htype;
	uint32_t			class;
	uint32_t			product;
	unsigned			lastbus;
	unsigned			version;
	unsigned			hardware;
	int				status;

	if( sdio_ctrl.phdl == -1 ) {
		return( ENOTSUP );
	}

	hc = TAILQ_FIRST( &sdio_ctrl.hlist );
	pci_present( &lastbus, &version, &hardware );

	for( bus = 0; bus <= lastbus; bus++ ) {
		for( dev = 0; dev < 32; dev++ ) {
			for( func = 0, nfuncs = 8; func < nfuncs; func++ ) {
				devfunc = (_uint8)( ( dev << 3 ) | ( func & 0x7 ) );
				if( ( product = sdio_pci_read_config32( bus, devfunc, offsetof( struct _pci_config_regs, Vendor_ID ) ) ) == 0xffffffff )
					continue;

				if( ( class = sdio_pci_read_config32( bus, devfunc, offsetof( struct _pci_config_regs, Revision_ID ) ) ) == 0xffffffff )
					continue;

				class = ( class >> 8 ) & ~0xff;

				if( ( htype = sdio_pci_read_config8( bus, devfunc, offsetof( struct _pci_config_regs, Header_Type ) ) ) == 0xff )
					continue;

				if( !func && !( htype & 0x80 ) )
					nfuncs = 1;

				if( ( prod = sdio_hc_lookup( product & 0xffff, product >> 16, class, "" ) ) ) {
					if( hc == NULL && ( hc = sdio_hc_alloc( ) ) == NULL ) {
						return( ENOMEM );
					}
					cfg				= &hc->cfg;
					cfg->vid		= product & 0xffff;
					cfg->did		= product >> 16;
					cfg->class		= class;
					cfg->bus		= bus;
					cfg->devfunc	= devfunc;
					if( ( status = sdio_pci_device( hc ) ) != EOK ) {
						return( status );
					}
					hc = NULL;
				}
			}
		}
	}
	return( EOK );
}

#endif

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/hardware/devb/sdmmc/sdiodi/pci.c $ $Rev: 805416 $")
#endif
