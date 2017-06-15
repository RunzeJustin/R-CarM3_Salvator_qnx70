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



#include <sim_sdmmc.h>

#define ASSD_TIME_DEFAULT			5000

#define ASSD_APDU_STL_SIZE			2
#define ASSD_APDU_MIN_DLEN			4	
#define ASSD_APDU_MAX_DLEN			512

#define ASSD_STATUS_SIZE			32
#define ASSD_STATE_IDX				0
#define ASSD_ERROR_IDX				1
#define ASSD_SEC_SYS_ERROR_IDX		2
	#define ASSD_SEC_SYS_ERR		0x80

#define ASSD_SPEC_A1_V1				0
#define ASSD_SPEC_A1_V1_1			1
#define ASSD_SPEC_A1_V2				2
#define ASSD_SPEC_A1_V3				3

#define ASSD_SS_MCEX				0

static int assd_control_system( SIM_HBA *hba, int srcid, int ssi, int op )
{
	SIM_SDMMC_EXT		*ext;
	struct sdio_device	*dev;
	struct sdio_cmd		*cmd;
	int					status;

	ext		= (SIM_SDMMC_EXT *)hba->ext;
	dev		= ext->device;

	if( ( status =  sdio_set_block_length( dev, SD_SEC_CMD_SIZE ) ) ) {
		return( status );
	}

	if( ( cmd = sdio_alloc_cmd( dev ) ) == NULL ) {
		return( ENOMEM );
	}

	sdio_setup_cmd( cmd, SCF_CTYPE_ADTC | SCF_RSP_R1B, SD_CONTROL_ASSD_SYSTEM, ( srcid << 12 ) | ( ssi << 8 ) | op );
	if( ( status = sdio_send_cmd( dev, cmd, NULL, ASSD_TIME_DEFAULT, 0 ) ) == EOK ) {

	}

	sdio_free_cmd( cmd );

	return( status );
}

static int assd_write_sec_cmd( SIM_HBA *hba, int len, uint8_t *data, int timeout )
{
	SIM_SDMMC_EXT		*ext;
	struct sdio_device	*dev;
	struct sdio_cmd		*cmd;
	int					status;
	sdio_sge_t			sge;

	ext		= (SIM_SDMMC_EXT *)hba->ext;
	dev		= ext->device;

#ifdef SDMMC_ASSD_DEBUG
	cam_slogf( _SLOGC_SIM_MMC, _SLOG_ERROR, 1, 1, "%s:  ", __FUNCTION__ );
#endif

	if( ( status =  sdio_set_block_length( dev, SD_SEC_CMD_SIZE ) ) ) {
		return( status );
	}

	if( ( cmd = sdio_alloc_cmd( dev ) ) == NULL ) {
		return( ENOMEM );
	}

	sdio_setup_cmd( cmd, SCF_CTYPE_ADTC | SCF_RSP_R1, SD_WRITE_SEC_CMD, len / SD_SEC_CMD_SIZE );
	sge.sg_count	= SD_SEC_CMD_SIZE;
	sge.sg_address	= SDIO_DATA_PTR_P( data );
	sdio_setup_cmd_io( cmd, SCF_DIR_OUT, 1, SD_SEC_CMD_SIZE, &sge, 1, NULL );
	if( ( status = sdio_send_cmd( dev, cmd, NULL, timeout, 0 ) ) == EOK ) {
		if( ( ext->eflags & SDMMC_EFLAG_ASSD_SEND_STOP ) ) {
			if( sdio_stop_transmission( dev, 0 ) != EOK ) {
				ext->eflags &= ~SDMMC_EFLAG_ASSD_SEND_STOP;
			}
		}
	}

	sdio_free_cmd( cmd );

	return( status );
}

