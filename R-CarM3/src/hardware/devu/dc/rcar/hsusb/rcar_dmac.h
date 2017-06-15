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

#ifndef __QNX_HS_USB_DMAC_H
#define __QNX_HS_USB_DMAC_H

#define USBDMA_BASE1    0xE65A0000
#define USBDMA_BASE2    0xE65B0000

#define VCR             0x00
#define SWR             0x08
#define DMICR           0x10
#define SAR_0           0x20
#define DAR_0           0x24
#define TCR_0           0x28
#define TOCNTR_0        0x2C
#define TOCSTR_0        0x30
#define CHCR_0          0x34
#define TEND_0          0x38
#define SAR_1           0x40
#define DAR_1           0x44
#define TCR_1           0x48
#define TOCNTR_1        0x4C
#define TOCSTR_1        0x50
#define CHCR_1          0x54
#define TEND_1          0x58
#define DMAOR           0x60

#define USBHS_DMAC_SAR(channel)     ((channel) ? SAR_1 : SAR_0)
#define USBHS_DMAC_DAR(channel)     ((channel) ? DAR_1 : DAR_0)
#define USBHS_DMAC_TCR(channel)     ((channel) ? TCR_1 : TCR_0)
#define USBHS_DMAC_TOCNTR(channel)  ((channel) ? TOCNTR_1 : TOCNTR_0)
#define USBHS_DMAC_TOCSTR(channel)  ((channel) ? TOCSTR_1 : TOCSTR_0)
#define USBHS_DMAC_CHCR(channel)    ((channel) ? CHCR_1 : CHCR_0)
#define USBHS_DMAC_TEND(channel)    ((channel) ? TEND_1 : TEND_0)

/* VCR Register */
#define ERR_SNT     (1 << 1)    /* Send Error Response */
#define ERR_RCV     (1 << 0)    /* Receive Error Response */

/* SWR Register */
#define SWR_RST     1           /* Software Reset */

/* DMICR Register */
#define SH_BSY1     (1 << 31)   /* CH1 SHwy Bus Busy Flag Monitor */
#define SH_BSY0     (1 << 23)   /* CH0 SHwy Bus Busy Flag Monitor */
#define AEI         (1 << 16)   /* Address Error Int Src */
#define TR1         (1 << 14)   /* CH1 Tx End;Rx Int Src */
#define BUF1        (1 << 13)   /* CH1 Buffer End Detect Int Sc */
#define RW1         (1 << 12)   /* CH1 Final Buffer Access Detect Int Src */
#define NULL1       (1 << 11)   /* CH1 NULL Packet Rx Int Src */
#define TO1         (1 << 10)   /* CH1 Timeout Int Src */
#define SP1         (1 << 9)    /* CH1 Short Packet Rx Int Src */
#define TE1         (1 << 8)    /* Ch1 Tx End Int Src */
#define TR0         (1 << 6)    /* CH0 Tx End;Rx Int Src */
#define BUF0        (1 << 5)    /* CH0 Buffer End Detect Int Src */
#define RW0         (1 << 4)    /* CH0 Final Buffer Access Detect Int Src */
#define NULL0       (1 << 3)    /* CH0 NULL Packet Rx Int Src */
#define TO0         (1 << 2)    /* CH0 Timeout Int Src */
#define SP0         (1 << 1)    /* CH0 Short Packet Rx Int Src */
#define TE0         (1 << 0)    /* Ch0 Tx End Int Src */

#define USBHS_DMAC_DMICR_NULL(channel)  ((channel) ? NULL1 : NULL0)
#define USBHS_DMAC_DMICR_SP(channel)    ((channel) ? SP1 : SP0)
#define USBHS_DMAC_DMICR_TE(channel)    ((channel) ? TE1 : TE0)

/* CHCR Register */
#define FTE         (1 << 24)   /* Forced TE Set Register */
#define SPIM        (1 << 20)   /* Short Packet Rx Int Mask */
#define TRE         (1 << 19)   /* Transaction End Detect Int Flag Enable */
#define BUFE        (1 << 18)   /* Buffer End Detect Int Flag Enable */
#define RWE         (1 << 17)   /* Final Buffer Access Detect Int Flag Enable */
#define NULLE       (1 << 16)   /* NULL Rx Int Flag Enable */
#define TR          (1 << 15)   /* Transaction End Detect Int Flag */
#define BUF         (1 << 14)   /* Buffer End Detect Int Flag */
#define RW          (1 << 13)   /* Final Buffer Access Detect Int Flag */
#define NULLF       (1 << 12)   /* NULL Rx Int Flag */
#define PRI         (15 << 8)   /* Priority */
#define   PRI_0         (0 << 8)  /* Priority0 (lowest) */
#define   PRI_1         (1 << 8)  /* Priority1 */
#define   PRI_2         (2 << 8)  /* Priority2 */
#define   PRI_3         (3 << 8)  /* Priority3 */
#define   PRI_4         (4 << 8)  /* Priority4 */
#define   PRI_5         (5 << 8)  /* Priority5 */
#define   PRI_6         (6 << 8)  /* Priority6 */
#define   PRI_7         (7 << 8)  /* Priority7 */
#define   PRI_8         (8 << 8)  /* Priority8 */
#define   PRI_9         (9 << 8)  /* Priority9 */
#define   PRI_10        (10 << 8) /* Priority10 */
#define   PRI_11        (11 << 8) /* Priority11 */
#define   PRI_12        (12 << 8) /* Priority12 */
#define   PRI_13        (13 << 8) /* Priority13 */
#define   PRI_14        (14 << 8) /* Priority14 */
#define   PRI_15        (15 << 8) /* Priority15 (Highest) */
#define TS          (3 << 6)    /* MA Transfer Size */
#define   TS_8          (0 << 6)  /* 8Byte */
#define   TS_16         (1 << 6)  /* 16Byte */
#define   TS_32         (2 << 6)  /* 32Byte */
#define IE          (1 << 5)    /* Interrupt Enable */
#define TOE         (1 << 4)    /* Timeout Enable */
#define TO          (1 << 3)    /* Timeout Flag */
#define SP          (1 << 2)    /* Short Packet Receive Flag */
#define TE          (1 << 1)    /* Transfer End Flag */
#define DE          (1 << 0)    /* DMA Enable */

/* DMAOR Register */
#define TID1        (1 << 6)    /* Response Err Ch Identity Info */
#define TID0        (1 << 5)    /* Response Err Ch Identity Info */
#define RM          (1 << 4)    /* Response Err Mask Mode */
#define PR          (3 << 2)    /* Priority Mode */
#define   PR_01         (0 << 2)  /* CH0 > CH1 */
#define   PR_10         (1 << 2)  /* CH1 > CH0 */
#define   PR_11         (3 << 2)  /* Round-robin Mode */
#define AE          (1 << 1)    /* Address Error Flag */
#define DME         (1 << 0)    /* DMA Master Enable */

#endif /* __QNX_HS_USB_DMAC_H */

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/hardware/devu/dc/rcar/hsusb/rcar_dmac.h $ $Rev: 810496 $")
#endif
