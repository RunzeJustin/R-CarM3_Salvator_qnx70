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

#ifndef _SD_H_INCLUDED
#define _SD_H_INCLUDED

#define SD_CID_SIZE					16
#define SD_CSD_SIZE					16
#define SD_ERASE_TIMEOUT			250			// 250ms

#define	SD_GO_IDLE_STATE			0
#define	SD_ALL_SEND_CID				2
#define	SD_SEND_RELATIVE_ADDR				3
	#define SD_SEND_RELATIVE_ADDR_RETRIES	255

#define	SD_SET_DSR					4
#define	SD_SLEEP_AWAKE				5
	#define SD_SA_SLEEP				(1 << 15 )

#define	SD_SWITCH_FUNC				6
	#define SD_SF_MODE_SET			0x1
	#define SD_SF_MODE_CHECK		0x0

	#define SD_SF_GRP_DFLT			0x0
	#define SD_SF_GRP_BUS_SPD		0x0		// Bus Speed
	#define SD_SF_GRP_CMD_EXT		0x1		// Command System Extension
	#define SD_SF_GRP_DRV_STR		0x2		// Driver Strength
	#define SD_SF_GRP_CUR_LMT		0x3		// Current Limit

	#define SD_SF_CUR_FCN			0xf

	#define SD_SF_STATUS_SIZE		64

#define	SD_SEL_DES_CARD				7
#define	SD_SEND_IF_COND				8
	#define SD_SIC_VHS_27_36V		1
	#define SD_SIC_TEST_PATTERN		0xaa

#define	SD_SEND_EXT_CSD				8
#define	SD_SEND_CSD					9
#define	SD_SEND_CID					10
#define	SD_VOLTAGE_SWITCH			11
#define	SD_STOP_TRANSMISSION		12
#define	SD_SEND_STATUS				13
// Card/Device Status Response Bits
	#define	CDS_OUT_OF_RANGE			(1 << 31)
	#define	CDS_ADDRESS_ERROR			(1 << 30)
	#define	CDS_BLOCK_LEN_ERROR			(1 << 29)
	#define	CDS_ERASE_SEQ_ERROR			(1 << 28)
	#define	CDS_ERASE_PARAM				(1 << 27)
	#define	CDS_WP_VIOLATION			(1 << 26)
	#define	CDS_CARD_IS_LOCKED			(1 << 25)
	#define	CDS_LOCK_UNLOCK_FAILED		(1 << 24)
	#define	CDS_COM_CRC_ERROR			(1 << 23)
	#define	CDS_ILLEGAL_COMMAND			(1 << 22)
	#define	CDS_CARD_ECC_FAILED			(1 << 21)
	#define	CDS_CC_ERROR				(1 << 20)
	#define	CDS_ERROR					(1 << 19)
	#define	CDS_UNDERRUN				(1 << 18)
	#define	CDS_OVERRUN					(1 << 17)
	#define	CDS_CID_CSD_OVERWRITE		(1 << 16)
	#define	CDS_WP_ERASE_SKIP			(1 << 15)
	#define	CDS_CARD_ECC_DISABLED		(1 << 14)
	#define	CDS_ERASE_RESET				(1 << 13)

	#define CDS_CUR_STATE_MSK			( 0xf << 9 )
	#define	CDS_CUR_STATE_IDLE			(0 << 9)
	#define	CDS_CUR_STATE_READY			(1 << 9)
	#define	CDS_CUR_STATE_IDENT			(2 << 9)
	#define	CDS_CUR_STATE_STANDBY		(3 << 9)
	#define	CDS_CUR_STATE_TRAN			(4 << 9)
	#define	CDS_CUR_STATE_DATA			(5 << 9)
	#define	CDS_CUR_STATE_RCV			(6 << 9)
	#define	CDS_CUR_STATE_PRG			(7 << 9)
	#define	CDS_CUR_STATE_DIS			(8 << 9)

	#define	CDS_READY_FOR_DATA			(1 << 8)
	#define	CDS_SWITCH_ERROR			(1 << 7)
	#define	CDS_URGENT_BKOPS			(1 << 6)
	#define	CDS_APP_CMD_S				(1 << 5)

	#define CDS_ERROR_MSK				(	CDS_OUT_OF_RANGE		| \
											CDS_ADDRESS_ERROR		| \
											CDS_BLOCK_LEN_ERROR		| \
											CDS_ERASE_SEQ_ERROR		| \
											CDS_ERASE_PARAM			| \
											CDS_WP_VIOLATION		| \
											CDS_CARD_IS_LOCKED		| \
											CDS_LOCK_UNLOCK_FAILED	| \
											CDS_COM_CRC_ERROR		| \
											CDS_ILLEGAL_COMMAND		| \
											CDS_CARD_ECC_FAILED		| \
											CDS_CC_ERROR			| \
											CDS_ERROR				| \
											CDS_SWITCH_ERROR 		| \
	CDS_CID_CSD_OVERWRITE )

#define	SD_GO_INACTIVE_STATE			15
#define	SD_SET_BLOCKLEN					16
#define	SD_READ_SINGLE_BLOCK			17
#define	SD_READ_MULTIPLE_BLOCK			18
#define SD_SEND_TUNING_BLOCK			19
#define	SD_SPEED_CLASS_CONTROL			20
#define SD_SET_BLOCK_COUNT				23
#define	SD_WRITE_BLOCK					24
#define	SD_WRITE_MULTIPLE_BLOCK			25
#define	SD_PROGRAM_CSD					27
#define	SD_SET_WRITE_PROT				28
#define	SD_CLR_WRITE_PROT				29
#define	SD_SEND_WRITE_PROT				30
#define	SD_ERASE_WR_BLK_START			32
#define	SD_ERASE_WR_BLK_END				33
#define	SD_ERASE						38
	#define SD_ERASE_NORM				0x00000000
	#define SD_ERASE_TRIM				0x00000001
	#define SD_ERASE_GARBAGE_COLLECT	0x80008001
	#define SD_ERASE_SECURE_TRIM		0x80000001
	#define SD_ERASE_SECURE				0x80000000
