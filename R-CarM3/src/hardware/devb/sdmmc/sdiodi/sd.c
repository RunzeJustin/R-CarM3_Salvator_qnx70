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

// Module Description:

#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <gulliver.h>

#include <internal.h>

const uint32_t sd_tran_speed_fu[] = { 10000, 100000, 1000000, 10000000, 0, 0, 0, 0 };
const uint32_t sd_tran_speed_mf[] = { 0, 10, 12, 13, 15, 20, 25, 30,
										35, 40, 45, 50, 55, 60, 70, 80 };

const uint32_t sd_tacc_tu[] = { 1, 10, 100, 1000, 100000, 1000000, 10000000 };
const uint32_t sd_tacc_mf[] = { 0, 10, 12, 13, 15, 20, 25, 30,
								35, 40, 45, 50, 55, 60, 70, 80 };

sdio_device_errata_t sd_errata[] = {
	// secusmart devices require extra 50ms after ASSD selection
	{ "SD0", 0x27, DEV_ERRATA_WILDCARD, 0, DEV_ERRATA_WILDCARD, DEV_ERRATA_WILDCARD, 0, 50 },

	{ NULL, 0, 0, 0, 0, 0, 0, 0 }
};

int sd_parse_cid( sdio_dev_t *dev, sdio_cid_t *cid, uint32_t *rsp )
{
	int		idx;

	memset( cid, 0, sizeof( sdio_cid_t ) );

	cid->mid	= sdio_extract_bits( rsp, 128, 120, 8 );
	cid->oid	= sdio_extract_bits( rsp, 128, 104, 16 );

	for( idx = 0; idx < 5; idx++ ) {
		cid->pnm[idx] = sdio_extract_bits( rsp, 128, 96 - idx * 8, 8 );
	}

	cid->psn	= sdio_extract_bits( rsp, 128, 24, 32 );
	cid->prv	= sdio_extract_bits( rsp, 128, 40, 8 );
	cid->month	= sdio_extract_bits( rsp, 128, 8, 4 );
	cid->year	= sdio_extract_bits( rsp, 128, 12, 8 ) + 2000;

	return( EOK );
}

int sd_parse_csd( sdio_dev_t *dev, sdio_csd_t *csd, uint32_t *rsp )
{
	uint32_t		blksz;
	uint32_t		csize;
	uint32_t		csizem;

	memset( csd, 0, sizeof( sdio_csd_t ) );
	csd->csd_structure      = sdio_extract_bits( rsp, 128, 126, 2 );
	csd->taac               = sdio_extract_bits( rsp, 128, 112, 8 );
	csd->nsac               = sdio_extract_bits( rsp, 128, 104, 8 );
	csd->tran_speed         = sdio_extract_bits( rsp, 128, 96, 8 );
	csd->ccc                = sdio_extract_bits( rsp, 128, 84, 12 );
	csd->read_bl_len        = sdio_extract_bits( rsp, 128, 80, 4 );
	csd->read_bl_partial    = sdio_extract_bits( rsp, 128, 79, 1 );
	csd->write_blk_misalign = sdio_extract_bits( rsp, 128, 78, 1 );
	csd->read_blk_misalign  = sdio_extract_bits( rsp, 128, 77, 1 );
	csd->dsr_imp            = sdio_extract_bits( rsp, 128, 76, 1 );

	switch( csd->csd_structure ) {
		case CSD_STRUCT_VER_10:			// Standard Capacity
			csd->c_size			= sdio_extract_bits( rsp, 128, 62, 12 );
			csd->vdd_r_curr_min	= sdio_extract_bits( rsp, 128, 59, 3 );
			csd->vdd_r_curr_max	= sdio_extract_bits( rsp, 128, 56, 3 );
			csd->vdd_w_curr_min	= sdio_extract_bits( rsp, 128, 53, 3 );
			csd->vdd_w_curr_max	= sdio_extract_bits( rsp, 128, 50, 3 );
			csd->c_size_mult	= sdio_extract_bits( rsp, 128, 47, 3 );
			blksz				= 1 << csd->read_bl_len;
			csize				= csd->c_size + 1;
			csizem				= 1 << ( csd->c_size_mult + 2 );
			break;

		case CSD_STRUCT_VER_20:			// High Capacity / Extended Capacity
			csd->c_size			= sdio_extract_bits( rsp, 128, 48, 22 );
			blksz  				= SDIO_DFLT_BLKSZ;
			csize  				= csd->c_size + 1;
			csizem 				= 1024;
			dev->caps			|= DEV_CAP_HC;
			break;

		default:
			return( EINVAL );
	}

	csd->erase_blk_en       = sdio_extract_bits( rsp, 128, 46, 1 );
	csd->sector_size        = sdio_extract_bits( rsp, 128, 39, 7 );
	csd->wp_grp_size        = sdio_extract_bits( rsp, 128, 32, 7 );
	csd->wp_grp_enable      = sdio_extract_bits( rsp, 128, 31, 1 );
	csd->r2w_factor         = sdio_extract_bits( rsp, 128, 26, 3 );
	csd->write_bl_len       = sdio_extract_bits( rsp, 128, 22, 4 );
	csd->write_bl_partial   = sdio_extract_bits( rsp, 128, 21, 1 );
//	csd->file_format_grp	= sdio_extract_bits( rsp, 128, 15, 1 );
//	csd->copy               = sdio_extract_bits( rsp, 128, 14, 1 );
	csd->write_protect		= sdio_extract_bits( rsp, 128, 12, 2 );
//	csd->file_format		= sdio_extract_bits( rsp, 128, 10, 2 );

		// force to 512 byte block
	if( blksz > SDIO_DFLT_BLKSZ && ( blksz % SDIO_DFLT_BLKSZ ) == 0 ) {
		csize = csize * ( blksz / SDIO_DFLT_BLKSZ );
		blksz = SDIO_DFLT_BLKSZ;
	}

	csd->blksz		= blksz;
	csd->sectors	= csize * csizem;
	csd->dtr_max 	= sd_tran_speed_fu[csd->tran_speed & 0x7] *
						sd_tran_speed_mf[(csd->tran_speed >> 3 ) & 0xf];

	return( EOK );
}

