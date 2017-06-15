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

#ifndef _MMC_H_INCLUDED
#define _MMC_H_INCLUDED

#define MMC_PARTITION_SWITCH_TIMEOUT	10			// 10ms
#define MMC_OUT_OF_INTERRUPT_TIMEOUT	10			// 10ms
#define MMC_ERASE_TIMEOUT			300			// 300ms
#define MMC_ERASE_TIMEOUT_MIN		2000		// set min timeout to 2 seconds
#define MMC_ERASE_GRP_512K			0x80000LL
#define MMC_PART_USER				0
#define MMC_PART_BOOT1				1
#define MMC_PART_BOOT2				2
#define MMC_PART_RPMB				3
#define MMC_PART_GP1				4
#define MMC_PART_GP2				5
#define MMC_PART_GP3				6
#define MMC_PART_GP4				7
#define MMC_PART_MSK				0x7

#define MMC_BOOT_PART_MAX			2
#define MMC_GP_PART_MAX				4
#define MMC_BOOT_SIZE_MULT			0x20000
#define MMC_RPMB_SIZE_MULT			0x20000
#define MMC_CID_SIZE				16
#define MMC_CSD_SIZE				16

#define	MMC_GO_IDLE_STATE			0

#define	MMC_SEND_OP_COND			1
	#define OCR_VDD_17_195		0x00000080	// VDD 1.7 - 1.95
	#define OCR_VDD_20_21		0x00000100	// VDD 2.0 - 2.1
	#define OCR_VDD_21_22		0x00000200	// VDD 2.1 - 2.2
	#define OCR_VDD_22_23		0x00000400	// VDD 2.2 - 2.3
	#define OCR_VDD_23_24		0x00000800	// VDD 2.3 - 2.4
	#define OCR_VDD_24_25		0x00001000	// VDD 2.4 - 2.5
	#define OCR_VDD_25_26		0x00002000	// VDD 2.5 - 2.6
	#define OCR_VDD_26_27		0x00004000	// VDD 2.6 - 2.7
	#define OCR_VDD_27_28		0x00008000	// VDD 2.7 - 2.8
	#define OCR_VDD_28_29		0x00010000	// VDD 2.8 - 2.9
	#define OCR_VDD_29_30		0x00020000	// VDD 2.9 - 3.0
	#define OCR_VDD_30_31		0x00040000	// VDD 3.0 - 3.1
	#define OCR_VDD_31_32		0x00080000	// VDD 3.1 - 3.2
	#define OCR_VDD_32_33		0x00100000	// VDD 3.2 - 3.3
	#define OCR_VDD_33_34		0x00200000	// VDD 3.3 - 3.4
	#define OCR_VDD_34_35		0x00400000	// VDD 3.4 - 3.5
	#define OCR_VDD_35_36		0x00800000	// VDD 3.5 - 3.6
	#define OCR_S18R			0x01000000	// Switching to 1.8V Request
	#define OCR_S18A			0x01000000	// Switching to 1.8V Accepted
	#define OCR_XPC				0x10000000	// SDXC Power Control > 150ma
	#define OCR_UHS_II			0x20000000	// UHS-II Card
	#define OCR_HCS				0x40000000	// Card Capacity Status
	#define OCR_PWRUP_CMP		0x80000000

	#define OCR_BUSY_RETRIES	500			// wait up to 5 seconds

#define	MMC_ALL_SEND_CID			2
	#define MDT_YEAR_1997			1997
	#define MDT_YEAR_2010			2010
	#define MDT_YEAR_2013			2013

#define	MMC_SET_RELATIVE_ADDR		3
#define	MMC_SET_DSR					4

#define	MMC_SLEEP_AWAKE				5
	#define MMC_SA_SLEEP			(1 << 15 )

#define	MMC_SWITCH					6
	#define MMC_SWITCH_MODE_WRITE	0x3
	#define MMC_SWITCH_MODE_CLR		0x2
	#define MMC_SWITCH_MODE_SET		0x1
	#define MMC_SWITCH_MODE_CMD_SET	0x0
	#define MMC_SWITCH_CMDSET_DFLT	0x1

