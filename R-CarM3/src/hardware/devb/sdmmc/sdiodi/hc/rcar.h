/*
 * $QNXLicenseC:
 * Copyright 2016, QNX Software Systems.
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

#ifndef _RCAR_MMCIF_H_INCLUDED
#define _RCAR_MMCIF_H_INCLUDED

#include <internal.h>
#include <hw/dma.h>

/*
 * MMCIF Memory-mapped registers
 */

#define CONFIG_MMCIF_FREQ           200000000

#define MMC_SD_CMD                  0x0000  //  16/32/64
#define MMC_SD_PORTSEL              0x0008  //  16/32/64
#define MMC_SD_ARG                  0x0010  //  16/32/64
#define MMC_SD_ARG1                 0x0018  //  16/32/64
#define MMC_SD_STOP                 0x0020  //  16/32/64
#define MMC_SD_SECCNT               0x0028  //  16/32/64
#define MMC_SD_RSP10                0x0030  //  16/32/64
#define MMC_SD_RSP1                 0x0038  //  16/32/64
#define MMC_SD_RSP32                0x0040  //  16/32/64
#define MMC_SD_RSP3                 0x0048  //  16/32/64
#define MMC_SD_RSP54                0x0050  //  16/32/64
#define MMC_SD_RSP5                 0x0058  //  16/32/64
#define MMC_SD_RSP76                0x0060  //  16/32/64
#define MMC_SD_RSP7                 0x0068  //  16/32/64
#define MMC_SD_INFO1                0x0070  //  16/32/64
#define MMC_SD_INFO2                0x0078  //  16/32/64
#define MMC_SD_INFO1_MASK           0x0080  //  16/32/64
#define MMC_SD_INFO2_MASK           0x0088  //  16/32/64
#define MMC_SD_CLK_CTRL             0x0090  //  16/32/64
#define MMC_SD_SIZE                 0x0098  //  16/32/64
#define MMC_SD_OPTION               0x00A0  //  16/32/64
#define MMC_SD_ERR_STS1             0x00B0  //  16/32/64
#define MMC_SD_ERR_STS2             0x00B8  //  16/32/64
#define MMC_SD_BUF0                 0x00C0  //  16/32/64
#define MMC_SDIO_MODE               0x00D0  //  16/32/64
#define MMC_SDIO_INFO1              0x00D8  //  16/32/64
#define MMC_SDIO_INFO1_MASK         0x00E0  //  16/32/64
#define MMC_CC_EXT_MODE             0x0360  //  16/32/64
#define MMC_SOFT_RST                0x0380  //  16/32/64
#define MMC_VERSION                 0x0388  //  16/32/64
#define MMC_HOST_MODE               0x0390  //  16/32/64
#define MMC_SDIF_MODE               0x0398  //  16/32/64
#define MMC_SD_STATUS               0x03C8  //  16/32/64
#define MMC_DM_CM_SEQ_MODE          0x0808  //  16/32/64
#define MMC_DM_CM_DTRAN_MODE        0x0820  //  16/32/64
#define MMC_DM_CM_DTRAN_CTRL        0x0828  //  16/32/64
#define MMC_DM_CM_RST               0x0830  //  16/32/64
#define MMC_DM_CM_INFO1             0x0840  //  16/32/64
#define MMC_DM_CM_INFO1_MASK        0x0848  //  16/32/64
#define MMC_DM_CM_INFO2             0x0850  //  16/32/64
#define MMC_DM_CM_INFO2_MASK        0x0858  //  16/32/64
#define MMC_DM_DTRAN_ADDR           0x0880  //  16/32/64
#define MMC_SCC_DTCNTL              0x1000  //  32
#define MMC_SCC_TAPSET              0x1008  //  32
#define MMC_SCC_DT2FF               0x1010  //  32
#define MMC_SCC_CKSEL               0x1018  //  32
#define MMC_SCC_RVSCNTL             0x1020  //  32
#define MMC_SCC_RVSREQ              0x1028  //  32
#define MMC_SCC_TMPPORT2            0x1038  //  32
#define MMC_SCC_TMPPORT3            0x1048  //  32

