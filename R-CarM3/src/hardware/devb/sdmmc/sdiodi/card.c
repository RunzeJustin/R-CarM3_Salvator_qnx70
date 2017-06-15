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
#include <atomic.h>
#include <string.h>
#include <malloc.h>

#include <internal.h>

extern sdio_ctrl_t				sdio_ctrl;

int _sdio_synchronize( struct sdio_device *device, int io, int ncmds )
{
	pthread_mutex_lock( &sdio_ctrl.mutex );
	if( !io ) {
		atomic_set( &device->flags, DEV_FLAG_RMV_PENDING );
		if( device->usage != 0 ) {
			pthread_mutex_unlock( &sdio_ctrl.mutex );
			return( EBUSY );
		}
	}
	else if( ncmds >= 0 ) {
		if( ( device->flags & DEV_FLAG_RMV_PENDING ) || ( device->dev->flags & DEV_FLAG_MEDIA_CHANGE ) ) {
			pthread_mutex_unlock( &sdio_ctrl.mutex );
			return( ENXIO );
		}
		device->usage += ncmds;
	}
	else {
		device->usage += ncmds;		// Note:  add since ncmds is negative
		if( device->usage == 0 && ( device->flags & DEV_FLAG_RMV_PENDING ) ) {
			pthread_cond_signal( &sdio_ctrl.cd_cond );
		}
	}
	pthread_mutex_unlock( &sdio_ctrl.mutex );
	return( EOK );
}

struct sdio_device *sdio_device_lookup( struct sdio_connection *connection,
		sdio_device_instance_t *instance )
{
	struct sdio_device		*device;

	device = NULL;
	pthread_mutex_lock( &sdio_ctrl.mutex );
	for( device = TAILQ_FIRST( &sdio_ctrl.dlist ); device != NULL; device = TAILQ_NEXT( device, dlink ) ) {
		if( !memcmp( &device->instance, instance, sizeof( sdio_device_instance_t ) ) ) {
			break;
		}
	}
	pthread_mutex_unlock( &sdio_ctrl.mutex );

	return( device );
}

void *sdio_client_hdl( struct sdio_device *device )
{
	return( device->user );
}

void *sdio_bs_hdl( struct sdio_device *device )
{
	return( device->dev->hc->bs_hdl );
}

sdio_cid_t *sdio_get_cid( struct sdio_device *device )
{
	return( &device->dev->cid );
}

void *sdio_get_raw_cid( struct sdio_device *device )
{
	return( &device->dev->raw_cid );
}

sdio_csd_t *sdio_get_csd( struct sdio_device *device )
{
	return( &device->dev->csd );
}

void *sdio_get_raw_csd( struct sdio_device *device )
{
	return( &device->dev->raw_csd );
}

sdio_ecsd_t *sdio_get_ecsd( struct sdio_device *device )
{
	return( &device->dev->ecsd );
}

void *sdio_get_raw_ecsd( struct sdio_device *device )
{
	return( &device->dev->raw_ecsd );
}

void *sdio_get_raw_scr( struct sdio_device *device )
{
	return( &device->dev->raw_scr );
}

int sdio_verbosity( struct sdio_device *device, int flags, int verbosity )
{
	sdio_hc_t		*hc;

	hc					= device->dev->hc;
	hc->cfg.flags		= flags;
	hc->cfg.verbosity	= verbosity;
	return( EOK );
}

struct sdio_cmd *sdio_alloc_cmd( )
{
	struct sdio_cmd		*cmd;

	pthread_mutex_lock( &sdio_ctrl.mutex );
	if( ( cmd = TAILQ_FIRST( &sdio_ctrl.clist ) ) ) {
		TAILQ_REMOVE( &sdio_ctrl.clist, cmd, clink );
		memset( cmd, 0, sizeof( struct sdio_cmd ) );
	}
	else {
		cmd = calloc( 1, sizeof( struct sdio_cmd ) ); 
	}
	pthread_mutex_unlock( &sdio_ctrl.mutex );
	return( cmd );
}

void sdio_free_cmd( struct sdio_cmd *cmd )
{
	pthread_mutex_lock( &sdio_ctrl.mutex );
	TAILQ_INSERT_TAIL( &sdio_ctrl.clist, cmd, clink );
	pthread_mutex_unlock( &sdio_ctrl.mutex );
}

int	sdio_cmd_status( struct sdio_cmd *cmd, uint32_t *status, uint32_t *rsp )
{
	if( status ) {
		*status = cmd->status;
	}

	if( rsp ) {
		memcpy( rsp, cmd->rsp, sizeof( cmd->rsp ) );
	}

	return( EOK );
}