int sd_parse_scr( sdio_dev_t *dev, sd_scr_t *scr, uint32_t *rscr )
{
	memset( scr, 0, sizeof( sd_scr_t ) );

	rscr[0] = ENDIAN_BE32( rscr[0] );
	rscr[1] = ENDIAN_BE32( rscr[1] );

	scr->scr_structure			= sdio_extract_bits( rscr, 64, 60, 4 );
	scr->sd_spec				= sdio_extract_bits( rscr, 64, 56, 4 );
	scr->data_stat_after_erase	= sdio_extract_bits( rscr, 64, 55, 1 );
	scr->sd_security			= sdio_extract_bits( rscr, 64, 52, 3 );
	scr->sd_bus_widths			= sdio_extract_bits( rscr, 64, 48, 4 );

	scr->sd_spec3				= sdio_extract_bits( rscr, 64, 47, 1 );
	scr->ex_security			= sdio_extract_bits( rscr, 64, 43, 4 );
	scr->cmd_support			= sdio_extract_bits( rscr, 64, 32, 2 );
	if( ( scr->cmd_support & SCR_CMD23_SUP ) ) {
		dev->caps				|= DEV_CAP_CMD23;
	}
	return( EOK );
}

int sdio_send_relative_addr( sdio_hc_t *hc, int *rca )
{
	sdio_dev_t	*dev;
	sdio_cmd_t	*cmd;
	int			status;
	int			retries;

	if( ( cmd = sdio_alloc_cmd( ) ) == NULL ) {
		return( ENOMEM );
	}

	dev	= &hc->device;
	*rca	= 0;
	retries	= SD_SEND_RELATIVE_ADDR_RETRIES;

	do {
		sdio_setup_cmd( cmd, SCF_CTYPE_BCR | SCF_RSP_R6, SD_SEND_RELATIVE_ADDR, 0 );
		if( ( status = _sdio_send_cmd( dev, cmd, NULL, SDIO_TIME_DEFAULT, SDIO_CMD_RETRIES ) ) ) {
			break;
		}
		if( ( *rca = ( cmd->rsp[0] >> 16 ) ) != 0 ) {	// wait for non-zero RCA
			break;
		}
	} while( retries-- );

	if( *rca == 0 ) {								// validate RCA
		status = EIO;
	}

	sdio_free_cmd( cmd );

	return( status );
}

int sd_voltage_switch( sdio_hc_t *hc )
{
	sdio_dev_t		*dev;
	sdio_cmd_t		*cmd;
	int				status;

	if( ( cmd = sdio_alloc_cmd( ) ) == NULL ) {
		return( ENOMEM );
	}

	dev	= &hc->device;
	sdio_setup_cmd( cmd, SCF_CTYPE_AC | SCF_RSP_R1, SD_VOLTAGE_SWITCH, 0 );
	status = _sdio_send_cmd( dev, cmd, NULL, SDIO_TIME_DEFAULT, 0 );
	sdio_free_cmd( cmd );

	return( status );
}

int sd_switch( sdio_dev_t *dev, int mode, int grp, uint8_t val, uint8_t *switch_status )
{
	sdio_cmd_t		*cmd;
	int				arg;
	uint8_t			*sbuf;
	int				status;
	uint16_t		sw_status;
	sdio_sge_t		sge;

	if( ( cmd = sdio_alloc_cmd( ) ) == NULL ) {
		return( ENOMEM );
	}

	if( ( sbuf = sdio_alloc( SD_SF_STATUS_SIZE ) ) == NULL ) {
		sdio_free_cmd( cmd );
		return( ENOMEM );
	}

	arg = mode << 31 | 0x00ffffff;
	arg &= ~( 0xf << ( grp * 4 ) );
	arg |= ( val << ( grp * 4 ) );

	sdio_setup_cmd( cmd, SCF_CTYPE_ADTC | SCF_RSP_R1, SD_SWITCH_FUNC, arg );
	sge.sg_count = SD_SF_STATUS_SIZE; sge.sg_address = SDIO_DATA_PTR_P( sbuf );
	sdio_setup_cmd_io( cmd, SCF_DIR_IN, 1, SD_SF_STATUS_SIZE, &sge, 1, NULL );

	if( ( status = _sdio_send_cmd( dev, cmd, NULL, SDIO_TIME_DEFAULT, 0 ) ) == EOK ) {
		memcpy( switch_status, sbuf, SD_SF_STATUS_SIZE );

		if( mode == SD_SF_MODE_SET ) {
				// check function busy status
			sw_status = switch_status[29 - ( grp * 2 )] | ( switch_status[28 - ( grp * 2 )] << 8 );
			if( sw_status & ( 1 << val ) ) {
				status = EBUSY;
			}
		}
	}
	else {
		sdio_slogf( _SLOGC_SDIODI, _SLOG_ERROR, 0, 0, "%s: Error Switching %d", __FUNCTION__, status );

	}

	sdio_free( sbuf, SD_SF_STATUS_SIZE );

	sdio_free_cmd( cmd );

	return( status );
}