#define	MMC_SEL_DES_CARD			7
#define	MMC_SEND_EXT_CSD			8
#define	MMC_SEND_CSD				9
#define	MMC_SEND_CID				10
#define	MMC_READ_DAT_UNTIL_STOP		11

#define	MMC_STOP_TRANSMISSION		12
	#define MMC_STOP_TRANSMISSION_HPI	(1 << 0)

#define	MMC_SEND_STATUS				13
	#define MMC_SEND_STATUS_HPI			(1 << 0)

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
	#define CDS_CUR_STATE( _r )			( ( (_r) >> 9 ) & 0xf )
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

#define	MMC_BUSTEST_R				14
#define	MMC_GO_INACTIVE_STATE		15
#define	MMC_SET_BLOCKLEN			16
#define	MMC_READ_SINGLE_BLOCK		17
#define	MMC_READ_MULTIPLE_BLOCK		18
#define	MMC_BUSTEST_W				19
#define	MMC_WRITE_DAT_UNTIL_STOP	20
#define MMC_SEND_TUNING_BLOCK		21
#define MMC_SET_BLOCK_COUNT         23
#define	MMC_WRITE_BLOCK				24
#define	MMC_WRITE_MULTIPLE_BLOCK	25
#define	MMC_PROGRAM_CID				26
#define	MMC_PROGRAM_CSD				27
#define	MMC_SET_WRITE_PROT			28
#define	MMC_CLR_WRITE_PROT			29
#define	MMC_SEND_WRITE_PROT			30
	#define MMC_SEND_WRITE_PROT_SIZE		4
#define	MMC_SEND_WRITE_PROT_TYPE	31
	#define MMC_SEND_WRITE_PROT_TYPE_SIZE	8
#define	MMC_TAG_SECTOR_START		32
#define	MMC_TAG_SECTOR_END			33
#define	MMC_UNTAG_SECTOR			34
#define	MMC_TAG_ERASE_GROUP_START	35
#define	MMC_TAG_ERASE_GROUP_END		36
#define	MMC_UNTAG_ERASE_GROUP		37

#define	MMC_ERASE					38
	#define MMC_ERASE_SECURE_TRIM_PURGE	0x80008000
	#define MMC_ERASE_SECURE_TRIM		0x80000001
	#define MMC_ERASE_SECURE			0x80000000
	#define MMC_ERASE_DISCARD			0x00000003
	#define MMC_ERASE_TRIM				0x00000001
	#define MMC_ERASE_NORM				0x00000000

#define	MMC_FAST_IO					39
#define	MMC_GO_IRQ_STATE			40
#define	MMC_LOCK_UNLOCK				42
	#define MMC_LU_ERASE				0x08
	#define MMC_LU_LOCK					0x04
	#define MMC_LU_UNLOCK				0x00
	#define MMC_LU_CLR_PWD				0x02
	#define MMC_LU_SET_PWD				0x01
	#define MMC_LU_PWD_SIZE				16		// max password size
	
#define	MMC_APP_CMD					55
#define	MMC_GEN_CMD					56
#define	MMC_READ_OCR				58
#define	MMC_CRC_ON_OFF				59

// EXT_CSD fields
#define MMC_EXT_CSD_SIZE			512	

#define ECSD_FLUSH_CACHE			32
	#define ECSD_FLUSH_TRIGGER			0x01

#define ECSD_CACHE_CTRL				33
	#define ECSD_CACHE_CTRL_EN			0x01
	#define ECSD_CACHE_CTRL_DIS			0x00

#define ECSD_POWER_OFF_NOTIFICATION	34
	#define ECSD_NO_POWER_NOTIFICATION	0x00
	#define ECSD_POWERED_ON				0x01
	#define ECSD_POWER_OFF_SHORT		0x02
	#define ECSD_POWER_OFF_LONG			0x03

#define ECSD_USE_NATIVE_SECTOR		62
	#define ECSD_USE_NATIVE_SECTOR_EN	0x01

