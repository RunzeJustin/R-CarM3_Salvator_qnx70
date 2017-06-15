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

// Module Description:

#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#include <internal.h>

const uint32_t mmc_tran_speed_fu[] = { 10000, 100000, 1000000, 10000000, 0, 0, 0, 0 };
const uint32_t mmc_tran_speed_mf[] = { 0, 10, 12, 13, 15, 20, 25, 30,
										35, 40, 45, 50, 55, 60, 70, 80 };

const uint32_t mmc_tacc_tu[] = { 1, 10, 100, 1000, 100000, 1000000, 10000000 };
const uint32_t mmc_tacc_mf[] = { 0, 10, 12, 13, 15, 20, 25, 30,
								35, 40, 45, 50, 55, 60, 70, 80 };
sdio_device_errata_t mmc_errata[] = {
//          pnm, mid, oid, prv_s, prv_e, erev, errata, rsettle
	// Samsung devices
		// fw bug where device will discard data when AC12 is used
	{ "MAG2GA", 0x15, 0x0, 0x0, 0x6, ECSD_REV_V4_41, DEV_ERRATA_ACMD12 | DEV_ERRATA_DISCARD_SUP, 0 },
		// fw added support for DISCARD
	{ "MAG2GA", 0x15, 0x0, 0x0, DEV_ERRATA_WILDCARD, ECSD_REV_V4_41, DEV_ERRATA_DISCARD_SUP, 0 },
		// fw bug where device will discard data when AC12 is used
	{ "MBG4GA", 0x15, 0x0, 0x0, 0x6, ECSD_REV_V4_41, DEV_ERRATA_ACMD12 | DEV_ERRATA_DISCARD_SUP, 0 },
		// fw added support for DISCARD
	{ "MBG4GA", 0x15, 0x0, 0x0, DEV_ERRATA_WILDCARD, ECSD_REV_V4_41, DEV_ERRATA_DISCARD_SUP, 0 },
	{ "MCG8GA", 0x15, 0x0, 0x0, DEV_ERRATA_WILDCARD, ECSD_REV_V4_41, DEV_ERRATA_DISCARD_SUP, 0 },

	// Sandisk devices
	{ "SEM08G", 0x45, 0x0, 0x0, DEV_ERRATA_WILDCARD, DEV_ERRATA_WILDCARD, DEV_ERRATA_DISCARD_SUP, 0 },

	{ NULL, 0, 0, 0, 0, 0, 0, 0 }
};

// Parse Card Identification Data
int mmc_parse_cid( sdio_dev_t *dev, sdio_cid_t *cid, uint32_t *rsp )
{
	int						idx;
	int						status;

	memset( cid, 0, sizeof( sdio_cid_t ) );

	status		= EOK;

	switch( dev->csd.spec_vers ) {
		case CSD_SPEC_VER_0:
		case CSD_SPEC_VER_1:
			cid->mid    = sdio_extract_bits( rsp, 128, 104, 24 );
			for( idx = 0; idx < 7; idx++ ) {
				cid->pnm[idx] = sdio_extract_bits( rsp, 128, 96 - idx * 8, 8 );
			}
			cid->prv    = sdio_extract_bits( rsp, 128, 40, 8 );
			cid->month  = sdio_extract_bits( rsp, 128, 12, 4 );
			cid->year   = sdio_extract_bits( rsp, 128, 8, 4 ) + 1997;
			break;

		case CSD_SPEC_VER_2:
		case CSD_SPEC_VER_3:
		case CSD_SPEC_VER_4:
			cid->mid    = sdio_extract_bits( rsp, 128, 120, 8 );
			cid->oid    = sdio_extract_bits( rsp, 128, 104, 8 );
			for( idx = 0; idx < 6; idx++ ) {
				cid->pnm[idx] = sdio_extract_bits( rsp, 128, 96 - idx * 8, 8 );
			}

			cid->prv    = sdio_extract_bits( rsp, 128, 48, 8 );
			cid->psn    = sdio_extract_bits( rsp, 128, 16, 32 );
			cid->month	= sdio_extract_bits( rsp, 128, 12, 4 );
			cid->year	= sdio_extract_bits( rsp, 128, 8, 4 ) + 1997;
			break;

		default:
			status = EINVAL;
			break;
	}

	return( status );
}

// Parse Card Specfic Data
int mmc_parse_csd( sdio_dev_t *dev, sdio_csd_t *csd, uint32_t *rsp )
{
	uint32_t		blksz;
	uint32_t		csize;
	uint32_t		csizem;

	memset( csd, 0, sizeof( sdio_csd_t ) );
	csd->csd_structure      = sdio_extract_bits( rsp, 128, 126, 2 );
	csd->spec_vers			= sdio_extract_bits( rsp, 128, 122, 4 );
	csd->taac               = sdio_extract_bits( rsp, 128, 112, 8 );
	csd->nsac               = sdio_extract_bits( rsp, 128, 104, 8 );
	csd->tran_speed         = sdio_extract_bits( rsp, 128, 96, 8 );
	csd->ccc                = sdio_extract_bits( rsp, 128, 84, 12 );
	csd->read_bl_len        = sdio_extract_bits( rsp, 128, 80, 4 );
	csd->read_bl_partial    = sdio_extract_bits( rsp, 128, 79, 1 );
	csd->write_blk_misalign = sdio_extract_bits( rsp, 128, 78, 1 );
	csd->read_blk_misalign  = sdio_extract_bits( rsp, 128, 77, 1 );
	csd->dsr_imp            = sdio_extract_bits( rsp, 128, 76, 1 );
	csd->c_size				= sdio_extract_bits( rsp, 128, 62, 12 );
	csd->vdd_r_curr_min		= sdio_extract_bits( rsp, 128, 59, 3 );
	csd->vdd_r_curr_max		= sdio_extract_bits( rsp, 128, 56, 3 );
	csd->vdd_w_curr_min		= sdio_extract_bits( rsp, 128, 53, 3 );
	csd->vdd_w_curr_max		= sdio_extract_bits( rsp, 128, 50, 3 );
	csd->c_size_mult		= sdio_extract_bits( rsp, 128, 47, 3 );
	switch( csd->csd_structure ) {
		case CSD_STRUCT_VER_10:
		case CSD_STRUCT_VER_11:
			csd->erase_grp_mult		= 1;
			csd->sector_size		= sdio_extract_bits( rsp, 128, 42, 5 );
			csd->erase_grp_size		= sdio_extract_bits( rsp, 128, 37, 5 );
			break;

		case CSD_STRUCT_VER_12:			// 4.1 - 4.3
		case CSD_STRUCT_VER_EXT_CSD:
			csd->erase_grp_mult	= sdio_extract_bits( rsp, 128, 37, 5 );
			csd->erase_grp_size	= sdio_extract_bits( rsp, 128, 42, 5 );
			break;

		default:
			break;
	}

	csd->wp_grp_size		= sdio_extract_bits( rsp, 128, 32, 7 );
	csd->wp_grp_enable		= sdio_extract_bits( rsp, 128, 31, 1 );
//	csd->default_ecc        = sdio_extract_bits( rsp, 128, 29, 2 );
	csd->r2w_factor         = sdio_extract_bits( rsp, 128, 26, 3 );
	csd->write_bl_len       = sdio_extract_bits( rsp, 128, 22, 4 );
	csd->write_bl_partial   = sdio_extract_bits( rsp, 128, 21, 1 );

//	csd->content_prot_app	= sdio_extract_bits( rsp, 128, 16, 1 );
//	csd->file_format_grp	= sdio_extract_bits( rsp, 128, 15, 1 );
//	csd->copy               = sdio_extract_bits( rsp, 128, 14, 1 );
	csd->write_protect		= sdio_extract_bits( rsp, 128, 12, 2 );
//	csd->file_format		= sdio_extract_bits( rsp, 128, 10, 2 );
//	csd->ecc				= sdio_extract_bits( rsp, 128, 8, 2 );

	blksz  = 1 << csd->read_bl_len;
	csize  = csd->c_size + 1;
	csizem = 1 << ( csd->c_size_mult + 2 );

		// force to 512 byte block
	if( blksz > SDIO_DFLT_BLKSZ && ( blksz % SDIO_DFLT_BLKSZ ) == 0 ) {
		csize = csize * ( blksz / SDIO_DFLT_BLKSZ );
		blksz = SDIO_DFLT_BLKSZ;
	}

	csd->blksz		= blksz;
	csd->sectors	= csize * csizem;
	csd->dtr_max 	= mmc_tran_speed_fu[csd->tran_speed & 0x7] *
						mmc_tran_speed_mf[(csd->tran_speed >> 3 ) & 0xf];

	return( EOK );
}