int sd_app_cmd( sdio_dev_t *dev )
{
	sdio_cmd_t		*cmd;
	int				ctype;
	int				status;

	if( ( cmd = sdio_alloc_cmd( ) ) == NULL ) {
		return( ENOMEM );
	}

	ctype = dev->rca ? SCF_CTYPE_BCR : SCF_CTYPE_BC;

	sdio_setup_cmd( cmd, ctype | SCF_RSP_R1, SD_APP_CMD, dev->rca << 16 );
	if( ( status = _sdio_send_cmd( dev, cmd, NULL, SDIO_TIME_DEFAULT, 0 ) ) == EOK ) {

	}

	sdio_free_cmd( cmd );

	return( status );
}

int sd_app_send_scr( sdio_dev_t *dev, uint32_t *scr )
{
	sdio_cmd_t		*cmd;
	uint32_t		*sbuf;
	int				status;
	sdio_sge_t		sge;

	if( ( cmd = sdio_alloc_cmd( ) ) == NULL ) {
		return( ENOMEM );
	}

	if( ( sbuf = sdio_alloc( SD_SCR_SIZE ) ) == NULL ) {
		sdio_free_cmd( cmd );
		return( ENOMEM );
	}

	sdio_setup_cmd( cmd, SCF_CTYPE_ADTC | SCF_RSP_R1 | SCF_APP_CMD, SD_AC_SEND_SCR, 0 );
	sge.sg_count = SD_SCR_SIZE; sge.sg_address = SDIO_DATA_PTR_P( sbuf );
	sdio_setup_cmd_io( cmd, SCF_DIR_IN, 1, SD_SCR_SIZE, &sge, 1, NULL );
	if( ( status = _sdio_send_cmd( dev, cmd, NULL, SDIO_TIME_DEFAULT, 0 ) ) == EOK ) {
		memcpy( scr, sbuf, SD_SCR_SIZE );
	}

	sdio_free( sbuf, SD_SCR_SIZE );
	sdio_free_cmd( cmd );

	return( status );
}

int sd_app_sd_status( sdio_dev_t *dev, uint32_t *sds )
{
	sdio_cmd_t		*cmd;
	uint32_t		*sbuf;
	int				status;
	sdio_sge_t		sge;

	if( ( cmd = sdio_alloc_cmd( ) ) == NULL ) {
		return( ENOMEM );
	}

	if( ( sbuf = sdio_alloc( SD_STATUS_SIZE ) ) == NULL ) {
		sdio_free_cmd( cmd );
		return( ENOMEM );
	}

	sdio_setup_cmd( cmd, SCF_CTYPE_ADTC | SCF_RSP_R1 | SCF_APP_CMD, SD_AC_SD_STATUS, 0 );
	sge.sg_count = SD_STATUS_SIZE; sge.sg_address = SDIO_DATA_PTR_P( sbuf );
	sdio_setup_cmd_io( cmd, SCF_DIR_IN, 1, SD_STATUS_SIZE, &sge, 1, NULL );
	if( ( status = _sdio_send_cmd( dev, cmd, NULL, SDIO_TIME_DEFAULT, 0 ) ) == EOK ) {
		memcpy( sds, sbuf, SD_STATUS_SIZE );
	}

	sdio_free( sbuf, SD_STATUS_SIZE );
	sdio_free_cmd( cmd );

	return( status );
}

uint64_t sd_erase_grp_size( sdio_dev_t *dev )
{
	uint64_t	erase_grp_size;

	if( dev->csd.erase_blk_en ) {
		erase_grp_size = 512;
	}
	else {
		erase_grp_size = ( dev->csd.sector_size + 1 ) * ( 1 << dev->csd.write_bl_len );
	}

	return( erase_grp_size );
}

uint64_t sd_erase_timeout( sdio_dev_t *dev, uint32_t etype, uint32_t nlba )
{
	uint64_t		timeout;

	if( dev->sds.erase_timeout ) {
		timeout = ( dev->sds.erase_timeout * nlba ) + dev->sds.erase_offset;
	}
	else {
		timeout = SD_ERASE_TIMEOUT * nlba;
	}

	return( timeout );
}