/*  MMC Command Define */
#define MMC_CMD0        0x00000000  // GO_IDLE_STATE
#define MMC_CMD1        0x00000701  // SEND_OP_COND
#define MMC_CMD2        0x00000002  // ALL_SEND_CID
#define MMC_CMD3        0x00000003  // SET_RELATIVE_ADDR
#define MMC_CMD4        0x00000004  // SET_DSR
#define MMC_CMD5        0x00000505  //
#define MMC_CMD6        0x00000406  // SWITCH - Not in the response busystate
#define MMC_CMD7        0x00000007  // SELECT/DESELECT_CARD
#define MMC_CMD8        0x00001C08  // SEND_EXT_CSD
#define MMC_CMD9        0x00000009  // SEND_CSD
#define MMC_CMD10       0x0000000A  //
#define MMC_CMD12       0x0000000C  //
#define MMC_CMD13       0x0000000D  // SEND_STATUS
#define MMC_CMD14       0x00001C0E  //
#define MMC_CMD15       0x0000000F  //
#define MMC_CMD16       0x00000010  // SET_BLOCKLEN
#define MMC_CMD17       0x00000011  // READ_SINGLE_BLOCK
#define MMC_CMD18       0x00007C12  // READ_MULTI_BLOCK
#define MMC_CMD19       0x00000C13  //
#define MMC_CMD21       0x00001C15  //
#define MMC_CMD23       0x00000017  //
#define MMC_CMD24       0x00000018  // WRITE_BLOCK
#define MMC_CMD25       0x00006C19  //
#define MMC_CMD26       0x00000C1A  //
#define MMC_CMD27       0x0000001B  //
#define MMC_CMD28       0x0000001C  //
#define MMC_CMD29       0x0000001D  //
#define MMC_CMD30       0x0000001E  //
#define MMC_CMD31       0x00001C1F  //
#define MMC_CMD35       0x00000423  //
#define MMC_CMD36       0x00000424  //
#define MMC_CMD38       0x00000026  //
#define MMC_CMD39       0x00000427  //
#define MMC_CMD40       0x00000428  //
#define MMC_CMD42       0x0000002A  //
#define MMC_CMD49       0x00000C31  //
#define MMC_CMD53       0x00007C35  //
#define MMC_CMD54       0x00006C36  //
#define MMC_CMD55       0x00000037  //
#define MMC_CMD56       0x00000038  //

// Stop register
#define SDH_STOP_STP            (1 <<  0)
#define SDH_STOP_SEC            (1 <<  8)

// CLK_CTRL register
#define SDH_CLKCTRL_SCLKEN      (1 <<  8)

// INFO1
#define SDH_INFO1_WP            (1 <<  7)   // write protect
#define SDH_INFO1_CD            (1 <<  5)   // card detection state
#define SDH_INFO1_INST          (1 <<  4)   // card insertion
#define SDH_INFO1_RMVL          (1 <<  3)   // card removal
#define SDH_INFO1_AE            (1 <<  2)   // access end
#define SDH_INFO1_RE            (1 <<  0)   // response end

// INFO2
#define SDH_INFO2_HPIRES        (1 << 16)
#define SDH_INFO2_ILA           (1 << 15)   // illegal access error
#define SDH_INFO2_CBSY          (1 << 14)   // command response busy
#define SDH_INFO2_SCLKDIVEN     (1 << 13)   // SD bus busy
#define SDH_INFO2_BWE           (1 <<  9)   // SD_BUF Write Enable
#define SDH_INFO2_BRE           (1 <<  8)   // SD_BUF Read Enable
#define SDH_INFO2_RTO           (1 <<  6)   // Response Timeout
#define SDH_INFO2_BIRA          (1 <<  5)   // SD_BUF Illegal Read Access
#define SDH_INFO2_BIWA          (1 <<  4)   // SD_BUF Illegal Write Access
#define SDH_INFO2_DTO           (1 <<  3)   // Timeout (except response timeout)
#define SDH_INFO2_ENDE          (1 <<  2)   // END Error
#define SDH_INFO2_CRCE          (1 <<  1)   // CRC Error
#define SDH_INFO2_CMDE          (1 <<  0)   // CMD Error
#define SDH_INFO2_ALL_ERR       (SDH_INFO2_CMDE | SDH_INFO2_CRCE | SDH_INFO2_ENDE | SDH_INFO2_DTO |    \
                                                  SDH_INFO2_BIWA | SDH_INFO2_BIRA | SDH_INFO2_RTO)

// SD Card Access Control Option Register (SD_OPTION) bit defination
#define SDH_OPTION_EXTOP        (1 << 9)
#define SDH_OPTION_WIDTH_1      (1 << 15)   // Data Bus Width 1 bit
#define SDH_OPTION_WIDTH_8      (1 << 13)   // Data Bus Width 8 bit

/* SOFT_RST */
#define SOFT_RST_ON             (0 << 0)
#define SOFT_RST_OFF            (1<< 0)