#define	SD_LOCK_UNLOCK					42
	#define SD_LU_ERASE					0x08
	#define SD_LU_LOCK					0x04
	#define SD_LU_UNLOCK				0x00
	#define SD_LU_CLR_PWD				0x02
	#define SD_LU_SET_PWD				0x01
	#define SD_LU_PWD_SIZE				16		// max password size

#define	SD_APP_CMD						55
#define	SD_GEN_CMD						56
#define	SD_READ_OCR						58
#define	SD_CRC_ON_OFF					59

// ACMDS - preceded with APP_CMD (CMD55)
#define	SD_AC_SET_BUS_WIDTH				6
	#define SD_BUS_WIDTH_1				0
	#define SD_BUS_WIDTH_4				2

#define SD_AC_SD_STATUS					13
	#define SD_STATUS_SIZE				64
#define SD_AC_SEND_NUM_WR_BLOCKS		22
#define SD_AC_SET_WR_BLK_ERASE_COUNT	23
#define	SD_AC_SEND_OP_COND				41
#define SD_AC_SET_CLR_CARD_DETECT		42
#define	SD_AC_SEND_SCR					51
	#define	SD_SCR_SIZE					8

// ASSD commands
#define SD_READ_SEC_CMD					34
#define SD_WRITE_SEC_CMD				35
	#define SD_SEC_CMD_SIZE				512
#define SD_SEND_PSI						36
	#define SD_PSI_ASSD_SR				0	// ASSD Status Register
	#define SD_PSI_ASSD_PR				4	// ASSD Properties Register
	#define SD_PSI_ASSD_RNR				6	// ASSD Random Number Register
	#define SD_PSI_SIZE					32
#define SD_CONTROL_ASSD_SYSTEM			37
#define SD_DIRECT_SECURE_READ			50
#define SD_DIRECT_SECURE_WRITE			57

typedef struct _sd_cid_t {
	_Uint8t		mid;		// Manufacture ID
	_Uint8t		oid[3];		// OEM/Application ID
	_Uint8t		pnm[6];		// Product name
	_Uint8t		prv;		// Product revision
	_Uint32t	psn;		// Product serial number
	_Uint16t	mdt;		// Manufacture date
} sd_cid_t;

typedef struct _sd_scr_t {
	_Uint8t		scr_structure;	// SCR structure

#define SCR_SPEC_VER_0		0	// Version 1.0 - 1.01
#define SCR_SPEC_VER_1		1	// Version 1.10
#define SCR_SPEC_VER_2		2	// Version 2.0 or Version 3.0X
#define SCR_SPEC_VER_3		3	// Version 40
	_Uint8t		sd_spec;		// Physical layer specification
	_Uint8t		data_stat_after_erase;
	_Uint8t		sd_security;

#define SCR_BUS_WIDTH_1		(1 << 0)
#define SCR_BUS_WIDTH_4		(1 << 2)
	_Uint8t		sd_bus_widths;
	_Uint8t		sd_spec3;
	_Uint8t		ex_security;
#define SCR_CMD23_SUP		(1 << 1)
#define SCR_CMD20_SUP		(1 << 0)
	_Uint8t		cmd_support;
} sd_scr_t;

typedef struct _sd_sds_t {		// SD Status
	_Uint32t	speed_class;
	_Uint32t	uhs_speed_grade;
	_Uint32t	au_size;
	_Uint32t	uhs_au_size;
	_Uint32t	erase_timeout;
	_Uint32t	erase_offset;
} sd_sds_t;

typedef struct _sd_switch_cap {
	_Uint32t		dtr_max_hs;

#define SD_BUS_MODE_LS			( 1 << 0 )
#define SD_BUS_MODE_SDR12		( 1 << 0 )
#define SD_BUS_MODE_HS			( 1 << 1 )
#define SD_BUS_MODE_SDR25		( 1 << 1 )
#define SD_BUS_MODE_SDR50		( 1 << 2 )
#define SD_BUS_MODE_SDR104		( 1 << 3 )
#define SD_BUS_MODE_DDR50		( 1 << 4 )
#define SD_BUS_MODE_UHS			( SD_BUS_MODE_SDR50 | SD_BUS_MODE_SDR104 | SD_BUS_MODE_DDR50 )
#define SD_BUS_MODE_MSK			0x1f
	_Uint32t		bus_mode;

#define SD_CMD_SYS_EC			( 1 << 1 )	// eCommerce
#define SD_CMD_SYS_OTP			( 1 << 3 )
#define SD_CMD_SYS_ASSD			( 1 << 4 )
	_Uint32t		cmd_sys;

#define SD_DRV_TYPE_B			0x01
#define SD_DRV_TYPE_A			0x02
#define SD_DRV_TYPE_C			0x04
#define SD_DRV_TYPE_D			0x08
#define SD_DRV_TYPE_MSK			0x0f
	_Uint32t		drv_type;

#define SD_CURR_LIMIT_200		( 1 << 0 )
#define SD_CURR_LIMIT_400		( 1 << 1 )
#define SD_CURR_LIMIT_600		( 1 << 2 )
#define SD_CURR_LIMIT_800		( 1 << 3 )
#define SD_CURR_LIMIT_MSK		0xf
	_Uint32t		curr_limit;
} sd_switch_cap_t;

#endif


#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/hardware/devb/sdmmc/sdiodi/include/sd.h $ $Rev: 756950 $")
#endif