static int assd_read_sec_cmd( SIM_HBA *hba, int len, uint8_t *data, int timeout )
{
	SIM_SDMMC_EXT		*ext;
	struct sdio_device	*dev;
	struct sdio_cmd		*cmd;
	int					status;
	sdio_sge_t			sge;

	ext		= (SIM_SDMMC_EXT *)hba->ext;
	dev		= ext->device;

#ifdef SDMMC_ASSD_DEBUG
	cam_slogf( _SLOGC_SIM_MMC, _SLOG_ERROR, 1, 1, "%s:  ", __FUNCTION__ );
#endif

	if( ( status =  sdio_set_block_length( dev, SD_SEC_CMD_SIZE ) ) ) {
		return( status );
	}

	if( ( cmd = sdio_alloc_cmd( dev ) ) == NULL ) {
		return( ENOMEM );
	}

	sdio_setup_cmd( cmd, SCF_CTYPE_ADTC | SCF_RSP_R1, SD_READ_SEC_CMD, len / SD_SEC_CMD_SIZE );
	sge.sg_count	= SD_SEC_CMD_SIZE;
	sge.sg_address	= SDIO_DATA_PTR_P( data );
	sdio_setup_cmd_io( cmd, SCF_DIR_IN, 1, SD_SEC_CMD_SIZE, &sge, 1, NULL );
	if( ( status = sdio_send_cmd( dev, cmd, NULL, timeout, 0 ) ) == EOK ) {
		if( ( ext->eflags & SDMMC_EFLAG_ASSD_SEND_STOP ) ) {
			if( sdio_stop_transmission( dev, 0 ) != EOK ) {
				ext->eflags &= ~SDMMC_EFLAG_ASSD_SEND_STOP;
			}
		}
	}

	sdio_free_cmd( cmd );

	return( status );
}

static int assd_send_psi( SIM_HBA *hba, int rid, uint8_t *psi )
{
	SIM_SDMMC_EXT		*ext;
	struct sdio_device	*dev;
	struct sdio_cmd		*cmd;
	int					status;
	sdio_sge_t			sge;

	ext		= (SIM_SDMMC_EXT *)hba->ext;
	dev		= ext->device;

#ifdef SDMMC_ASSD_DEBUG
	cam_slogf( _SLOGC_SIM_MMC, _SLOG_ERROR, 1, 1, "%s:  rid %d ", __FUNCTION__, rid );
#endif

	if( ( status =  sdio_set_block_length( dev, SD_PSI_SIZE ) ) ) {
		// apparently some cards fail this, but still work...
	}

	if( ( cmd = sdio_alloc_cmd( dev ) ) == NULL ) {
		return( ENOMEM );
	}

	sdio_setup_cmd( cmd, SCF_CTYPE_ADTC | SCF_RSP_R1, SD_SEND_PSI, rid );
	sge.sg_count	= SD_PSI_SIZE;
	sge.sg_address	= SDIO_DATA_PTR_P( psi );
	sdio_setup_cmd_io( cmd, SCF_DIR_IN, 1, SD_PSI_SIZE, &sge, 1, NULL );
	if( ( status = sdio_send_cmd( dev, cmd, NULL, ASSD_TIME_DEFAULT, 0 ) ) == EOK ) {

	}

	sdio_free_cmd( cmd );

	if( ( status =  sdio_set_block_length( dev, 512 ) ) ) {
		// apparently some cards fail this, but still work...
	}

	return( status );
}

static int assd_status( SIM_HBA *hba, SDMMC_ASSD_STATUS *as )
{
	uint8_t				*psi;
	int					status;

#ifdef SDMMC_ASSD_DEBUG
	cam_slogf( _SLOGC_SIM_MMC, _SLOG_ERROR, 1, 1, "%s:  ", __FUNCTION__ );
#endif

	if( ( psi = sdio_alloc( ASSD_STATUS_SIZE ) ) == NULL ) {
		return( ENOMEM );
	}

		// read status register
	if( ( status = assd_send_psi( hba, SD_PSI_ASSD_SR, psi ) ) == EOK ) {
		as->assd_state			= psi[0];
		as->assd_err_state		= psi[1];
		as->assd_sec_sys_err	= psi[2] >> 7;
		as->pmem_state			= psi[3];
		as->auth_alg			= psi[4];
		as->enc_alg				= psi[5];
		as->active_sec_system	= psi[6];
		as->sec_token_prot		= psi[7];
		as->read_block_count	= ( psi[8] << 8 ) | psi[9];
		as->suspended_sec_sys	= ( psi[10] << 8 ) | psi[11];
	}
	
	sdio_free( psi, ASSD_STATUS_SIZE );
	return( status );
}