/* CC_EXT_MODE */
#define BUF_ACC_DMAWEN          (1 << 1)

/* DM_CM_DTRAN_MODE */
#define CH_NUM_DOWNSTREAM       (0 << 16)
#define CH_NUM_UPSTREAM         (1 << 16)
#define BUS_WID_64BIT           (3 << 4)
#define FIXED_ADDRESS           (0 << 0)
#define INCREMENT_ADDRESS       (1 << 0)

/* DM_CM_DTRAN_CTRL */
#define DM_START                (1 << 0)
#define DM_INFO2_DTRAN_ERR0     (1 << 16)
#define DM_INFO2_DTRAN_ERR1     (1 << 17)
#define DM_INFO1_DTRAN_END0     (1 << 16)
#define DM_INFO1_DTRAN_END1     (1 << 17)

/* MMC_DM_CM_RST */	
#define DM_RST_DTRANRST0	    (1 << 8)
#define DM_RST_DTRANRST1	    (1 << 9)	
    
/* Tuning execution */

/* Definitions for values the RCAR_SDHI_SCC_DTCNTL register */
#define RCAR_SDHI_SCC_DTCNTL_TAPEN      (1 << 0)
/* Definitions for values the RCAR_SDHI_SCC_CKSEL register */
#define RCAR_SDHI_SCC_CKSEL_DTSEL       (1 << 0)
/* Definitions for values the RCAR_SDHI_SCC_RVSCNTL register */
#define RCAR_SDHI_SCC_RVSCNTL_RVSEN     (1 << 0)
/* Definitions for values the RCAR_SDHI_SCC_RVSREQ register */
#define RCAR_SDHI_SCC_RVSREQ_RVSERR     (1 << 2)

#define RCAR_SDHI_HAS_UHS_SCC       1

#define RCAR_SDHI_MAX_TAP           3

#define RCAR_SDHI_TUNING_TIMEOUT    150
#define RCAR_SDHI_TUNING_RETRIES    40

#define SDHI_TMOUT                  1000000

#define DMA_DESC_MAX                256

// Command register bits
#define SDH_CMD_AC12            (0 << 14)   // CMD12 is automatically issued
#define SDH_CMD_NOAC12          (1 << 14)
#define SDH_CMD_DAT_MULTI       (1 << 13)   // multi block transfer
#define SDH_CMD_DAT_READ        (1 << 12)   // read
#define SDH_CMD_ADTC            (1 << 11)   // command with data
#define SDH_CMD_NORSP           (3 <<  8)   // no response
#define SDH_CMD_RSPR1           (4 <<  8)   // R1, R5, R6, R7
#define SDH_CMD_RSPR1B          (5 <<  8)   // R1b
#define SDH_CMD_RSPR2           (6 <<  8)   // R2
#define SDH_CMD_RSPR3           (7 <<  8)   // R3, R4
#define SDH_CMD_ACMD            (6 <<  8)   // ACMD

typedef struct _mmcsd_scc_t {
    uint32_t    clk;    /* clock for SDR104 */
    uint32_t    tap;    /* sampling clock position for SDR104 */
} sdmmc_scc_t;

typedef struct _rcar_sdmmc_t {
    void            *bshdl;
    paddr_t         pbase;
    uint32_t        clock;      // MMC clock
    uint32_t        pclk;
    uint32_t        imask;
    uint32_t        datw;
    int             blksz;
    int             busclk;     // MMC bus clock
    uintptr_t       vbase;
    uint32_t        flags;
#define OF_DMA_ACTIVE     1

    sdio_cmd_t     *cmd;
    int             irq;
    int             dma_iid;
    int             dma_irq;
    struct sigevent dma_ev;
    int             cs;
    sdio_sge_t      sgl[DMA_DESC_MAX];
} rcar_sdmmc_t;

extern int rcar_sdmmc_init(sdio_hc_t *hc);
extern int rcar_sdmmc_dinit(sdio_hc_t *hc);


static inline uint32_t sdmmc_read(uintptr_t base, int reg)
{
    return in32(base + reg);
}

static inline void sdmmc_write(uintptr_t base, int reg, uint32_t val)
{
    out32(base + reg, val);
}

static inline void sdmmc_bitset(uintptr_t base, int reg, uint32_t val)
{
    out32(base + reg, val | in32(base + reg));
}

static inline void sdmmc_bitclr(uintptr_t base, int reg, uint32_t val)
{
    out32(base + reg, ~val & in32(base + reg));
}
#endif

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/hardware/devb/sdmmc/sdiodi/hc/rcar.h $ $Rev: 810496 $")
#endif