// Parse Extended Card Specfic Data
int mmc_parse_ext_csd( sdio_dev_t *dev, sdio_ecsd_t *ecsd, uint8_t *raw_ecsd )
{
	sdio_hc_t		*hc;

	memset( ecsd, 0, sizeof( sdio_ecsd_t ) );

	hc					= dev->hc;
	dev->pactive		= 0;
	ecsd->ext_csd_rev	= raw_ecsd[ECSD_REV];
	ecsd->blksz			= SDIO_DFLT_BLKSZ;

	dev->caps			|= DEV_CAP_CMD23;

	if( ecsd->ext_csd_rev >= ECSD_REV_V4_2 ) {
		ecsd->sectors =	raw_ecsd[ECSD_SEC_CNT + 0] << 0 |
						raw_ecsd[ECSD_SEC_CNT + 1] << 8 |
						raw_ecsd[ECSD_SEC_CNT + 2] << 16 |
						raw_ecsd[ECSD_SEC_CNT + 3] << 24;
		if( ecsd->sectors > ECSD_SEC_CNT_2GB ) {
			dev->caps	|= DEV_CAP_HC;
		}
	}

	if( ecsd->ext_csd_rev >= ECSD_REV_V4_3 ) {
		// boot partitions
		ecsd->part_config	= raw_ecsd[ECSD_PART_CONFIG] & ECSD_PC_ACCESS_MSK;
		dev->pactive		= ecsd->part_config;

		if( raw_ecsd[ECSD_ACC_SIZE] ) {
			ecsd->acc_size	= 512 * ( 1 << ( raw_ecsd[ECSD_ACC_SIZE] - 1 ) );
		}

		// reliable write

		// sleep modes
		dev->caps	|= DEV_CAP_SLEEP;

		if( raw_ecsd[ECSD_S_A_TIMEOUT] && raw_ecsd[ECSD_S_A_TIMEOUT] <= 0x17 ) {
			ecsd->s_a_timeout = 1 << raw_ecsd[ECSD_S_A_TIMEOUT];
		}
	}

	if( ecsd->ext_csd_rev >= ECSD_REV_V4_4 ) {
		// RPMB
		// DDR dual data rate

		// TRIM
		if( raw_ecsd[ECSD_SEC_FEATURE_SUPPORT] & ECSD_SEC_GB_CL_EN ) {
			dev->caps	|= DEV_CAP_TRIM;
		}

		// SECURE
		if( raw_ecsd[ECSD_SEC_FEATURE_SUPPORT] & ECSD_SEC_ER_EN ) {
			dev->caps	|= DEV_CAP_SECURE;
		}
	}

	if( ecsd->ext_csd_rev >= ECSD_REV_V4_41 ) {
		// GP partitions

		// BKOPS
		if( ( raw_ecsd[ECSD_BKOPS_SUPPORTED] & ECSD_BKOPS_SUP ) ) {
			dev->caps |= DEV_CAP_BKOPS;
			if( ( raw_ecsd[ECSD_BKOPS_EN] & ECSD_BKOPS_ENABLE ) ) {
				dev->flags	|= DEV_FLAG_BKOPS;
			}
		}

		// HPI

		// MDT handle 2012 roll over
		if( dev->cid.year < MDT_YEAR_2010 ) {
			dev->cid.year += ( MDT_YEAR_2013 - MDT_YEAR_1997 );
		}
	}

	if( ecsd->ext_csd_rev >= ECSD_REV_V4_5 ) {
		// HS200
		// context management
		// packed commands
		// exception events
		// cache
		// dynamic capacity management
		// large sector size
		// power off notification

		if( ( raw_ecsd[ECSD_USE_NATIVE_SECTOR] & ECSD_USE_NATIVE_SECTOR_EN ) ) {
			switch( ( raw_ecsd[ECSD_NATIVE_SECTOR_SIZE] & ECSD_NATIVE_SECTOR_MSK ) ) {
				case ECSD_NATIVE_SECTOR_512:
					ecsd->blksz = SDIO_BLKSZ_512; break;

				case ECSD_NATIVE_SECTOR_4K:
					ecsd->blksz = SDIO_BLKSZ_4K; break;

				default:
					sdio_slogf( _SLOGC_SDIODI, _SLOG_ERROR, hc->cfg.verbosity, 0, "%s: Invalid ECSD_NATIVE_SECTOR_SIZE 0x%x", __FUNCTION__, raw_ecsd[ECSD_NATIVE_SECTOR_SIZE] );
					return( EINVAL );
			}
		}

		if( raw_ecsd[ECSD_CACHE_SIZE + 0] || raw_ecsd[ECSD_CACHE_SIZE + 1] || raw_ecsd[ECSD_CACHE_SIZE + 2] || raw_ecsd[ECSD_CACHE_SIZE + 3] ) {
			dev->caps	|= DEV_CAP_CACHE;
		}

		ecsd->driver_strength = raw_ecsd[ECSD_DRIVER_STRENGTH];

		if( raw_ecsd[ECSD_SEC_FEATURE_SUPPORT] & ECSD_SEC_SANITIZE ) {
			dev->caps	|= DEV_CAP_SANITIZE;
		}
		dev->caps		|= DEV_CAP_DISCARD;		// DISCARD support
		dev->caps		|= DEV_CAP_PWROFF_NOTIFY;       // Power off notification support
	}

	if( ecsd->ext_csd_rev >= ECSD_REV_V5 ) {
		// HS400
	}

	ecsd->card_type = raw_ecsd[ECSD_CARD_TYPE] & ECSD_CARD_TYPE_MSK;

	if( ( hc->caps & HC_CAP_HS400 ) && ( ecsd->card_type & ECSD_CARD_TYPE_HS400 ) ) {
		if( ( ( hc->caps & HC_CAP_SV_1_8V ) && ( ecsd->card_type & ECSD_CARD_TYPE_HS400_1_8V ) ) ||
				( ( hc->caps & HC_CAP_SV_1_2V ) && ( ecsd->card_type & ECSD_CARD_TYPE_HS400_1_2V ) ) ) {
			dev->caps			|= DEV_CAP_HS400 | DEV_CAP_HS;
			ecsd->dtr_max_hs	= DTR_MAX_HS400;
		}
	}

	if( ( hc->caps & HC_CAP_HS200 ) && ( ecsd->card_type & ECSD_CARD_TYPE_HS200 ) ) {
		if( ( ( hc->caps & HC_CAP_SV_1_8V ) && ( ecsd->card_type & ECSD_CARD_TYPE_HS200_1_8V ) ) ||
				( ( hc->caps & HC_CAP_SV_1_2V ) && ( ecsd->card_type & ECSD_CARD_TYPE_HS200_1_2V ) ) ) {
			dev->caps			|= DEV_CAP_HS200 | DEV_CAP_HS;
			ecsd->dtr_max_hs	= DTR_MAX_HS200;
		}
	}

	if( !( dev->caps & DEV_CAP_HS ) && ( hc->caps & HC_CAP_HS ) && ( ecsd->card_type & ECSD_CARD_TYPE_52MHZ ) ) {
		dev->caps			|= DEV_CAP_HS;
		ecsd->dtr_max_hs	= DTR_MAX_HS52;
	}

	if( !( dev->caps & DEV_CAP_HS )	) {
		ecsd->dtr_max_hs = DTR_MAX_HS26;
	}

	if( ( hc->caps & HC_CAP_DDR50 ) && ( ecsd->card_type & ECSD_CARD_TYPE_DDR ) ) {
		if( ( ( hc->caps & ( HC_CAP_SV_3_0V | HC_CAP_SV_1_8V ) ) && ( ecsd->card_type & ECSD_CARD_TYPE_DDR_1_8V ) ) ||
				( ( hc->caps & HC_CAP_SV_1_2V ) && ( ecsd->card_type & ECSD_CARD_TYPE_DDR_1_2V ) ) ) {
			dev->caps |= DEV_CAP_DDR50;
		}
	}

	ecsd->erase_grp_def			= raw_ecsd[ECSD_ERASE_GRP_DEF];
	ecsd->hc_erase_group_size	= raw_ecsd[ECSD_ERASE_GRP_SIZE];
	ecsd->hc_wp_grp_size		= raw_ecsd[ECSD_HC_WP_GRP_SIZE];
	ecsd->user_wp				= raw_ecsd[ECSD_USER_WP];

	return( EOK );
}