static int assd_properties( SIM_HBA *hba, SDMMC_ASSD_PROPERTIES *prop )
{
	uint8_t				*psi;
	int					status;

	status	= EINVAL;

#ifdef SDMMC_ASSD_DEBUG
	cam_slogf( _SLOGC_SIM_MMC, _SLOG_ERROR, 1, 1, "%s:  ", __FUNCTION__ );
#endif

	if( ( psi = sdio_alloc( ASSD_STATUS_SIZE ) ) == NULL ) {
		return( ENOMEM );
	}
	
		// read properties register
	if( ( status = assd_send_psi( hba, SD_PSI_ASSD_PR, psi ) ) == EOK ) {
		prop->sec_read_latency			= psi[0];
		prop->sec_write_latency			= psi[1];
		prop->assd_version				= psi[2];
		prop->cl_support				= ( psi[3] << 7 ) | ( psi[4] >> 1 );
		prop->pmem_support				= psi[4] & 0x1;
		prop->pmem_rd_time				= psi[5];
		prop->pmem_wr_time				= psi[6];
		prop->wr_sec_bus_busy			= psi[7];
		prop->sup_auth_alg				= ( psi[8] << 8 ) | psi[9];
		prop->sup_enc_alg				= ( psi[10] << 8 ) | psi[11];
		prop->assd_sec_sys				= ( psi[12] << 8 ) | psi[13];
		prop->assd_sec_sys_vendor_id	= psi[14];
		prop->ctrl_sys_bus_busy			= psi[15];
		prop->suspendible_sec_sys		= ( psi[16] << 8 ) | psi[17];
	}
	
	sdio_free( psi, ASSD_STATUS_SIZE );
	return( status );
}

static int assd_wait_scc( SIM_HBA *hba, uint32_t msec )
{
	uint8_t				*psi;
	int					status;

	status	= EOK;

#ifdef SDMMC_ASSD_DEBUG
	cam_slogf( _SLOGC_SIM_MMC, _SLOG_ERROR, 1, 1, "%s:  ", __FUNCTION__ );
#endif

	if( ( psi = sdio_alloc( ASSD_STATUS_SIZE ) ) == NULL ) {
		return( ENOMEM );
	}

	for( msec = max( msec, 1 ); msec; msec-- ) {
		if( ( status = assd_send_psi( hba, SD_PSI_ASSD_SR, psi ) ) != EOK ) {
			break;
		}

		if( psi[ASSD_STATE_IDX] == ASSD_STATE_SCC ) {
			status = EOK; break;
		}

		if( psi[ASSD_STATE_IDX] == ASSD_STATE_SCA ) {
			status = EIO; break;
		}

		if( ( psi[ASSD_SEC_SYS_ERROR_IDX] & ASSD_SEC_SYS_ERR ) ) {
			status = EIO; break;
		}

		delay( 1 );
	}

	if( !msec ) {
		status = ETIMEDOUT;
	}

	if( status ) {
		cam_slogf( _SLOGC_SIM_MMC, _SLOG_ERROR, 1, 1, "%s: %s (%d), msec %d, state %d, error %d, sec sys err %d", __FUNCTION__, strerror( status ), status, msec, psi[ASSD_STATE_IDX], psi[ASSD_ERROR_IDX], psi[ASSD_SEC_SYS_ERROR_IDX] );
	}

	sdio_free( psi, ASSD_STATUS_SIZE );

	return( status );
}