#define ECSD_NATIVE_SECTOR_SIZE		63
	#define ECSD_NATIVE_SECTOR_512		0x00
	#define ECSD_NATIVE_SECTOR_4K		0x01
	#define ECSD_NATIVE_SECTOR_MSK		0x01

#define ECSD_ENH_START_ADDR			136
#define ECSD_ENH_SIZE_MULT			140
#define ECSD_GP_SIZE				143
#define ECSD_GP1_SIZE				143
#define ECSD_GP2_SIZE				146
#define ECSD_GP3_SIZE				149
#define ECSD_GP4_SIZE				152

#define ECSD_PARTITION_SETTING		155
	#define ECSD_PS_CMP					0x01

#define ECSD_PARTITIONS_ATTR		156
	#define ECSD_PA_ENH_4				0x10
	#define ECSD_PA_ENH_3				0x08
	#define ECSD_PA_ENH_2				0x04
	#define ECSD_PA_ENH_1				0x02
	#define ECSD_PA_ENH_USR				0x01

#define ECSD_MAX_ENH_SIZE_MULT		157

#define ECSD_PARTITIONING_SUP		160
	#define ECSD_PS_EXT_ATTR_EN			0x04
	#define ECSD_PS_ENH_ATTR_EN			0x02
	#define ECSD_PS_PART_EN				0x01

#define ECSD_BKOPS_EN				163  // Background operation enable
	#define ECSD_BKOPS_ENABLE			1

#define ECSD_BKOPS_START			164  // Background operation start
	#define ECSD_BKOPS_INITIATE			1

#define ECSD_SANITIZE_START			165  // Sanitize operation start
	#define ECSD_SANITIZE_INITIATE			1

#define ECSD_WR_REL_PARAM			166
	#define ECSD_WRP_EN_REL_WR			0x02
	#define ECSD_WRP_WR_DATA_REL		0x01

#define ECSD_WR_REL_SET				167
	#define ECSD_WRS_4					0x10
	#define ECSD_WRS_3					0x08
	#define ECSD_WRS_2					0x04
	#define ECSD_WRS_1					0x02
	#define ECSD_WRS_USR				0x01

#define ECSD_RPMB_SIZE_MULT			168

#define ECSD_USER_WP				171
	#define ECSD_USER_WP_US_PWR_WP_EN	(1<<0)
	#define ECSD_USER_WP_US_PERM_WP_EN	(1<<2)
	#define ECSD_USER_WP_US_PWR_WP_DIS	(1<<3)
	#define ECSD_USER_WP_US_PERM_WP_DIS	(1<<4)
	#define ECSD_USER_WP_CD_PERM_WP_DIS	(1<<6)
	#define ECSD_USER_WP_PERM_PSWD_DIS	(1<<7)

#define ECSD_BOOT_WP				173

#define ECSD_ERASE_GRP_DEF			175
	#define ECSD_EGD_EN					0x01

#define ECSD_PART_CONFIG			179
	#define ECSD_PC_ACCESS_MSK			0x7

#define ECSD_ERASED_MEM_CONT		181

#define ECSD_BUS_WIDTH				183
	#define ECSD_BUS_WIDTH_8			2	// Card is in 8 bit mode
	#define ECSD_BUS_WIDTH_4			1	// Card is in 4 bit mode
	#define ECSD_BUS_WIDTH_1			0	// Card is in 1 bit mode
	#define ECSD_BUS_WIDTH_DDR			4	// Add to width for DDR

#define ECSD_HS_TIMING				185
	#define ECSD_HS_TIMING_DRV_TYPE_SHFT	4
	#define ECSD_HS_TIMING_HS400			0x3
	#define ECSD_HS_TIMING_HS200			0x2
	#define ECSD_HS_TIMING_HS				0x1
	#define ECSD_HS_TIMING_LS				0x0

#define ECSD_DRIVER_STRENGTH		197