int sd_erase( sdio_dev_t *dev, int flgs, uint64_t lba, int nlba )
{
	sdio_cmd_t		*cmd;
	int				status;
	uint64_t		timeout;
	uint64_t		addr;

	status			= EOK;

	if( ( cmd = sdio_alloc_cmd( ) ) == NULL ) {
		return( ENOMEM );
	}

	while( nlba ) {
		addr = ( dev->caps & DEV_CAP_HC ) ? lba : ( lba * dev->csd.blksz );
		sdio_setup_cmd( cmd, SCF_CTYPE_AC | SCF_RSP_R1, SD_ERASE_WR_BLK_START, addr );
		if( ( status = _sdio_send_cmd( dev, cmd, NULL, SDIO_TIME_DEFAULT, 0 ) ) != EOK ) {
			break;
		}
		
		timeout = sd_erase_timeout( dev, flgs, nlba );
		lba		+= nlba;
		nlba	-= nlba;
		lba--;

		addr = ( dev->caps & DEV_CAP_HC ) ? lba : ( lba * dev->csd.blksz );
		sdio_setup_cmd( cmd, SCF_CTYPE_AC | SCF_RSP_R1, SD_ERASE_WR_BLK_END, addr );
		if( ( status = _sdio_send_cmd( dev, cmd, NULL, SDIO_TIME_DEFAULT, 0 ) ) != EOK ) {
			break;
		}

		sdio_setup_cmd( cmd, SCF_CTYPE_AC | SCF_RSP_R1B | SCF_WAIT_DRDY, SD_ERASE, flgs );
		if( ( status = _sdio_send_cmd( dev, cmd, NULL, timeout, 0 ) ) != EOK ) {
			break;
		}
	}

	sdio_free_cmd( cmd );

	return( status );
}

int sd_send_if_cond( sdio_hc_t *hc, uint32_t vhs )
{
	sdio_dev_t		*dev;
	sdio_cmd_t		*cmd;
	int				status;

	if( ( cmd = sdio_alloc_cmd( ) ) == NULL ) {
		return( ENOMEM );
	}

	dev	= &hc->device;

	sdio_setup_cmd( cmd, SCF_CTYPE_BCR | SCF_RSP_R7, SD_SEND_IF_COND, ( vhs << 8 ) | SD_SIC_TEST_PATTERN );
	if( ( status = _sdio_send_cmd( dev, cmd, NULL, SDIO_TIME_DEFAULT, 0 ) ) == EOK ) {
		if( ( cmd->rsp[0] & 0xff ) != SD_SIC_TEST_PATTERN ) {
			status = EIO;
		}
	}
	else {
		if( cmd->status == CS_CMD_TO_ERR ) {
			status = ETIMEDOUT;
		}
		else {
			status = EIO;
		}
	}

	sdio_free_cmd( cmd );

	return( status );
}

int sd_app_send_op_cond( sdio_hc_t *hc, uint32_t ocr, uint32_t *rocr )
{
	sdio_dev_t		*dev;
	sdio_cmd_t		*cmd;
	int				retry;
	int				status;

	if( ( cmd = sdio_alloc_cmd( ) ) == NULL ) {
		return( ENOMEM );
	}

	status	= EOK;
	dev		= &hc->device;

	sdio_setup_cmd( cmd, SCF_CTYPE_BCR | SCF_RSP_R3 | SCF_APP_CMD, SD_AC_SEND_OP_COND, ocr );

	for( retry = OCR_BUSY_RETRIES; retry; retry-- ) {
		if( ( status = _sdio_send_cmd( dev, cmd, NULL, SDIO_TIME_DEFAULT, SDIO_CMD_RETRIES ) ) != EOK ) {
			break;
		}

		if( !ocr ) {
			break;
		}

		if( ( cmd->rsp[0] & OCR_PWRUP_CMP ) ) {
			break;
		}

		delay( 10 );
	}

	if( hc->cfg.verbosity > 3 ) {
		sdio_slogf( _SLOGC_SDIODI, _SLOG_ERROR, hc->cfg.verbosity, 0, "%s: retries %d, rsp0 0x%x", __FUNCTION__, OCR_BUSY_RETRIES - retry, cmd->rsp[0] );
	}

	if( rocr ) {
		*rocr = cmd->rsp[0];
	}

	sdio_free_cmd( cmd );

	if( !retry ) status = ETIMEDOUT;

	return( status );
}

int sd_read_switch( sdio_dev_t *dev )
{
	sdio_hc_t			*hc;
	sd_switch_cap_t		*swcaps;
	int					status;
	uint8_t				ss[SD_SF_STATUS_SIZE];

	hc		= dev->hc;
	swcaps	= &dev->swcaps;

	if( !( dev->csd.ccc & CCC_SWITCH ) ) {
		return( EOK );
	}

		// get supported bus speeds
	if( ( status = sd_switch( dev, SD_SF_MODE_CHECK, SD_SF_GRP_BUS_SPD, SD_SF_CUR_FCN, ss ) ) ) {
		return( status );
	}

	if( ( ss[13] & 0x2 ) ) {
		swcaps->dtr_max_hs	= 50000000;

		if( ( hc->caps & HC_CAP_HS ) ) {
			dev->caps			|= DEV_CAP_HS;
		}
	}

	if( dev->scr.sd_spec3 ) {
		swcaps->bus_mode = ss[13] & SD_BUS_MODE_MSK;
		swcaps->drv_type = ss[9] & SD_DRV_TYPE_MSK;
		swcaps->curr_limit = ss[7] & SD_CURR_LIMIT_MSK;

		if( ( swcaps->bus_mode & SD_BUS_MODE_UHS ) ) {
			dev->caps |= DEV_CAP_UHS;
		}
	}

	swcaps->cmd_sys = ss[11];

	if( ( ss[11] & SD_CMD_SYS_EC ) || ( ss[11] & SD_CMD_SYS_ASSD ) ) {
		dev->caps |= DEV_CAP_ASSD;
	}

	return( status );
}