int sdio_setup_cmd( struct sdio_cmd *cmd, uint32_t flgs, int op, int arg )
{
	cmd->opcode		= op;
	cmd->arg		= arg;
	cmd->flags		= flgs;

	return( EOK );
}

int sdio_setup_cmd_io( struct sdio_cmd *cmd, uint32_t flgs, int blks, int blksz, void *sgl, int sgc, void *mhdl )
{
	cmd->mhdl		= mhdl;
	cmd->flags		|= flgs;
	cmd->blks		= blks;
	cmd->blksz		= blksz;
	cmd->sgl		= sgl;
	cmd->sgc		= sgc;

	return( EOK );
}

int sdio_send_cmd( struct sdio_device *device, struct sdio_cmd *cmd,
		void (*func)( struct sdio_device *, struct sdio_cmd *, void *),
		uint32_t timeout, int retries )
{
	int				status;

	if( ( status = _sdio_synchronize( device, !0, 1 ) ) != EOK ) {
		return( status );
	}

	status =_sdio_send_cmd( device->dev, cmd, func, timeout, retries );

	_sdio_synchronize( device, !0, -1 );

	return( status );
}

int sdio_stop_transmission( struct sdio_device *device, int hpi )
{
	int				status;

	if( ( status = _sdio_synchronize( device, !0, 1 ) ) != EOK ) {
		return( status );
	}

	status = _sdio_stop_transmission( device->dev, hpi );

	_sdio_synchronize( device, !0, -1 );
	
	return( status );
}

int sdio_set_partition( struct sdio_device *device, uint32_t partition )
{
	sdio_dev_t	*dev;
	int			status;

	dev = device->dev;

	if( ( status = _sdio_synchronize( device, !0, 1 ) ) != EOK ) {
		return( status );
	}

	status = ( dev->dtype == DEV_TYPE_MMC ) ? mmc_set_partition( dev, partition ) : EOK;

	_sdio_synchronize( device, !0, -1 );
	
	return( status );
}

int sdio_set_block_count( struct sdio_device *device, int blkcnt )
{
	int				status;

	if( ( status = _sdio_synchronize( device, !0, 1 ) ) != EOK ) {
		return( status );
	}

	status = _sdio_set_block_count( device->dev, blkcnt );

	_sdio_synchronize( device, !0, -1 );
	
	return( status );
}

int sdio_set_block_length( struct sdio_device *device, int blklen )
{
	int				status;

	if( ( status = _sdio_synchronize( device, !0, 1 ) ) != EOK ) {
		return( status );
	}

	status = _sdio_set_block_length( device->dev, blklen );

	_sdio_synchronize( device, !0, -1 );
	
	return( status );
}

int sdio_sd_switch( struct sdio_device *device, int mode, int grp, uint8_t val, uint8_t *switch_status )
{
	int				status;

	if( ( status = _sdio_synchronize( device, !0, 1 ) ) != EOK ) {
		return( status );
	}

	status = sd_switch( device->dev, mode, grp, val, switch_status );

	_sdio_synchronize( device, !0, -1 );
	
	return( status );
}

int sdio_mmc_switch( struct sdio_device *device, uint32_t cmdset, uint32_t mode, uint32_t index, uint32_t value, uint32_t timeout )
{
	int				status;

	if( ( status = _sdio_synchronize( device, !0, 1 ) ) != EOK ) {
		return( status );
	}

	status = mmc_switch( device->dev, cmdset, mode, index, value, timeout );

	_sdio_synchronize( device, !0, -1 );
	
	return( status );
}

int sdio_send_ext_csd( struct sdio_device *device, uint8_t *csd )
{
	int				status;

	if( ( status = _sdio_synchronize( device, !0, 1 ) ) != EOK ) {
		return( status );
	}

	status = mmc_send_ext_csd( device->dev, csd );

	_sdio_synchronize( device, !0, -1 );
	
	return( status );
}

int sdio_wait_card_status( struct sdio_device *device, uint32_t *rsp, uint32_t mask, uint32_t val, uint32_t msec )
{
	int				status;

	if( ( status = _sdio_synchronize( device, !0, 1 ) ) != EOK ) {
		return( status );
	}

	status = _sdio_wait_card_status( device->dev, rsp, mask, val, msec );

	_sdio_synchronize( device, !0, -1 );
	
	return( status );
}