#ifdef SDMMC_ASSD_SWITCH
static int assd_cmd_sys( SIM_HBA *hba, int state )
{
	SIM_SDMMC_EXT		*ext;
	struct sdio_device	*dev;
	int					idx;
	int					status;
	uint8_t				ss[64] = { 0 };

	ext		= (SIM_SDMMC_EXT *)hba->ext;
	dev		= ext->device;
	idx		= 0;

	if( state ) {
		if( ( status = sdio_sd_switch( dev, SD_SF_MODE_CHECK, SD_SF_GRP_CMD_EXT, SD_SF_CUR_FCN, ss ) ) != EOK ) {
			return( status );
		}

		if( ( ss[11] & SD_CMD_SYS_EC ) || ( ss[11] & SD_CMD_SYS_ASSD ) ) {
//			dev->caps |= DEV_CAP_ASSD;
		}

			// get index to eC/ASSD
		idx = ffs( ss[11] & ( SD_CMD_SYS_EC | SD_CMD_SYS_ASSD ) ) - 1;
	}

	if( ( status = sdio_sd_switch( dev, SD_SF_MODE_SET, SD_SF_GRP_CMD_EXT, idx, ss ) ) == EOK ) {
		if( ( ( ss[16] >> 4 ) & 0x0F ) != idx ) {
			status = EIO;
		}
	}

	return( status );
}
#endif

static int assd_state( SIM_HBA *hba, int reinit )
{
	SIM_SDMMC_EXT		*ext;
	SDMMC_ASSD_STATUS	as;
	int					status;

	ext		= (SIM_SDMMC_EXT *)hba->ext;
	status		= EOK;


	if( !( ext->dev_inf.caps & DEV_CAP_ASSD ) ) {
		return( ENOTSUP );
	}

	if( !( ext->eflags & SDMMC_EFLAG_ASSD_INIT ) ) {
		return( EOK );
	}

#ifdef SDMMC_ASSD_SWITCH
	if( ( status = assd_cmd_sys( hba, CAM_TRUE ) ) != EOK ) {
		return( status );
	}
#endif

	if( !reinit ) {
		if( ( status = assd_properties( hba, &ext->assd_properties ) ) == EOK ) {
		}
	}

	if( ext->assd_active_sec_sys != -1 ) {
		if( ( status = assd_control_system( hba, 0, ext->assd_active_sec_sys, SDMMC_AC_OP_SELECT_RESET ) ) == EOK ) {
			if( ( status = assd_status( hba, &as ) ) == EOK ) {
				if( ext->assd_active_sec_sys != as.active_sec_system ) {
					status = EIO;
				}
			}
		}
	}

	if( status == EOK ) {
		atomic_clr( &ext->eflags, SDMMC_EFLAG_ASSD_INIT );
	}

	return( status );
}