int sd_read_status( sdio_dev_t *dev )
{
	int				idx;
	int				status;
	uint32_t		au;
	uint32_t		e_size;
	uint32_t		e_offset;
	uint32_t		e_timeout;
	uint32_t		sds[SD_STATUS_SIZE / 4];


	if( !( dev->csd.ccc & CCC_APP_SPEC ) ) {
		return( EOK );
	}

	if( ( status = sd_app_sd_status( dev, sds ) ) ) {
		return( status );
	}

	for( idx = 0; idx < 16; idx++ ) {
		sds[idx] = ENDIAN_BE32( sds[idx] );
	}

	au = sdio_extract_bits( sds, 512, 428, 4 );
	if( au ) {
		dev->sds.au_size = 1 << ( au + 4 );

		e_size			= sdio_extract_bits( sds, 512, 408, 16 );
		e_offset		= sdio_extract_bits( sds, 512, 400, 2 );
		e_timeout		= sdio_extract_bits( sds, 512, 402, 6 );
		if( e_size && e_timeout ) {
			dev->sds.erase_offset		= e_offset * 1000;
			dev->sds.erase_timeout		= ( e_timeout * 1000 ) / e_size;
		}
	}

	au = sdio_extract_bits( sds, 512, 396, 4 );
	if( au ) {
//		dev->sds.uhs_au_size = 1 << ( au + 4 );
	}


	dev->sds.speed_class		= sdio_extract_bits( sds, 512, 440, 8 );
	dev->sds.uhs_speed_grade	= sdio_extract_bits( sds, 512, 396, 4 );
	return( status );
}

int sd_switch_hs( sdio_dev_t *dev )
{
	sdio_hc_t	*hc;
	int			status;
	uint8_t		sw_status[64] = { 0 };

	hc = dev->hc;

	if( ( status = sd_switch( dev, SD_SF_MODE_SET, SD_SF_GRP_BUS_SPD, 
				1, sw_status ) ) == EOK ) {
		if( ( sw_status[16] & 0xF ) != 0x1 ) {
			status = EIO;
		}
	}

	if( status ) {
		sdio_slogf( _SLOGC_SDIODI, _SLOG_ERROR, hc->cfg.verbosity, 0, "%s: Error Switching", __FUNCTION__ );
	}

	return( status );
}

int sd_app_set_bus_width( sdio_dev_t *dev, int width )
{
	sdio_cmd_t		*cmd;
	int				status;

	if( ( cmd = sdio_alloc_cmd( ) ) == NULL ) {
		return( ENOMEM );
	}

	sdio_setup_cmd( cmd, SCF_CTYPE_AC | SCF_APP_CMD | SCF_RSP_R1, SD_AC_SET_BUS_WIDTH, width );
	status = _sdio_send_cmd( dev, cmd, NULL, SDIO_TIME_DEFAULT, SDIO_CMD_RETRIES );
	sdio_free_cmd( cmd );

	return( status );
}

int sd_set_bus_width( sdio_dev_t *dev, int width )
{
	sdio_hc_t	*hc;
	int			bus_width;
	int			status;

	hc		= dev->hc;
	status	= EOK;

	switch( width ) {
		case BUS_WIDTH_1:
			bus_width = SD_BUS_WIDTH_1;
			break;

		case BUS_WIDTH_4:
			if( ( hc->caps & HC_CAP_BW4 ) && ( dev->scr.sd_bus_widths & SCR_BUS_WIDTH_4 ) ) {
				bus_width = SD_BUS_WIDTH_4;
			}
			else {
				status = EINVAL;
			}
			break;

		case BUS_WIDTH_8:
		default:
			status = EINVAL;
			break;
	}

	if( status == EOK && ( status = sd_app_set_bus_width( dev, bus_width ) ) == EOK ) {
		sdio_bus_width( hc, BUS_WIDTH_4 );
	}
	else {
		sdio_slogf( _SLOGC_SDIODI, _SLOG_ERROR, hc->cfg.verbosity, 0, "%s: Error Setting Bus Width %d", __FUNCTION__, width );
	}

	return( status );
}

int sd_set_drv_type( sdio_dev_t *dev, int bus_spd, int drv_type )
{
	sdio_hc_t	*hc;
	int			idx;
	int			status;
	int			driver_strength;
	static int	timing[5]	= { TIMING_SDR12, TIMING_SDR25, TIMING_SDR50,
								TIMING_SDR104, TIMING_DDR50 };
	uint8_t		sw_status[64] = { 0 };

	hc = dev->hc;

	if( !HC_CAP_DRV_TYPES( hc->caps ) || !hc->entry.driver_strength ) {
		return( EOK );
	}

	idx			= ( bus_spd ) ? ( fls( bus_spd ) ) - 1 : 0;

	if( idx >= SDIO_NELEMS( timing ) ) {
		return( EINVAL );
	}

	drv_type	= hc->entry.driver_strength( hc, timing[idx], drv_type | SD_DRV_TYPE_B );
	driver_strength = fls( drv_type ) - 1;
	if( ( status = sd_switch( dev, SD_SF_MODE_SET, SD_SF_GRP_DRV_STR, driver_strength, sw_status ) ) == EOK ) {
		if( ( sw_status[15] & 0xF ) == driver_strength ) {
			sdio_drv_type( hc, drv_type );
		}
		else {
			status = EIO;
		}
	}

	if( status ) {
		sdio_slogf( _SLOGC_SDIODI, _SLOG_ERROR, hc->cfg.verbosity, 0, "%s: Error Switching drive strength %d", __FUNCTION__, driver_strength );
	}

	return( status );
}

