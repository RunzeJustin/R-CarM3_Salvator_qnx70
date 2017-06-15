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

/*
 *  dcmd_sim_sdmmc.h   Non-portable low-level devctl definitions
 *
*/

#ifndef __DCMD_SIM_SDMMC_H_INCLUDED
#define __DCMD_SIM_SDMMC_H_INCLUDED

#ifndef _DEVCTL_H_INCLUDED
 #include <devctl.h>
#endif

#include <_pack64.h>

typedef struct _sdmmc_write_protect {
#define SDMMC_WP_ACTION_CLR			0x00
#define SDMMC_WP_ACTION_SET			0x01
#define SDMMC_WP_ACTION_PROT		0x02
 /* 32 write protection bits (representing 32 write protect groups) */
#define SDMMC_WP_ACTION_PROT_TYPE	0x03
 /* 64 write protection bits (representing 32 write protect groups) where:
     0x0 WPG not protected 
     0x1 WPG temporary WP
     0x2 WPG power-on WP
     0x3 WPG permanent WP */

	_Uint32t		action;

#define SDMMC_WP_MODE_PWR_WP_EN		0x01  /* Apply Power-On Period Protection */
	_Uint32t		mode;
	_Uint64t		lba;
	_Uint64t		nlba;
	_Uint64t		prot;
	_Uint32t		rsvd[4];
} SDMMC_WRITE_PROTECT;

#define SDMMC_ERASE_ACTION_NORMAL			0x00
#define SDMMC_ERASE_ACTION_SECURE			0x01
#define SDMMC_ERASE_ACTION_TRIM				0x02
#define SDMMC_ERASE_ACTION_SECURE_TRIM		0x03
#define SDMMC_ERASE_ACTION_SECURE_PURGE		0x04
#define SDMMC_ERASE_ACTION_DISCARD			0x05
#define SDMMC_ERASE_ACTION_SANITIZE			0x06
typedef struct _sdmmc_erase {
	_Uint32t		action;
	_Uint32t		rsvd;
	_Uint64t		lba;
	_Uint64t		nlba;
	_Uint64t		rsvd2;
} SDMMC_ERASE;

typedef struct _sdmmc_card_register {
#define SDMMC_CR_ACTION_READ	0x00
#define SDMMC_CR_ACTION_WRITE	0x01
	_Uint32t		action;

#define SDMMC_REG_TYPE_CID		0x00
#define SDMMC_REG_TYPE_CSD		0x01
#define SDMMC_REG_TYPE_EXT_CSD	0x02
#define SDMMC_REG_TYPE_SCR		0x03
#define SDMMC_CARD_TYPE_UNKNOWN	0x0
#define SDMMC_CARD_TYPE_MMC		0x1
#define SDMMC_CARD_TYPE_SD		0x2
	_Uint32t		type;      /* REG_TYPE_XXX on entry, CARD_TYPE_XXX on return */
	_Uint32t		address;
	_Uint32t		length;
	_Uint32t		rsvd[2];
/*	_Uint8t			data[ length ];	variable length data */
} SDMMC_CARD_REGISTER;

typedef struct _sandisk_health {
	_Uint32t		mid;				/* Manufacture ID */
	_Uint8t			lifetime;
	_Uint8t			nv_avg_pe;			/* NV Cache avg P/E cycle */
	_Uint8t			eua_avg_pe;			/* Enhanced User Area avg P/E cycle */
	_Uint8t			mlc_avg_pe;			/* MLC avg P/E cycle */
} SANDISK_HEALTH;

typedef struct _samsung_health {
	_Uint32t		mid;				/* Manufacture ID */
	_Uint32t		bank0_rsvd_blocks;
	_Uint32t		bank1_rsvd_blocks;
	_Uint32t		bank2_rsvd_blocks;
	_Uint32t		bank3_rsvd_blocks;
	_Uint32t		init_bad_blocks;
	_Uint32t		runtime_bad_blocks;
	_Uint32t		slc_max_ec;			/* SLC Maximum Erase Count */
	_Uint32t		slc_min_ec;			/* SLC Minimum Erase Count */
	_Uint32t		slc_avg_ec;			/* SLC Average Erase Count */
	_Uint32t		mlc_max_ec;			/* MLC Maximum Erase Count */
	_Uint32t		mlc_min_ec;			/* MLC Minimum Erase Count */
	_Uint32t		mlc_avg_ec;			/* MLC Average Erase Count */
	_Uint32t		max_ec;				/* Overall Maximum Erase Count */
	_Uint32t		min_ec;				/* Overall Minimum Erase Count */
	_Uint32t		avg_ec;				/* Overall Average Erase Count */
	_Uint32t		read_reclaim;
	_Uint32t		num_banks;
	_Uint32t		bank4_rsvd_blocks;
	_Uint32t		bank5_rsvd_blocks;
	_Uint32t		bank6_rsvd_blocks;
	_Uint32t		bank7_rsvd_blocks;
} SAMSUNG_HEALTH;