uint64_t mmc_wp_grp_size( sdio_dev_t *dev )
{
	uint64_t	wp_grp_size;

	if( dev->ecsd.erase_grp_def ) {
		wp_grp_size =	(uint64_t )dev->ecsd.hc_wp_grp_size *
						(uint64_t )dev->ecsd.hc_erase_group_size *
						MMC_ERASE_GRP_512K;
	}
	else {
		wp_grp_size =	( dev->csd.erase_grp_size + 1 ) *
						( dev->csd.erase_grp_mult + 1 ) *
						( dev->csd.wp_grp_size + 1 ) * 512;
	}

	return( wp_grp_size );
}

uint64_t mmc_erase_grp_size( sdio_dev_t *dev )
{
	uint64_t	erase_grp_size;

	if( dev->ecsd.erase_grp_def ) {
		erase_grp_size = (uint64_t )dev->ecsd.hc_erase_group_size * MMC_ERASE_GRP_512K;
	}
	else {
		erase_grp_size = ( dev->csd.erase_grp_size + 1 ) * ( dev->csd.erase_grp_mult + 1 ) * 512;
	}

	return( erase_grp_size );
}

int mmc_switch( sdio_dev_t *dev, uint32_t cmdset, uint32_t mode, uint32_t index, uint32_t value, uint32_t timeout )
{
	sdio_cmd_t		*cmd;
	int				status;

	if( ( cmd = sdio_alloc_cmd( ) ) == NULL ) {
		return( ENOMEM );
	}

	sdio_setup_cmd( cmd, SCF_CTYPE_AC | SCF_RSP_R1B, MMC_SWITCH,
			( mode << 24 ) | ( index << 16 ) | ( value << 8 ) | cmdset );

	if( ( status = _sdio_send_cmd( dev, cmd, NULL, SDIO_TIME_DEFAULT, SDIO_CMD_RETRIES ) ) == EOK ) {
			// SanDisk errata.  The parts can't handle
			// commands within 100us after the switch.
//		nanospin_ns( 500000L );
		delay( 1 );
		if( ( status = _sdio_wait_card_status( dev, NULL, CDS_READY_FOR_DATA, CDS_READY_FOR_DATA, timeout ) ) != EOK ) {
		}
	}

	sdio_free_cmd( cmd );

	return( status );
}