int sd_set_bus_speed_mode( sdio_dev_t *dev, int bus_spd )
{
	sdio_hc_t	*hc;
	int			idx;
	int			status;
	static int	dtr[5]		= { DTR_MAX_SDR12, DTR_MAX_SDR25, DTR_MAX_SDR50,
								DTR_MAX_SDR104, DTR_MAX_DDR50 };
	static int	timing[5]	= { TIMING_SDR12, TIMING_SDR25, TIMING_SDR50,
								TIMING_SDR104, TIMING_DDR50 };
	uint8_t		sw_status[64] = { 0 };

	if( !bus_spd ) {
		return( EOK );
	}

	hc	= dev->hc;
	idx	= fls( bus_spd ) - 1;		// get index to fastest speed

	if( idx >= SDIO_NELEMS( timing ) ) {
		return( EINVAL );
	}

	if( ( status = sd_switch( dev, SD_SF_MODE_SET, SD_SF_GRP_BUS_SPD, idx, sw_status ) ) == EOK ) {
		if( ( sw_status[16] & 0xF ) == idx ) {
			sdio_timing( hc, timing[idx] );
			sdio_clock( hc, dtr[idx] );
		}
		else {
			status = EIO;
		}
	}

	if( status ) {
		sdio_slogf( _SLOGC_SDIODI, _SLOG_ERROR, hc->cfg.verbosity, 0, "%s: Error Switching speed %d", __FUNCTION__, idx );
	}

	return( status );
}

int sd_set_current_limit( sdio_dev_t *dev, int bus_spd, int curr_limit )
{
	sdio_hc_t	*hc;
	int			idx;
	int			status;
	uint8_t		sw_status[64] = { 0 };

	hc			= dev->hc;
	idx			= 0;

		// reconcile hc and device current limits
	curr_limit	= HC_CAP_CURRENT( hc->caps ) & curr_limit;

	if( curr_limit && ( bus_spd & ( SD_BUS_MODE_SDR50 | SD_BUS_MODE_SDR104 | SD_BUS_MODE_DDR50 ) ) ) {
		idx = fls( curr_limit ) - 1;	// get index to highest current
	}

	if( ( status = sd_switch( dev, SD_SF_MODE_SET, SD_SF_GRP_CUR_LMT, idx, sw_status ) ) == EOK ) {
		if( ( ( sw_status[15] >> 4 ) & 0x0F ) != idx ) {
			status = EIO;
		}
	}

	if( status ) {
		sdio_slogf( _SLOGC_SDIODI, _SLOG_ERROR, hc->cfg.verbosity, 0, "%s: Error Switching speed %d, current %d", __FUNCTION__, bus_spd, idx );
	}

	return( status );
}

int sd_init_assd( sdio_dev_t *dev )
{
	sdio_hc_t	*hc;
	sd_switch_cap_t		*swcaps;
	int					idx;
	int					status;
	uint8_t				sw_status[64] = { 0 };

	hc	= dev->hc;
	swcaps	= &dev->swcaps;

		// get index to eC/ASSD
	idx = ffs( swcaps->cmd_sys & ( SD_CMD_SYS_EC | SD_CMD_SYS_ASSD ) ) - 1;
	if( ( status = sd_switch( dev, SD_SF_MODE_SET, SD_SF_GRP_CMD_EXT, idx, sw_status ) ) == EOK ) {
		if( ( ( sw_status[16] >> 4 ) & 0x0F ) != idx ) {
			status = EIO;
		}
		else {
			if( dev->rsettle ) {	// some devices require more time
				delay( dev->rsettle );
			}
		}
	}

	if( status ) {
		sdio_slogf( _SLOGC_SDIODI, _SLOG_ERROR, hc->cfg.verbosity, 0, "%s: Error Switching ASSD %d, sw_status 0x%x, status %d", __FUNCTION__, idx, sw_status[16] >> 4, status );
	}

	return( status );
}

int sd_select_bus_mode( sdio_dev_t *dev, int bus_mode )
{
	sdio_hc_t		*hc;
	int				bus_spd_mode;

	hc				= dev->hc;

		// reconcile hc and device bus speed modes
	bus_spd_mode	= HC_CAP_UHS( hc->caps ) & bus_mode;

	if( bus_spd_mode & SD_BUS_MODE_SDR104 )
		bus_spd_mode = SD_BUS_MODE_SDR104;
	else if( bus_spd_mode & SD_BUS_MODE_DDR50 )
		bus_spd_mode = SD_BUS_MODE_DDR50;
	else if( bus_spd_mode & SD_BUS_MODE_SDR50 )
		bus_spd_mode = SD_BUS_MODE_SDR50;
	else if( bus_spd_mode & SD_BUS_MODE_SDR25 )
		bus_spd_mode = SD_BUS_MODE_SDR25;
	else
		bus_spd_mode = SD_BUS_MODE_SDR12;

	return( bus_spd_mode );
}