#define ECSD_CARD_TYPE				196
	#define ECSD_CARD_TYPE_HS400_1_2V	(1<<7)	// Card can run at DDR 200MHz 1.2V
	#define ECSD_CARD_TYPE_HS400_1_8V	(1<<6)	// Card can run at DDR 200MHz 1.8V

	#define ECSD_CARD_TYPE_HS200_1_2V	(1<<5)	// Card can run at SDR 200MHz 1.2V
	#define ECSD_CARD_TYPE_HS200_1_8V	(1<<4)	// Card can run at SDR 200MHz 1.8V

	#define ECSD_CARD_TYPE_DDR_1_2V		(1<<3)	// Card can run at 52MHz 1.2V
	#define ECSD_CARD_TYPE_DDR_1_8V		(1<<2)	// Card can run at 52MHz 1.8V - 3.0V

	#define ECSD_CARD_TYPE_52			(1<<1)	// Card can run at 52MHz
	#define ECSD_CARD_TYPE_26			(1<<0)	// Card can run at 26MHz

	#define ECSD_CARD_TYPE_DDR			( ECSD_CARD_TYPE_DDR_1_8V | ECSD_CARD_TYPE_DDR_1_2V )
	#define ECSD_CARD_TYPE_HS400		( ECSD_CARD_TYPE_HS400_1_8V | ECSD_CARD_TYPE_HS400_1_2V )
	#define ECSD_CARD_TYPE_HS200		( ECSD_CARD_TYPE_HS200_1_8V | ECSD_CARD_TYPE_HS200_1_2V )
	#define ECSD_CARD_TYPE_52MHZ		( ECSD_CARD_TYPE_DDR_1_8V | ECSD_CARD_TYPE_DDR_1_2V | ECSD_CARD_TYPE_52 )
	#define ECSD_CARD_TYPE_MSK			0xff

#define ECSD_REV					192
	#define ECSD_REV_V5					7
	#define ECSD_REV_V4_5				6
	#define ECSD_REV_V4_41				5
	#define ECSD_REV_V4_4				4
	#define ECSD_REV_V4_3				3
	#define ECSD_REV_V4_2				2
	#define ECSD_REV_V4_1				1
	#define ECSD_REV_V4					0

#define ECSD_OUT_OF_INTERRUPT_TIME	198
#define ECSD_PARTITION_SWITCH_TIME	199

#define ECSD_SEC_CNT				212
	#define ECSD_SEC_CNT_2GB			0x400000

#define ECSD_S_A_TIMEOUT			217
#define ECSD_HC_WP_GRP_SIZE			221
#define ECSD_ERASE_MULT				223	// Erase Timeout Multiplier
#define ECSD_ACC_SIZE				225
#define ECSD_ERASE_GRP_SIZE			224
#define ECSD_BOOT_SIZE_MULT			226
#define ECSD_SEC_TRIM_MULT			229
#define ECSD_SEC_ERASE_MULT			230 // Secure Erase Timeout Multiplier

#define ECSD_SEC_FEATURE_SUPPORT	231
	#define ECSD_SEC_SANITIZE			0x40 // SANITIZE support
	#define ECSD_SEC_GB_CL_EN			0x10 // TRIM support
	#define ECSD_SEC_BD_BLK_EN			0x04 // Secure purge Bad blk mgnt support
	#define ECSD_SEC_ER_EN				0x01 // Secure purge support

#define ECSD_TRIM_MULT				232

#define ECSD_BKOPS_STATUS			246  // Background operation support
	#define ECSD_BS_OPERATIONS_NONE			0
	#define ECSD_BS_OPERATIONS_NON_CRITICAL	1
	#define ECSD_BS_OPERATIONS_IMPACTED		2
	#define ECSD_BS_OPERATIONS_CRITICAL		3

#define ECSD_CACHE_SIZE				249

#define ECSD_POWER_OFF_LONG_TIME	247  // Power off long switch timeout

#define ECSD_BKOPS_SUPPORTED		502  // Background operation support
	#define ECSD_BKOPS_SUP				1

#define ECSD_HPI_FEATURES			503
	#define EXT_HPI_FEATURES_SUP_CMD12	0x02
	#define EXT_HPI_FEATURES_SUPPORTED	0x01

#define ECSD_S_CMD_SET				504

#endif


#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/hardware/devb/sdmmc/sdiodi/include/mmc.h $ $Rev: 805416 $")
#endif