typedef struct _toshiba_health {		/* supported from v4.41 onwards */
	_Uint32t		mid;				/* Manufacture ID */
	_Uint32t		lifetime_total;
	_Uint32t		lifetime_rsvd_blk;
	_Uint32t		lifetime_avg_pe;
	_Uint32t		mlc_max_pe;			/* MLC Maximum P/E cycle range */
	_Uint32t		mlc_avg_pe;			/* MLC Average P/E cycle range */
	_Uint32t		slc_max_pe;			/* SLC Maximum P/E cycle range */
	_Uint32t		slc_avg_pe;			/* SLC Average P/E cycle range */
} TOSHIBA_HEALTH;

typedef union _sdmmc_device_health {
	_Uint32t		mid;				/* Manufacture ID */
	SANDISK_HEALTH	sandisk;
	SAMSUNG_HEALTH	samsung;
	TOSHIBA_HEALTH	toshiba;
	_Uint8t			bytes[512];
} SDMMC_DEVICE_HEALTH;

typedef struct _sdmmc_device_info {
#define DEV_TYPE_MMC			1
#define DEV_TYPE_SD				2
	_Uint32t			dtype;

#define DEV_FLAG_CARD_LOCKED	0x02
#define DEV_FLAG_WP				0x01
	_Uint32t			flags;

#define MID_MMC_SANDISK			0x02
#define MID_MMC_SANDISK_2		0x45
#define MID_MMC_TOSHIBA			0x11
#define MID_MMC_MICRON			0x13
#define MID_MMC_SAMSUNG			0x15
#define MID_MMC_HYNIX			0x90
#define MID_MMC_NUMONYX			0xFE
	_Uint32t			mid;			/* Manufacture ID */
	_Uint32t			oid;			/* OEM ID */
	_Uint8t				pnm[8];			/* Product name */
	_Uint32t			prv;			/* Product revision */
	_Uint32t			psn;			/* Product serial number */
	_Uint32t			month;			/* Month */
	_Uint32t			year;			/* Year */
	_Uint8t				vu[8];			/* Vendor Unique ie SanDisk fw revision */

	_Uint32t			rca;
	_Uint32t			spec_vers;
	_Uint32t			spec_rev;
	_Uint32t			security;

#define DEV_CAP_HC				(1 << 0)	/* high capacity */
#define DEV_CAP_HS				(1 << 1)	/* high speed */
#define DEV_CAP_HS200			(1 << 2)	/* high speed 200 */
#define DEV_CAP_DDR50			(1 << 3)	/* DDR */
#define DEV_CAP_UHS				(1 << 4)	/* UHS */
#define DEV_CAP_TRIM			(1 << 5)	/* TRIM supported */
#define DEV_CAP_SECURE			(1 << 6)	/* Secure Purge supported */
#define DEV_CAP_SANITIZE		(1 << 7)	/* SANITIZE supported */
#define DEV_CAP_BKOPS			(1 << 8)	/* Background Operations supported */
#define DEV_CAP_CMD23			(1 << 9)	/* CMD23 supported */
#define DEV_CAP_SLEEP			(1 << 10)	/* SLEEP/AWAKE supported */
#define DEV_CAP_ASSD			(1 << 11)	/* ASSD */
#define DEV_CAP_HPI_CMD12		(1 << 12)
#define DEV_CAP_HPI_CMD13		(1 << 13)
#define DEV_CAP_DISCARD			(1 << 14)	/* Discard supported */
#define DEV_CAP_CACHE			(1 << 15)
#define DEV_CAP_HS400			(1 << 16)	/* high speed 400 */
#define DEV_CAP_PWROFF_NOTIFY		(1 << 17)	/* power off notify supported */
	_Uint64t			caps;

	_Uint32t			dtr;			/* current data transfer rate */

#define TIMING_HS400			9
#define TIMING_HS200			8
#define TIMING_SDR104			7
#define TIMING_SDR50			6
#define TIMING_SDR25			5
#define TIMING_SDR12			4
#define TIMING_DDR50			3
#define TIMING_HS				2
#define TIMING_LS				1
	_Uint32t			timing;			/* current timing */
	_Uint32t			bus_width;		/* current bus width */
	_Uint32t			sectors;
	_Uint32t			sector_size;
	_Uint32t			super_page_size;
	_Uint32t			native_sector_size;
	_Uint32t			wp_size;
	_Uint32t			erase_size;
	_Uint32t			optimal_trim_size;
	_Uint32t			optimal_read_size;
	_Uint32t			optimal_write_size;

#define SPEED_CLASS_0			0x00	/* Legacy/Non Compliant */
#define SPEED_CLASS_2			0x01	/* Approximately 2MB/sec */
#define SPEED_CLASS_4			0x02	/* Approximately 4MB/sec */
#define SPEED_CLASS_6			0x03	/* Approximately 6MB/sec */
#define SPEED_CLASS_10			0x04	/* Approximately 10MB/sec */
	_Uint32t			speed_class;
	
	_Uint32t			start_sector;	/* Physical Start Sector */

	_Uint32t			rsvd[34];
} SDMMC_DEVICE_INFO;