int sd_init_uhs( sdio_dev_t *dev )
{
	sdio_hc_t			*hc;
	sd_switch_cap_t		*swcaps;
	int					status;
	int					bus_spd_mode;

	hc		= dev->hc;
	swcaps	= &dev->swcaps;

	if( ( status = sd_set_bus_width( dev, BUS_WIDTH_4 ) ) != EOK ) {
		return( status );
	}

	bus_spd_mode = sd_select_bus_mode( dev, swcaps->bus_mode );

	if( ( status = sd_set_drv_type( dev, bus_spd_mode, swcaps->drv_type ) ) != EOK ) {
		return( status );
	}

	if( ( status = sd_set_current_limit( dev, bus_spd_mode, swcaps->curr_limit ) ) ) {
		return( status );
	}

	if( ( status = sd_set_bus_speed_mode( dev, bus_spd_mode ) ) != EOK ) {
		return( status );
	}

	sdio_preset( hc, SDIO_TRUE );
	if( ( status = sdio_tune( hc, SD_SEND_TUNING_BLOCK ) ) ) {
			// tuning may have failed, but we should should be able to continue.
			// ie hc will use defaults an we will re-tune if we get a CRC error
		sdio_slogf( _SLOGC_SDIODI, _SLOG_ERROR, hc->cfg.verbosity, 0, "%s: tuning failure %s (%d)", __FUNCTION__, strerror( status ), status );
	}

	dev->flags |= DEV_FLAG_UHS;

	return( EOK );
}

int sd_init_bus( sdio_hc_t *hc )
{
	sdio_dev_t		*dev;
	uint32_t		dtr;
	int				timing;
	int				status;

	dev		= &hc->device;
	dtr		= dev->csd.dtr_max;
	timing	= TIMING_LS;

	if( hc->signal_voltage == SIGNAL_VOLTAGE_1_8 ) {	// UHS card
		if( ( status = sd_init_uhs( dev ) ) != EOK ) {
			return( status );
		}
	}
	else {
		if( ( dev->caps & DEV_CAP_HS ) ) {				// HS card
			if( ( status = sd_switch_hs( dev ) ) != EOK ) {
				return( status );
			}

			timing		= TIMING_HS;
			dtr			= dev->swcaps.dtr_max_hs;
			dev->flags	|= DEV_FLAG_HS;
		}

		if( ( hc->caps & HC_CAP_BW4 ) && ( dev->scr.sd_bus_widths & SCR_BUS_WIDTH_4 ) ) {
			if( ( status = sd_set_bus_width( dev, BUS_WIDTH_4 ) ) != EOK ) {
				return( status );
			}
		}

		sdio_timing( hc, timing );
		sdio_clock( hc, dtr );
	
		if( ( status = _sdio_set_block_length( dev, SDIO_DFLT_BLKSZ ) ) != EOK ) {
			return( status );
		}
	}

	if( ( dev->caps & DEV_CAP_ASSD ) ) {
		status = sd_init_assd( dev );
	}

	return( status );
}

int sd_capabilities( sdio_dev_t *dev )
{
	int		status;

	if( ( status = sd_app_send_scr( dev, dev->raw_scr ) ) != EOK ) {
		return( status );
	}

	if( ( status = sd_parse_scr( dev, &dev->scr, dev->raw_scr ) ) != EOK ) {
		return( status );
	}

	if( ( status = sd_read_status( dev ) ) != EOK ) {
		return( status );
	}

	if( ( status = sd_read_switch( dev ) ) != EOK ) {
		return( status );
	}

//	dev->sector_size	= dev->csd.blksz;
//	dev->sectors		= dev->csd.sectors;
	dev->erase_size		= sd_erase_grp_size( dev );

	return( status );
}

int sd_lock_unlock( sdio_dev_t *dev, int action, uint8_t *pwd, int pwd_len )
{
	sdio_hc_t	*hc;
	uint32_t	oflags;
	uint32_t	status;

	hc		= dev->hc;
	oflags	= dev->flags;

	if( pwd_len ) {
		memcpy( &dev->pwd, pwd, pwd_len );
		dev->pwd_len = pwd_len;
	}

	if( ( status = _sdio_lock_unlock( dev, action, pwd, pwd_len ) ) ) {

	}

	if( ( dev->flags & DEV_FLAG_LOCKED ) ) {
		if( action == SD_LU_UNLOCK ) {
			status = EACCES;
		}
	}
	else {
		if( ( oflags & DEV_FLAG_LOCKED ) ) {
			if( ( status = sd_capabilities( dev ) ) ) {
				return( status );
			}
			if( ( status = sd_init_bus( hc ) ) != EOK ) {
				return( status );
			}
		}
	}

	return( status );
}

int sd_unlock( sdio_dev_t *dev )
{
	sdio_hc_t	*hc;
	int			status;

	hc		= dev->hc;
	status	= EOK;

	if( ( dev->flags & DEV_FLAG_LOCKED ) && dev->pwd_len ) {
		if( ( status = _sdio_lock_unlock( dev, SD_LU_UNLOCK, dev->pwd, dev->pwd_len ) ) ) {
			sdio_slogf( _SLOGC_SDIODI, _SLOG_ERROR, hc->cfg.verbosity, 0, "%s: SD UNLOCK failure", __FUNCTION__ );
		}
	}

	return( status );
}