int sdio_send_status( struct sdio_device *device, uint32_t *rsp, int hpi )
{
	int				status;

	if( ( status = _sdio_synchronize( device, !0, 1 ) ) != EOK ) {
		return( status );
	}

	status = _sdio_send_status( device->dev, rsp, hpi );

	_sdio_synchronize( device, !0, -1 );
	
	return( status );
}

int sdio_hc_info( struct sdio_device *device, sdio_hc_info_t *info )
{
	sdio_hc_t		*hc;

	hc					= device->dev->hc;

	info->caps			= hc->caps & 0xffffffff;
	info->sg_max		= hc->cfg.sg_max;
	info->dtr_max		= hc->clk_max;
	info->dtr			= hc->clk;
	info->timing		= hc->timing;
	info->bus_width		= hc->bus_width;
	info->idle_time		= hc->cfg.idle_time;
	info->sleep_time	= hc->cfg.sleep_time;
	strcpy( info->name, hc->cfg.name );

	return( EOK );
}

int sdio_dev_info( struct sdio_device *device, sdio_dev_info_t *info )
{
	sdio_hc_t		*hc;
	sdio_dev_t		*dev;
	sdio_cid_t		*cid;
	sdio_csd_t		*csd;
	sdio_ecsd_t		*ecsd;

	dev				= device->dev;
	hc				= dev->hc;
	cid				= &dev->cid;
	csd				= &dev->csd;
	ecsd			= &dev->ecsd;

	memset( info, 0, sizeof( sdio_dev_info_t ) );

	info->dtype		= dev->dtype;
	info->rca		= dev->rca;

	if( csd->write_protect || !( csd->ccc & CCC_BLOCK_WRITE ) || ( dev->flags & DEV_FLAG_WRITE_PROTECT ) ) {
		info->flags |= DEV_FLAG_WP;
	}

	if( ( dev->flags & DEV_FLAG_LOCKED ) ) {
		info->flags |= DEV_FLAG_CARD_LOCKED;
	}

	info->mid		= cid->mid;
	info->oid		= cid->oid;
	info->prv		= cid->prv;
	info->psn		= cid->psn;
	info->month		= cid->month;
	info->year		= cid->year;
	memcpy( info->pnm, cid->pnm, sizeof( cid->pnm ) );

	info->caps					= dev->caps;

	info->sector_size			= csd->blksz;
	info->native_sector_size	= SDIO_BLKSZ_4K;
	info->sectors				= csd->sectors;
	info->super_page_size		= SDIO_BLKSZ_4K;

	if( dev->dtype == DEV_TYPE_MMC ) {
		if( csd->spec_vers >= CSD_SPEC_VER_4 && ecsd->sectors ) {
			info->sectors			= ecsd->sectors;
			info->sector_size		= ecsd->blksz;
			info->super_page_size	= ecsd->acc_size;
		}
		info->spec_vers			= dev->csd.spec_vers;
		info->spec_rev			= ecsd->ext_csd_rev;
	}
	else {
		info->spec_vers			= dev->scr.sd_spec;
		info->security			= dev->scr.sd_security;
		info->speed_class		= dev->sds.speed_class;
	}

	info->wp_size				= dev->wp_size;
	info->erase_size			= dev->erase_size;

	info->optimal_trim_size		= info->super_page_size;
	info->optimal_read_size		= info->super_page_size;
	info->optimal_write_size	= info->super_page_size;

	info->dtr					= hc->clk;
	info->timing				= hc->timing;
	info->bus_width				= hc->bus_width;

	return( EOK );
}

int sdio_cache( struct sdio_device *device, int op, uint32_t timeout )
{
	sdio_dev_t	*dev;
	int			status;

	dev = device->dev;

	if( ( status = _sdio_synchronize( device, !0, 1 ) ) != EOK ) {
		return( status );
	}

	if( ( dev->dtype == DEV_TYPE_MMC ) ) {
		status = mmc_cache( dev, op, timeout );
	}
	else {
		status = EINVAL;
	}

	_sdio_synchronize( device, !0, -1 );

	return( status );
}

int sdio_erase( struct sdio_device *device, int partition, int flgs, uint64_t lba, int nlba )
{
	sdio_dev_t	*dev;
	int			status;

	dev = device->dev;

	if( ( status = _sdio_synchronize( device, !0, 1 ) ) != EOK ) {
		return( status );
	}

	if( ( dev->dtype == DEV_TYPE_MMC ) ) {
		status = mmc_erase( dev, partition, flgs, lba, nlba );
	}
	else {
		status = sd_erase( dev, flgs, lba, nlba );
	}

	_sdio_synchronize( device, !0, -1 );

	return( status );
}