typedef struct _sdmmc_assd_status {
#define ASSD_STATE_SCP			1		/* Secure Command in Progress */
#define ASSD_STATE_SCC			2		/* Secure Command Complete */
#define ASSD_STATE_SCA			3		/* Secure Command Aborted */
	_Uint8t		assd_state;

#define ASSD_ERR_STATE_NE		0		/* No Error */
#define ASSD_ERR_STATE_AE		1		/* Auth Error */
#define ASSD_ERR_STATE_ANF		2		/* Area Not Found */
#define ASSD_ERR_STATE_RO		3		/* Range Over */
#define ASSD_ERR_STATE_CE		4		/* Condition Error */
	_Uint8t		assd_err_state;
	_Uint8t		assd_sec_sys_err;
	_Uint8t		pmem_state;
	_Uint8t		auth_alg;
	_Uint8t		enc_alg;
	_Uint8t		active_sec_system;
	_Uint8t		sec_token_prot;
	_Uint16t	read_block_count;
	_Uint16t	suspended_sec_sys;
	_Uint32t	rsvd[6];
} SDMMC_ASSD_STATUS;

typedef struct _sdmmc_assd_properties {
	_Uint8t		assd_version;
	_Uint8t		assd_sec_sys_vendor_id;
	_Uint16t	assd_sec_sys;

	_Uint16t	suspendible_sec_sys;
	_Uint16t	sup_auth_alg;
	_Uint16t	sup_enc_alg;
	_Uint16t	cl_support;

	_Uint8t		sec_read_latency;		/* 250ms units */
	_Uint8t		sec_write_latency;		/* 250ms units */
	_Uint8t		wr_sec_bus_busy;		/* 250ms units */
	_Uint8t		ctrl_sys_bus_busy;		/* 250ms units */

	_Uint8t		pmem_support;
	_Uint8t		pmem_rd_time;			/* 100ms units */
	_Uint8t		pmem_wr_time;			/* 250ms units */

	_Uint8t		rsvd[17];
} SDMMC_ASSD_PROPERTIES;

typedef struct _sdmmc_assd_control {
#define SDMMC_AC_OP_START_SUSPEND	0x3
#define SDMMC_AC_OP_CLEAR_SUSPEND	0x2
#define SDMMC_AC_OP_SELECT_RESET	0x1
	_Uint8t		operation;

#define SDMMC_AC_SSI_MAX			0xf
	_Uint8t		sec_sys_idx;
	_Uint16t	rsvd[11];
} SDMMC_ASSD_CONTROL;

typedef struct _sdmmc_assd_apdu {
	_Uint32t	length;
	_Uint32t	rsvd[7];
/*	_Uint8t		data[ length ];		variable length data */
} SDMMC_ASSD_APDU;

typedef struct _sdmmc_lock_unlock {
#define SDMMC_LU_ACTION_ERASE		0x08
#define SDMMC_LU_ACTION_LOCK		0x04
#define SDMMC_LU_ACTION_CLR			0x02
#define SDMMC_LU_ACTION_SET			0x01
#define SDMMC_LU_ACTION_UNLOCK		0x00
	_Uint32t	action;
	_Uint32t	pwd_len;
#define SDMMC_LU_PWD_SIZE			16
	_Uint8t		pwd[SDMMC_LU_PWD_SIZE];
	_Uint32t	rsvd[8];
} SDMMC_LOCK_UNLOCK;

