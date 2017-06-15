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

// Module Description:  SIM SDMMC header file

#ifndef _SIM_SDMMC_H_INCLUDED
#define _SIM_SDMMC_H_INCLUDED

#include <time.h>
#include <ctype.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <stddef.h>
#include <malloc.h>
#include <string.h>
#include <signal.h>
#include <atomic.h>
#include <stdarg.h>
#include <stdbool.h>
#include <pthread.h>
#include <gulliver.h>
#include <hw/inout.h>
#include <sys/mman.h>
#include <sys/disk.h>
#include <sys/types.h>
#include <sys/trace.h>
#include <sys/resmgr.h>
#include <sys/procmgr.h>
#include <sys/syspage.h>

// CAM specific includes
#include <module.h>
#include <ntocam.h>
#include <sim.h>
#include <sys/dcmd_cam.h>
#include <hw/dcmd_sim_sdmmc.h>

#include <sim_bs.h>

#include <sdiodi.h>
#include <sd.h>
#include <mmc.h>

//#define SDMMC_WRITE_VERIFY
#define SDMMC_TRIM_SUP
//#define SDMMC_SIM_RETRY

//#define SDMMC_TRACE
//#define SDMMC_SIM_DEBUG

#define SDMMC_TRACE_EVENT				1

#define SDMMC_SIM_VERSION				100
#define SDMMC_SIM_LETTER				'A'

#define SDMMC_RW_RETRIES				2
#define SDMMC_STACK_SIZE				16384
#define SDMMC_SCHED_PRIORITY			21
#define SDMMC_TIME_DEFAULT				5
#define SDMMC_TIME_INFINITY				0xffffffff

#define SDMMC_PM_TIMER					0x40		// Timer event

#define SDMMC_MAX_BUS					10

#define SDMMC_MAX_HBA					8
#define SDMMC_MAX_TARGET				8
#define SDMMC_MAX_SG					128

#define SDMMC_TRIM_MAX_LBA				0xffffffff
#define SDMMC_TIMEOUT_MS_TO_NS( _to )	( (uint64_t)( _to ) * 1000LL * 1000LL )
#define SDMMC_TIMEOUT_S_TO_NS( _to )	( (uint64_t)( _to ) * 1000LL * 1000LL * 1000LL )

#define SDMMC_ARG_VAL( _o, _v ) if( (_v) == NULL || *(_v) == '\0' ) { cam_slogf( _SLOGC_SIM_MMC, _SLOG_ERROR, 1, 1, "%s:  Missing argument for '%s'", __FUNCTION__, _o ); break; }
#define SDMMC_ARG_NOVAL( _o, _v ) if( (_v) != NULL ) { cam_slogf( _SLOGC_SIM_MMC, _SLOG_ERROR, 1, 1, "%s:  Unexpected argument for '%s'", __FUNCTION__, _o ); break; }
#define SDMMC_ARG_NUMBER( _o, _n, _f ) if( ( *(_n) = (_f)) == -1 ) { cam_slogf( _SLOGC_SIM_MMC, _SLOG_ERROR, 1, 1, "%s:  Invalid numeric argument for '%s'", __FUNCTION__, _o ); break; }
#define SDMMC_ARG_MISSING( _o, _a ) if( !(_a) ) { cam_slogf( _SLOGC_SIM_MMC, _SLOG_ERROR, 1, 1, "%s:  Missing argument component for '%s'", __FUNCTION__, _o ); break; }

typedef struct _sdmmc_ctrl {
	TAILQ_HEAD(,_sim_hba)	hlist;			// linked list of hba's

#define SDMMC_CFLAG_SCAN		0x01		// auto detect interfaces
#define SDMMC_CFLAG_ENUMERATING	0x02
	uint32_t				cflags;
	uint32_t				verbosity;
	uint32_t				nhba;			// number of hba's
	uint32_t				pathid_max;		// max path id

	int						argc;
	char					**argv;
	struct sdio_connection	*connection;
} SDMMC_CTRL;

#define SDMMC_TARGET_MAX				8
#define SDMMC_PARTITION_MAX				8

typedef struct _sdmmc_partition {
	_Uint32t		pflags;
	_Uint32t		config;
	char			name[20];
	_Uint32t		blk_shft;
	_Uint32t		slba;			// starting lba
	_Uint32t		elba;			// ending lba
	_Uint32t		nlba;			// num lba
	_Uint64t		rc;				// Read Count
	_Uint64t		wc;				// Written Count
	_Uint64t		tc;				// TRIM Count
	_Uint64t		ec;				// Erase Count
	_Uint64t		dc;				// Discard Count
} SDMMC_PARTITION;

typedef struct _sdmmc_target {
	_Uint32t			nluns;
	_Uint32t			blksz;
	SDMMC_PARTITION		partitions[SDMMC_PARTITION_MAX];
} SDMMC_TARGET;