int mmc_set_partition( sdio_dev_t *dev, uint32_t partition )
{
	int		status;

	if( dev->pactive == partition ) {
		return( EOK );
	}

	if( ( status = mmc_switch( dev, MMC_SWITCH_CMDSET_DFLT, MMC_SWITCH_MODE_WRITE, ECSD_PART_CONFIG, partition, SDIO_TIME_DEFAULT ) ) == EOK ) {
		dev->pactive = partition;
	}

	return( status );
}

int mmc_sleep_awake( sdio_dev_t *dev, int flgs )
{
	struct sdio_cmd		*cmd;
	uint32_t			arg;
	int					status;

	if( ( cmd = sdio_alloc_cmd( ) ) == NULL ) {
		return( ENOMEM );
	}

	arg		= dev->rca << 16;

	if( flgs ) {
		arg |= MMC_SA_SLEEP;
	}

	sdio_setup_cmd( cmd, SCF_CTYPE_AC | SCF_RSP_R1B, MMC_SLEEP_AWAKE, arg );

	if( ( status = _sdio_send_cmd( dev, cmd, NULL, SDIO_TIME_DEFAULT, 0 ) ) == EOK ) {
		if( !( dev->hc->caps & HC_CAP_BSY ) ) {
				// convert 100ns units to ms
			delay( ( dev->ecsd.s_a_timeout + 10000 - 1 ) / 10000 );
		}
	}

	sdio_free_cmd( cmd );

	return( status );
}

int mmc_cache( sdio_dev_t *dev, int op, uint32_t timeout )
{
	int	status;

	switch( op ) {
		case SDIO_CACHE_DISABLE:
			if( ( status = mmc_switch( dev, MMC_SWITCH_CMDSET_DFLT, MMC_SWITCH_MODE_WRITE, ECSD_CACHE_CTRL, ECSD_CACHE_CTRL_DIS, timeout ) ) == EOK ) {
				dev->flags &= ~DEV_FLAG_WCE;
			}
			break;

		case SDIO_CACHE_ENABLE:
			if( ( status = mmc_switch( dev, MMC_SWITCH_CMDSET_DFLT, MMC_SWITCH_MODE_WRITE, ECSD_CACHE_CTRL, ECSD_CACHE_CTRL_EN, timeout ) ) == EOK ) {
				dev->flags |= DEV_FLAG_WCE;
			}
			break;

		case SDIO_CACHE_FLUSH:
			if( ( dev->flags & DEV_FLAG_WCE ) ) {
				status = mmc_switch( dev, MMC_SWITCH_CMDSET_DFLT, MMC_SWITCH_MODE_WRITE, ECSD_FLUSH_CACHE, ECSD_FLUSH_TRIGGER, timeout );
			}
			else {
				status = EOK;
			}
			break;

		default:
			status = EINVAL; break;
	}

	return( status );
}

uint64_t mmc_erase_timeout( sdio_dev_t *dev, uint32_t etype, uint64_t nlba )
{
	uint8_t			*ecsd;
	uint32_t		tmult;
	uint64_t		timeout;
	uint64_t		erase_grp_size;

	ecsd			= dev->raw_ecsd;
	tmult			= MMC_ERASE_TIMEOUT;
	erase_grp_size	= mmc_erase_grp_size( dev ) / 512;

	if( dev->ecsd.erase_grp_def ) {
		switch( etype ) {
			case MMC_ERASE_NORM:
				tmult = MMC_ERASE_TIMEOUT * ecsd[ECSD_ERASE_MULT];
				break;

			case MMC_ERASE_TRIM:
			case MMC_ERASE_DISCARD:
				tmult = MMC_ERASE_TIMEOUT * ecsd[ECSD_TRIM_MULT];
				break;

			case MMC_ERASE_SECURE:
				tmult = MMC_ERASE_TIMEOUT * ecsd[ECSD_ERASE_MULT] * ecsd[ECSD_SEC_ERASE_MULT];
				break;

			case MMC_ERASE_SECURE_TRIM:
			case MMC_ERASE_SECURE_TRIM_PURGE:
				tmult = MMC_ERASE_TIMEOUT * ecsd[ECSD_ERASE_MULT] * ecsd[ECSD_SEC_TRIM_MULT];
				break;

			default:
				tmult = MMC_ERASE_TIMEOUT * ecsd[ECSD_ERASE_MULT];
				break;
		}
	}

		// +2 for potential TRIM/DISCARD aligment
	timeout = tmult * ( ( nlba / erase_grp_size ) + 2 );

	timeout = max( timeout, MMC_ERASE_TIMEOUT_MIN );

	return( timeout );
}

int mmc_erase( sdio_dev_t *dev, int partition, int flgs, uint64_t lba, int nlba )
{
	sdio_cmd_t		*cmd;
	int				status;
	uint64_t		timeout;
	uint64_t		addr;

	if( ( status = mmc_set_partition( dev, partition ) ) != EOK ) {
		return( status );
	}

	if( ( cmd = sdio_alloc_cmd( ) ) == NULL ) {
		return( ENOMEM );
	}

	while( nlba ) {
		addr = ( dev->caps & DEV_CAP_HC ) ? lba : ( lba * dev->csd.blksz );
		sdio_setup_cmd( cmd, SCF_CTYPE_AC | SCF_RSP_R1, MMC_TAG_ERASE_GROUP_START, addr );
		if( ( status = _sdio_send_cmd( dev, cmd, NULL, SDIO_TIME_DEFAULT, 0 ) ) != EOK ) {
			break;
		}

		timeout = mmc_erase_timeout( dev, flgs, nlba );
		lba		+= nlba - 1;
		nlba	-= nlba;

		addr = ( dev->caps & DEV_CAP_HC ) ? lba : ( lba * dev->csd.blksz );
		sdio_setup_cmd( cmd, SCF_CTYPE_AC | SCF_RSP_R1, MMC_TAG_ERASE_GROUP_END, addr );
		if( ( status = _sdio_send_cmd( dev, cmd, NULL, SDIO_TIME_DEFAULT, 0 ) ) != EOK ) {
			break;
		}

		sdio_setup_cmd( cmd, SCF_CTYPE_AC | SCF_RSP_R1B | SCF_WAIT_DRDY, MMC_ERASE, flgs );
		if( ( status = _sdio_send_cmd( dev, cmd, NULL, timeout, 0 ) ) != EOK ) {
			break;
		}

		if( ( status = _sdio_wait_card_status( dev, NULL, CDS_READY_FOR_DATA | CDS_CUR_STATE_MSK, CDS_READY_FOR_DATA | CDS_CUR_STATE_TRAN, timeout ) ) != EOK ) {
			break;
		}
	}

	sdio_free_cmd( cmd );

	return( status );
}