typedef struct _sdmmc_partition_info {
#define SDMMC_PI_ACTION_GET		0x00
#define SDMMC_PI_ACTION_CLR		0x01
	_Uint32t		action;
	_Uint32t		rsvd;

#define SDMMC_PTYPE_USER		0
#define SDMMC_PTYPE_BOOT1		1
#define SDMMC_PTYPE_BOOT2		2
#define SDMMC_PTYPE_RPMB		3
#define SDMMC_PTYPE_GP1			4
#define SDMMC_PTYPE_GP2			5
#define SDMMC_PTYPE_GP3			6
#define SDMMC_PTYPE_GP4			7
	_Uint32t		ptype;
#define SDMMC_PFLAG_WP			0x01
#define SDMMC_PFLAG_ENH			0x02
#define SDMMC_PFLAG_VIRTUAL		0x04
	_Uint32t		pflags;
	_Uint64t		start_lba;			/* Starting lba */
	_Uint64t		num_lba;			/* Num lba */
	_Uint64t		rc;					/* Read Count (sectors) */
	_Uint64t		wc;					/* Written Count (sectors) */
	_Uint64t		tc;					/* TRIM Count (sectors) */
	_Uint64t		ec;					/* Erase Count (sectors) */
	_Uint64t		dc;					/* Discard Count (sectors) */
	_Uint32t		rsvd1[64];
} SDMMC_PARTITION_INFO;

typedef struct _sdmmc_pwr_mgnt {
#define SDMMC_PM_ACTION_GET		0x00
#define SDMMC_PM_ACTION_SET		0x01
	_Uint32t		action;
	_Uint32t		rsvd;

	_Uint64t		idle_time;			/* time in ms til device enters idle */
	_Uint64t		sleep_time;			/* time in ms til device enters sleep */
	_Uint32t		rsvd1[16];
} SDMMC_PWR_MGNT;

#define DCMD_SDMMC_DEVICE_INFO			__DIOF(_DCMD_CAM, _SIM_SDMMC + 0, struct _sdmmc_device_info)
#define DCMD_SDMMC_DEVICE_HEALTH		__DIOF(_DCMD_CAM, _SIM_SDMMC + 1, union _sdmmc_device_health)
#define DCMD_SDMMC_ERASE 			  	__DIOTF(_DCMD_CAM, _SIM_SDMMC + 2, struct _sdmmc_erase)
#define DCMD_SDMMC_WRITE_PROTECT	   	__DIOTF(_DCMD_CAM, _SIM_SDMMC + 3, struct _sdmmc_write_protect)
#define DCMD_SDMMC_CARD_REGISTER		__DIOTF(_DCMD_CAM, _SIM_SDMMC + 4, struct _sdmmc_card_register)
#define DCMD_SDMMC_ASSD_STATUS			__DIOF(_DCMD_CAM, _SIM_SDMMC + 5, struct _sdmmc_assd_status)
#define DCMD_SDMMC_ASSD_PROPERTIES		__DIOF(_DCMD_CAM, _SIM_SDMMC + 6, struct _sdmmc_assd_properties)
#define DCMD_SDMMC_ASSD_CONTROL			__DIOT(_DCMD_CAM, _SIM_SDMMC + 7, struct _sdmmc_assd_control)
#define DCMD_SDMMC_ASSD_APDU			__DIOTF(_DCMD_CAM, _SIM_SDMMC + 8, struct _sdmmc_assd_apdu)
#define DCMD_SDMMC_LOCK_UNLOCK			__DIOT(_DCMD_CAM, _SIM_SDMMC + 9, struct _sdmmc_lock_unlock)
#define DCMD_SDMMC_PART_INFO			__DIOTF(_DCMD_CAM, _SIM_SDMMC + 10, struct _sdmmc_partition_info)
#define DCMD_SDMMC_PWR_MGNT				__DIOTF(_DCMD_CAM, _SIM_SDMMC + 11, struct _sdmmc_pwr_mgnt)

#include <_packpop.h>

#endif


#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/hardware/devb/sdmmc/public/hw/dcmd_sim_sdmmc.h $ $Rev: 813652 $")
#endif