typedef struct _sim_sdmmc_ext {
	SIM_HBA					*hba;

#define SDMMC_EFLAG_PRESENT				(1 << 0)
#define SDMMC_EFLAG_TIMER				(1 << 1)
#define SDMMC_EFLAG_BKOPS				(1 << 2)
#define SDMMC_EFLAG_DEVNAME				(1 << 3)
#define SDMMC_EFLAG_PARTITIONS			(1 << 4)	// create hw partitions
#define SDMMC_EFLAG_ASSD_INIT			(1 << 5)
#define SDMMC_EFLAG_ASSD_SEND_STOP		(1 << 6)
#define SDMMC_EFLAG_DEV_BUSY			(1 << 7)
#define SDMMC_EFLAG_CACHE				(1 << 8)
#define SDMMC_EFLAG_PWROFF_NOTIFY		(1 << 9)
#define SDMMC_EFLAG_BS					(1 << 24)
	_Uint32t				eflags;
	_Uint8t					priority;
	_Uint8t					pwroff_notify;
	_Uint8t					rsvd[2];

	CCB_SCSIIO				*nexus;

	struct sdio_device		*device;
	sdio_device_instance_t	instance;
	sdio_hc_info_t			hc_inf;
	sdio_dev_info_t			dev_inf;

	timer_t					pm_timerid;

	_Uint64t				pm_timestamp;
	_Uint64t				pm_idle_time_ns;
	_Uint64t				pm_sleep_time_ns;

#define SDMMC_PM_IDLE							0
#define SDMMC_PM_SLEEP							1
#define SDMMC_PM_ACTIVE							2
	_Uint32t				pm_state;

#define BKOPS_NC_TICKS							10
#define BKOPS_IMPACTED_TICKS					5
	_Uint32t				bkops_ticks;
#define BKOPS_STATUS_OPERATIONS_NONE			0
#define BKOPS_STATUS_OPERATIONS_NON_CRITICAL	1
#define BKOPS_STATUS_OPERATIONS_IMPACTED		2
#define BKOPS_STATUS_OPERATIONS_CRITICAL		3
#define BKOPS_STATUS_OPERATIONS_INPROG			0x80000000
	_Uint32t				bkops_status;
#define SDMMC_TIME_BKOPS			( SDIO_TIME_DEFAULT	* 5 )

	SDMMC_ASSD_PROPERTIES	assd_properties;
	int						assd_active_sec_sys;

	_Uint32t				ntargs;
	SDMMC_TARGET			targets[SDMMC_TARGET_MAX];

#ifdef SDMMC_WRITE_VERIFY
#define SDMMC_VER_BSIZE		( 512 * 256 )
	char					*ver_vaddr;
	paddr64_t				ver_paddr;
#endif

} SIM_SDMMC_EXT;

// sim_sdmmc.c
extern int sdmmc_sim_args( char *options );
extern int sdmmc_sim_detach( void );
extern int sdmmc_sim_attach( CAM_ENTRY *centry );
extern int sdmmc_sim_init( SIM_HBA *hba, int path );
extern int sdmmc_sim_action( SIM_HBA *hba, CCB *ccb_ptr );
extern void *sdmmc_driver_thread( void *data );

extern int sdmmc_bkops_cfg( SIM_HBA *hba );
extern int sdmmc_pwroff_notify( SIM_HBA *hba, uint8_t cfg );
extern int sdmmc_unit_ready( SIM_HBA *hba, CCB_SCSIIO *ccb );
extern int sdmmc_wp_devctl( SIM_HBA *hba, CCB_DEVCTL *ccb );
extern int sdmmc_erase_devctl( SIM_HBA *hba, CCB_DEVCTL *ccb );
extern int sdmmc_card_register_devctl( SIM_HBA *hba, CCB_DEVCTL *ccb );
extern int sdmmc_rw( SIM_HBA *hba, SDMMC_PARTITION *part, int flgs, uint32_t addr, int dlen, sdio_sge_t *sgl, int sgc, void *mhdl, uint32_t timeout );
extern int sim_bs_partition_config( SIM_HBA *hba );
extern int sim_bs_devctl( SIM_HBA *hba, CCB_DEVCTL *ccb );
extern int sim_bs_pass_through( SIM_HBA *hba, CCB_SCSIIO *ccb );

// sim_assd.c
int sdmmc_assd_init( SIM_HBA *hba );
int sdmmc_assd_apdu_devctl( SIM_HBA *hba, CCB_DEVCTL *ccb );
int sdmmc_assd_status_devctl( SIM_HBA *hba, CCB_DEVCTL *ccb );
int sdmmc_assd_control_devctl( SIM_HBA *hba, CCB_DEVCTL *ccb );
int sdmmc_assd_properties_devctl( SIM_HBA *hba, CCB_DEVCTL *ccb );

#ifndef EXTERN
#define EXTERN_ADDED
#define EXTERN extern
#define	VALUE(x)
#else
#define VALUE(x) = { x }
#endif

EXTERN SDMMC_CTRL sdmmc_ctrl
#ifndef EXTERN_ADDED
= { { 0 },0 }
#endif
;

#ifdef EXTERN_ADDED
#undef EXTERN_ADDED
#undef EXTERN
#endif
#undef VALUE

#endif


#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/hardware/devb/sdmmc/sim_sdmmc.h $ $Rev: 805883 $")
#endif
