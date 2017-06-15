/*
 * $QNXLicenseC:
 * Copyright 2014, QNX Software Systems.
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


// Module Description:  SDMMC SIM initialization modulue.

#define EXTERN
#include <sim_sdmmc.h>
#undef EXTERN

extern int cam_configure( const MODULE_ENTRY *sim_entry, int nsims, int argc, char *argv[] );

const MODULE_ENTRY sim_module = {
	"sdmmc",
	&sdmmc_sim_args,		// called once for every time it is on the cmd line
	&sdmmc_sim_attach,		// start setup, and call xpt_bus_register
	&sdmmc_sim_detach
};

CAM_SIM_ENTRY sdmmc_sim_entry = {
	&sdmmc_sim_init,
	&sdmmc_sim_action
};

#ifdef SDMMC_TRACE
void sdmmc_trace_start( void )
{
	TraceEvent( _NTO_TRACE_DELALLCLASSES );
	TraceEvent( _NTO_TRACE_CLRCLASSPID, _NTO_TRACE_KERCALL );
	TraceEvent( _NTO_TRACE_CLRCLASSTID, _NTO_TRACE_KERCALL );
	TraceEvent( _NTO_TRACE_CLRCLASSPID, _NTO_TRACE_THREAD );
	TraceEvent( _NTO_TRACE_CLRCLASSTID, _NTO_TRACE_THREAD );

#if 1
	TraceEvent( _NTO_TRACE_ADDALLCLASSES );
#else
	TraceEvent( _NTO_TRACE_ADDCLASS, _NTO_TRACE_CONTROL );
	TraceEvent( _NTO_TRACE_ADDCLASS, _NTO_TRACE_INT );
	TraceEvent( _NTO_TRACE_ADDCLASS, _NTO_TRACE_KERCALL );
	TraceEvent( _NTO_TRACE_ADDCLASS, _NTO_TRACE_PROCESS );
	TraceEvent( _NTO_TRACE_ADDCLASS, _NTO_TRACE_THREAD );
#endif

	TraceEvent( _NTO_TRACE_START );
}

void sdmmc_trace_stop( void )
{
	TraceEvent( _NTO_TRACE_STOP );
}

void sdmmc_trace_event( int event, const char *fmt, ... )
{
	char		buf[255];
	va_list		arglist;

	va_start( arglist, fmt );
	vsnprintf( buf, 254, fmt, arglist );
	va_end( arglist );
	TraceEvent( _NTO_TRACE_INSERTUSRSTREVENT, event, buf ); 
}
#endif

// The main routine for the SIM driver
int main( int argc, char *argv[] )
{
	cam_slogf( _SLOGC_SIM_MMC, _SLOG_ERROR, 1, 1, "%s %d.%02d%c (%s %s)", CAM_STRINGIZE(__DEVB_NAME__),
		SDMMC_SIM_VERSION / 100, SDMMC_SIM_VERSION % 100, SDMMC_SIM_LETTER,
		__DATE__, __TIME__ );

		// Enable IO capability.
	if( ThreadCtl(_NTO_TCTL_IO, NULL) == -1 ) {
		cam_slogf( _SLOGC_SIM_MMC, _SLOG_ERROR, 1, 1, "%s:  ThreadCtl - %s", __FUNCTION__, strerror( errno ) );
		exit( EXIT_FAILURE );
	}

	TAILQ_INIT( &sdmmc_ctrl.hlist );

	sdmmc_ctrl.argc		= argc;
	sdmmc_ctrl.argv		= argv;

#ifdef SDMMC_TRACE
	sdmmc_trace_start( );
	sdmmc_trace_event( SDMMC_TRACE_EVENT, "SDMMC Trace" );
	sdmmc_trace_stop( );
#endif

	return( cam_configure( &sim_module, 1, argc, argv ) );
}

SIM_HBA *sdmmc_alloc_hba( )
{
	SIM_HBA			*hba;
	SIM_SDMMC_EXT	*ext;

	if( ( hba = sim_alloc_hba( sizeof( SIM_SDMMC_EXT ) ) ) == NULL ) {
		return( NULL );
	}

	ext = (SIM_SDMMC_EXT *)hba->ext;

	hba->pathid			= hba->coid = hba->chid = hba->tid = hba->iid = -1;
	hba->verbosity		= sdmmc_ctrl.verbosity;

	ext->ntargs			= 0;
	ext->priority		= SDMMC_SCHED_PRIORITY;
	ext->pm_timerid		= -1;

	ext->assd_active_sec_sys = -1;

	memset( &ext->instance, SDIO_CONNECT_WILDCARD, sizeof( sdio_device_instance_t ) );

		// add hba to drivers hba list
	pthread_sleepon_lock( );
	TAILQ_INSERT_TAIL( &sdmmc_ctrl.hlist, hba, hlink );
	sdmmc_ctrl.nhba++;
	pthread_sleepon_unlock( );

//	cam_slogf( _SLOGC_SIM_MMC, _SLOG_ERROR, 1, 1, "%s:  hba %p", __FUNCTION__, hba );

	return( hba );
}

int sdmmc_free_hba( SIM_HBA *hba )
{
	pthread_sleepon_lock( );
	TAILQ_REMOVE( &sdmmc_ctrl.hlist, hba, hlink );
	sdmmc_ctrl.nhba--;
	pthread_sleepon_unlock( );
	sim_free_hba( hba );

//	cam_slogf( _SLOGC_SIM_MMC, _SLOG_ERROR, 1, 1, "%s:  hba %p", __FUNCTION__, hba );
	return( CAM_SUCCESS );
}

int sdmmc_partition_enum( SIM_HBA *hba )
{
	SIM_SDMMC_EXT		*ext;
	SDMMC_PARTITION		*part;
	uint8_t				*ecsd;
	int					pidx;
	int					pconf;
	uint32_t			nlba;
	uint64_t			gp_size;
	uint64_t			wp_grp_size;
	
	ext				= (SIM_SDMMC_EXT *)hba->ext;
	ecsd			= sdio_get_raw_ecsd( ext->device );
	pconf			= ecsd[ECSD_PART_CONFIG] & ~ECSD_PC_ACCESS_MSK;
	wp_grp_size		= (uint64_t )ecsd[ECSD_HC_WP_GRP_SIZE ] * (uint64_t )ecsd[ECSD_ERASE_GRP_SIZE ] * MMC_ERASE_GRP_512K;

		// boot area partitions (1/2)
	for( pidx = 0; pidx < MMC_BOOT_PART_MAX; pidx++, part++ ) {
		part			= &ext->targets[1 + pidx].partitions[0];
		part->config	= pconf | ( 1 + pidx );
		part->slba		= 0;
		part->nlba		= (uint64_t)ecsd[ECSD_BOOT_SIZE_MULT] * MMC_BOOT_SIZE_MULT / 512;
		part->elba		= part->nlba - 1;
		strcpy( part->name, "boot" );
		if( ( ecsd[ECSD_PARTITIONING_SUP] & ECSD_PS_PART_EN ) ) {
			part->pflags |= SDMMC_PFLAG_ENH;
		}
		ext->ntargs++;
	}

		// RPMB area
	part			= &ext->targets[3].partitions[0];
	part->config		= pconf | 3;
	part->slba		= 0;
	part->nlba		= (uint64_t)ecsd[ECSD_RPMB_SIZE_MULT] * MMC_RPMB_SIZE_MULT / 512;
	part->elba		= part->slba + part->nlba - 1;
	strcpy( part->name, "rpmb" );
	ext->ntargs++;

		// GP partitions
	for( pidx = 0; pidx < MMC_GP_PART_MAX; pidx++, part++ ) {
		gp_size = (uint64_t)ecsd[ECSD_GP_SIZE + 0 + ( pidx * 3 ) ];
		gp_size |= (uint64_t)ecsd[ECSD_GP_SIZE + 1 + ( pidx * 3 ) ] << 8;
		gp_size |= (uint64_t)ecsd[ECSD_GP_SIZE + 2 + ( pidx * 3 ) ] << 16;
		nlba = ( gp_size * wp_grp_size ) / 512;

		part			= &ext->targets[ 4 + pidx].partitions[0];
		part->config	= pconf | ( 4 + pidx );
		part->slba		= 0;
		part->nlba		= nlba;
		part->elba		= nlba ? ( part->nlba - 1 ) : 0;
		strcpy( part->name, "gp" );
		ext->ntargs++;

		if( ( ecsd[ECSD_PARTITIONS_ATTR] & ( ECSD_PA_ENH_1 << pidx ) ) ) {
			part->pflags |= SDMMC_PFLAG_ENH;
		}
	}

	return( EOK );
}

int sdmmc_partition_config( SIM_HBA *hba )
{
	SIM_SDMMC_EXT	*ext;
	SDMMC_TARGET	*targ;
	SDMMC_PARTITION	*part;
	uint8_t			*ecsd;
	uint32_t		nlba;
	uint32_t		tidx;
	uint32_t		blksz;
	
	ext				= (SIM_SDMMC_EXT *)hba->ext;
	targ			= ext->targets;
	part			= targ->partitions;

	nlba			= ext->dev_inf.sectors;
	blksz			= ext->dev_inf.sector_size;

	for( tidx = 0; tidx < SDMMC_TARGET_MAX; tidx++ ) {
		targ 			= &ext->targets[tidx];
		targ->blksz		= blksz;
	}

	if( ( ext->dev_inf.flags & DEV_FLAG_WP ) ) {
		part->pflags |= SDMMC_PFLAG_WP;
	}

	part->config	= 0;
	part->slba		= 0;
	part->nlba		= nlba;
	part->elba		= part->nlba - 1;
	strlcpy( part->name, "uda", sizeof( part->name ) );

	if( ext->instance.ident.dtype == DEV_TYPE_MMC ) {
		ecsd			= sdio_get_raw_ecsd( ext->device );
		part->config	= ecsd[ECSD_PART_CONFIG] & ~ECSD_PC_ACCESS_MSK;
#ifdef SIM_BS_PARTITION_CONFIG
		sim_bs_partition_config( hba );
#else
		if( ( ext->eflags & SDMMC_EFLAG_PARTITIONS ) ) {
			sdmmc_partition_enum( hba );
		}
#endif
	}

	return( CAM_SUCCESS );
}

int sdmmc_detach( SIM_HBA *hba )
{
	SIM_SDMMC_EXT	*ext;

	ext		= (SIM_SDMMC_EXT *)hba->ext;

//	cam_slogf( _SLOGC_SIM_MMC, _SLOG_ERROR, 1, 1, "%s:  hba %p", __FUNCTION__, hba );

	if( ( ext->eflags & SDMMC_EFLAG_PWROFF_NOTIFY ) ) {
		sdmmc_pwroff_notify( hba, ext->pwroff_notify );
	}

	if( hba->pathid != -1 ) {
		xpt_bus_deregister( hba->pathid );
	}

	if( hba->coid != -1 ) {
		ConnectDetach( hba->coid );
	}

	if( hba->chid != -1 ) {
		ChannelDestroy( hba->chid );
	}

	if( hba->tid != -1 ) {
		pthread_join( hba->tid, NULL );
	}

	if( ext->pm_timerid != -1 ) {
		timer_delete( ext->pm_timerid );
	}

	if( hba->simq ) {
		simq_dinit( hba->simq );
	}

	if( ext->device ) {
		sdio_detach( ext->device );
	}

#ifdef SDMMC_WRITE_VERIFY
	if( ext->ver_vaddr ) {
		xpt_free( ext->ver_vaddr, SDMMC_VER_BSIZE );
	}
#endif

	sdmmc_free_hba( hba );

	return( CAM_SUCCESS );
}

int sdmmc_reg( SIM_HBA *hba )
{
	SIM_SDMMC_EXT	*ext;
	pthread_attr_t		attr;
	struct sched_param	param;


	ext		= (SIM_SDMMC_EXT *)hba->ext;

	pthread_attr_init( &attr );
	pthread_attr_setschedpolicy( &attr, SCHED_RR );
	param.sched_priority = ext->priority;
	pthread_attr_setschedparam( &attr, &param );
	pthread_attr_setinheritsched( &attr, PTHREAD_EXPLICIT_SCHED );
	pthread_attr_setstacksize( &attr, CAM_STACK_SIZE );

#ifdef SDMMC_WRITE_VERIFY
	if( ( ext->ver_vaddr = xpt_alloc( XPT_ALLOC_CONTIG | XPT_ALLOC_NOCACHE, SDMMC_VER_BSIZE, NULL ) ) == MAP_FAILED ) {
		cam_slogf( _SLOGC_SIM_MMC, _SLOG_ERROR, 1, 1, "%s: xpt_alloc verify buffer failure", __FUNCTION__ );
		return( CAM_FAILURE );
	}

	ext->ver_paddr = xpt_vtop( ext->ver_vaddr, NULL );
#endif

	if( cam_create_thread( &hba->tid, &attr, sdmmc_driver_thread, hba, ext->priority, &hba->state, "sdmmc_driver_thread" ) != EOK ) {
		cam_slogf( _SLOGC_SIM_MMC, _SLOG_ERROR, 1, 1, "%s: sdmmc_driver_thread creation failure", __FUNCTION__ );
		return( CAM_FAILURE );
	}

	atomic_set( &ext->eflags, SDMMC_EFLAG_PRESENT );

	hba->pathid		= xpt_bus_register( &sdmmc_sim_entry, hba );

	if( hba->pathid > sdmmc_ctrl.pathid_max ) {
		sdmmc_ctrl.pathid_max = hba->pathid;
	}

	xpt_async( AC_SIM_REGISTER, hba->pathid, -1, -1, NULL, 0 );

	return( CAM_SUCCESS );
}

int sdmmc_attach( SIM_HBA *hba, struct sdio_connection *connection, sdio_device_instance_t *instance )
{
	SIM_SDMMC_EXT		*ext;
	int					status;

	ext		= (SIM_SDMMC_EXT *)hba->ext;
	status	= CAM_FAILURE;

	if( sdio_attach( connection, instance, &ext->device, hba ) == EOK ) {
		ext->instance	= *instance;
		sdio_hc_info( ext->device, &ext->hc_inf );
		sdio_dev_info( ext->device, &ext->dev_inf );
		ext->pm_idle_time_ns	= SDMMC_TIMEOUT_MS_TO_NS( ext->hc_inf.idle_time );
		ext->pm_sleep_time_ns	= SDMMC_TIMEOUT_MS_TO_NS( ext->hc_inf.sleep_time );

		sdmmc_partition_config( hba );

		if( ( ext->dev_inf.caps & DEV_CAP_CACHE ) ) {
			cam_slogf( _SLOGC_SIM_MMC, _SLOG_ERROR, 1, 1, "%s:  %s volatile cache", __FUNCTION__, ( ext->eflags & SDMMC_EFLAG_CACHE ) ? "Enabling" : "Disabling" );
			sdio_cache( ext->device, ( ext->eflags & SDMMC_EFLAG_CACHE ) ? SDIO_CACHE_ENABLE : SDIO_CACHE_DISABLE, SDIO_TIME_DEFAULT );
		}

		if( ( ext->eflags & SDMMC_EFLAG_BKOPS ) ) {
			if( sdmmc_bkops_cfg( hba ) != EOK ) {
				ext->eflags &= ~SDMMC_EFLAG_BKOPS;
			}
		}

		if( ( ext->eflags & SDMMC_EFLAG_PWROFF_NOTIFY ) ) {
			if( sdmmc_pwroff_notify( hba, ECSD_POWERED_ON ) != EOK ) {
				ext->eflags &= ~SDMMC_EFLAG_PWROFF_NOTIFY;
			}
		}

		if( ( ext->dev_inf.caps & DEV_CAP_ASSD ) ) {
			sdmmc_assd_init( hba );
		}

		status = sdmmc_reg( hba );
	}

	return( status );
}

void sdmmc_sdio_insertion( struct sdio_connection *connection, sdio_device_instance_t *instance )
{
	SIM_HBA				*hba;

	cam_slogf( _SLOGC_SIM_MMC, _SLOG_ERROR, sdmmc_ctrl.verbosity, 1, "%s:  ", __FUNCTION__ );

	if( !( sdmmc_ctrl.cflags & SDMMC_CFLAG_SCAN ) ||
			( sdmmc_ctrl.cflags & SDMMC_CFLAG_ENUMERATING ) ) {
		return;
	}

	if( ( hba = sdmmc_alloc_hba( ) ) ) {
		if( sdmmc_attach( hba, connection, instance ) != CAM_SUCCESS ) {
			sdmmc_detach( hba );
		}
	}
}

void sdmmc_sdio_removal( struct sdio_connection *connection, sdio_device_instance_t *instance )
{
	SIM_HBA				*hba;
	SIM_SDMMC_EXT		*ext;
	struct sdio_device	*device;

	cam_slogf( _SLOGC_SIM_MMC, _SLOG_ERROR, sdmmc_ctrl.verbosity, 1, "%s:  ", __FUNCTION__ );

	if( ( sdmmc_ctrl.cflags & SDMMC_CFLAG_ENUMERATING ) ) {
		return;
	}

	if( ( device = sdio_device_lookup( connection, instance ) ) != NULL ) {
		hba = sdio_client_hdl( device );
		ext	= (SIM_SDMMC_EXT *)hba->ext;
		atomic_clr( &ext->eflags, SDMMC_EFLAG_PRESENT );
		sdmmc_detach( hba );
	}
}

void sdmmc_sdio_event( struct sdio_connection *connection, sdio_device_instance_t *instance, int ev )
{
	SIM_HBA				*hba;
	SIM_SDMMC_EXT		*ext;
	struct sdio_device	*device;

	cam_slogf( _SLOGC_SIM_MMC, _SLOG_ERROR, sdmmc_ctrl.verbosity, 1, "%s:  ", __FUNCTION__ );

	if( ( device = sdio_device_lookup( connection, instance ) ) != NULL ) {
		hba = sdio_client_hdl( device );
		ext	= (SIM_SDMMC_EXT *)hba->ext;
		if( ( ext->dev_inf.caps & DEV_CAP_ASSD ) ) {
			atomic_set( &ext->eflags, SDMMC_EFLAG_ASSD_INIT );
		}
	}
}

int sdmmc_post_ccb( SIM_HBA *hba, CCB_SCSIIO *ccb )
{
	SIM_SDMMC_EXT	*ext;
	struct timespec	ts;

	ext = (SIM_SDMMC_EXT *)hba->ext;

	clock_gettime( CLOCK_MONOTONIC, &ts );
	ext->pm_timestamp = timespec2nsec( &ts );

	ext->nexus = NULL;

#ifdef SDMMC_TRACE
	sdmmc_trace_event( SDMMC_TRACE_EVENT, "%s:  ccb %p", __FUNCTION__, ccb );
#endif

	if( hba->verbosity > 3 ) {
		cam_slogf( _SLOGC_SIM_MMC, _SLOG_ERROR, 1, 1, "%s:  ccb %p",__FUNCTION__, ccb );
	}

	simq_post_ccb( hba->simq, ccb );

	return( CAM_SUCCESS );
}

int sdmmc_sense( CCB_SCSIIO *ccb, int sense, int asc, int ascq )
{
	SCSI_SENSE			*sptr;

	if( ccb->cam_ch.cam_func_code != XPT_SCSI_IO ) {
		return( CAM_REQ_CMP_ERR );
	}

	ccb->cam_scsi_status	= SCS_CHECK;
	ccb->cam_ch.cam_status	= CAM_REQ_CMP_ERR;
	sptr					= (SCSI_SENSE *)ccb->cam_sense_ptr;

	if( sptr == NULL || ( ccb->cam_ch.cam_flags & CAM_DIS_AUTOSENSE ) ) {
		return( ccb->cam_ch.cam_status );
	}

	ccb->cam_ch.cam_status |= CAM_AUTOSNS_VALID;

	memset( sptr, 0, sizeof( *sptr ) );
	sptr->error	= 0x70;		// Error code
	sptr->sense = sense;	// Sense key
	sptr->asc	= asc;		// Additional sense code (Invalid field in CDB)
	sptr->ascq	= ascq;		// Additional sense code qualifier

	return( ccb->cam_ch.cam_status );
}

int sdmmc_error( SIM_HBA *hba, CCB_SCSIIO *ccb, int status )
{
	SIM_SDMMC_EXT	*ext;

	ext		= (SIM_SDMMC_EXT *)hba->ext;

	switch( status ) {
		case EIO:
			status = sdmmc_sense( ccb, SK_MEDIUM, 0x00, 0x00 );
			break;

		case ENXIO:
			status = sdmmc_sense( ccb, SK_NOT_RDY, ASC_MEDIA_NOT_PRESENT, 0 );
			break;

		case EACCES:
			status = sdmmc_sense( ccb, SK_ILLEGAL, ASC_COPY_PROTECTION, ASCQ_READ_SCRAMBLED );
			break;

		case ENODEV:
			atomic_clr( &ext->eflags, SDMMC_EFLAG_PRESENT );
			status = CAM_NO_HBA; break;

		case EINVAL:
			status = sdmmc_sense( ccb, SK_ILLEGAL, ASC_INVALID_FIELD, 0 );
			break;

		case EROFS:
			status = sdmmc_sense( ccb, SK_DATA_PROT, ASC_WRITE_PROTECTED, ASCQ_VU_WRITE_PROTECTED );
			break;

		case ETIMEDOUT:
			status = CAM_CMD_TIMEOUT; break;

		default:
			status = CAM_REQ_CMP_ERR; break;
	}

	return( status );
}

int sdmmc_unit_ready( SIM_HBA *hba, CCB_SCSIIO *ccb )
{
	SIM_SDMMC_EXT	*ext;
	int				status;

	ext		= (SIM_SDMMC_EXT *)hba->ext;
	status	= CAM_REQ_CMP;

	if( !( ext->eflags & SDMMC_EFLAG_PRESENT ) ) {
		status = sdmmc_error( hba, ccb, ENODEV );
	}

	if( !ext->targets[ccb->cam_ch.cam_target_id].partitions[ccb->cam_ch.cam_target_lun].nlba ) {
		status = sdmmc_error( hba, ccb, ENXIO );
	}

	return( status );
}

int sdmmc_mpage_rw_err( SIM_HBA *hba, CCB_SCSIIO *ccb, int op )
{
	SIM_SDMMC_EXT		*ext;
	MODE_PARM_HEADER10	*mph;
	int					status;

	ext = (SIM_SDMMC_EXT *)hba->ext;
	mph	= CAM_DATA_PTR_V( ccb->cam_data.cam_data_ptr );
	memset( mph, 0, sizeof( MODE_PARM_HEADER10 ) );

	if( op ) {
		status = sdmmc_sense( ccb, SK_ILLEGAL, ASC_INVALID_FIELD, 0 );
	}
	else {
		if( ( ext->targets[ccb->cam_ch.cam_target_id].partitions[ccb->cam_ch.cam_target_lun].pflags & SDMMC_PFLAG_WP ) ) {
			mph->device_specific |= MP_DS_WP;
		}
		status = CAM_REQ_CMP;
	}

	return( status );
}

int sdmmc_mpage_caching( SIM_HBA *hba, CCB_SCSIIO *ccb, int op )
{
	SIM_SDMMC_EXT		*ext;
	MPAGE_CACHING		*mpc;
	MODE_PARM_HEADER10	*mph;
	int			status;

	ext = (SIM_SDMMC_EXT *)hba->ext;
	mph	= CAM_DATA_PTR_V( ccb->cam_data.cam_data_ptr );
	mpc	= (MPAGE_CACHING *)(mph + 1 );

	if( op ) {
		if( ( ext->dev_inf.caps & DEV_CAP_CACHE ) ) {
			if( ( status = sdio_cache( ext->device, ( mpc->flags & MP_CACHE_WCE ) ? SDIO_CACHE_ENABLE : SDIO_CACHE_DISABLE, SDIO_TIME_DEFAULT ) ) == EOK ) {
				status		= CAM_REQ_CMP;
				ext->eflags	&= ~SDMMC_EFLAG_CACHE;
				if( ( mpc->flags & MP_CACHE_WCE ) ) {
					ext->eflags	|= SDMMC_EFLAG_CACHE;
				}
			}
			else {
				status = sdmmc_error( hba, ccb, status );
			}
		}
		else {
			status = sdmmc_error( hba, ccb, EINVAL );
		}

	}
	else {
		memset( mph, 0, ccb->cam_dxfer_len );
		mph->data_len[1]	= 26;
		mpc->pc_page		= MP_CACHING;
		mpc->page_length	= 18;
		mpc->flags			= ( ext->dev_inf.caps & DEV_CAP_CACHE ) ? MP_CACHE_WCE : 0;
		status				= CAM_REQ_CMP;
	}

	return( status );
}

int sdmmc_mode_select( SIM_HBA *hba, CCB_SCSIIO *ccb )
{
	MPAGE_CACHING		*mp;
	MODE_PARM_HEADER10	*mph;
	int					status;

	mph	= CAM_DATA_PTR_V( ccb->cam_data.cam_data_ptr );
	mp	= (MPAGE_CACHING *)(mph + 1 );

	switch( mp->pc_page ) {
		case MP_CACHING:
			status = sdmmc_mpage_caching( hba, ccb, CAM_TRUE );
			break;

		default:
			status = sdmmc_error( hba, ccb, EINVAL );
			break;
	}

	return( status );
}

int sdmmc_mode_sense( SIM_HBA *hba, CCB_SCSIIO *ccb )
{
	int				status;

	if( ( status = sdmmc_unit_ready( hba, ccb ) ) != CAM_REQ_CMP ) {
		return( status );
	}

	switch( ccb->cam_cdb_io.cam_cdb_bytes[2] ) {
		case MP_RW_ERR:
			status = sdmmc_mpage_rw_err( hba, ccb, CAM_FALSE );
			break;

		case MP_CACHING:
			status = sdmmc_mpage_caching( hba, ccb, CAM_FALSE );
			break;

		default:
			status = sdmmc_error( hba, ccb, EINVAL );
			break;
	}

	return( status );
}

int sdmmc_evpd_inquiry( SIM_HBA *hba, CCB_SCSIIO *ccb )
{
	SIM_SDMMC_EXT					*ext;
	VPD_HEADER						*vh;
	BLOCK_LIMITS					*bl;
	BLOCK_DEVICE_CHARACTERISTICS	*bdc;
	uint8_t							*vpd;
	uint16_t						lps;
	int								cstatus;

	ext	= (SIM_SDMMC_EXT *)hba->ext;

	vpd	= CAM_DATA_PTR_V( ccb->cam_data.cam_data_ptr );
	vh	= (VPD_HEADER *)vpd;
	bl	= (BLOCK_LIMITS *)(vh+1);
	bdc	= (BLOCK_DEVICE_CHARACTERISTICS *)(vh+1);
	memset( vpd, 0, ccb->cam_dxfer_len );

	cstatus = CAM_REQ_CMP;

	switch( ccb->cam_cdb_io.cam_cdb_bytes[2] ) {
		case INQ_PC_VPD_SUPPORTED: {
			uint8_t		evp_sup[] = { D_DIR_ACC, INQ_PC_VPD_SUPPORTED, 0, 3,
							INQ_PC_VPD_SUPPORTED, INQ_PC_BLOCK_LIMITS,
							INQ_PC_BLOCK_DEVICE_CHARACTERISTICS };

			if( ccb->cam_dxfer_len < sizeof( evp_sup ) ) {
				return( sdmmc_error( hba, ccb, EINVAL ) );
			}
			memcpy( vpd, evp_sup, sizeof( evp_sup ) );
			break;
		}

		case INQ_PC_BLOCK_LIMITS:
			if( ccb->cam_dxfer_len < ( sizeof( *vh ) + sizeof( *bl ) ) ) {
				return( sdmmc_error( hba, ccb, EINVAL ) );
			}
			vh->peripheral		= D_DIR_ACC;
			vh->page_code		= INQ_PC_BLOCK_LIMITS;
			vh->page_length		= ENDIAN_BE16( 60 );

			lps = 1;
			bl->optimal_xfer_len_granularity	= ENDIAN_BE16( lps );

			if( ( ext->dev_inf.caps & ( DEV_CAP_TRIM | DEV_CAP_DISCARD ) ) ) {
				bl->max_unmap_lba_count			= ENDIAN_BE32( SDMMC_TRIM_MAX_LBA );
				bl->max_unmap_desc_count		= ENDIAN_BE32( 1 );
				bl->optimal_unmap_granularity	= ENDIAN_BE32( 1 );
			}
			break;

		case INQ_PC_BLOCK_DEVICE_CHARACTERISTICS:
			if( ccb->cam_dxfer_len < ( sizeof( *vh ) + sizeof( *bdc ) ) ) {
				return( sdmmc_error( hba, ccb, EINVAL ) );
			}
			vh->peripheral				= D_DIR_ACC;
			vh->page_code				= INQ_PC_BLOCK_DEVICE_CHARACTERISTICS;
			vh->page_length				= ENDIAN_BE16( 60 );

			bdc->medium_rotation_rate	= ENDIAN_BE16( 1 );		// 1 indicates non rotating
			bdc->nominal_form_factor	= 0;
			break;

		default:
			return( sdmmc_error( hba, ccb, EINVAL ) );
			break;
	}

	return( cstatus );
}

int sdmmc_std_inquiry( SIM_HBA *hba, CCB_SCSIIO *ccb )
{
	SIM_SDMMC_EXT	*ext;
	SCSI_INQUIRY	*iptr;

	ext		= (SIM_SDMMC_EXT *)hba->ext;
	iptr	= CAM_DATA_PTR_V( ccb->cam_data.cam_data_ptr );

	memset( iptr, 0, sizeof( *iptr ) );

	iptr->peripheral	= D_DIR_ACC;
	iptr->rmb			= ( ext->hc_inf.caps & HC_CAP_SLOT_TYPE_EMBEDDED ) ? CAM_FALSE : CAM_TRUE;
	iptr->version		= INQ_VER_SPC3;		// SPC-3
	iptr->adlen			= 96 - 5;	// nbytes after adlen field

	strlcpy( (char *)iptr->vend_id, "SDMMC:", sizeof( iptr->vend_id ) );
	strlcpy( (char *)iptr->prod_id, (char *)ext->dev_inf.pnm, sizeof( iptr->prod_id ) );

	if( ( ext->dev_inf.prv >> 4 ) < 10 )
		iptr->prod_rev[0] = ( ext->dev_inf.prv >> 4 ) + '0';
	else
		iptr->prod_rev[0] = ( ext->dev_inf.prv >> 4 ) - 10 + 'a';

	iptr->prod_rev[1] = '.';

	if( ( ext->dev_inf.prv & 0x0F) < 10 )
		iptr->prod_rev[2] = ( ext->dev_inf.prv & 0x0F ) + '0';
	else
		iptr->prod_rev[2] = ( ext->dev_inf.prv & 0x0F ) - 10 + 'a';

	if( ccb->cam_dxfer_len > 56 ) {			// check len for vend_spc
		strlcpy( (char *)iptr->vend_spc, ext->targets[ccb->cam_ch.cam_target_id].partitions[ccb->cam_ch.cam_target_lun].name, sizeof( iptr->vend_spc ) );
	}

	return( CAM_REQ_CMP );
}

int sdmmc_inquiry( SIM_HBA *hba, CCB_SCSIIO *ccb )
{
	int				status;

	if( ( ccb->cam_cdb_io.cam_cdb_bytes[1] & INQ_OPT_EVPD ) ) {
		status = sdmmc_evpd_inquiry( hba, ccb );
	}
	else {
		status = sdmmc_std_inquiry( hba, ccb );
	}

	return( status );
}

int sdmmc_spindle( SIM_HBA *hba, CCB_SCSIIO *ccb )
{
	int				status;

	if( ( status = sdmmc_unit_ready( hba, ccb ) ) != CAM_REQ_CMP ) {
		return( status );
	}

	switch( ccb->cam_cdb_io.cam_cdb_bytes[4] ) {
		case LD_CMD_STOP:
		case LD_CMD_START:
			status	= CAM_REQ_CMP;
			break;

		case LD_CMD_EJECT:
		case LD_CMD_LOAD:
		default:
			status = sdmmc_error( hba, ccb, EINVAL );
			break;
	}
	return( status );
}

int sdmmc_capacity( SIM_HBA *hba, CCB_SCSIIO *ccb )
{
	SIM_SDMMC_EXT	*ext;
	SDMMC_TARGET	*targ;
	READ_CAPACITY	*cptr;
	uint32_t		nlba;
	int				status;

	ext		= (SIM_SDMMC_EXT *)hba->ext;
	cptr	= CAM_DATA_PTR_V( ccb->cam_data.cam_data_ptr );
	targ	= &ext->targets[ccb->cam_ch.cam_target_id];
	nlba	= targ->partitions[ccb->cam_ch.cam_target_lun].nlba;

	if( ( status = sdmmc_unit_ready( hba, ccb ) ) == CAM_REQ_CMP ) {
		nlba--;
		cptr->lba				= ENDIAN_BE32( nlba );
		cptr->blk_size			= ENDIAN_BE32( targ->blksz );
	}

	return( status );
}

int sdmmc_write_same( SIM_HBA *hba, CCB_SCSIIO *ccb )
{
	SIM_SDMMC_EXT	*ext;
	CDB				*cdb;
	SDMMC_PARTITION	*part;
	uint64_t		lba;
	uint32_t		nlba;
	int				status;

	ext		= (SIM_SDMMC_EXT *)hba->ext;
	cdb		= cam_cdb( ccb );
	status	= CAM_REQ_CMP;
	part	= &ext->targets[ccb->cam_ch.cam_target_id].partitions[ccb->cam_ch.cam_target_lun];
	lba		= ENDIAN_BE64( UNALIGNED_RET64( cdb->write_same16.lba ) );
	nlba	= ENDIAN_BE32( UNALIGNED_RET32( cdb->write_same16.transfer_len ) );
	lba		+= part->slba;

	if( ( status = sdmmc_unit_ready( hba, ccb ) ) != CAM_REQ_CMP ) {
		return( status );
	}

	if( !( ext->dev_inf.caps & DEV_CAP_TRIM ) || !( cdb->write_same16.opt & WS_OPT_UNMAP ) ) {
		status = sdmmc_error( hba, ccb, EINVAL );
	}
	else if( ( status = sdio_erase( ext->device, part->config, MMC_ERASE_TRIM, lba, nlba ) ) ) {
		status = sdmmc_error( hba, ccb, status );
	}

	if( status == EOK ) {
		part->tc += nlba;
	}

	return( status ? status : CAM_REQ_CMP );
}

int sdmmc_erase12( SIM_HBA *hba, CCB_SCSIIO *ccb )
{
	SIM_SDMMC_EXT	*ext;
	CDB				*cdb;
	SDMMC_PARTITION	*part;
	uint32_t		lba;
	uint32_t		nlba;
	uint32_t		egs;
	uint32_t		pend;
	int				status;

	ext		= (SIM_SDMMC_EXT *)hba->ext;
	cdb		= cam_cdb( ccb );
	part	= &ext->targets[ccb->cam_ch.cam_target_id].partitions[ccb->cam_ch.cam_target_lun];

	egs		= ext->dev_inf.erase_size / 512;
	lba		= ENDIAN_BE32( UNALIGNED_RET32( cdb->erase12.lba ) );
	nlba	= ENDIAN_BE32( UNALIGNED_RET32( cdb->erase12.transfer_len ) );
	lba		+= part->slba;
	pend	= part->slba + part->nlba - 1;

	if( ( status = sdmmc_unit_ready( hba, ccb ) ) == CAM_REQ_CMP ) {
		if( lba > pend || ( lba + nlba - 1 ) > pend ||
				( lba % egs ) || ( nlba % egs ) ) {
			status = sdmmc_error( hba, ccb, EINVAL );
		}
		else if( ( status = sdio_erase( ext->device, part->config, MMC_ERASE_SECURE, lba, nlba ) ) ) {
			status = sdmmc_error( hba, ccb, status );
		}
	}

	if( status == EOK ) {
		part->ec += nlba;
	}

	return( status ? status : CAM_REQ_CMP );
}

int sdmmc_sync( SIM_HBA *hba, CCB_SCSIIO *ccb )
{
	SIM_SDMMC_EXT	*ext;
	int				status;

	ext		= (SIM_SDMMC_EXT *)hba->ext;

	if( ( status = sdmmc_unit_ready( hba, ccb ) ) == CAM_REQ_CMP ) {
		if( ( ext->dev_inf.caps & DEV_CAP_CACHE ) && ( ext->eflags & SDMMC_EFLAG_CACHE ) ) {
			if( ( status = sdio_cache( ext->device, SDIO_CACHE_FLUSH, SDIO_TIME_DEFAULT * 5 ) ) != EOK ) {
				status = sdmmc_error( hba, ccb, status );
			}
		}
	}
	return( status ? status : CAM_REQ_CMP );
}

int sdmmc_timer_settime( int tid, uint64_t nsec, int repeat )
{
	struct timespec		ts;
	struct itimerspec	value;

	memset( &value, 0, sizeof( value ) );

	if( nsec ) {
		nsec2timespec( &ts, nsec );
		value.it_value = ts;
		if( repeat ) {
			value.it_interval.tv_sec	= ts.tv_sec;
			value.it_interval.tv_nsec	= ts.tv_nsec;
		}
	}

	if( timer_settime( tid, 0, &value, NULL ) == -1 ) {
		return( errno );	
	}

	return( EOK );	
}	

int sdmmc_pm( SIM_HBA *hba, int op )
{
	SIM_SDMMC_EXT	*ext;

	ext = (SIM_SDMMC_EXT *)hba->ext;

	if( ext->pm_state == op ) {
		return( EOK );
	}

#if 0
{
	static const char	*name[3] = { "idle", "active", "sleep" };
	cam_slogf( _SLOGC_SIM_MMC, _SLOG_ERROR, 1, 1, "%s: %s", __FUNCTION__, name[op] );
}
#endif

	switch( op ) {
		case PM_IDLE:
			sdio_pwrmgnt( ext->device, PM_IDLE );

			if( ( ext->hc_inf.caps & HC_CAP_SLEEP ) ) {
				sdmmc_timer_settime( ext->pm_timerid, ext->pm_sleep_time_ns, CAM_FALSE );
			}
			else {
				sdmmc_timer_settime( ext->pm_timerid, 0, CAM_FALSE );
			}

			break;

		case PM_ACTIVE:
			if( ext->pm_state == PM_SLEEP ) {
				if( ( ext->dev_inf.caps & DEV_CAP_ASSD ) ) {
					atomic_set( &ext->eflags, SDMMC_EFLAG_ASSD_INIT );
				}
			}
			sdmmc_timer_settime( ext->pm_timerid, ext->pm_idle_time_ns, CAM_TRUE );

			break;

		case PM_SLEEP:
			sdmmc_timer_settime( ext->pm_timerid, 0, CAM_FALSE );
			if( sdio_pwrmgnt( ext->device, PM_SLEEP ) ) {			// sleep failed, re-arm timer
				op = PM_IDLE;
				sdmmc_timer_settime( ext->pm_timerid, ext->pm_sleep_time_ns, CAM_FALSE );
			}
			break;

		default:
			return( EINVAL );
	}
		
	ext->pm_state = op;

	return( EOK );
}


int sdmmc_pwroff_notify( SIM_HBA *hba, uint8_t cfg )
{
	SIM_SDMMC_EXT	*ext;
	struct sdio_cmd	*cmd;
	int				status;
	uint32_t		cstatus;
	uint8_t			*ecsd;

	ext		= (SIM_SDMMC_EXT *)hba->ext;

	if( !( ext->dev_inf.caps & DEV_CAP_PWROFF_NOTIFY ) ) {
		return( ENOTSUP );
	}

	if( ( cmd = sdio_alloc_cmd( ) ) == NULL ) {
		return( ENOMEM );
	}

	sdio_setup_cmd( cmd, SCF_CTYPE_AC | SCF_RSP_R1B, MMC_SWITCH,
			( MMC_SWITCH_MODE_WRITE << 24 ) | ( ECSD_POWER_OFF_NOTIFICATION << 16 ) | ( cfg << 8 ) | MMC_SWITCH_CMDSET_DFLT );

	if( ( status = sdio_send_cmd( ext->device, cmd, NULL, SDIO_TIME_DEFAULT, 0 ) ) != EOK ) {
		sdio_cmd_status( cmd, &cstatus, NULL );
		if( cstatus == CS_CMD_TO_ERR && cfg == ECSD_POWER_OFF_LONG ) {
			ecsd = sdio_get_raw_ecsd( ext->device );
			delay( 10 * ecsd[ECSD_POWER_OFF_LONG_TIME] );
		}
	}

	sdio_free_cmd( cmd );

	return( status );
}



// WARNING:  BKOPS enable is permanent (not in spec).
//           Firmware will not do them even if
//           required to continue functioning.
int sdmmc_bkops_cfg( SIM_HBA *hba )
{
	SIM_SDMMC_EXT	*ext;
	int				status;

	ext		= (SIM_SDMMC_EXT *)hba->ext;

	if( !( ext->dev_inf.caps & DEV_CAP_BKOPS ) ) {
		return( ENOTSUP );
	}

	if( ( status = sdio_mmc_switch( ext->device, MMC_SWITCH_CMDSET_DFLT, MMC_SWITCH_MODE_WRITE, ECSD_BKOPS_EN, ECSD_BKOPS_ENABLE, SDIO_TIME_DEFAULT ) ) != EOK ) {
		cam_slogf( _SLOGC_SIM_MMC, _SLOG_ERROR, 1, 1, "%s: switch ext_csd_bkops_en", __FUNCTION__ );
	}

	return( status );
}

int sdmmc_bkops( SIM_HBA *hba, int tick )
{
	SIM_SDMMC_EXT	*ext;
	uint8_t			ecsd[MMC_EXT_CSD_SIZE];

	ext		= (SIM_SDMMC_EXT *)hba->ext;

	if( !( ext->eflags & SDMMC_EFLAG_BKOPS ) ) {
		return( EOK );
	}

	if( tick ) {							// Timer event, poll status
		sdmmc_pm( hba, PM_ACTIVE );

		if( ext->bkops_status ) {
			ext->bkops_ticks++;
		}

		if( sdio_send_ext_csd( ext->device, ecsd ) == EOK ) {
			if( ext->bkops_status != ecsd[ECSD_BKOPS_STATUS] ) {
				ext->bkops_ticks	= 0;
				ext->bkops_status	= ecsd[ECSD_BKOPS_STATUS];
			}
		}
	}

	switch( ext->bkops_status ) {
		case BKOPS_STATUS_OPERATIONS_NONE:
			break;
		
		case BKOPS_STATUS_OPERATIONS_NON_CRITICAL:
			if( ext->bkops_ticks < BKOPS_NC_TICKS ) {
				break;
			}

			// fall through

		case BKOPS_STATUS_OPERATIONS_IMPACTED:
			if( ext->bkops_ticks < BKOPS_IMPACTED_TICKS ) {
				break;
			}

			// fall through

		case BKOPS_STATUS_OPERATIONS_CRITICAL:
			if( sdio_mmc_switch( ext->device, MMC_SWITCH_CMDSET_DFLT, MMC_SWITCH_MODE_WRITE, ECSD_BKOPS_START, ECSD_BKOPS_INITIATE, SDIO_TIME_DEFAULT ) == EOK ) {
				ext->bkops_status	= BKOPS_STATUS_OPERATIONS_NONE;
				ext->bkops_ticks	= 0;
			}
			else {
				cam_slogf( _SLOGC_SIM_MMC, _SLOG_ERROR, 1, 1, "%s: BKOPS_START failure", __FUNCTION__ );
			}
			break;
	}
	
	return( EOK );
}

int sdmmc_write_protect( SIM_HBA *hba, int op, int partition, int mode, uint32_t lba, uint32_t nlba, uint64_t *prot )
{
	SIM_SDMMC_EXT		*ext;
	struct sdio_device	*dev;
	struct sdio_cmd		*cmd;
	sdio_sge_t			sge;
	uint8_t				*ecsd;
	int					sz;
	int					blksz;
	int					status;
	void				*pdata;
	uint8_t				pmode;
	uint64_t			addr;
	uint64_t			wp_grp_size;

	ext			= (SIM_SDMMC_EXT *)hba->ext;
	dev			= ext->device;
	blksz		= ext->dev_inf.sector_size;
	wp_grp_size = ext->dev_inf.wp_size / 512;
	sz			= MMC_SEND_WRITE_PROT_SIZE;
	ecsd		= sdio_get_raw_ecsd( ext->device );

	if( !wp_grp_size ) {
		return( ENOTSUP );
	}

	if( ( lba % wp_grp_size ) ) {
		return( EINVAL );
	}

	if( ( status = sdio_set_partition( dev, partition ) ) != EOK ) {
		cam_slogf( _SLOGC_SIM_MMC, _SLOG_ERROR, 1, 1, "%s: sdio_set_partition failure %s", __FUNCTION__, strerror( status ) );
		return( status );
	}

	if( op == MMC_SET_WRITE_PROT ) {
		switch( ( partition & MMC_PART_MSK ) ) {
			case MMC_PART_USER:
				if( mode & SDMMC_WP_MODE_PWR_WP_EN ) {	// Power-on
					pmode = ecsd[ECSD_USER_WP] & ~( ECSD_USER_WP_US_PWR_WP_EN | ECSD_USER_WP_US_PERM_WP_EN | ECSD_USER_WP_US_PWR_WP_DIS );
					pmode |= mode;
				}
				else {									// Temporary 
					pmode = ecsd[ECSD_USER_WP] & ~( ECSD_USER_WP_US_PWR_WP_EN | ECSD_USER_WP_US_PERM_WP_EN);
				}

				if( ( status = sdio_mmc_switch( dev, MMC_SWITCH_CMDSET_DFLT, MMC_SWITCH_MODE_WRITE, ECSD_USER_WP, pmode, SDIO_TIME_DEFAULT ) ) != EOK ) {
					cam_slogf( _SLOGC_SIM_MMC, _SLOG_ERROR, 1, 1, "%s: switch ext_csd_user_wp mode %d, user_wp 0x%x", __FUNCTION__, pmode, ecsd[ECSD_USER_WP] );
					return( status );
				}
				break;

			case MMC_PART_BOOT1:
			case MMC_PART_BOOT2:
				if( ( status = sdio_mmc_switch( dev, MMC_SWITCH_CMDSET_DFLT, MMC_SWITCH_MODE_WRITE, ECSD_BOOT_WP, mode, SDIO_TIME_DEFAULT ) ) != EOK ) {
					cam_slogf( _SLOGC_SIM_MMC, _SLOG_ERROR, 1, 1, "%s: switch ext_csd_boot_wp", __FUNCTION__ );
					return( status );
				}
				break;

			default:
				return( EINVAL );
		}
	}

	if( ( cmd = sdio_alloc_cmd( ) ) == NULL ) {
		return( ENOMEM );
	}

	switch( op ) {
		case MMC_SET_WRITE_PROT:
		case MMC_CLR_WRITE_PROT:
			while( nlba ) {
				addr = ( ext->dev_inf.caps & DEV_CAP_HC ) ? lba : ( lba * blksz );
				sdio_setup_cmd( cmd, SCF_CTYPE_AC | SCF_RSP_R1B | SCF_WAIT_DRDY, op, addr );
				if( ( status = sdio_send_cmd( dev, cmd, NULL, SDMMC_TIME_DEFAULT * 1000, 0 ) ) != EOK ) {
					break;
				}

				lba		+= wp_grp_size;
				nlba	-= min( nlba, wp_grp_size );
			}
			break;

		case MMC_SEND_WRITE_PROT:
		case MMC_SEND_WRITE_PROT_TYPE:
			addr = ( ext->dev_inf.caps & DEV_CAP_HC ) ? lba : ( lba * blksz );
			if( op == MMC_SEND_WRITE_PROT_TYPE && ext->dev_inf.spec_rev >= ECSD_REV_V4_41 ) {
				sz = MMC_SEND_WRITE_PROT_TYPE_SIZE;
			}

			if( ( pdata = sdio_alloc( sz ) ) == NULL ) {
				status = ENOMEM; break;
			}

			sge.sg_count = sz; sge.sg_address = CAM_DATA_PTR_P( pdata );
			sdio_setup_cmd( cmd, SCF_CTYPE_ADTC | SCF_RSP_R1, op, addr );
			sdio_setup_cmd_io( cmd, SCF_DIR_IN, 1, sz, &sge, 1, NULL );
			if( ( status = sdio_send_cmd( dev, cmd, NULL, SDMMC_TIME_DEFAULT * 1000, 0 ) ) == EOK ) {
				*prot = ( sz == 4 ) ?	ENDIAN_BE32( *(uint32_t *)pdata ) :
										ENDIAN_BE64( *(uint64_t *)pdata );
			}

			sdio_free( pdata, sz );
			break;

		default:
			status = EINVAL; break;
	}

	sdio_free_cmd( cmd );

	return( status );
}

int sdmmc_set_partition_attr( SIM_HBA *hba, int partition, int flags )
{
	SIM_SDMMC_EXT		*ext;
	SDMMC_TARGET		*targ;
	SDMMC_PARTITION		*part;
	int			pidx;
	int			tidx;
	
	ext		= (SIM_SDMMC_EXT *)hba->ext;

	for( tidx = 0; tidx <= ext->ntargs; tidx++ ) {
		targ = &ext->targets[tidx];
		for( pidx = 0; pidx <= targ->nluns; pidx++ ) {
			part			= &targ->partitions[pidx];
			if( ( part->config & MMC_PART_MSK ) == partition ) {
				part->pflags |= flags;
			}
		}
	}
	return( CAM_SUCCESS );
}

int sdmmc_reset( SIM_HBA *hba )
{
	SIM_SDMMC_EXT	*ext;

	ext		= (SIM_SDMMC_EXT *)hba->ext;

	sdio_reset( ext->device );

	if( ( ext->dev_inf.caps & DEV_CAP_CACHE ) && ( ext->eflags & SDMMC_EFLAG_CACHE ) ) {
		// Mark device user partition as read only/write protected after a reset when eMMC cache is enabled.
		// This is done since we don't know if there was pending data in the eMMC cache.
		sdmmc_set_partition_attr( hba, MMC_PART_USER, SDMMC_PFLAG_WP );
		ext->eflags	&= ~SDMMC_EFLAG_CACHE;
		cam_slogf( _SLOGC_SIM_MMC, _SLOG_ERROR, 1, 1, "%s:  sdio_reset cache lost - device marked read only", __FUNCTION__ );
	}

	return( EOK );
}

int sdmmc_rw( SIM_HBA *hba, SDMMC_PARTITION *part, int flgs, uint32_t addr, int dlen, sdio_sge_t *sgl, int sgc, void *mhdl, uint32_t timeout )
{
	SIM_SDMMC_EXT		*ext;
	struct sdio_cmd		*cmd;
	sdio_dev_info_t		*di;
	struct sdio_device	*dev;
	int					rst;
	int					op;
	int					blks;
	int					blksz;
	int					status;
	int					bus_err;
	uint32_t			cstatus;
	uint32_t			rsp[4];

	ext		= (SIM_SDMMC_EXT *)hba->ext;
	dev		= ext->device;
	di		= &ext->dev_inf;

	rst		= CAM_FALSE;
	bus_err	= CAM_FALSE;
	timeout	*= 1000;
	blksz	= di->sector_size;
	blks	= dlen / blksz;
	addr	= ( di->caps & DEV_CAP_HC ) ? addr : ( addr * blksz );
	op		= ( flgs & SCF_DIR_IN ) ? MMC_READ_SINGLE_BLOCK : MMC_WRITE_BLOCK;

	if( dlen > blksz ) {
		if( !( ext->hc_inf.caps & HC_CAP_ACMD12 ) ) {
			if( ( di->caps & DEV_CAP_CMD23 ) ) {
				flgs |= SCF_SBC;
			}
		}
		flgs |= SCF_MULTIBLK;
	}

	if( ( flgs & SCF_MULTIBLK ) ) {
		op++;
	}

	if( ( cmd = sdio_alloc_cmd( ) ) == NULL ) {
		return( ENOMEM );
	}

	sdio_setup_cmd( cmd, SCF_CTYPE_ADTC | SCF_RSP_R1, op, addr );
	sdio_setup_cmd_io( cmd, flgs, blks, blksz, sgl, sgc, mhdl );
	status = sdio_send_cmd( dev, cmd, NULL, timeout, 0 );
	sdio_cmd_status( cmd, &cstatus, rsp );
	sdio_free_cmd( cmd );

	if( status ) {
		if( status == ENXIO ) {				// card has been removed
			return( status );
		}

		if( sdio_send_status( dev, rsp, 0 ) || ( rsp[0] & ( CDS_READY_FOR_DATA | CDS_CUR_STATE_MSK ) ) != ( CDS_READY_FOR_DATA | CDS_CUR_STATE_TRAN ) ) {
			if( sdio_stop_transmission( dev, 0 ) ) {
				status = ETIMEDOUT;
			}
		}
	}
	else {
		if( ( flgs & SCF_MULTIBLK ) ) {
			if( ( !( flgs & SCF_SBC ) && ( dlen > blksz ) && !( ext->hc_inf.caps & HC_CAP_ACMD12 ) ) ) {
				if( sdio_stop_transmission( dev, 0 ) ) {
					status = ETIMEDOUT;
				}
			}
		}

		if( status == EOK && ( flgs & SCF_DIR_OUT ) ) {
			if( ( status = sdio_wait_card_status( dev, rsp, CDS_READY_FOR_DATA | CDS_CUR_STATE_MSK, CDS_READY_FOR_DATA | CDS_CUR_STATE_TRAN, timeout ) ) ) {
				sdio_stop_transmission( dev, 0 );
			}
		}

			// don't update counts for RPMB
		if( ( part->config & MMC_PART_MSK ) != MMC_PART_RPMB ) {
			if( ( flgs & SCF_DIR_IN ) ) {
				part->rc += blks;
			}
			else {
				part->wc += blks;
			}
		}
	}

	switch( cstatus ) {
		case CS_CMD_CMP:
			break;

		case CS_CMD_TO_ERR:
		case CS_CMD_CRC_ERR:
		case CS_CMD_END_ERR:
			status	= ETIMEDOUT;		// set timeout so CAM layer will retry
			break;


		case CS_DATA_TO_ERR:
			status	= ETIMEDOUT;
			break;

		case CS_DATA_CRC_ERR:
		case CS_DATA_END_ERR:
			bus_err	= CAM_TRUE;
			status	= ETIMEDOUT;
			break;

		default:
			status	= ETIMEDOUT;
			break;
	}

	if( rsp[0] ) {
		if( ( rsp[0] & CDS_URGENT_BKOPS ) ) {
			ext->bkops_status = ECSD_BS_OPERATIONS_CRITICAL;
		}
		if( ( rsp[0] & CDS_ERROR ) ) {
			status = EIO;
		}
		if( ( rsp[0] & CDS_WP_VIOLATION ) ) {
			status = EROFS;
		}
		if( ( rsp[0] & CDS_CARD_IS_LOCKED ) ) {
			status = EACCES;
		}
		if( ( rsp[0] & CDS_CARD_ECC_FAILED ) ) {
			status = EIO;
		}
	}

	if( status ) {
			// reset when we are not ready for data and in the transfer state
		if( rst || sdio_send_status( dev, rsp, 0 ) || ( rsp[0] & ( CDS_READY_FOR_DATA | CDS_CUR_STATE_MSK ) ) != ( CDS_READY_FOR_DATA | CDS_CUR_STATE_TRAN ) ) {
			sdmmc_reset( hba );
		}

		if( bus_err && ( ext->instance.ident.dtype == DEV_TYPE_SD ) ) {	// inform sdio layer of bus error (layer will reduce transfer mode after 2 errors).
			sdio_bus_error( ext->device );
		}

		sdio_dev_info( ext->device, &ext->dev_inf );	// device info may have been updated after bus_err or reset

		if( status == ETIMEDOUT && ( flgs & SCF_SBC ) ) {
				// some SD cards have buggy firmware and will
				// hang when pre-defined block counts are used
			if( ext->instance.ident.dtype == DEV_TYPE_SD ) {
				di->caps &= ~DEV_CAP_CMD23;
				cam_slogf( _SLOGC_SIM_MMC, _SLOG_ERROR, 1, 1, "%s:  disable CMD23", __FUNCTION__ );
			}
		}

		cam_slogf( _SLOGC_SIM_MMC, _SLOG_ERROR, 1, 1, "%s:  flgs 0x%x, lba %d, len %d, status 0x%x, cstatus 0x%x, rsp[0] 0x%x",
			__FUNCTION__, flgs, addr, dlen, status, cstatus, rsp[0] );
	}

	return( status );
}

#ifdef SDMMC_WRITE_VERIFY
int sdmmc_write_verify( SIM_HBA *hba, CCB_SCSIIO *ccb )
{
	SIM_SDMMC_EXT	*ext;
	SDMMC_PARTITION	*part;
	uint32_t		lba;
	int				status;
	int				sgc;
	sdio_sge_t		wsgp;
	sdio_sge_t		*sgp;
	sdio_sge_t		sge;
	int				verification;	

	char			*rptr;
	char			*wptr;

	ext		= (SIM_SDMMC_EXT *)hba->ext;
	part	= &ext->targets[ccb->cam_ch.cam_target_id].partitions[ccb->cam_ch.cam_target_lun];
	lba		= ENDIAN_BE32( UNALIGNED_RET32( &ccb->cam_cdb_io.cam_cdb_bytes[2] ) );

	if( part->blk_shft ) {
		lba <<= part->blk_shft;
	}	

	lba		+= part->slba;

	if( ( ccb->cam_ch.cam_flags & CAM_SCATTER_VALID ) ) {
		sgc				= ccb->cam_sglist_cnt;
		sgp				= (sdio_sge_t *)ccb->cam_data.cam_sg_ptr;
	}
	else {
		sgc				= 1;
		sgp				= &sge;
		sgp->sg_count	= ccb->cam_dxfer_len;
		sgp->sg_address	= ccb->cam_data.cam_data_ptr;
	}

	verification	= CAM_SUCCESS;
	rptr			= (char *)ext->ver_vaddr;
	wsgp.sg_count	= ccb->cam_dxfer_len;
	wsgp.sg_address	= ext->ver_paddr;

	if( ccb->cam_dxfer_len > SDMMC_VER_BSIZE ) {
		cam_slogf( _SLOGC_SIM_MMC, _SLOG_ERROR, 1, 1, "%s:  request too large (%d)", __FUNCTION__, ccb->cam_dxfer_len );
		return( CAM_REQ_CMP );
	}

	if( ( status = sdmmc_rw( hba, part, SCF_DIR_IN | SCF_DATA_PHYS, lba, ccb->cam_dxfer_len, &wsgp, 1, NULL, ccb->cam_timeout ) ) != EOK ) {
		cam_slogf( _SLOGC_SIM_MMC, _SLOG_ERROR, 1, 1, "%s:  sdmmc_rw failed (%d)", __FUNCTION__, status );
		return( status );
	}
	else {
		for( ; sgc; sgc--, sgp++ ) {
			wptr = (char *)sgp->sg_address;
			if( ( ccb->cam_ch.cam_flags & CAM_DATA_PHYS ) ) {
				if( ( wptr = mmap( NULL, sgp->sg_count, PROT_READ | PROT_WRITE | PROT_NOCACHE, MAP_SHARED | MAP_PHYS, -1, sgp->sg_address ) ) == MAP_FAILED ) {
					verification = CAM_FAILURE; break;
				}
			}

			if( memcmp( wptr, rptr, sgp->sg_count ) ) {
				verification = CAM_FAILURE;
				cam_slogf( _SLOGC_SIM_MMC, _SLOG_ERROR, 1, 1, "%s:  write verification failed wrote", __FUNCTION__ );
				cam_dump( wptr, sgp->sg_count );
				cam_slogf( _SLOGC_SIM_MMC, _SLOG_ERROR, 1, 1, "%s:  write verification failed read", __FUNCTION__ );
				cam_dump( rptr, sgp->sg_count );
			}

			if( ( ccb->cam_ch.cam_flags & CAM_DATA_PHYS ) ) {
				munmap( wptr, sgp->sg_count );
			}

			rptr += sgp->sg_count;
			wptr += sgp->sg_count;
		}
	}

	if( verification != CAM_SUCCESS ) {
		cam_slogf( _SLOGC_SIM_MMC, _SLOG_ERROR, 1, 1, "%s:  write verification failed", __FUNCTION__ );
		status = sdmmc_error( hba, ccb, EIO );
	}

	return( status ? status : CAM_REQ_CMP );
}
#endif

int sdmmc_read_write( SIM_HBA *hba, CCB_SCSIIO *ccb, int flgs )
{
	SIM_SDMMC_EXT	*ext;
	SDMMC_PARTITION	*part;
	uint32_t		lba;
	int				status;
	int				sgc;
	sdio_sge_t		*sgp;
	sdio_sge_t		sge;
#ifdef SDMMC_SIM_RETRY
	int			retry;
#endif

	ext		= (SIM_SDMMC_EXT *)hba->ext;
	part	= &ext->targets[ccb->cam_ch.cam_target_id].partitions[ccb->cam_ch.cam_target_lun];

	if( ( status = sdmmc_unit_ready( hba, ccb ) ) != CAM_REQ_CMP ) {
		return( status );
	}

	if( ( ext->dev_inf.flags & DEV_FLAG_CARD_LOCKED ) ) {
		return( sdmmc_error( hba, ccb, EACCES ) );
	}

	if( ( flgs & SCF_DIR_OUT ) && ( part->pflags & SDMMC_PFLAG_WP ) ) {
		return( sdmmc_error( hba, ccb, EROFS ) );
	}

	if( ( part->config & MMC_PART_MSK ) == MMC_PART_RPMB ) {		// no read/write to RPMB
		return( CAM_PROVIDE_FAIL );
	}

	if( ( status = sdio_set_partition( ext->device, part->config ) ) != EOK ) {
		cam_slogf( _SLOGC_SIM_MMC, _SLOG_ERROR, 1, 1, "%s: sdio_set_partition failure %s", __FUNCTION__, strerror( status ) );
		sdmmc_reset( hba );
		return( sdmmc_error( hba, ccb, ETIMEDOUT ) );
	}

	sdmmc_bkops( hba, CAM_FALSE );	// Check for urgent background operations

	if( ( ccb->cam_ch.cam_flags & CAM_SCATTER_VALID ) ) {
		sgc				= ccb->cam_sglist_cnt;
		sgp				= (sdio_sge_t *)ccb->cam_data.cam_sg_ptr;
	}
	else {
		sgc				= 1;
		sgp				= &sge;
		sgp->sg_count	= ccb->cam_dxfer_len;
		sgp->sg_address	= ccb->cam_data.cam_data_ptr;
	}

	if( ( ccb->cam_ch.cam_flags & CAM_DATA_PHYS ) ) {
		flgs |= SCF_DATA_PHYS;
	}

	lba		= ENDIAN_BE32( UNALIGNED_RET32( &ccb->cam_cdb_io.cam_cdb_bytes[2] ) );

	if( part->blk_shft ) {
		lba <<= part->blk_shft;
	}	

	lba		+= part->slba;

#ifdef SDMMC_SIM_RETRY
	retry = SDMMC_RW_RETRIES;
	do {
		if( ( status = sdmmc_rw( hba, part, flgs, lba, ccb->cam_dxfer_len, sgp, sgc, ccb->cam_req_map, ccb->cam_timeout ) ) == EOK ) {
			break;
		}
	} while( --retry && status == ETIMEDOUT );

	if( status ) {
		if( status == ETIMEDOUT ) {					// map timeout to eio
			status = EIO;
		}
		status = sdmmc_error( hba, ccb, status );
	}
#else
	if( ( status = sdmmc_rw( hba, part, flgs, lba, ccb->cam_dxfer_len, sgp, sgc, ccb->cam_req_map, ccb->cam_timeout ) ) != EOK ) {
		status = sdmmc_error( hba, ccb, status );
	}
#endif
	return( status ? status : CAM_REQ_CMP );
}

int sdmmc_dsn_devctl( SIM_HBA *hba, CCB_DEVCTL *ccb )
{
	SIM_SDMMC_EXT	*ext;
	char			*dsn;

	ext		= (SIM_SDMMC_EXT *)hba->ext;
	dsn		= (char *)(ccb->cam_devctl_data);
	ccb->cam_devctl_status = EOK;
	
	memset( dsn, 0, ccb->cam_devctl_size );

	if( sdmmc_unit_ready( hba, (CCB_SCSIIO *)ccb ) != CAM_REQ_CMP ) {
		ccb->cam_devctl_status = EIO;
	}
	else {
		snprintf( dsn, ccb->cam_devctl_size, "%08x", ext->dev_inf.psn );
	}

	return( CAM_REQ_CMP );
}

int sdmmc_wp_devctl( SIM_HBA *hba, CCB_DEVCTL *ccb )
{
	SIM_SDMMC_EXT		*ext;
	SDMMC_WRITE_PROTECT	*wp;
	SDMMC_PARTITION		*part;
	int					op;
	uint32_t			nlba;
	uint32_t			slba;
	uint32_t			pend;
	int					status;

	ext			= (SIM_SDMMC_EXT *)hba->ext;
	wp			= (SDMMC_WRITE_PROTECT *)ccb->cam_devctl_data;
	part		= &ext->targets[ccb->cam_ch.cam_target_id].partitions[ccb->cam_ch.cam_target_lun];
	nlba		= max( 1, wp->nlba );
	slba		= part->slba + wp->lba;
	pend		= part->slba + part->nlba - 1;
	wp->mode	&= SDMMC_WP_MODE_PWR_WP_EN;	// limit mode to nothing permanent
	status		= EOK;

	switch( wp->action ) {
		case SDMMC_WP_ACTION_CLR:
			op = MMC_CLR_WRITE_PROT; break;
		case SDMMC_WP_ACTION_SET:
			op = MMC_SET_WRITE_PROT; break;
		case SDMMC_WP_ACTION_PROT:
			op = MMC_SEND_WRITE_PROT; break;
		case SDMMC_WP_ACTION_PROT_TYPE:
			op = MMC_SEND_WRITE_PROT_TYPE; break;
		default:
			op = 0; break;				
	}

	if( sdmmc_unit_ready( hba, (CCB_SCSIIO *)ccb ) != CAM_REQ_CMP ) {
		status = EIO;
	}
	else if( !op || slba > pend || ( slba + nlba - 1 ) > pend ) {
		status = EINVAL;
	}
	else if( ext->instance.ident.dtype != DEV_TYPE_MMC ) {
		status	= ENOTSUP;
	}
	else if( ( status = sdmmc_write_protect( hba, op, part->config, wp->mode, slba, nlba, &wp->prot ) ) ) {
	}

	ccb->cam_devctl_status = status;

	return( CAM_REQ_CMP );
}

int sdmmc_erase_devctl( SIM_HBA *hba, CCB_DEVCTL *ccb )
{
	SIM_SDMMC_EXT		*ext;
	SDMMC_ERASE			*erase;
	SDMMC_PARTITION		*part;
	uint64_t			egs;
	uint64_t			nlba;
	uint64_t			slba;
	uint64_t			pend;
	int					status;

	ext				= (SIM_SDMMC_EXT *)hba->ext;
	erase			= (SDMMC_ERASE *)ccb->cam_devctl_data;
	part			= &ext->targets[ccb->cam_ch.cam_target_id].partitions[ccb->cam_ch.cam_target_lun];
	egs				= ext->dev_inf.erase_size / 512;
	nlba			= erase->nlba;
	slba			= part->slba + erase->lba;
	pend			= part->slba + part->nlba - 1;
	status			= EINVAL;

	if( sdmmc_unit_ready( hba, (CCB_SCSIIO *)ccb ) != CAM_REQ_CMP ) {
		status = EIO;
	}
	else if( slba > pend || ( slba + nlba - 1 ) > pend ) {
		status = EINVAL;			// verify request is within partition
	}
	else {
		switch( erase->action ) {
			case SDMMC_ERASE_ACTION_NORMAL:
					// verify for erase group alignment
				if( !( slba % egs ) && !( nlba % egs ) ) {
					if( ( status = sdio_erase( ext->device, part->config, MMC_ERASE_NORM, slba, nlba ) ) == EOK ) {
						part->ec += nlba;
					}
				}
				break;

			case SDMMC_ERASE_ACTION_SECURE:
					// verify SECURE cap and erase group alignment
				if( ( ext->dev_inf.caps & DEV_CAP_SECURE ) &&
						!( slba % egs ) && !( nlba % egs ) ) {
					if( ( status = sdio_erase( ext->device, part->config, MMC_ERASE_SECURE, slba, nlba ) ) == EOK ) {
						part->ec += nlba;
					}
				}
				break;

			case SDMMC_ERASE_ACTION_TRIM:
				if( ( ext->dev_inf.caps & DEV_CAP_TRIM ) ) {
					if( ( status = sdio_erase( ext->device, part->config, MMC_ERASE_TRIM, slba, nlba ) ) == EOK ) {
						part->tc += nlba;
					}
				}
				break;

			case SDMMC_ERASE_ACTION_SECURE_TRIM:
				if( ( ext->dev_inf.caps & DEV_CAP_SECURE_TRIM ) == DEV_CAP_SECURE_TRIM ) {
					if( ( status = sdio_erase( ext->device, part->config, MMC_ERASE_SECURE_TRIM, slba, nlba ) ) == EOK ) {
						part->tc += nlba;
					}
				}
				break;

			case SDMMC_ERASE_ACTION_SECURE_PURGE:
				if( ( ext->dev_inf.caps & DEV_CAP_SECURE_TRIM ) == DEV_CAP_SECURE_TRIM ) {
					status = sdio_erase( ext->device, part->config, MMC_ERASE_SECURE_TRIM_PURGE, slba, nlba );
				}
				break;

			case SDMMC_ERASE_ACTION_DISCARD:
				if( ( ext->dev_inf.caps & DEV_CAP_TRIM ) ) {
					if( ( status = sdio_erase( ext->device, part->config, MMC_ERASE_DISCARD, slba, nlba ) ) == EOK ) {
						part->dc += nlba;
					}
				}
				break;

#if 0
			case SDMMC_ERASE_ACTION_SANITIZE:
				if( ( ext->dev_inf.caps & DEV_CAP_SANITIZE ) ) {
					if( ( status = sdio_mmc_switch( ext->device, MMC_SWITCH_CMDSET_DFLT, MMC_SWITCH_MODE_WRITE, ECSD_SANITIZE_START, ECSD_SANITIZE_INITIATE, SDIO_TIME_DEFAULT ) ) != EOK ) {
							// set flag to indicate we have started a sanitize
						ext->eflags |= SDMMC_EFLAG_DEV_BUSY;
					}
				}
				break;
#endif

			default:
				status = EINVAL;
		}	
	}

	ccb->cam_devctl_status = status;

	return( CAM_REQ_CMP );
}

int sdmmc_lock_unlock_devctl( SIM_HBA *hba, CCB_DEVCTL *ccb )
{
	SIM_SDMMC_EXT		*ext;
	SDMMC_LOCK_UNLOCK	*lock;
	int					status;

	ext				= (SIM_SDMMC_EXT *)hba->ext;
	lock			= (SDMMC_LOCK_UNLOCK *)ccb->cam_devctl_data;

	ccb->cam_devctl_status = EOK;

	if( sdmmc_unit_ready( hba, (CCB_SCSIIO *)ccb ) != CAM_REQ_CMP ) {
		ccb->cam_devctl_status = EIO;
	}
	else if( lock->pwd_len > SDMMC_LU_PWD_SIZE ||
			( lock->action != SDMMC_LU_ACTION_LOCK &&
			lock->action != SDMMC_LU_ACTION_SET &&
			lock->action != SDMMC_LU_ACTION_CLR &&
			lock->action != SDMMC_LU_ACTION_UNLOCK &&
			lock->action != SDMMC_LU_ACTION_ERASE ) ) {
		ccb->cam_devctl_status = EINVAL;
		return( CAM_REQ_CMP );
	}

	if( ( status = sdio_lock_unlock( ext->device, lock->action, lock->pwd, lock->pwd_len ) ) == EOK ) {
		sdio_dev_info( ext->device, &ext->dev_inf );	// update device info
	}

	ccb->cam_devctl_status = status;

	return( CAM_REQ_CMP );
}

int sdmmc_card_register_devctl( SIM_HBA *hba, CCB_DEVCTL *ccb )
{
	SIM_SDMMC_EXT			*ext;
	SDMMC_CARD_REGISTER		*cr;
	int						len;
	uint8_t					*dptr;
	uint8_t					data[512];

	ext		= (SIM_SDMMC_EXT *)hba->ext;
	cr		= (SDMMC_CARD_REGISTER *)ccb->cam_devctl_data;
	len 	= ccb->cam_devctl_size - sizeof( SDMMC_CARD_REGISTER );
	dptr	= (uint8_t *)(cr + 1);
	ccb->cam_devctl_status = EOK;

	if( sdmmc_unit_ready( hba, (CCB_SCSIIO *)ccb ) != CAM_REQ_CMP ) {
		ccb->cam_devctl_status = EIO;
		return( CAM_REQ_CMP );
	}

	if( cr->action == SDMMC_CR_ACTION_READ ) {
		switch( cr->type ) {
			case SDMMC_REG_TYPE_CID:
				memcpy( dptr, sdio_get_raw_cid( ext->device ), min( len, MMC_CID_SIZE ) );
				break;

			case SDMMC_REG_TYPE_CSD:
				memcpy( dptr, sdio_get_raw_csd( ext->device), min( len, MMC_CSD_SIZE ) );
				break;

			case SDMMC_REG_TYPE_EXT_CSD:
				if( ext->instance.ident.dtype != DEV_TYPE_MMC ) {
					ccb->cam_devctl_status	= ENOTSUP;
				}
				else {
					if( sdio_send_ext_csd( ext->device, data ) == EOK ) {
						memcpy( dptr, data, min( len, sizeof( data ) ) );
					}
					else {
						ccb->cam_devctl_status	= EIO;
					}
				}
				break;

			case SDMMC_REG_TYPE_SCR:
				if( ext->instance.ident.dtype != DEV_TYPE_SD ) {
					ccb->cam_devctl_status	= ENOTSUP;
				}
				else {
					memcpy( dptr, sdio_get_raw_scr( ext->device ), min( len, SD_SCR_SIZE ) );
				}
				break;
					
			default:
				ccb->cam_devctl_status	= EINVAL;
				break;
		}
		cr->type = ext->instance.ident.dtype;
	}
	else if( cr->action == SDMMC_CR_ACTION_WRITE ) {
		if( cr->type == SDMMC_REG_TYPE_EXT_CSD && ext->instance.ident.dtype == DEV_TYPE_MMC ) {
			if( cr->address >= ECSD_REV ) {
				ccb->cam_devctl_status  = EINVAL;
			}
			else if( sdio_mmc_switch( ext->device, MMC_SWITCH_CMDSET_DFLT, MMC_SWITCH_MODE_WRITE, cr->address, *dptr, SDIO_TIME_DEFAULT ) != EOK ) {
				ccb->cam_devctl_status  = EIO;
			}
		}
		else {
			ccb->cam_devctl_status  = ENOTSUP;
		}
	}
	else {
		ccb->cam_devctl_status	= ENOTSUP;
	}

	return( CAM_REQ_CMP );
}


int sdmmc_verbosity_devctl( SIM_HBA *hba, CCB_DEVCTL *ccb )
{
	SIM_SDMMC_EXT		*ext;
	CAM_VERBOSITY		*ver;

	ext = (SIM_SDMMC_EXT *)hba->ext;
	ver	= (CAM_VERBOSITY *)ccb->cam_devctl_data;

	if( ver->modules == CAM_MODULE_SIM ) {
		ccb->cam_devctl_status	= EOK;
		if( ver->flags ) {
			sdio_verbosity( ext->device, ver->flags, ver->verbosity );
		}
		else {
			hba->verbosity			= ver->verbosity;
		}
	}

	return( CAM_REQ_CMP );
}

static int sdmmc_device_info_devctl( SIM_HBA *hba, CCB_DEVCTL *ccb )
{
	SIM_SDMMC_EXT		*ext;
	SDMMC_DEVICE_INFO	*di;
	SDMMC_PARTITION		*part;

	ext		= (SIM_SDMMC_EXT *)hba->ext;
	di		= (SDMMC_DEVICE_INFO *)ccb->cam_devctl_data;
	part	= &ext->targets[ccb->cam_ch.cam_target_id].partitions[ccb->cam_ch.cam_target_lun];
	ccb->cam_devctl_status = EOK;

	memcpy( di, &ext->dev_inf, sizeof( sdio_dev_info_t ) );

#if defined( SIM_BS_DEVICE_INFO ) && defined( SIM_BS_DEVCTL )
	sim_bs_devctl( hba, ccb );
#endif

	di->sectors			= part->nlba;
	di->start_sector	= part->slba;

	return( CAM_REQ_CMP );
}

int sdmmc_dsm( SIM_HBA *hba, SDMMC_PARTITION *part, DATA_SET_MGNT *dsm, int dtype )
{
	SIM_SDMMC_EXT		*ext;
	DATA_SET_MGNT_RANGE *dsmr;
	uint64_t			lba;
	uint32_t			nlba;
	uint64_t			pend;
	int					status;

	ext		= (SIM_SDMMC_EXT *)hba->ext;
	dsmr	= (DATA_SET_MGNT_RANGE *)(dsm + 1);
	pend	= part->slba + part->nlba - 1;
	status  = EOK;

		// Note:  dsm->nranges == 0 is used to test for support
	for( ; dsm->nranges; dsmr++, dsm->nranges-- ) {
		nlba	= dsmr->nlba;
		lba		= part->slba + dsmr->lba;
		if( lba > pend || ( lba + nlba - 1 ) > pend ) {
			status = EINVAL;
			break;
		}

		if( ( status = sdio_erase( ext->device, part->config, dtype, lba, nlba ) ) ) {
			break;
		}

		switch( dtype ) {
			case MMC_ERASE_TRIM:
				part->tc += nlba; break;
			case MMC_ERASE_DISCARD:
				part->dc += nlba; break;
			default:
				break;
		}
	}

	return( status );
}

int sdmmc_dsm_devctl( SIM_HBA *hba, CCB_DEVCTL *ccb )
{
	SIM_SDMMC_EXT		*ext;
	SDMMC_PARTITION		*part;
	DATA_SET_MGNT		*dsm;
	int					status;

	ext		= (SIM_SDMMC_EXT *)hba->ext;
	dsm		= (DATA_SET_MGNT *)ccb->cam_devctl_data;
	part	= &ext->targets[ccb->cam_ch.cam_target_id].partitions[ccb->cam_ch.cam_target_lun];

	if( sdmmc_unit_ready( hba, (CCB_SCSIIO *)ccb ) != CAM_REQ_CMP ) {
		status = EIO;
	}
	else if( ccb->cam_devctl_size < ( sizeof( DATA_SET_MGNT ) + sizeof( DATA_SET_MGNT_RANGE ) * dsm->nranges ) ) {
		status = EINVAL;
	}
	else {
		switch( dsm->opt ) {
			case DSM_OPT_TRIM:
				if( !( ext->dev_inf.caps & DEV_CAP_TRIM ) ) {
					status = EINVAL;
					break;
				}
				status = sdmmc_dsm( hba, part, dsm, MMC_ERASE_TRIM );
				break;

			case DSM_OPT_DISCARD:
				if( !( ext->dev_inf.caps & DEV_CAP_DISCARD ) ) {
					status = EINVAL;
					break;
				}
				status = sdmmc_dsm( hba, part, dsm, MMC_ERASE_DISCARD );
				break;

			default:
				status = EINVAL;
				break;
		}
	}

	ccb->cam_devctl_status = status;

	return( CAM_REQ_CMP );
}

int sdmmc_part_info_devctl( SIM_HBA *hba, CCB_DEVCTL *ccb )
{
	SIM_SDMMC_EXT			*ext;
	SDMMC_PARTITION			*part;
	SDMMC_PARTITION_INFO	*pi;
	int						status;

	ext		= (SIM_SDMMC_EXT *)hba->ext;
	pi		= (SDMMC_PARTITION_INFO *)ccb->cam_devctl_data;
	part	= &ext->targets[ccb->cam_ch.cam_target_id].partitions[ccb->cam_ch.cam_target_lun];
	status	= EOK;

	if( sdmmc_unit_ready( hba, (CCB_SCSIIO *)ccb ) != CAM_REQ_CMP ) {
		status = EIO;
	}
	else if( ccb->cam_devctl_size < ( sizeof( SDMMC_PARTITION_INFO ) ) ) {
		status = EINVAL;
	}
	else {
		switch( pi->action ) {
			case SDMMC_PI_ACTION_GET:
				pi->ptype		= part->config & MMC_PART_MSK;
				pi->pflags		= part->pflags;
				pi->start_lba	= part->slba;
				pi->num_lba		= part->nlba;
				pi->rc			= part->rc;
				pi->wc			= part->wc;
				pi->dc			= part->dc;
				pi->ec			= part->ec;
				pi->tc			= part->tc;
				break;

			case SDMMC_PI_ACTION_CLR:
				pi->ptype		= part->config & MMC_PART_MSK;
				pi->pflags		= part->pflags;
				pi->start_lba	= part->slba;
				pi->num_lba		= part->nlba;
				pi->rc			= part->rc;
				pi->wc			= part->wc;
				pi->dc			= part->dc;
				pi->ec			= part->ec;
				pi->tc			= part->tc;
				part->rc = part->wc = part->dc = part->ec = part->tc = 0;
				break;

			default:
				status = EINVAL;
				break;
		}
	}

	ccb->cam_devctl_status = status;

	return( CAM_REQ_CMP );
}

int sdmmc_pwr_mgnt_devctl( SIM_HBA *hba, CCB_DEVCTL *ccb )
{
	SIM_SDMMC_EXT			*ext;
	SDMMC_PWR_MGNT			*pm;
	int						status;

	ext		= (SIM_SDMMC_EXT *)hba->ext;
	pm		= (SDMMC_PWR_MGNT *)ccb->cam_devctl_data;
	status	= EOK;

	if( sdmmc_unit_ready( hba, (CCB_SCSIIO *)ccb ) != CAM_REQ_CMP ) {
		status = EIO;
	}
	else if( ccb->cam_devctl_size < ( sizeof( SDMMC_PWR_MGNT ) ) ) {
		status = EINVAL;
	}
	else {
		switch( pm->action ) {
			case SDMMC_PM_ACTION_GET:
				pm->idle_time	= ext->pm_idle_time_ns / 1000000;
				pm->sleep_time	= ext->pm_sleep_time_ns / 1000000;
				break;

			case SDMMC_PM_ACTION_SET:
					// use chipset defaults as min limit
				if( ( pm->idle_time < ext->hc_inf.idle_time ) ||
						( pm->sleep_time < ext->hc_inf.sleep_time ) ) {
					status = EINVAL;
					break;
				}
				ext->pm_idle_time_ns	= SDMMC_TIMEOUT_MS_TO_NS( pm->idle_time );
				ext->pm_sleep_time_ns	= SDMMC_TIMEOUT_MS_TO_NS( pm->sleep_time );
				break;

			default:
				status = EINVAL;
				break;
		}
	}

	ccb->cam_devctl_status = status;

	return( CAM_REQ_CMP );
}

int sdmmc_devctl( SIM_HBA *hba, CCB_DEVCTL *ccb )
{
	int	status;

	status	= CAM_REQ_CMP;

	switch( ccb->cam_devctl_dcmd ) {
		case DCMD_SDMMC_DEVICE_INFO:
			status = sdmmc_device_info_devctl( hba, ccb );
			break;

		case DCMD_SDMMC_ERASE:
			status = sdmmc_erase_devctl( hba, ccb );
			break;

		case DCMD_SDMMC_WRITE_PROTECT:
			status = sdmmc_wp_devctl( hba, ccb );
			break;

		case DCMD_SDMMC_CARD_REGISTER:
			status = sdmmc_card_register_devctl( hba, ccb );
			break;

		case DCMD_SDMMC_ASSD_STATUS:
			status = sdmmc_assd_status_devctl( hba, ccb );
			break;

		case DCMD_SDMMC_ASSD_PROPERTIES:
			status = sdmmc_assd_properties_devctl( hba, ccb );
			break;

		case DCMD_SDMMC_ASSD_CONTROL:
			status = sdmmc_assd_control_devctl( hba, ccb );
			break;

		case DCMD_SDMMC_ASSD_APDU:
			status = sdmmc_assd_apdu_devctl( hba, ccb );
			break;

		case DCMD_SDMMC_LOCK_UNLOCK:
			status = sdmmc_lock_unlock_devctl( hba, ccb );
			break;

		case DCMD_SDMMC_PART_INFO:
			status = sdmmc_part_info_devctl( hba, ccb );
			break;

		case DCMD_SDMMC_PWR_MGNT:
			status = sdmmc_pwr_mgnt_devctl( hba, ccb );
			break;

		case DCMD_CAM_VERBOSITY:
			status = sdmmc_verbosity_devctl( hba, ccb );
			break;

		case DCMD_CAM_DEV_SERIAL_NUMBER:
			status = sdmmc_dsn_devctl( hba, ccb );
			break;

		case DCMD_CAM_DATA_SET_MGNT:
			status = sdmmc_dsm_devctl( hba, ccb );
			break;

		default:
#ifdef SIM_BS_DEVCTL
			status = sim_bs_devctl( hba, ccb );
#endif
			break;
	}

	return( status );
}

// interpret SCSI commands
int sdmmc_scsi_io( SIM_HBA *hba, CCB_SCSIIO *ccb )
{
	SIM_SDMMC_EXT	*ext;
	int				cmd;
	int				status;

	ext = (SIM_SDMMC_EXT *)hba->ext;
	cmd = ccb->cam_cdb_io.cam_cdb_bytes[0];

	if( hba->verbosity > 3 ) {
		xpt_display_ccb( ccb, hba->verbosity );
	}

	if( ccb->cam_dxfer_len && ( ccb->cam_ch.cam_flags & CAM_DATA_PHYS ) &&
			!( ext->hc_inf.caps & HC_CAP_DMA ) ) {
		return( CAM_PROVIDE_FAIL );
	}

	switch( cmd ) {
		case SC_UNIT_RDY:
			status = sdmmc_unit_ready( hba, ccb );
			break;

		case SC_INQUIRY:
			status = sdmmc_inquiry( hba, ccb );
			break;

		case SC_SPINDLE:
			status = sdmmc_spindle( hba, ccb );
			break;

		case SC_RD_CAP:
			status = sdmmc_capacity( hba, ccb );
			break;

		case SC_READ10:
			status = sdmmc_read_write( hba, ccb, SCF_DIR_IN );
			break;

		case SC_WRITE10:
			status = sdmmc_read_write( hba, ccb, SCF_DIR_OUT );

#ifdef SDMMC_WRITE_VERIFY
			if( status == CAM_REQ_CMP ) {
				status = sdmmc_write_verify( hba, ccb );
			}
#endif
			break;

		case SC_SYNC:
			status = sdmmc_sync( hba, ccb );
			break;

		case SC_MSELECT10:
			status = sdmmc_mode_select( hba, ccb );
			break;

		case SC_MSENSE10:
			status = sdmmc_mode_sense( hba, ccb );
			break;

		case SC_ERASE12:
			status = sdmmc_erase12( hba, ccb );
			break;

#ifdef SDMMC_TRIM_SUP
		case SC_WR_SAME16:
			status = sdmmc_write_same( hba, ccb );
			break;
#endif

		default:
#if defined SIM_BS_PASS_THROUGH
			status = sim_bs_pass_through( hba, ccb );
			if( status != CAM_REQ_INVALID )
				break;
#endif
			return( sdmmc_error( hba, ccb, EINVAL ) );
	}

	return( status );
}

void sdmmc_start_ccb( SIM_HBA *hba )
{
	SIM_SDMMC_EXT	*ext;
	CCB_SCSIIO		*ccb;
	int				status;

	ext = (SIM_SDMMC_EXT *)hba->ext;

	do {
		if( ( ext->nexus = ccb = simq_ccb_dequeue( hba->simq ) ) == NULL ) {
#ifdef SDMMC_AGGRESSIVE_PM
				// In aggressive pm mode we direct call the sdio layer,
				// so we don't have the overhead of enabling/disabling
				// the local PM timer.
			sdio_pwrmgnt( ext->device, PM_IDLE );
#endif
			break;
		}

		sdmmc_pm( hba, PM_ACTIVE );

		switch( ccb->cam_ch.cam_func_code ) {
			case XPT_SCSI_IO:
				status = sdmmc_scsi_io( hba, (CCB_SCSIIO *)ccb );
				break;

			case XPT_DEVCTL:
				status = sdmmc_devctl( hba, (CCB_DEVCTL *)ccb );
				break;

			default:
				cam_slogf( _SLOGC_SIM_MMC, _SLOG_ERROR, 1, 1,
						"%s:  unsupported func code %d", __FUNCTION__, ccb->cam_ch.cam_func_code );
				status = CAM_REQ_CMP_ERR;
				break;
		}

		if( status != CAM_REQ_INPROG ) {
			ccb->cam_ch.cam_status = status;
			sdmmc_post_ccb( hba, ccb );
		}

	} while( ext->nexus == NULL );
}


int sdmmc_timer( SIM_HBA *hba )
{
	SIM_SDMMC_EXT	*ext;
	int				status;
	int				pm_state;
	uint64_t		timestamp;
	struct timespec	ts;

	ext = (SIM_SDMMC_EXT *)hba->ext;

	status = EOK;
	pm_state = ext->pm_state;
	if( ext->nexus || pm_state == PM_SLEEP ) {
		return( status );
	}

	clock_gettime( CLOCK_MONOTONIC, &ts );

	timestamp = timespec2nsec( &ts );

	if( pm_state == PM_ACTIVE ) {
		if( timestamp >= ( ext->pm_timestamp + ext->pm_idle_time_ns ) ) {
			sdmmc_pm( hba, PM_IDLE );
		}
	}
	else if( pm_state == PM_IDLE && ( ext->hc_inf.caps & HC_CAP_SLEEP ) ) {
		if( timestamp >= ( ext->pm_timestamp + ext->pm_sleep_time_ns ) ) {
			sdmmc_pm( hba, PM_SLEEP );
		}
	}

	return( status );
}

void *sdmmc_driver_thread( void *hdl )
{
	SIM_HBA			*hba;
	SIM_SDMMC_EXT	*ext;
	struct _pulse	pulse;
	struct sigevent	event;
	int				rid;
	int				stat;

	hba		= (SIM_HBA *)hdl;
	ext		= (SIM_SDMMC_EXT *)hba->ext;
	stat	= CAM_FALSE;

	if( ( hba->chid = ChannelCreate( _NTO_CHF_DISCONNECT | _NTO_CHF_UNBLOCK ) ) == -1 ) {
		cam_slogf( _SLOGC_SIM_MMC, _SLOG_ERROR, 1, 1, "%s ChannelCreate failure %s", __FUNCTION__, strerror( errno ) ); 
		stat = CAM_TRUE;
	}

	if( !stat && ( hba->coid = ConnectAttach( 0, 0, hba->chid, _NTO_SIDE_CHANNEL, 0 ) ) == -1 ) {
		cam_slogf( _SLOGC_SIM_MMC, _SLOG_ERROR, 1, 1, "%s ConnectAttach failure %s", __FUNCTION__, strerror( errno ) ); 
		stat = CAM_TRUE;
	}

	SIGEV_PULSE_INIT( &event, hba->coid, ext->priority, SDMMC_PM_TIMER, NULL );
	if( !stat && timer_create( CLOCK_REALTIME, &event, &ext->pm_timerid ) == -1 ) {
		cam_slogf( _SLOGC_SIM_MMC, _SLOG_ERROR, 1, 1, "%s:  timer_create", __FUNCTION__ );
		stat = CAM_TRUE;
	}

		// initialize SIM queue routines
	if( !stat && ( hba->simq = simq_init( hba->coid, hba, MAX_NARROW_TARGET,
			MAX_LUN, 2, 1, 2, ( ext->eflags & SDMMC_EFLAG_BKOPS ) ? 1 : 0 ) ) == NULL ) {
		cam_slogf( _SLOGC_SIM_MMC, _SLOG_ERROR, 1, 1, "%s:  simq_init failure", __FUNCTION__ );
		stat = CAM_TRUE;
	}

	if( stat ) {
		cam_set_thread_state( &hba->state, CAM_TSTATE_INIT_FAILURE );
		return( NULL );
	}

	cam_set_thread_state( &hba->state, CAM_TSTATE_INITIALIZED );

	while( 1 ) {
		if( ( rid = MsgReceivePulse( hba->chid, &pulse, sizeof( pulse ), NULL ) ) == -1 ) {
			break;
		}

		switch( pulse.code ) {
			case SIM_ENQUEUE:
				sdmmc_start_ccb( hba );
				break;

			case SIM_TIMER:
				if( ext->pm_state == PM_ACTIVE ) {
					sdmmc_bkops( hba, CAM_TRUE );
				}
				break;

			case SDMMC_PM_TIMER:
				sdmmc_timer( hba );
				break;

			case _PULSE_CODE_DISCONNECT:
				return( NULL );
				
			default:
//				cam_slogf( _SLOGC_SIM_MMC, _SLOG_ERROR, 1, 1, "%s: unknown pulse rid %d, type %x, subtype %, pulse %x, value %x, scoid %x", __FUNCTION__, rid, pulse.type, pulse.subtype, pulse.code, pulse.value, pulse.scoid );
				break;
		}

		if( ext->nexus == NULL ) {
			sdmmc_start_ccb( hba );
		}
	}

	return( NULL );
}

// This function is called by the XPT to request that the SIM be initialized.
int sdmmc_sim_init( SIM_HBA *hba, int path )
{
	hba = hba, path = path;

	return( CAM_SUCCESS );
}

// This fuction is called when the driver is terminating.  It is provided
// to perform any necessary cleanup before exiting.
int sdmmc_sim_detach( )
{
	SIM_HBA			*hba;
	SIM_HBA			*nhba;

	sdmmc_ctrl.cflags |= SDMMC_CFLAG_ENUMERATING;
	if( sdio_disconnect( sdmmc_ctrl.connection ) == EBUSY ) {
		for( hba = TAILQ_FIRST( &sdmmc_ctrl.hlist ); hba; hba = nhba ) {
			nhba = TAILQ_NEXT(hba, hlink);
			sdmmc_detach( hba );
		}
		sdio_disconnect( sdmmc_ctrl.connection );
	}

	return( CAM_SUCCESS );
}

// This function is used to reset the specified SCSI bus.  This
// request shall slways result in the SCSI RST signal being
// asserted.
int sdmmc_reset_bus( SIM_HBA *hba, CCB_RESETBUS *ccb )
{
	simq_scsi_reset( hba->simq );
	xpt_async( AC_BUS_RESET, hba->pathid, -1, -1, NULL, 0 );
	return( CAM_REQ_CMP );
}

// This function is used to reset the specified SCSI target.  This
// function should not be used in normal operation, but if I/O to a
// particular device hangs up for some reason, drivers can abort the
// I/O and reset the device before trying again.  This request shall
// always result in a bus device reset message being issued over SCSI.
int sdmmc_reset_dev( SIM_HBA *hba, CCB_RESETDEV *ccb )
{
	simq_reset_dev( hba->simq, ccb );
	xpt_async( AC_SENT_BDR, hba->pathid, -1, -1, NULL, 0 );
	return( CAM_REQ_CMP );
}

// This function is provided so that the peripheral driver can
// release a frozen SIM queue for the selected Logical Unit.
int sdmmc_rel_simq( SIM_HBA *hba, CCB_RELSIM *ccb )
{
	SIM_SDMMC_EXT	*ext;

	ext = (SIM_SDMMC_EXT *)hba->ext;

	simq_rel_simq( hba->simq, ccb );
	if( MsgSendPulse( hba->coid, ext->priority, SIM_ENQUEUE, 0 ) == -1 ) {
	}
	return( CAM_REQ_CMP );
}

// This function requests that a SCSI I/O request be terminated by
// identifying the CCB associated with the request.  This request
// does not necessarily result in a terminated I/O process message
// being issued over SCSI.
int sdmmc_term_io( SIM_HBA *hba, CCB_TERMIO *ccb ) 
{
	hba = hba, ccb = ccb;
	return( CAM_REQ_CMP_ERR );
}

// This function is used to get information on the installed HBA
// hardware, including number of HBAs installed
int sdmmc_path_inq( SIM_HBA *hba, CCB_PATHINQ *ccb )
{
	SIM_SDMMC_EXT	*ext;

	ext = (SIM_SDMMC_EXT *)hba->ext;

	ccb->cam_version_num	= CAM_VERSION;
	ccb->cam_initiator_id	= 7;
	ccb->cam_hba_inquiry	= 0;
	ccb->cam_target_sprt	= 0;
	ccb->cam_hba_misc		= 0;
	ccb->cam_hba_eng_cnt	= 0;
	ccb->cam_sim_priv		= SIM_PRIV;
	ccb->cam_async_flags	= AC_BUS_RESET;
	memset( ccb->cam_vuhba_flags, 0x00, sizeof( *ccb->cam_vuhba_flags ) );

	if( !( ext->hc_inf.caps & HC_CAP_DMA ) ) {
		ccb->cam_vuhba_flags[CAM_VUHBA_FLAGS]	= CAM_VUHBA_FLAG_PTR;
	}
	else {
		ccb->cam_vuhba_flags[CAM_VUHBA_FLAGS]	= CAM_VUHBA_FLAG_PTR | CAM_VUHBA_FLAG_DMA;
		if( !( ext->hc_inf.caps & HC_CAP_DMA64 ) ) {
			ccb->cam_vuhba_flags[CAM_VUHBA_EFLAGS]	|= CAM_VUHBA_EFLAG_DMA_32;
		}
	}

	ccb->cam_vuhba_flags[CAM_VUHBA_FLAGS]		|= CAM_VUHBA_FLAG_MLUN;

	if( ( ext->eflags & SDMMC_EFLAG_DEVNAME ) ) {
		ccb->cam_vuhba_flags[CAM_VUHBA_EFLAGS]	|= CAM_VUHBA_EFLAG_DEVNAME;
	}

	ccb->cam_vuhba_flags[CAM_VUHBA_MAX_LINKED]	= 1;
	ccb->cam_vuhba_flags[CAM_VUHBA_MAX_SG]		= ext->hc_inf.sg_max;

	strlcpy( (char *)ccb->cam_sim_vid, "SDMMC", sizeof( ccb->cam_sim_vid ) );
	strlcpy( (char *)ccb->cam_hba_vid, (char *)ext->hc_inf.name, sizeof( ccb->cam_hba_vid ) );

	return( CAM_REQ_CMP );
}

// All CCB requests to the SIM are made through this function.
int sdmmc_sim_action( SIM_HBA *hba, CCB *ccbp )
{
	SIM_SDMMC_EXT	*ext;
	CCB_HEADER		*ccb;
	int				status;

	ext		= (SIM_SDMMC_EXT *)hba->ext;
	ccb		= (CCB_HEADER *)ccbp;
	status	= CAM_REQ_CMP;

	if( ( ccb->cam_flags & ( CAM_CDB_PHYS | CAM_NXT_CCB_PHYS |
			CAM_CALLBCK_PHYS | CAM_SNS_BUF_PHYS ) ) ) {
		ccb->cam_status = CAM_NO_HBA;
		return( CAM_FAILURE );
	}

	switch( ccb->cam_func_code ) {
		case XPT_NOOP:
			break;

		case XPT_SCSI_IO:
		case XPT_RESET_BUS:
		case XPT_RESET_DEV:
		case XPT_ABORT:
		case XPT_DEVCTL:
			if( ccb->cam_target_id > ext->ntargs ) {
				ccb->cam_status = CAM_TID_INVALID;
				return( CAM_FAILURE );
			}
			else if( ccb->cam_target_lun > ext->targets[ccb->cam_target_id].nluns ) {
				ccb->cam_status = CAM_LUN_INVALID;
				return( CAM_FAILURE );
			}
			status = CAM_REQ_INPROG;
			break;

		case XPT_REL_SIMQ:
			status = sdmmc_rel_simq( hba, (CCB_RELSIM *)ccb );
			break;

		case XPT_TERM_IO:
			status = sdmmc_term_io( hba, (CCB_TERMIO *)ccb );
			break;

		case XPT_PATH_INQ:
			status = sdmmc_path_inq( hba, (CCB_PATHINQ *)ccb );
			break;

		case XPT_GDEV_TYPE:
		case XPT_SDEV_TYPE:
		case XPT_SASYNC_CB:
			// These are not serviced by the SIM, the XPT should do them
			status = CAM_REQ_INVALID;
			break;

		case XPT_EN_LUN:
		case XPT_TARGET_IO:
			status = CAM_FUNC_NOTAVAIL;
			break;

		case XPT_VUNIQUE:
				// No special vendor unique commands
			status = CAM_REQ_INVALID;
			break;

		case XPT_ENG_INQ:
		case XPT_ENG_EXEC:
		default:
			status = CAM_REQ_INVALID;
			break;
	}

	if( ( ccb->cam_status = status ) == CAM_REQ_INPROG ) {
#ifdef SDMMC_TRACE
		sdmmc_trace_event( SDMMC_TRACE_EVENT, "%s:  ccb %p, cmd %x", __FUNCTION__, ccb, ((CCB_SCSIIO *)ccb)->cam_cdb_io.cam_cdb_bytes[0] );
#endif
		simq_ccb_enqueue( hba->simq, (CCB_SCSIIO *)ccb );
		if( MsgSendPulse( hba->coid, ext->priority, SIM_ENQUEUE, 0 ) == -1 ) {
		}
	}

	return( CAM_SUCCESS );
}

int sdmmc_sim_attach( CAM_ENTRY *centry )
{
	SIM_HBA					*hba;
	SIM_HBA					*nhba;
	SIM_SDMMC_EXT			*ext;
	int						busno;
	sdio_device_instance_t	*inst;
	sdio_funcs_t			funcs = { 2, sdmmc_sdio_insertion, sdmmc_sdio_removal, NULL };
	sdio_device_ident_t		interest = { SDIO_CONNECT_WILDCARD, SDIO_CONNECT_WILDCARD, SDIO_CONNECT_WILDCARD, SDIO_CONNECT_WILDCARD };
	sdio_connect_parm_t		connect_parm = { 0 };

	connect_parm.vsdio	= SDIO_VERSION;
	connect_parm.argc	= sdmmc_ctrl.argc;
	connect_parm.argv	= sdmmc_ctrl.argv;

	memcpy( &connect_parm.funcs, &funcs, sizeof( sdio_funcs_t ) );
	memcpy( &connect_parm.ident, &interest, sizeof( sdio_device_ident_t ) );

	sdmmc_ctrl.cflags |= SDMMC_CFLAG_ENUMERATING;

		// null insertion callback on targeted attach
	if( sdmmc_ctrl.nhba && !( sdmmc_ctrl.cflags & SDMMC_CFLAG_SCAN ) ) {
		funcs.insertion = NULL;
	}

	if( sdio_connect( &connect_parm, &sdmmc_ctrl.connection ) != EOK ) {
		return( CAM_FAILURE );
	}

	if( ( sdmmc_ctrl.cflags & SDMMC_CFLAG_SCAN ) ) {
		sdmmc_ctrl.cflags |= SDMMC_CFLAG_ENUMERATING;
		hba = TAILQ_FIRST( &sdmmc_ctrl.hlist );

		for( busno = 0; busno < SDMMC_MAX_BUS ; busno++ ) {
			if( hba == NULL && ( hba = sdmmc_alloc_hba( ) ) == NULL ) {
				break;
			}
			ext			= (SIM_SDMMC_EXT *)hba->ext;
			inst	 	= &ext->instance;
			inst->path	= busno;
			inst->func	= 0;

			if( sdmmc_attach( hba, sdmmc_ctrl.connection, inst ) != CAM_SUCCESS ) {
				sdmmc_detach( hba );
			}

			hba = NULL;
		}
	}
	else {		// attach to specific buses/devices
		for( hba = TAILQ_FIRST( &sdmmc_ctrl.hlist ); hba; hba = nhba ) {
			nhba				= TAILQ_NEXT( hba, hlink );
			ext					= (SIM_SDMMC_EXT *)hba->ext;
			inst				= &ext->instance;
			inst->ident.vid		= hba->cfg.Device_ID.DevID & 0xffff;
			inst->ident.did		= hba->cfg.Device_ID.DevID >> 16;

			if( sdmmc_attach( hba, sdmmc_ctrl.connection, inst ) != CAM_SUCCESS ) {
				sdmmc_detach( hba );
			}
		}
	}

	sdmmc_ctrl.cflags &= ~SDMMC_CFLAG_ENUMERATING;

	sdio_enum( sdmmc_ctrl.connection, SDIO_ENUM_ENABLE );	// enable change detect

	return( CAM_SUCCESS );
}

int sdmmc_sim_args( char *options )
{
	SIM_HBA				*hba;
	SIM_SDMMC_EXT		*ext;
	char				*value;
	int					val;
	int					opt;
	static char			*opts[] = {
        						"priority",
							"busno",
							"func",
							"bkops",
							"cache",
							"partitions",
							"bs",
							"pwroff_notify",
							NULL
						};

	if( *options == '\0' ) {		// called when command name includes the module name
		if( sdmmc_ctrl.nhba == 0 ) {
			sdmmc_ctrl.cflags |= SDMMC_CFLAG_SCAN;
		}
		return( 0 );
	}

	if( sdmmc_ctrl.nhba > SDMMC_MAX_HBA ) {	// max hba's reached
		return( -1 );
	}

	if( ( hba = sdmmc_alloc_hba( ) ) == NULL ) {
		return( -1 );
	}

	ext = (SIM_SDMMC_EXT *)hba->ext;

	while( *options != '\0' ) {
		if( ( opt = getsubopt( &options, opts, &value ) ) == -1 ) {
			if( sim_drvr_options( hba, value ) != EOK ) {
				continue;
			}
		}

		switch( opt ) {
			case 0:			// priority
				SDMMC_ARG_VAL( opts[opt], value );
				if( ( val = cam_parse_number( value ) ) != CAM_INVALID_NUM ) {
					if( val >= sched_get_priority_min( SCHED_RR ) && val <= sched_get_priority_max( SCHED_RR ) ) {
						ext->priority = val;
					}
					else {
						cam_slogf( _SLOGC_SIM_MMC, _SLOG_ERROR, 1, 1, "%s:  Invalid priority", __FUNCTION__ );
					}
				}
				break;

			case 1:							// bus number
				SDMMC_ARG_VAL( opts[opt], value );
				if( ( val = cam_parse_number( value ) ) != CAM_INVALID_NUM ) {
					ext->instance.path = val;
				}
				break;

			case 2:							// function
				SDMMC_ARG_VAL( opts[opt], value );
				if( ( val = cam_parse_number( value ) ) != CAM_INVALID_NUM ) {
					ext->instance.func = val;
				}
				break;

			case 3:							// bkops
				SDMMC_ARG_VAL( opts[opt], value );
				if( !strcmp( value, "on" ) ) {
					ext->eflags |= SDMMC_EFLAG_BKOPS;
				}
				break;

			case 4:							// cache
				SDMMC_ARG_VAL( opts[opt], value );
#ifdef SDMMC_CACHE_SUP
				if( !strcmp( value, "on" ) ) {
					ext->eflags |= SDMMC_EFLAG_CACHE;
				}
#endif
				break;

			case 5:							// partitions
				SDMMC_ARG_VAL( opts[opt], value );
				if( !strcmp( value, "on" ) ) {
					ext->eflags |= SDMMC_EFLAG_PARTITIONS;
				}
				break;

			case 6:
				sim_bs_args( hba, value );
				break;

			case 7:							// pwroff_notify
				SDMMC_ARG_VAL( opts[opt], value );
				if( !strcmp( value, "short" ) ) {
					ext->eflags |= SDMMC_EFLAG_PWROFF_NOTIFY;
					ext->pwroff_notify = ECSD_POWER_OFF_SHORT;
				} else if( !strcmp( value, "long" ) ) {
					ext->eflags |= SDMMC_EFLAG_PWROFF_NOTIFY;
					ext->pwroff_notify = ECSD_POWER_OFF_LONG;
				} else
					cam_slogf( _SLOGC_SIM_MMC, _SLOG_ERROR, 1, 1, "%s:  Invalid power off notification mode", __FUNCTION__ );

				break;


			default:
				break;
		}
	}

	if( ext->instance.path == SDIO_CONNECT_WILDCARD ) {
		sdmmc_ctrl.cflags |= SDMMC_CFLAG_SCAN;
	}

	return( CAM_SUCCESS );
}


#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/hardware/devb/sdmmc/sim_sdmmc.c $ $Rev: 813652 $")
#endif