int sdmmc_assd_control_devctl( SIM_HBA *hba, CCB_DEVCTL *ccb )
{
	SIM_SDMMC_EXT			*ext;
	SDMMC_ASSD_STATUS		as;
	SDMMC_ASSD_CONTROL		*ctrl;
	SDMMC_ASSD_PROPERTIES	*prop;
	int						status;

	ext		= (SIM_SDMMC_EXT *)hba->ext;
	ctrl	= (SDMMC_ASSD_CONTROL *)ccb->cam_devctl_data;
	prop	= &ext->assd_properties;
	status	= EINVAL;

	if( sdmmc_unit_ready( hba, (CCB_SCSIIO *)ccb ) != CAM_REQ_CMP ) {
		ccb->cam_devctl_status = EIO;
		return( CAM_REQ_CMP );
	}

	if( ( status = assd_state( hba, CAM_TRUE ) ) ) {
		ccb->cam_devctl_status = status;
		return( CAM_REQ_CMP );
	}

	if( ctrl->sec_sys_idx > SDMMC_AC_SSI_MAX ) {
		ccb->cam_devctl_status = EINVAL;
		return( CAM_REQ_CMP );
	}

	switch( ctrl->operation ) {
		case SDMMC_AC_OP_START_SUSPEND:
#if 0
// untested/supported
			if( prop->assd_version < ASSD_SPEC_A1_V2 ||
					!( prop->assd_sec_sys & ( 1 << ctrl->sec_sys_idx ) ) ||
					!( prop->suspendible_sec_sys & ( 1 << ctrl->sec_sys_idx ) ) ) {
				status = EINVAL; break;
			}

			if( ( status = assd_control_system( hba, ctrl->suspend_resume_id, ctrl->sec_sys_idx, ctrl->operation ) ) == EOK ) {
				if( ( status = assd_status( hba, &as ) ) == EOK ) {
					if( ( ( 1 << ctrl->sec_sys_idx ) & as.suspended_sec_sys ) ) {
						status = EIO; break;
					}
				}
			}
			break;
#else
			status = EINVAL; break;
#endif

		case SDMMC_AC_OP_CLEAR_SUSPEND:
#if 0
// untested/supported
			if( prop->assd_version < ASSD_SPEC_A1_V2 ||
					!( prop->assd_sec_sys & ( 1 << ctrl->sec_sys_idx ) ) ||
					!( prop->suspendible_sec_sys & ( 1 << ctrl->sec_sys_idx ) ) ) {
				status = EINVAL; break;
			}

			if( ( status = assd_control_system( hba, ctrl->suspend_resume_id, ctrl->sec_sys_idx, ctrl->operation ) ) == EOK ) {
				if( ( status = assd_status( hba, &as ) ) == EOK ) {
					if( ctrl->sec_sys_idx != as.active_sec_system ) {
						status = EIO; break;
					}
					ext->assd_active_sec_sys = as.active_sec_system;
				}
			}
			break;
#else
			status = EINVAL; break;
#endif

		case SDMMC_AC_OP_SELECT_RESET:
			if( prop->assd_version < ASSD_SPEC_A1_V2 ) {
				if( ctrl->sec_sys_idx != ASSD_SS_MCEX ) {
					status = EINVAL; break;
				}
			}
			else if( !( prop->assd_sec_sys & ( 1 << ctrl->sec_sys_idx ) ) ) {
				status = EINVAL; break;
			}

			if( ( status = assd_control_system( hba, 0, ctrl->sec_sys_idx, ctrl->operation ) ) == EOK ) {
				if( ( status = assd_status( hba, &as ) ) == EOK ) {
					if( ctrl->sec_sys_idx != as.active_sec_system ) {
						status = EIO; break;
					}
					else {
						ext->assd_active_sec_sys = as.active_sec_system;
					}
				}
			}

			break;

		default:
			status = EINVAL; break;
	}

	ccb->cam_devctl_status = status;

	return( CAM_REQ_CMP );
}

int sdmmc_assd_status_devctl( SIM_HBA *hba, CCB_DEVCTL *ccb )
{
	SDMMC_ASSD_STATUS		*as;
	int						status;

	as		= (SDMMC_ASSD_STATUS *)ccb->cam_devctl_data;

	if( sdmmc_unit_ready( hba, (CCB_SCSIIO *)ccb ) == CAM_REQ_CMP ) {
		if( ( status = assd_state( hba, CAM_TRUE ) ) == EOK ) {
			status = assd_status( hba, as );
		}
	}
	else {
		status = EIO;
	}

	ccb->cam_devctl_status = status;

	return( CAM_REQ_CMP );
}

int sdmmc_assd_properties_devctl( SIM_HBA *hba, CCB_DEVCTL *ccb )
{
	SDMMC_ASSD_PROPERTIES	*prop;
	int						status;

	prop	= (SDMMC_ASSD_PROPERTIES *)ccb->cam_devctl_data;

	if( sdmmc_unit_ready( hba, (CCB_SCSIIO *)ccb ) != CAM_REQ_CMP ) {
		ccb->cam_devctl_status = EIO;
		return( CAM_REQ_CMP );
	}

	if( ( status = assd_state( hba, CAM_TRUE ) ) == EOK ) {
		if( ( status = assd_properties( hba, prop ) ) == EOK ) {
		}
	}

	ccb->cam_devctl_status = status;

	return( CAM_REQ_CMP );
}