int mmc_send_ext_csd( sdio_dev_t *dev, uint8_t *csd )
{
	sdio_cmd_t		*cmd;
	uint8_t			*ecsd;
	int				status;
	sdio_sge_t		sge;

	if( ( cmd = sdio_alloc_cmd( ) ) == NULL ) {
		return( ENOMEM );
	}

	if( ( ecsd = sdio_alloc( MMC_EXT_CSD_SIZE ) ) == NULL ) {
		sdio_free_cmd( cmd );
		return( ENOMEM );
	}

	sdio_setup_cmd( cmd, SCF_CTYPE_ADTC | SCF_RSP_R1, MMC_SEND_EXT_CSD, 0 );
	sge.sg_count = MMC_EXT_CSD_SIZE; sge.sg_address = SDIO_DATA_PTR_P( ecsd );
	sdio_setup_cmd_io( cmd, SCF_DIR_IN, 1, MMC_EXT_CSD_SIZE, &sge, 1, NULL );

	if( ( status = _sdio_send_cmd( dev, cmd, NULL, SDIO_TIME_DEFAULT, 0 ) ) == EOK ) {
		memcpy( csd, ecsd, MMC_EXT_CSD_SIZE );
	}

	sdio_free( ecsd, MMC_EXT_CSD_SIZE );
	sdio_free_cmd( cmd );

	return( status );
}

int mmc_set_relative_addr( sdio_dev_t *dev, int addr )
{
	sdio_cmd_t		*cmd;
	int				status;

	if( ( cmd = sdio_alloc_cmd( ) ) == NULL ) {
		return( ENOMEM );
	}

	sdio_setup_cmd( cmd, SCF_CTYPE_AC | SCF_RSP_R1, MMC_SET_RELATIVE_ADDR, addr << 16 );
	if( ( status = _sdio_send_cmd( dev, cmd, NULL, SDIO_TIME_DEFAULT, SDIO_CMD_RETRIES ) ) == EOK ) {
	}

	sdio_free_cmd( cmd );

	return( status );
}

int mmc_send_op_cond( sdio_hc_t *hc, uint32_t ocr, uint32_t *rocr )
{
	sdio_dev_t		*dev;
	sdio_cmd_t		*cmd;
	int				status;
	int				retry;

	status = EOK;

	dev	= &hc->device;
	if( ( cmd = sdio_alloc_cmd( ) ) == NULL ) {
		return( ENOMEM );
	}

	sdio_setup_cmd( cmd, SCF_CTYPE_BCR | SCF_RSP_R3, MMC_SEND_OP_COND, ocr );
	for( retry = 100; retry; retry-- ) {
		if( ( status = _sdio_send_cmd( dev, cmd, NULL, SDIO_TIME_DEFAULT, 0 ) ) != EOK ) {
			break;
		}

		if( !ocr ) {
			break;
		}

		if( cmd->rsp[0] & OCR_PWRUP_CMP ) {
			break;
		}

		delay( 10 );
	}

	if( rocr ) {
		*rocr = cmd->rsp[0];
	}

	sdio_free_cmd( cmd );

	if( !retry ) status = ETIMEDOUT;

	return( status );
}

int mmc_bus_test( sdio_dev_t *dev, int width )
{
	sdio_cmd_t		*cmd;
	sdio_sge_t		sge;
	int				idx;
	uint8_t			*data;
	int				status;
	uint8_t			*pattern;
	static uint8_t	pattern8[8] = { 0x55, 0xaa, 0, 0, 0, 0, 0, 0 };
	static uint8_t	pattern4[4] = { 0x5a, 0, 0, 0 };

	if( ( cmd = sdio_alloc_cmd( ) ) == NULL ) {
		return( ENOMEM );
	}

	if( ( data = sdio_alloc( width ) ) == NULL ) {
		sdio_free_cmd( cmd );
		return( ENOMEM );
	}

	pattern			= width == 8 ? pattern8 : pattern4;
	sge.sg_count	= width;
	sge.sg_address	= SDIO_DATA_PTR_P( data );
	memcpy( data, pattern, width );
	sdio_setup_cmd( cmd, SCF_CTYPE_ADTC | SCF_RSP_R1, MMC_BUSTEST_W, 0 );
	sdio_setup_cmd_io( cmd, SCF_DIR_OUT, 1, width, &sge, 1, NULL );
	_sdio_send_cmd( dev, cmd, NULL, SDIO_TIME_DEFAULT, 0 );

	sdio_setup_cmd( cmd, SCF_CTYPE_ADTC | SCF_RSP_R1, MMC_BUSTEST_R, 0 );
	sdio_setup_cmd_io( cmd, SCF_DIR_IN, 1, width, &sge, 1, NULL );
	if( ( status = _sdio_send_cmd( dev, cmd, NULL, SDIO_TIME_DEFAULT, 0 ) ) == EOK ) {
		for( idx = 0; idx < width / 4; idx++ ) {
			if( ( pattern[idx] ^ data[idx] ) != 0xff ) {
				status = EIO;
				break;
			}
		}
	}

	sdio_free( data, width );
	sdio_free_cmd( cmd );
	return( status );
}

int mmc_bus_width( sdio_dev_t *dev, int flg )
{
	sdio_hc_t	*hc;
	int			width;
	int			status;

	hc	= dev->hc;

	if( ( hc->caps & HC_CAP_BW8 ) ) {
		width		= BUS_WIDTH_8;
	}
	else if( hc->caps & HC_CAP_BW4 ) {
		width		= BUS_WIDTH_4;
	}
	else {
		return( BUS_WIDTH_1 );
	}

	if( !flg ) {
		return( width );
	}

		// NOTE:  the BUSTEST_W seems to take about 3 seconds
	for( ; width >= 4; width >>= 1 ) {
		if( ( status = mmc_switch( dev, MMC_SWITCH_CMDSET_DFLT, MMC_SWITCH_MODE_WRITE, ECSD_BUS_WIDTH, width / 4, SDIO_TIME_DEFAULT ) ) ) {
			continue;
		}

		sdio_bus_width( hc, width );

		if( ( status = mmc_bus_test( dev, width ) ) == EOK ) {
			return( width );
		}
	}

	return( BUS_WIDTH_1 );
}