int sd_init_device( sdio_hc_t *hc, uint32_t ocr, int flgs )
{
	sdio_dev_t				*dev;
	uint32_t				rocr;
	uint32_t				cid[4];
	int						status;
	sdio_device_errata_t	*errata;

	dev			= &hc->device;
	dev->rca	= 0;

	sd_send_if_cond( hc, SD_SIC_VHS_27_36V );
	sdio_go_idle( hc );

		// determing card version
	if( ( status = sd_send_if_cond( hc, SD_SIC_VHS_27_36V ) ) == EOK ) {
		ocr |= OCR_HCS;				// 2.0 card, enable high capacity
	}
	else if( status != ETIMEDOUT ) {
		return( EIO );
	}
	else if( status != ETIMEDOUT ) {
		return( EIO );
	}
	

		// HC supports 1.8V signalling
	if( !( dev->flags & DEV_FLAG_SIG_ERR ) && HC_CAP_UHS( hc->caps ) && ( hc->caps & HC_CAP_SV_1_8V ) ) {
		if( !flgs || ( flgs && dev->swcaps.bus_mode ) ) {
			ocr |= OCR_S18R;
		}
	}

		// verify HC can supply > 150ma
	if( HC_CAP_XPC( hc->caps ) ) {
		ocr |= OCR_XPC;
	}

	while( 1 ) {
		if( ( status = sd_app_send_op_cond( hc, ocr, &rocr ) ) != EOK ) {
			return( status );
		}

		if( ( rocr & ( OCR_HCS | OCR_S18A ) ) == ( OCR_HCS | OCR_S18A ) ) {
			if( ( status = sd_voltage_switch( hc ) ) != EOK ) {
				ocr &= ~OCR_S18R;
				continue;
			}
			if( ( status = sdio_signal_voltage( hc, SIGNAL_VOLTAGE_1_8 ) ) ) {
				dev->flags |= DEV_FLAG_SIG_ERR;
				return( status );
			}
		}
		break;
	}
	
	if( ( status = sdio_all_send_cid( hc, cid ) ) != EOK ) {
		return( status );
	}

	if( flgs && memcmp( dev->raw_cid, cid, sizeof( cid ) ) ) {
		return( ENXIO );
	}

	memcpy( dev->raw_cid, cid, sizeof( cid ) );

	if( ( status = sdio_send_relative_addr( hc, &dev->rca ) ) != EOK ) {
		return( status );
	}

	sdio_bus_mode( hc, BUS_MODE_PUSH_PULL );

	if( !flgs ) {
		if( ( status = sd_parse_cid( dev, &dev->cid, dev->raw_cid ) ) != EOK ) {
			return( status );
		}

		if( ( status = sdio_send_csd( dev, dev->raw_csd ) ) != EOK ) {
			return( status );
		}

		if( ( status = sd_parse_csd( dev, &dev->csd, dev->raw_csd ) ) != EOK ) {
			return( status );
		}

		if( ( errata = sdio_device_errata( dev, sd_errata ) ) ) {
			sdio_reconcile_errata( dev, errata );
			dev->rsettle = errata->rsettle;
		}
	}

	if( ( status = sdio_select_card( dev, dev->rca ) ) != EOK ) {
		return( status );
	}

	if( ( status = sd_unlock( dev ) ) ) {		// unlock card if needed
		return( status );
	}

	if( ( dev->flags & DEV_FLAG_LOCKED ) ) {	// card locked, wait for unlock
		return( EOK );
	}

	if( !flgs ) {
		if( ( status = sd_capabilities( dev ) ) ) {
			return( status );
		}
	}

	if( ( status = sd_init_bus( hc ) ) != EOK ) {
		return( status );
	}

	return( status );
}

int sd_bus_error( sdio_dev_t *dev )
{
	sdio_csd_t			*csd;
	sd_switch_cap_t		*swcaps;

	csd		= &dev->csd;
	swcaps	= &dev->swcaps;

	if( ( dev->flags & DEV_FLAG_UHS ) ) {
		dev->flags &= ~DEV_FLAG_UHS;
		swcaps->bus_mode &= ~sd_select_bus_mode( dev, swcaps->bus_mode );
	}
	else if( ( dev->flags & DEV_FLAG_HS ) ) {
		dev->flags	&= ~DEV_FLAG_HS;
		swcaps->dtr_max_hs = max( DTR_MAX_HS26, swcaps->dtr_max_hs - DTR_HS_DEC );
	}
	else {
		csd->dtr_max = max( DTR_MIN_LS, csd->dtr_max - DTR_LS_DEC );
	}

	return( sd_init_bus( dev->hc ) );
}

int sd_ident( sdio_hc_t *hc )
{
	sdio_dev_t	*dev;
	uint32_t	ocr;
	int			status;

	dev		= &hc->device;
	dev->rca	= 0;

	sdio_go_idle( hc );
	sd_send_if_cond( hc, SD_SIC_VHS_27_36V );
	if( ( status = sd_app_send_op_cond( hc, 0, &ocr ) ) == EOK ) {
		if( ( status = sdio_select_voltage( hc, ocr ) ) == EOK ) {
			if( ( status = sd_init_device( hc, dev->ocr, SDIO_FALSE ) ) == EOK ) {
				return( status );
			}
		}
	}
	
	return( status );
}


#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/hardware/devb/sdmmc/sdiodi/sd.c $ $Rev: 805416 $")
#endif