int sdmmc_assd_apdu_devctl( SIM_HBA *hba, CCB_DEVCTL *ccb )
{
	SIM_SDMMC_EXT		*ext;
	SDMMC_ASSD_APDU		*apdu;
	struct sdio_device	*dev;
	uint8_t				*data;
	uint8_t				*apdu_data;
	uint32_t				len;
	int					status;

	ext			= (SIM_SDMMC_EXT *)hba->ext;
	apdu		= (SDMMC_ASSD_APDU *)ccb->cam_devctl_data;
	apdu_data	= (uint8_t *)(apdu + 1);
	dev			= ext->device;
	len			= apdu->length + ASSD_APDU_STL_SIZE;	// add 2 for STL

#ifdef SDMMC_ASSD_DEBUG
	cam_slogf( _SLOGC_SIM_MMC, _SLOG_ERROR, 1, 1, "%s:  ", __FUNCTION__ );
#endif

	if( sdmmc_unit_ready( hba, (CCB_SCSIIO *)ccb ) != CAM_REQ_CMP ) {
		ccb->cam_devctl_status = EIO;
		return( CAM_REQ_CMP );
	}

	if( ( status = assd_state( hba, CAM_TRUE ) ) ) {
		ccb->cam_devctl_status = status;
		return( CAM_REQ_CMP );
	}

	if( ( len + sizeof( SDMMC_ASSD_APDU ) < ccb->cam_devctl_size ) &&
			len < ASSD_APDU_MAX_DLEN && len > ASSD_APDU_MIN_DLEN ) {
		if( ( data = sdio_alloc( ASSD_APDU_MAX_DLEN ) ) ) {
			data[0] = len >> 8; data[1] = len;
			memcpy( data + ASSD_APDU_STL_SIZE, apdu_data, len - ASSD_APDU_STL_SIZE );
			if( len < ASSD_APDU_MAX_DLEN ) {
				memset( data + len, 0, ASSD_APDU_MAX_DLEN - len );
			}

#ifdef SDMMC_ASSD_DEBUG
			sdio_dump( data, len );
#endif

			if( ( status = assd_write_sec_cmd( hba, ASSD_APDU_MAX_DLEN, data, ASSD_TIME_DEFAULT ) ) == EOK ) {
				if( ( status = sdio_wait_card_status( dev, NULL, CDS_READY_FOR_DATA | CDS_CUR_STATE_MSK, CDS_READY_FOR_DATA | CDS_CUR_STATE_TRAN, ASSD_TIME_DEFAULT ) ) == EOK ) {
					if( ( status = assd_wait_scc( hba, ASSD_TIME_DEFAULT ) ) == EOK ) {
						if( ( status = assd_read_sec_cmd( hba, ASSD_APDU_MAX_DLEN, data, ASSD_TIME_DEFAULT ) ) == EOK ) {
							apdu->length			= 0;
							len				= ( ( data[0] << 8 ) + data[1] );
							if( len > ASSD_APDU_STL_SIZE ) {
								len -= ASSD_APDU_STL_SIZE;
								apdu->length	= min( len, ASSD_APDU_MAX_DLEN );
								memcpy( apdu_data, data + ASSD_APDU_STL_SIZE, apdu->length );
							}
#ifdef SDMMC_ASSD_DEBUG
							sdio_dump( data, len + ASSD_APDU_STL_SIZE );
#endif
						}
					}
				}
			}
			sdio_free( data, ASSD_APDU_MAX_DLEN );
		}
		else {
			status = ENOMEM;
		}
	}
	else {
		status = EINVAL;
	}

	ccb->cam_devctl_status = status;

	return( CAM_REQ_CMP );
}

int sdmmc_assd_init( SIM_HBA *hba )
{
	SIM_SDMMC_EXT		*ext;
	int					status;

	ext			= (SIM_SDMMC_EXT *)hba->ext;

// Some non conforming cards require a stop command during APDU.
// Don't enable this feature until we find/test one of them.
//	ext->eflags |= SDMMC_EFLAG_ASSD_SEND_STOP;

	atomic_set( &ext->eflags, SDMMC_EFLAG_ASSD_INIT );

	if( ( status = assd_state( hba, CAM_FALSE ) ) == EOK ) {

	}

	return( status );
}


#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/hardware/devb/sdmmc/sim_assd.c $ $Rev: 805416 $")
#endif