int mmc_init_hs( sdio_hc_t *hc )
{
	sdio_dev_t		*dev;
	sdio_ecsd_t		*ecsd;
	int			status;

	dev	= &hc->device;
	ecsd	= &dev->ecsd;

	if( ecsd->dtr_max_hs > DTR_MAX_HS52 ) {
		ecsd->dtr_max_hs = DTR_MAX_HS52;		// max dtr for HS is 52MHz
	}

	if( ( status = mmc_switch( dev, MMC_SWITCH_CMDSET_DFLT, MMC_SWITCH_MODE_WRITE, ECSD_HS_TIMING, ECSD_HS_TIMING_HS, SDIO_TIME_DEFAULT ) ) ) {
		sdio_slogf( _SLOGC_SDIODI, _SLOG_ERROR, hc->cfg.verbosity, 0, "%s: switch ECSD_HS_TIMING HS", __FUNCTION__ );
		return( status );
	}

	if( ( status = sdio_timing( hc, TIMING_HS ) ) ) {
		sdio_slogf( _SLOGC_SDIODI, _SLOG_ERROR, hc->cfg.verbosity, 0, "%s: sdio_timing (HS)", __FUNCTION__ );
		return( status );
	}

	sdio_clock( hc, ecsd->dtr_max_hs );

	dev->flags |= DEV_FLAG_HS;

	return( status );
}

int _mmc_init_hs200( sdio_hc_t *hc, int bus_width )
{
	sdio_dev_t		*dev;
	int				status;
	int				bus_mode;

	dev	= &hc->device;
	status	= EINVAL;

		// set bus width
	bus_mode	= bus_width / 4;
	if( ( status = mmc_switch( dev, MMC_SWITCH_CMDSET_DFLT, MMC_SWITCH_MODE_WRITE, ECSD_BUS_WIDTH, bus_mode, SDIO_TIME_DEFAULT ) ) ) {
		sdio_slogf( _SLOGC_SDIODI, _SLOG_ERROR, hc->cfg.verbosity, 0, "%s: switch ext_csd_bus_width (%dbit)", __FUNCTION__, bus_width );
		return( status );
	}

	sdio_bus_width( hc, bus_width );

		// set HS200
	if( ( status = mmc_switch( dev, MMC_SWITCH_CMDSET_DFLT, MMC_SWITCH_MODE_WRITE, ECSD_HS_TIMING, ECSD_HS_TIMING_HS200, SDIO_TIME_DEFAULT ) ) ) {
		sdio_slogf( _SLOGC_SDIODI, _SLOG_ERROR, hc->cfg.verbosity, 0, "%s: switch ECSD_HS_TIMING HS200", __FUNCTION__ );
		return( status );
	}

	if( ( status = sdio_timing( hc, TIMING_HS200 ) ) ) {
		sdio_slogf( _SLOGC_SDIODI, _SLOG_ERROR, hc->cfg.verbosity, 0, "%s: sdio_timing (HS200)", __FUNCTION__ );
		return( status );
	}

	sdio_clock( hc, dev->ecsd.dtr_max_hs );

	if( ( status = hc->entry.tune( hc, MMC_SEND_TUNING_BLOCK ) ) ) {
		sdio_slogf( _SLOGC_SDIODI, _SLOG_ERROR, hc->cfg.verbosity, 0, "%s: tune failure", __FUNCTION__ );
		return( status );
	}

	return( status );
}

int mmc_init_hs200( sdio_hc_t *hc, int bus_width )
{
	sdio_dev_t		*dev;
	int				status;

	dev	= &hc->device;
	status	= EINVAL;

	if( ( dev->ecsd.card_type & ECSD_CARD_TYPE_HS200_1_2V ) && ( hc->caps & HC_CAP_SV_1_2V ) ) {
		if( ( status = sdio_signal_voltage( hc, SIGNAL_VOLTAGE_1_2 ) ) != EOK ) {
			sdio_slogf( _SLOGC_SDIODI, _SLOG_ERROR, hc->cfg.verbosity, 0, "%s: sdio_signal_voltage 1.2V failure", __FUNCTION__ );
		}
	}

	if( status && ( dev->ecsd.card_type & ECSD_CARD_TYPE_HS200_1_8V ) && ( hc->caps & HC_CAP_SV_1_8V ) ) {
		if( ( status = sdio_signal_voltage( hc, SIGNAL_VOLTAGE_1_8 ) ) != EOK ) {
			sdio_slogf( _SLOGC_SDIODI, _SLOG_ERROR, hc->cfg.verbosity, 0, "%s: sdio_signal_voltage 1.8V failure", __FUNCTION__ );
		}
	}

	if( status ) {
		return( status );
	}

	if( ( status = _mmc_init_hs200( hc, bus_width ) ) == EOK ) {
		dev->flags |= DEV_FLAG_HS200;
	}

	return( status );
}