int sdio_lock_unlock( struct sdio_device *device, int action, uint8_t *pwd, int pwd_len )
{
	sdio_dev_t	*dev;
	int			status;

	dev = device->dev;

	if( ( status = _sdio_synchronize( device, !0, 1 ) ) != EOK ) {
		return( status );
	}

	if( ( dev->dtype == DEV_TYPE_MMC ) ) {
		status = EINVAL;
	}
	else {
		status = sd_lock_unlock( dev, action, pwd, pwd_len );
	}

	_sdio_synchronize( device, !0, -1 );

	return( status );
}

int sdio_bus_error( struct sdio_device *device )
{
	int				status;

	if( ( status = _sdio_synchronize( device, !0, 1 ) ) != EOK ) {
		return( status );
	}

	_sdio_bus_error( device->dev );

	_sdio_synchronize( device, !0, -1 );

	return( status );
}

int sdio_reset( struct sdio_device *device )
{
	sdio_dev_t		*dev;
	int				status;

	dev		= device->dev;

	if( ( status = _sdio_synchronize( device, !0, 1 ) ) != EOK ) {
		return( status );
	}

	_sdio_reset( dev );

	_sdio_synchronize( device, !0, -1 );

	return( status );
}

int sdio_idle( struct sdio_device *device )
{
	int		status;

	if( ( status = _sdio_synchronize( device, !0, 1 ) ) != EOK ) {
		return( status );
	}

//	status = _sdio_idle( device->dev );

	_sdio_synchronize( device, !0, -1 );

	return( status );
}

int sdio_pwrmgnt( struct sdio_device *device, int action )
{
	int		status;

	if( ( status = _sdio_synchronize( device, !0, 1 ) ) != EOK ) {
		return( status );
	}

	status = _sdio_pwrmgnt( device->dev, action );

	_sdio_synchronize( device, !0, -1 );

	return( status );
}

int sdio_detach( struct sdio_device *device )
{
	if( device->usage ) {
		return( EBUSY );
	}

	pthread_mutex_lock( &sdio_ctrl.mutex );
	TAILQ_REMOVE( &sdio_ctrl.dlist, device, dlink );
	pthread_mutex_unlock( &sdio_ctrl.mutex );

	free( device );

	return( EOK );
}

int sdio_attach( struct sdio_connection *connection,
					sdio_device_instance_t *instance,
					struct sdio_device **hdl,
					void *client_hdl )
{
	sdio_hc_t				*hc;
	struct sdio_device		*device;

	device = NULL;
	pthread_mutex_lock( &sdio_ctrl.mutex );
	for( hc = TAILQ_FIRST( &sdio_ctrl.hlist ); hc; hc = TAILQ_NEXT( hc, hlink ) ) {
		if( hc->path == instance->path && ( hc->device.flags & DEV_FLAG_PRESENT ) ) {
			if( instance->generation == SDIO_CONNECT_WILDCARD ||
					instance->generation == hc->generation ) {
				break;
			}
		}
	}

	if( hc && ( device = calloc( 1, sizeof( struct sdio_device ) ) ) ) {
		TAILQ_INSERT_TAIL( &sdio_ctrl.dlist, device, dlink );
		instance->path			= hc->path;
		instance->func			= 0;
		instance->generation	= hc->generation;
		instance->ident.vid		= 0;
		instance->ident.did		= 0;
		instance->ident.ccd		= 0;
		instance->ident.dtype	= hc->device.dtype;

		memcpy( &device->instance, instance, sizeof( sdio_device_instance_t ) );
		device->user	= client_hdl;
		device->hc		= hc;
		device->dev		= &hc->device;
		*hdl			= device;
	}

	pthread_mutex_unlock( &sdio_ctrl.mutex );

	return( device ? EOK : ENODEV );
}

int sdio_enum( struct sdio_connection *connection, int action )
{
	connection = connection;

	pthread_sleepon_lock( );
	sdio_ctrl.cd_enum = action;
	pthread_sleepon_signal( &sdio_ctrl.cd_enum );
	pthread_sleepon_unlock( );

	return( EOK );
}

int sdio_disconnect( struct sdio_connection *connection )
{
	connection = connection;

	return( _sdio_disconnect( ) );
}

int sdio_connect( sdio_connect_parm_t *parm, struct sdio_connection **connection )
{
	*connection		= (void *)&sdio_ctrl;

	memcpy( &sdio_ctrl.connect_parm, parm, sizeof( sdio_connect_parm_t ) );

	return( _sdio_connect( parm, connection ) );
}


#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/hardware/devb/sdmmc/sdiodi/card.c $ $Rev: 770028 $")
#endif