int mmc_init_hs400( sdio_hc_t *hc, int bus_width )
{
	sdio_dev_t		*dev;
	int			status;

	dev	= &hc->device;
	status	= EINVAL;

	if( ( dev->ecsd.card_type & ECSD_CARD_TYPE_HS400_1_2V ) && ( hc->caps & HC_CAP_SV_1_2V ) ) {
		if( ( status = sdio_signal_voltage( hc, SIGNAL_VOLTAGE_1_2 ) ) != EOK ) {
			sdio_slogf( _SLOGC_SDIODI, _SLOG_ERROR, hc->cfg.verbosity, 0, "%s: sdio_signal_voltage 1.2V failure", __FUNCTION__ );
		}
	}

	if( status && ( dev->ecsd.card_type & ECSD_CARD_TYPE_HS400_1_8V ) && ( hc->caps & HC_CAP_SV_1_8V ) ) {
		if( ( status = sdio_signal_voltage( hc, SIGNAL_VOLTAGE_1_8 ) ) != EOK ) {
			sdio_slogf( _SLOGC_SDIODI, _SLOG_ERROR, hc->cfg.verbosity, 0, "%s: sdio_signal_voltage 1.8V failure", __FUNCTION__ );
		}
	}

	if( status ) {
		return( status );
	}

	if( ( status = _mmc_init_hs200( hc, bus_width ) ) ) {
		return( status );

	}

	sdio_timing( hc, TIMING_LS );
	sdio_clock( hc, dev->csd.dtr_max );

	if( ( status = _sdio_wait_card_status( dev, NULL, CDS_READY_FOR_DATA, CDS_READY_FOR_DATA, SDIO_TIME_DEFAULT ) ) != EOK ) {
		sdio_slogf( _SLOGC_SDIODI, _SLOG_ERROR, hc->cfg.verbosity, 0, "%s: card not ready", __FUNCTION__ );
	}

		// set HS
	if( ( status = mmc_switch( dev, MMC_SWITCH_CMDSET_DFLT, MMC_SWITCH_MODE_WRITE, ECSD_HS_TIMING, ECSD_HS_TIMING_HS, SDIO_TIME_DEFAULT ) ) ) {
		sdio_slogf( _SLOGC_SDIODI, _SLOG_ERROR, hc->cfg.verbosity, 0, "%s: switch ECSD_HS_TIMING (HS)", __FUNCTION__ );
		return( status );
	}

		// set buswidth to 8bit DDR
	if( ( status = mmc_switch( dev, MMC_SWITCH_CMDSET_DFLT, MMC_SWITCH_MODE_WRITE, ECSD_BUS_WIDTH, ECSD_BUS_WIDTH_8 | ECSD_BUS_WIDTH_DDR, SDIO_TIME_DEFAULT ) ) ) {
		sdio_slogf( _SLOGC_SDIODI, _SLOG_ERROR, hc->cfg.verbosity, 0, "%s: switch ECDD_BUS_WIDTH (8bit ddr)", __FUNCTION__ );
		return( status );
	}

		// set HS400
	if( ( status = mmc_switch( dev, MMC_SWITCH_CMDSET_DFLT, MMC_SWITCH_MODE_WRITE, ECSD_HS_TIMING, ECSD_HS_TIMING_HS400, SDIO_TIME_DEFAULT ) ) ) {
		sdio_slogf( _SLOGC_SDIODI, _SLOG_ERROR, hc->cfg.verbosity, 0, "%s: switch ECSD_HS_TIMING (HS400)", __FUNCTION__ );
		return( status );
	}

	sdio_clock( hc, dev->ecsd.dtr_max_hs );

	if( ( status = sdio_timing( hc, TIMING_HS400 ) ) ) {
		sdio_slogf( _SLOGC_SDIODI, _SLOG_ERROR, hc->cfg.verbosity, 0, "%s: sdio_timing (HS400)", __FUNCTION__ );
		return( status );
	}

	dev->flags |= DEV_FLAG_HS400;

	return( status );
}

int mmc_init_ddr( sdio_hc_t *hc, int bus_width )
{
	sdio_dev_t		*dev;
	int				sv;
	int				status;
	int				bus_mode;

	dev		= &hc->device;

	sv		= ( ( dev->ecsd.card_type & ECSD_CARD_TYPE_DDR_1_2V ) && ( hc->caps & HC_CAP_SV_1_2V ) ) ?
				SIGNAL_VOLTAGE_1_2 : SIGNAL_VOLTAGE_1_8;

		// we only need to change voltage for 1_2V since 1_8V supports 1.8 to 3.3V
	if( sv == SIGNAL_VOLTAGE_1_2 && ( status = sdio_signal_voltage( hc, sv ) ) != EOK ) {
		sdio_slogf( _SLOGC_SDIODI, _SLOG_ERROR, hc->cfg.verbosity, 0, "%s: sdio_signal_voltage %d failure", __FUNCTION__, sv );
		return( status );
	}

		// set bus width/DDR
	bus_mode = ( bus_width / 4 ) | ECSD_BUS_WIDTH_DDR;
	if( ( status = mmc_switch( dev, MMC_SWITCH_CMDSET_DFLT, MMC_SWITCH_MODE_WRITE, ECSD_BUS_WIDTH, bus_mode, SDIO_TIME_DEFAULT ) ) ) {
		sdio_slogf( _SLOGC_SDIODI, _SLOG_ERROR, hc->cfg.verbosity, 0, "%s: switch ECSD_BUS_WIDTH %x", __FUNCTION__, bus_mode );
		return( status );
	}

	sdio_bus_width( hc, bus_width );
	sdio_timing( hc, TIMING_DDR50 );

	dev->flags |= DEV_FLAG_DDR;

	return( status );
}

int mmc_init_bus( sdio_hc_t *hc )
{
	sdio_dev_t		*dev;
	int				status;
	int				bus_width;

	dev			= &hc->device;

	bus_width	= mmc_bus_width( dev, 0 );

		// enable HS400 if supported
	if( ( dev->caps & DEV_CAP_HS400 ) && ( dev->ecsd.dtr_max_hs > DTR_MAX_HS52 ) && bus_width >= BUS_WIDTH_8 ) {
		if( ( status = mmc_init_hs400( hc, bus_width ) ) ) {
		}
		
	}

		// enable HS200 if supported
	if( !( dev->flags & DEV_FLAG_HS400 ) && ( dev->caps & DEV_CAP_HS200 ) && ( dev->ecsd.dtr_max_hs > DTR_MAX_HS52 ) && bus_width >= BUS_WIDTH_4 ) {
		if( ( status = mmc_init_hs200( hc, bus_width ) ) ) {
		}
	}

		// enable HS if supported
	if( !( dev->flags & ( DEV_FLAG_HS200 | DEV_FLAG_HS400 ) ) && ( dev->caps & DEV_CAP_HS ) ) {
		if( ( status = mmc_init_hs( hc ) ) ) {
		}
	}

	if( ( dev->flags & DEV_FLAG_HS ) ) {
		if( bus_width >= BUS_WIDTH_4 ) {
			if( ( dev->caps & DEV_CAP_DDR50 ) ) {		// enable DDR50 if supported
				if( ( status = mmc_init_ddr( hc, bus_width ) ) ) {
				}
			}

			if( !( dev->flags & DEV_FLAG_DDR ) ) {		// enable 4/8 bit mode
				if( ( status = mmc_switch( dev, MMC_SWITCH_CMDSET_DFLT, MMC_SWITCH_MODE_WRITE, ECSD_BUS_WIDTH, bus_width / 4, SDIO_TIME_DEFAULT ) ) ) {
					sdio_slogf( _SLOGC_SDIODI, _SLOG_ERROR, hc->cfg.verbosity, 0, "%s: switch ECSD_BUS_WIDTH %x", __FUNCTION__, bus_width / 4 );
				}
				else {
					sdio_bus_width( hc, bus_width );
				}
			}
		}
	}

	if( !( dev->flags & ( DEV_FLAG_HS400 | DEV_FLAG_HS200 | DEV_FLAG_HS ) ) ) {
			// we either have a device where dev->csd.spec_vers < CSD_SPEC_VER_4
			// or all higher modes have failed, so set set max ls timing
		sdio_timing( hc, TIMING_LS );
		sdio_clock( hc, dev->csd.dtr_max );
	}

	if( ( status = _sdio_set_block_length( dev, SDIO_DFLT_BLKSZ ) ) != EOK ) {
		sdio_slogf( _SLOGC_SDIODI, _SLOG_ERROR, hc->cfg.verbosity, 0, "%s: sdio_set_block_length", __FUNCTION__ );
		return( status );
	}

    return( status );
}

int mmc_init_device( sdio_hc_t *hc, uint32_t ocr, int flgs )
{
	sdio_dev_t				*dev;
	uint32_t				rocr;
	uint32_t				cid[4];
	int						status;
	sdio_device_errata_t	*errata;

	dev		= &hc->device;

	if( !( hc->flags & HC_FLAG_SKIP_PWRUP ) ) {
		if( ( status = sdio_go_idle( hc ) ) ) {
			return( status );
		}
	}

	if( ( status = mmc_send_op_cond( hc, ocr | OCR_HCS, &rocr ) ) ) {
		return( status );
	}

	if( ( status = sdio_all_send_cid( hc, cid ) ) ) {
		return( status );
	}

	if( flgs && memcmp( dev->raw_cid, cid, sizeof( cid ) ) ) {
		return( ENXIO );
	}

	memcpy( dev->raw_cid, cid, sizeof( cid ) );

	dev->rca		= 1;
	if( ( status = mmc_set_relative_addr( dev, dev->rca ) ) ) {
		return( status );
	}

	sdio_bus_mode( hc, BUS_MODE_PUSH_PULL );

	if( !flgs ) {
		if( ( status = sdio_send_csd( dev, dev->raw_csd ) ) ) {
			return( status );
		}

		if( ( status = mmc_parse_csd( dev, &dev->csd, dev->raw_csd ) ) ) {
			return( status );
		}

		if( ( status = mmc_parse_cid( dev, &dev->cid, dev->raw_cid ) ) ) {
			return( status );
		}
	}

	if( ( status = sdio_select_card( dev, dev->rca ) ) ) {
		return( status );
	}

	if( ( dev->flags & DEV_FLAG_LOCKED ) ) {
		return( EOK );
	}

	if( dev->csd.spec_vers >= CSD_SPEC_VER_4 ) {
		if( !flgs ) {
			if( ( status = mmc_send_ext_csd( dev, dev->raw_ecsd ) ) ) {
				return( status );
			}

			if( ( status = mmc_parse_ext_csd( dev, &dev->ecsd, dev->raw_ecsd ) ) ) {
				return( status );
			}
		}

		if( dev->raw_ecsd[ECSD_ERASE_GRP_SIZE] && dev->raw_ecsd[ECSD_ERASE_MULT] ) {
			if( ( status = mmc_switch( dev, MMC_SWITCH_CMDSET_DFLT, MMC_SWITCH_MODE_WRITE, ECSD_ERASE_GRP_DEF, ECSD_EGD_EN, SDIO_TIME_DEFAULT ) ) != EOK ) {
				sdio_slogf( _SLOGC_SDIODI, _SLOG_ERROR, hc->cfg.verbosity, 0, "%s: switch ext_csd_erase_grp_def", __FUNCTION__ );
				return( status );
			}
			dev->ecsd.erase_grp_def = 1;
		}

		if( ( rocr & OCR_HCS ) ) {
			dev->caps	|= DEV_CAP_HC;
		}
	}

	if( !flgs && ( errata = sdio_device_errata( dev, mmc_errata ) ) ) {
		sdio_reconcile_errata( dev, errata );
		if( ( errata->errata & DEV_ERRATA_DISCARD_SUP ) ) {
			dev->caps		|= DEV_CAP_DISCARD;		// DISCARD support
		}
	}

	dev->wp_size	= mmc_wp_grp_size( dev );
	dev->erase_size	= mmc_erase_grp_size( dev );

	if( ( status = mmc_init_bus( hc ) ) ) {	// set bus width/timing
		return( status );
	}

	return( status );
}

int mmc_bus_error( sdio_dev_t *dev )
{
	sdio_ecsd_t		*ecsd;

	ecsd	= &dev->ecsd;

	if( ( dev->flags & DEV_FLAG_HS400 ) ) {
		dev->caps		&= ~DEV_CAP_HS400;
		dev->flags		&= ~DEV_FLAG_HS400;
	}
	else if( ( dev->flags & DEV_FLAG_HS200 ) ) {
		dev->caps		&= ~DEV_CAP_HS200;
		dev->flags		&= ~DEV_FLAG_HS200;
		ecsd->dtr_max_hs	= DTR_MAX_HS52;
	}
	else if( ( dev->flags & DEV_FLAG_DDR ) ) {
		dev->caps		&= ~DEV_CAP_DDR50;
		dev->flags		&= ~DEV_FLAG_DDR;
	}
	else if( ( dev->flags & DEV_FLAG_HS ) ) {
		dev->flags		&= ~DEV_FLAG_HS;
		if( ecsd->dtr_max_hs == DTR_MAX_HS26 ) {
//			dev->caps	&= ~DEV_CAP_HS;
		}
		else {
			ecsd->dtr_max_hs = max( DTR_MAX_HS26, ecsd->dtr_max_hs - DTR_HS_DEC );
		}
	}
	else {
		dev->csd.dtr_max = max( DTR_MIN_LS, dev->csd.dtr_max - DTR_LS_DEC );
	}

	return( mmc_init_bus( dev->hc ) );
}

int mmc_ident( sdio_hc_t *hc )
{
	sdio_dev_t		*dev;
	uint32_t		ocr;
	int				status;

	dev			= &hc->device;

	if( hc->flags & HC_FLAG_SKIP_PWRUP ) {
		if( ( mmc_init_device( hc, dev->ocr, SDIO_FALSE ) ) == EOK ) {
			return( EOK );
		}
	}

	if( ( status = sdio_go_idle( hc ) ) ) {
		return( status );
	}

	if( ( status = mmc_send_op_cond( hc, 0, &ocr ) ) ) {
		return( status );
	}

	if( ( status = sdio_select_voltage( hc, ocr ) ) != EOK ) {
		return( status );
	}

	if( ( status = mmc_init_device( hc, dev->ocr, SDIO_FALSE ) ) ) {
		return( status );
	}

	return( EOK );
}

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/hardware/devb/sdmmc/sdiodi/mmc.c $ $Rev: 814800 $")
#endif
