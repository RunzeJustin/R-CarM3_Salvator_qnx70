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

#ifndef RAVB_H
#define RAVB_H

#include <io-pkt/iopkt_driver.h>
#include <sys/device.h>
#include <sys/io-pkt.h>
#include <sys/slog.h>
#include <sys/slogcodes.h>
#if (_NTO_VERSION < 660)
#include <drvr/nicsupport.h>
#else
#include <netdrvr/nicsupport.h>
#endif

#define AVB_REG_BASE                    0xE6800000
#define AVB_REG_SIZE                    0x0800
#define AVB_DMAC_IRQ                    (61 + 32)
#define AVB_EMAC_IRQ                    (63 + 32)

/* Driver's parameters */
#define NUM_TX_DESC                     128         // min 64 max 1024
#define NUM_RX_DESC                     128         // min 64 max 2048
#define PKT_BUF_SZ                      1518

/* Hardware time stamp */
#define RAVB_TXTSTAMP_VALID             0x00000001  /* TX timestamp valid */
#define RAVB_TXTSTAMP_ENABLED           0x00000010  /* Enable TX timestamping */

#define RAVB_RXTSTAMP_VALID             0x00000001  /* RX timestamp valid */
#define RAVB_RXTSTAMP_TYPE              0x00000006  /* RX type mask */
#define RAVB_RXTSTAMP_TYPE_V2_L2_EVENT  0x00000002
#define RAVB_RXTSTAMP_TYPE_ALL          0x00000006
#define RAVB_RXTSTAMP_ENABLED           0x00000010  /* Enable RX timestamping */

#define VERBOSE_TX          (1<<3)
#define VERBOSE_RX          (1<<2)
#define VERBOSE_PHY         (1<<1)

/* AVB-DMAC registers */
#define CCC                 0x0000      // AVB-DMAC Mode Register
    #define CCC_FCE             (1 << 25)       // Flow control enabled
    #define CCC_LBME            (1 << 24)       // Loopback mode is enabled
    #define CCC_BOC             (1 << 20)       // First Ethernet byte in URAM[31:24]; 0: First Ethernet byte in URAM[7:0]
    #define CCC_CSEL            (3 << 16)       // gPTP Clock Select
    #define CCC_CSEL_HPB        (1 << 16)       // High-speed peripheral bus clock
    #define CCC_DTSR            (1 << 8)        // Requests suspension; 0: Normal operation
    #define CCC_GAC             (1 << 7)        // gPTP support active in CONFIG mode; 0: Normal operation
    #define CCC_OPC             (3 << 0)        // Configuration Mode
    #define CCC_OPC_RESET       (0 << 0)        // Reset mode
    #define CCC_OPC_CONFIG      (1 << 0)        // Configuration mode
    #define CCC_OPC_OPERATION   (2 << 0)        // Operation mode
    #define CCC_OPC_STANDBY     (3 << 0)        // Standby mode
#define DBAT                0x0004      // Descriptor Base Address Table Register
#define DLR                 0x0008      // Descriptor Base Address Load Request Register
    /*  0: No load request is issued
        1: When written: A request for loading the corresponding base address is issued.
        When read: The given base address is being loaded */
    #define DLR_LBA21           (1 << 21)       // Base Address Load Request (Rx17: Stream 15)
    #define DLR_LBA20           (1 << 20)       // Base Address Load Request (Rx16: Stream 14)
    #define DLR_LBA19           (1 << 19)       // Base Address Load Request (Rx15: Stream 13)
    #define DLR_LBA18           (1 << 18)       // Base Address Load Request (Rx14: Stream 12)
    #define DLR_LBA17           (1 << 17)       // Base Address Load Request (Rx13: Stream 11)
    #define DLR_LBA16           (1 << 16)       // Base Address Load Request (Rx12: Stream 10)
    #define DLR_LBA15           (1 << 15)       // Base Address Load Request (Rx11: Stream 9)
    #define DLR_LBA14           (1 << 14)       // Base Address Load Request (Rx10: Stream 8)
    #define DLR_LBA13           (1 << 13)       // Base Address Load Request (Rx9: Stream 7)
    #define DLR_LBA12           (1 << 12)       // Base Address Load Request (Rx8: Stream 6)
    #define DLR_LBA11           (1 << 11)       // Base Address Load Request (Rx7: Stream 5)
    #define DLR_LBA10           (1 << 10)       // Base Address Load Request (Rx6: Stream 4)
    #define DLR_LBA9            (1 << 9)        // Base Address Load Request (Rx5: Stream 3)
    #define DLR_LBA8            (1 << 8)        // Base Address Load Request (Rx4: Stream 2)
    #define DLR_LBA7            (1 << 7)        // Base Address Load Request (Rx3: Stream 1)
    #define DLR_LBA6            (1 << 6)        // Base Address Load Request (Rx2: Stream 0)
    #define DLR_LBA5            (1 << 5)        // Base Address Load Request (Rx1: Network Control)
    #define DLR_LBA4            (1 << 4)        // Base Address Load Request (Rx0: Best Effort)
    #define DLR_LBA3            (1 << 3)        // Base Address Load Request (Tx3: Stream Class A)
    #define DLR_LBA2            (1 << 2)        // Base Address Load Request (Tx2: Stream Class B)
    #define DLR_LBA1            (1 << 1)        // Base Address Load Request (Tx1: Network Control)
    #define DLR_LBA0            (1 << 0)        // Base Address Load Request (Tx0: Best Effort)
#define CSR                 0x000C      // AVB-DMAC Status Register
    #define CSR_TDUO            (1 << 21)       // Pending descriptor update in URAM; 0: No pending descriptor update
    #define CSR_RPO             (1 << 20)       // Reception is in progress; 0: Normal operation
    #define CSR_TPO_M           (0xF << 16)
    #define CSR_TPO3            (1 << 19)       // Transmit Process Status 3 (Stream Class A)
    #define CSR_TPO2            (1 << 18)       // Transmit Process Status 2 (Stream Class B)
    #define CSR_TPO1            (1 << 17)       // Transmit Process Status 1 (Network Control)
    #define CSR_TPO0            (1 << 16)       // Transmit Process Status 0 (Best Effort)
    #define CSR_DTS             (1 << 8)        // Data Transmission Suspended Status
    #define CSR_OPS             (0xF << 0)      //
    #define CSR_OPS_RESET       (1 << 0)        // Reset mode
    #define CSR_OPS_CONFIG      (2 << 0)        // Configuration mode
    #define CSR_OPS_OPERATION   (4 << 0)        // Operation mode
    #define CSR_OPS_STANDBY     (8 << 0)        // standby mode
#define CDAR0               0x0010      // Current Descriptor Address Register
#define CDAR1               0x0014
#define CDAR2               0x0018
#define CDAR3               0x001C
#define CDAR4               0x0020
#define CDAR5               0x0024
#define CDAR6               0x0028
#define CDAR7               0x002C
#define CDAR8               0x0030
#define CDAR9               0x0034
#define CDAR10              0x0038
#define CDAR11              0x003C
#define CDAR12              0x0040
#define CDAR13              0x0044
#define CDAR14              0x0048
#define CDAR15              0x004C
#define CDAR16              0x0050
#define CDAR17              0x0054
#define CDAR18              0x0058
#define CDAR19              0x005C
#define CDAR20              0x0060
#define CDAR21              0x0064
#define ESR                 0x0088      // Error Status Register
    #define ESR_EIL             (1 << 12)       // Lost of error information detected
    #define ESR_ET_0            (0 << 8)        // Read descriptor from URAM
    #define ESR_ET_1            (1 << 8)        // Write descriptor to URAM
    #define ESR_ET_2            (2 << 8)        // Interpret data descriptor
    #define ESR_ET_3            (3 << 8)        // Tx-buffer is corrupted
    #define ESR_ET_4            (4 << 8)        // Read data from URAM
    #define ESR_ET_5            (5 << 8)        // Write data or timestamp to URAM
    #define ESR_ET_6            (6 << 8)        // Reading from Rx-FIFO
    #define ESR_ET_7            (7 << 8)        // Rx-FIFO is corrupted
    #define ESR_ET_8            (8 << 8)        // Frame size error during reception detected
    #define ESR_ET_9            (9 << 8)        // Frame size error during transmission detected
    #define ESR_ET_10           (10 << 8)       // Tx-buffer overflow
    #define ESR_ET_11           (11 << 8)       // AVTP FIFO error
    #define ESR_EQN             (0x1F << 0)     // Error Queue Number
#define APSR                0x008C      // AVB-DMAC Product Specific Register
    #define APSR_CMSW           (1 << 4)        // 4clock period pulse; 0: 1clock period pulse
#define RCR                 0x0090      // Receive Configuration Register
    #define RCR_RFCL_M          (0x1FFF << 16)  // Receive FIFO Caution Level, Recommended value: 1800H
    #define RCR_SBFS            (1 << 7)        // Storage of broadcast frames disabled; 0: enabled
    #define RCR_ETS2            (1 << 5)        // Time stamping is enabled; 0: disabled; Recommended value: 0
    #define RCR_ETS0            (1 << 4)        // Enables the inclusion of time-stamp information in reception queue 0; ETS2: queue 2 to 17
    #define RCR_ESF_M           (3 << 2)        // Stream Filtering Select
    #define RCR_ESF_0           (0 << 2)        // Filtering is disabled
    #define RCR_ESF_1           (1 << 2)        // The filter for both AVB stream frames and non-AVB stream frames is enabled
    #define RCR_ESF_2           (2 << 2)        // The filter for separating AVB stream frames from non-AVB stream frames is enabled
    #define RCR_ESF_3           (3 << 2)        // The filter for separating AVB stream frames from non-AVB stream frames is enabled
    #define RCR_ENCF            (1 << 1)        // Network Control Filtering Enable
    #define RCR_EFFS            (1 << 0)        // Error Frame Enable
#define RQC0                0x0094      // Receive Queue Configuration Register
#define RQC1                0x0098
#define RQC2                0x009C
#define RQC3                0x00A0
#define RQC4                0x00A4
    #define RQC_PIA3            (1 << 30)       // No gaps between frame data in incremental data area; 0: Frame data starts always 32 bit aligned
    #define RQC_UFCC3_M         (3 << 28)       // Unread Frame Counter Configuration
    #define RQC_TSEL3_M         (3 << 26)       // Truncation SELection
    #define RQC_TSEL3_00        (0 << 26)       // Maximum frame length defined by RTC0.MFL0
    #define RQC_TSEL3_01        (1 << 26)       // Maximum frame length defined by RTC0.MFL1
    #define RQC_RSM3_M          (3 << 24)       // Receive Synchronous Mode
    #define RQC_RSM3_00         (0 << 24)       // Mode with write-back
    #define RQC_RSM3_01         (1 << 24)       // Keep DT mode (no update of DT field at descriptor write back)
    #define RQC_RSM3_10         (2 << 24)       // No write-back (no descriptor write back)
    #define RQC_PIA2            (1 << 22)       // No gaps between frame data in incremental data area; 0: Frame data starts always 32 bit aligned
    #define RQC_UFCC2_M         (3 << 20)       // Unread Frame Counter Configuration
    #define RQC_TSEL2_M         (3 << 18)       // Truncation SELection
    #define RQC_TSEL2_00        (0 << 18)       // Maximum frame length defined by RTC0.MFL0
    #define RQC_TSEL2_01        (1 << 18)       // Maximum frame length defined by RTC0.MFL1
    #define RQC_RSM2_M          (3 << 16)       // Receive Synchronous Mode
    #define RQC_RSM2_00         (0 << 16)       // Mode with write-back
    #define RQC_RSM2_01         (1 << 16)       // Keep DT mode (no update of DT field at descriptor write back)
    #define RQC_RSM2_10         (2 << 16)       // No write-back (no descriptor write back)
    #define RQC_PIA1            (1 << 14)       // No gaps between frame data in incremental data area; 0: Frame data starts always 32 bit aligned
    #define RQC_UFCC1_M         (3 << 12)       // Unread Frame Counter Configuration
    #define RQC_TSEL1_M         (3 << 10)       // Truncation SELection
    #define RQC_TSEL1_00        (0 << 10)       // Maximum frame length defined by RTC0.MFL0
    #define RQC_TSEL1_01        (1 << 10)       // Maximum frame length defined by RTC0.MFL1
    #define RQC_RSM1_M          (3 << 8)        // Receive Synchronous Mode
    #define RQC_RSM1_00         (0 << 8)        // Mode with write-back
    #define RQC_RSM1_01         (1 << 8)        // Keep DT mode (no update of DT field at descriptor write back)
    #define RQC_RSM1_10         (2 << 8)        // No write-back (no descriptor write back)
    #define RQC_PIA0            (1 << 6)        // No gaps between frame data in incremental data area; 0: Frame data starts always 32 bit aligned
    #define RQC_UFCC0_M         (3 << 4)        // Unread Frame Counter Configuration
    #define RQC_TSEL0_M         (3 << 2)        // Truncation SELection
    #define RQC_TSEL0_00        (0 << 2)        // Maximum frame length defined by RTC0.MFL0
    #define RQC_TSEL0_01        (1 << 2)        // Maximum frame length defined by RTC0.MFL1
    #define RQC_RSM0_M          (3 << 0)        // Receive Synchronous Mode
    #define RQC_RSM0_00         (0 << 0)        // Mode with write-back
    #define RQC_RSM0_01         (1 << 0)        // Keep DT mode (no update of DT field at descriptor write back)
    #define RQC_RSM0_10         (2 << 0)        // No write-back (no descriptor write back)
#define RPC                 0x00B0      // Receive Padding Configuration Register
    #define RPC_DCNT_M          (0xFF << 16)    // Stored data counter
    #define RPC_PCNT_M          (0x7 << 8)      // Stored padding counter
#define RTC                 0x00B4      // Reception Truncation Configuration register
    #define RTC_MFL1_M          (0xFFF << 16)   // Maximum frame length 1
    #define RTC_MFL0_M          (0xFFF << 0)    // Maximum frame length 0
#define UFCW                0x00BC      // Unread Frame Counter Warning Level Configuration Register
    #define UFCW_WL3_M          (0x3F << 24)    // Unread frame count warning level 3
    #define UFCW_WL2_M          (0x3F << 16)    // Unread frame count warning level 2
    #define UFCW_WL1_M          (0x3F << 8)     // Unread frame count warning level 1
    #define UFCW_WL0_M          (0x3F << 0)     // Unread frame count warning level 0
#define UFCS                0x00C0      // Unread Frame Counter Stop Level Configuration Register
    #define UFCS_SL3_M          (0x3F << 24)    // Unread frame count stop level 3
    #define UFCS_SL2_M          (0x3F << 16)    // Unread frame count stop level 2
    #define UFCS_SL1_M          (0x3F << 8)     // Unread frame count stop level 1
    #define UFCS_SL0_M          (0x3F << 0)     // Unread frame count stop level 0
#define UFCV0               0x00C4      // Unread Frame Counter Register
#define UFCV1               0x00C8
#define UFCV2               0x00CC
#define UFCV3               0x00D0
#define UFCV4               0x00D4
    /*  The UFCV0 register indicates the number of unread frames in reception queues 0 to 3.
        The UFCV1 register indicates the number of unread frames in reception queues 4 to 7.
        The UFCV2 register indicates the number of unread frames in reception queues 8 to 11.
        The UFCV3 register indicates the number of unread frames in reception queues 12 to 15.
        The UFCV4 register indicates the number of unread frames in reception queues 16 and 17. */
    #define UFCV_CV3_M          (0x3F << 24)    // Number of unread frames in reception queue 3+4×i
    #define UFCV_CV2_M          (0x3F << 16)    // Number of unread frames in reception queue 2+4×i
    #define UFCV_CV1_M          (0x3F << 8)     // Number of unread frames in reception queue 1+4×i
    #define UFCV_CV0_M          (0x3F << 0)     // Number of unread frames in reception queue 0+4×i
#define UFCD0               0x00E0      // Unread Frame Counter Decrement Register
#define UFCD1               0x00E4
#define UFCD2               0x00E8
#define UFCD3               0x00EC
#define UFCD4               0x00F0
    /*  The UFCD0 register is used to decrement unread counters in reception queues 0 to 3.
        The UFCD1 register is used to decrement unread counters in reception queues 4 to 7.
        The UFCD2 register is used to decrement unread counters in reception queues 8 to 11.
        The UFCD3 register is used to decrement unread counters in reception queues 12 to 15.
        The UFCD4 register is used to decrement unread counters in reception queues 16 and 17. */
    #define UFCD_DV3_M          (0x3F << 24)    // Unread frame decrement value for reception queue 3+4×i
    #define UFCD_DV2_M          (0x3F << 16)    // Unread frame decrement value for reception queue 2+4×i
    #define UFCD_DV1_M          (0x3F << 8)     // Unread frame decrement value for reception queue 1+4×i
    #define UFCD_DV0_M          (0x3F << 0)     // Unread frame decrement value for reception queue 0+4×i
#define SFO                 0x00FC      // Separation Filter Offset Register
#define SFP0                0x0100      // Separation Filter Pattern Register
#define SFP1                0x0104
#define SFP2                0x0108
#define SFP3                0x010C
#define SFP4                0x0110
#define SFP5                0x0114
#define SFP6                0x0118
#define SFP7                0x011C
#define SFP8                0x0120
#define SFP9                0x0124
#define SFP10               0x0128
#define SFP11               0x012C
#define SFP12               0x0130
#define SFP13               0x0134
#define SFP14               0x0138
#define SFP15               0x013C
#define SFP16               0x0140
#define SFP17               0x0144
#define SFP18               0x0148
#define SFP19               0x014C
#define SFP20               0x0150
#define SFP21               0x0154
#define SFP22               0x0158
#define SFP23               0x015C
#define SFP24               0x0160
#define SFP25               0x0164
#define SFP26               0x0168
#define SFP27               0x016C
#define SFP28               0x0170
#define SFP29               0x0174
#define SFP30               0x0178
#define SFP31               0x017C
#define SFV0                0x01B8      // Separation Filter Value register
#define SFV1                0x01BC
#define SFM0                0x01C0      // Separation Filter Mask Register
#define SFM1                0x01C4
#define SFL                 0x01C8      // Separation Filter Load register
#define PCRC                0x01CC      // Payload CRC register
#define CIAR0               0x0200      // Current Incremental Address
#define CIAR1               0x0204
#define CIAR2               0x0208
#define CIAR3               0x020C
#define CIAR4               0x0210
#define CIAR5               0x0214
#define CIAR6               0x0218
#define CIAR7               0x021C
#define CIAR8               0x0220
#define CIAR9               0x0224
#define CIAR10              0x0228
#define CIAR11              0x022C
#define CIAR12              0x0230
#define CIAR13              0x0234
#define CIAR14              0x0238
#define CIAR15              0x023C
#define CIAR16              0x0240
#define CIAR17              0x0244
#define LIAR0               0x0280      // Last Incremental Address Register
#define LIAR1               0x0284
#define LIAR2               0x0288
#define LIAR3               0x028C
#define LIAR4               0x0290
#define LIAR5               0x0294
#define LIAR6               0x0298
#define LIAR7               0x029C
#define LIAR8               0x02A0
#define LIAR9               0x02A4
#define LIAR10              0x02A8
#define LIAR11              0x02AC
#define LIAR12              0x02B0
#define LIAR13              0x02B4
#define LIAR14              0x02B8
#define LIAR15              0x02BC
#define LIAR16              0x02C0
#define LIAR17              0x02C4
#define TGC                 0x0300      // Transmit Configuration Register
    #define TGC_TBD3_M          (3 << 20)       // Transmit FIFO Size (Stream Class A), Write 2 to these bits
    #define TGC_TBD2_M          (3 << 16)       // Transmit FIFO Size (Stream Class B), Write 2 to these bits
    #define TGC_TBD1_M          (3 << 12)       // Transmit FIFO Size (Network Control), Write 2 to these bits
    #define TGC_TBD0_M          (3 << 8)        // Transmit FIFO Size (Best Effort), Write 2 to these bits
    #define TGC_TQP_M           (3 << 4)        // Transmit Queue Priority
    #define TGC_TQP_NONAVB      (0 << 4)        // Non-ABV mode
    #define TGC_TQP_AVBMODE1    (1 << 4)        // AVB mode 1
    #define TGC_TQP_AVBMODE2    (3 << 4)        // AVB mode 2
    #define TGC_TSM3_M          (1 << 3)        // Transmit Synchronous Mode (Stream Class A)
    #define TGC_TSM2_M          (1 << 2)        // Transmit Synchronous Mode (Stream Class B)
    #define TGC_TSM1_M          (1 << 1)        // Transmit Synchronous Mode (Network Control)
    #define TGC_TSM0_M          (1 << 0)        // Transmit Synchronous Mode (Best Effort)
#define TCCR                0x0304      // Transmit Configuration Control Register
    /*  TSRQ
        Only 1 can be written to the bit. Writing 0 to the bit has no effect.
        0: Transmission queue is empty or stopped.
        1: When written: A transmission start request is issued.
        When read: Fetching of data for transmission is pending.*/
    #define TCCR_MFR            (1 << 17)       // Release oldest entry of E-MAC status FIFO; 0: No request to E-MAC status FIFO
    #define TCCR_MFEN           (1 << 16)       // E-MAC status FIFO ENable
    #define TCCR_TFR            (1 << 9)        // Releases the oldest entry in the time-stamp FIFO; 0: (Not operating)
    #define TCCR_TFEN           (1 << 8)        // Recording of transmission time stamps in the time-stamp FIFO is enabled
    #define TCCR_TSRQ3          (1 << 3)        // Transmit Start Request (Queue 3 (Stream Class A))
    #define TCCR_TSRQ2          (1 << 2)        // Transmit Start Request (Queue 2 (Stream Class B))
    #define TCCR_TSRQ1          (1 << 1)        // Transmit Start Request (Queue 1 (Network Control))
    #define TCCR_TSRQ0          (1 << 0)        // Transmit Start Request (Queue 0 (Best Effort))
#define TSR                 0x0308      // Transmit Status Register
    #define TSR_CCS1_00         (0 << 2)        // The current credit value is within the limit
    #define TSR_CCS1_01         (1 << 2)        // The current credit value is less than or equal to the lower limit
    #define TSR_CCS1_10         (2 << 2)        // The current credit value is greater than or equal to the upper limit
    #define TSR_CCS0_00         (0 << 0)        // The current credit value is within the limit
    #define TSR_CCS0_01         (1 << 0)        // The current credit value is less than or equal to the lower limit
    #define TSR_CCS0_10         (2 << 0)        // The current credit value is greater than or equal to the upper limit
#define MFA                 0x030C      // E-MAC status FIFO Access register
#define TFA0                0x0310      // Time Stamp FIFO Access Register
#define TFA1                0x0314
#define TFA2                0x0318
#define CIVR0               0x0320      // CBS Increment Value Register
#define CIVR1               0x0324
#define CDVR0               0x0328      // CBS Decrement Value Register
#define CDVR1               0x032C
#define CUL0                0x0330      // CBS Upper Limit Register
#define CUL1                0x0334
#define CLL0                0x0338      // CBS Lower Limit Register
#define CLL1                0x033C
#define DIC                 0x0350      // Descriptor Interrupt Control Register
    #define DIC_DPE15           (1 << 15)       // Descriptor Interrupt Enable 15
    #define DIC_DPE14           (1 << 14)       // Descriptor Interrupt Enable 14
    #define DIC_DPE13           (1 << 13)       // Descriptor Interrupt Enable 13
    #define DIC_DPE12           (1 << 12)       // Descriptor Interrupt Enable 12
    #define DIC_DPE11           (1 << 11)       // Descriptor Interrupt Enable 11
    #define DIC_DPE10           (1 << 10)       // Descriptor Interrupt Enable 10
    #define DIC_DPE9            (1 << 9)        // Descriptor Interrupt Enable 9
    #define DIC_DPE8            (1 << 8)        // Descriptor Interrupt Enable 8
    #define DIC_DPE7            (1 << 7)        // Descriptor Interrupt Enable 7
    #define DIC_DPE6            (1 << 6)        // Descriptor Interrupt Enable 6
    #define DIC_DPE5            (1 << 5)        // Descriptor Interrupt Enable 5
    #define DIC_DPE4            (1 << 4)        // Descriptor Interrupt Enable 4
    #define DIC_DPE3            (1 << 3)        // Descriptor Interrupt Enable 3
    #define DIC_DPE2            (1 << 2)        // Descriptor Interrupt Enable 2
    #define DIC_DPE1            (1 << 1)        // Descriptor Interrupt Enable 1
#define DIS                 0x0354      // Descriptor Interrupt Status Register
    #define DIS_DPF15           (1 << 15)       // Descriptor Interrupt Status 15
    #define DIS_DPF14           (1 << 14)       // Descriptor Interrupt Status 14
    #define DIS_DPF13           (1 << 13)       // Descriptor Interrupt Status 13
    #define DIS_DPF12           (1 << 12)       // Descriptor Interrupt Status 12
    #define DIS_DPF11           (1 << 11)       // Descriptor Interrupt Status 11
    #define DIS_DPF10           (1 << 10)       // Descriptor Interrupt Status 10
    #define DIS_DPF9            (1 << 9)        // Descriptor Interrupt Status 9
    #define DIS_DPF8            (1 << 8)        // Descriptor Interrupt Status 8
    #define DIS_DPF7            (1 << 7)        // Descriptor Interrupt Status 7
    #define DIS_DPF6            (1 << 6)        // Descriptor Interrupt Status 6
    #define DIS_DPF5            (1 << 5)        // Descriptor Interrupt Status 5
    #define DIS_DPF4            (1 << 4)        // Descriptor Interrupt Status 4
    #define DIS_DPF3            (1 << 3)        // Descriptor Interrupt Status 3
    #define DIS_DPF2            (1 << 2)        // Descriptor Interrupt Status 2
    #define DIS_DPF1            (1 << 1)        // Descriptor Interrupt Status 1
#define EIC                 0x0358      // Error Interrupt Control Register
    #define EIC_TBFE            (1 << 10)       // Tx-buffer full interrupt enable
    #define EIC_MFFE            (1 << 9)        // Mac status FIFO Full interrupt Enable
    #define EIC_TFFE            (1 << 8)        // Time Stamp FIFO Full-Error Interrupt Enable
    #define EIC_CULE1           (1 << 7)        // CBS Upper Limit Error Interrupt Enable (Class A)
    #define EIC_CULE0           (1 << 6)        // CBS Upper Limit Error Interrupt Enable (Class B)
    #define EIC_CLLE1           (1 << 5)        // CBS Lower Limit Error Interrupt Enable (Class A)
    #define EIC_CLLE0           (1 << 4)        // CBS Lower Limit Error Interrupt Enable (Class B)
    #define EIC_SEE             (1 << 3)        // Separation Error Interrupt Enable
    #define EIC_QEE             (1 << 2)        // Queue Error Interrupt Enable
    #define EIC_MTEE            (1 << 1)        // E-MAC Transmission Error Interrupt Enable
    #define EIC_MREE            (1 << 0)        // E-MAC Reception Error Interrupt Enable
#define EIS                 0x035C      // Error Interrupt Status Register
    #define EIS_QFS             (1 << 16)       // Queue Full Error Interrupt Status
    #define EIS_TBFF            (1 << 10)       // Tx-Buffer full condition detected; 0: No interrupt pending
    #define EIS_MFFF            (1 << 9)        // E-MAC status FIFO full, oldest E-MAC status lost; 0: No interrupt pending
    #define EIS_TFFF            (1 << 8)        // Time Stamp FIFO Full Error Interrupt Status
    #define EIS_CULF1           (1 << 7)        // CBS Upper Limit Error Interrupt Status (Class A)
    #define EIS_CULF0           (1 << 6)        // CBS Upper Limit Error Interrupt Status (Class B)
    #define EIS_CLLF1           (1 << 5)        // CBS Lower Limit Error Interrupt Status (Class A)
    #define EIS_CLLF0           (1 << 4)        // CBS Lower Limit Error Interrupt Status (Class B)
    #define EIS_SEF             (1 << 3)        // Separation Error Flag
    #define EIS_QEF             (1 << 2)        // Queue Error Flag
    #define EIS_MTEF            (1 << 1)        // E-MAC Transmission Error Flag
    #define EIS_MREF            (1 << 0)        // E-MAC Reception Error Flag
#define RIC0                0x0360      // Receive Interrupt Control Register
    #define RIC0_FRE17          (1 << 17)       // Receive Frame Enable 17 (Stream)
    #define RIC0_FRE16          (1 << 16)       // Receive Frame Enable 16 (Stream)
    #define RIC0_FRE15          (1 << 15)       // Receive Frame Enable 15 (Stream)
    #define RIC0_FRE14          (1 << 14)       // Receive Frame Enable 14 (Stream)
    #define RIC0_FRE13          (1 << 13)       // Receive Frame Enable 13 (Stream)
    #define RIC0_FRE12          (1 << 12)       // Receive Frame Enable 12 (Stream)
    #define RIC0_FRE11          (1 << 11)       // Receive Frame Enable 11 (Stream)
    #define RIC0_FRE10          (1 << 10)       // Receive Frame Enable 10 (Stream)
    #define RIC0_FRE9           (1 << 9)        // Receive Frame Enable 9 (Stream)
    #define RIC0_FRE8           (1 << 8)        // Receive Frame Enable 8 (Stream)
    #define RIC0_FRE7           (1 << 7)        // Receive Frame Enable 7 (Stream)
    #define RIC0_FRE6           (1 << 6)        // Receive Frame Enable 6 (Stream)
    #define RIC0_FRE5           (1 << 5)        // Receive Frame Enable 5 (Stream)
    #define RIC0_FRE4           (1 << 4)        // Receive Frame Enable 4 (Stream)
    #define RIC0_FRE3           (1 << 3)        // Receive Frame Enable 3 (Stream)
    #define RIC0_FRE2           (1 << 2)        // Receive Frame Enable 2 (Stream)
    #define RIC0_FRE1           (1 << 1)        // Receive Frame Enable 1 (Stream)
    #define RIC0_FRE0           (1 << 0)        // Receive Frame Enable 0 (Stream)
#define RIS0                0x0364      // Receive Interrupt Status Register
    #define RIS0_FRF17          (1 << 17)       // Receive Frame Interrupt Status 17 (Stream)
    #define RIS0_FRF16          (1 << 16)       // Receive Frame Interrupt Status 16 (Stream)
    #define RIS0_FRF15          (1 << 15)       // Receive Frame Interrupt Status 15 (Stream)
    #define RIS0_FRF14          (1 << 14)       // Receive Frame Interrupt Status 14 (Stream)
    #define RIS0_FRF13          (1 << 13)       // Receive Frame Interrupt Status 13 (Stream)
    #define RIS0_FRF12          (1 << 12)       // Receive Frame Interrupt Status 12 (Stream)
    #define RIS0_FRF11          (1 << 11)       // Receive Frame Interrupt Status 11 (Stream)
    #define RIS0_FRF10          (1 << 10)       // Receive Frame Interrupt Status 10 (Stream)
    #define RIS0_FRF9           (1 << 9)        // Receive Frame Interrupt Status 9 (Stream)
    #define RIS0_FRF8           (1 << 8)        // Receive Frame Interrupt Status 8 (Stream)
    #define RIS0_FRF7           (1 << 7)        // Receive Frame Interrupt Status 7 (Stream)
    #define RIS0_FRF6           (1 << 6)        // Receive Frame Interrupt Status 6 (Stream)
    #define RIS0_FRF5           (1 << 5)        // Receive Frame Interrupt Status 5 (Stream)
    #define RIS0_FRF4           (1 << 4)        // Receive Frame Interrupt Status 4 (Stream)
    #define RIS0_FRF3           (1 << 3)        // Receive Frame Interrupt Status 3 (Stream)
    #define RIS0_FRF2           (1 << 2)        // Receive Frame Interrupt Status 2 (Stream)
    #define RIS0_FRF1           (1 << 1)        // Receive Frame Interrupt Status 1 (Stream)
    #define RIS0_FRF0           (1 << 0)        // Receive Frame Interrupt Status 0 (Stream)
#define RIC1                0x0368
    #define RIC1_RFWE           (1 << 31)       // Receive FIFO Warning Interrupt Enable
    #define RIC1_RWE17          (1 << 17)       // Reception Warning interrupt Enable 17
    #define RIC1_RWE16          (1 << 16)       // Reception Warning interrupt Enable 16
    #define RIC1_RWE15          (1 << 15)       // Reception Warning interrupt Enable 15
    #define RIC1_RWE14          (1 << 14)       // Reception Warning interrupt Enable 14
    #define RIC1_RWE13          (1 << 13)       // Reception Warning interrupt Enable 13
    #define RIC1_RWE12          (1 << 12)       // Reception Warning interrupt Enable 12
    #define RIC1_RWE11          (1 << 11)       // Reception Warning interrupt Enable 11
    #define RIC1_RWE10          (1 << 10)       // Reception Warning interrupt Enable 10
    #define RIC1_RWE9           (1 << 9)        // Reception Warning interrupt Enable 9
    #define RIC1_RWE8           (1 << 8)        // Reception Warning interrupt Enable 8
    #define RIC1_RWE7           (1 << 7)        // Reception Warning interrupt Enable 7
    #define RIC1_RWE6           (1 << 6)        // Reception Warning interrupt Enable 6
    #define RIC1_RWE5           (1 << 5)        // Reception Warning interrupt Enable 5
    #define RIC1_RWE4           (1 << 4)        // Reception Warning interrupt Enable 4
    #define RIC1_RWE3           (1 << 3)        // Reception Warning interrupt Enable 3
    #define RIC1_RWE2           (1 << 2)        // Reception Warning interrupt Enable 2
    #define RIC1_RWE1           (1 << 1)        // Reception Warning interrupt Enable 1
    #define RIC1_RWE0           (1 << 0)        // Reception Warning interrupt Enable 0
#define RIS1                0x036C
    #define RIS1_RFWF           (1 << 31)       // Receive FIFO Warning Interrupt Status
    #define RIS1_RWF17          (1 << 17)       // Reception Warning Flag 17
    #define RIS1_RWF16          (1 << 16)       // Reception Warning Flag 16
    #define RIS1_RWF15          (1 << 15)       // Reception Warning Flag 15
    #define RIS1_RWF14          (1 << 14)       // Reception Warning Flag 14
    #define RIS1_RWF13          (1 << 13)       // Reception Warning Flag 13
    #define RIS1_RWF12          (1 << 12)       // Reception Warning Flag 12
    #define RIS1_RWF11          (1 << 11)       // Reception Warning Flag 11
    #define RIS1_RWF10          (1 << 10)       // Reception Warning Flag 10
    #define RIS1_RWF9           (1 << 9)        // Reception Warning Flag 9
    #define RIS1_RWF8           (1 << 8)        // Reception Warning Flag 8
    #define RIS1_RWF7           (1 << 7)        // Reception Warning Flag 7
    #define RIS1_RWF6           (1 << 6)        // Reception Warning Flag 6
    #define RIS1_RWF5           (1 << 5)        // Reception Warning Flag 5
    #define RIS1_RWF4           (1 << 4)        // Reception Warning Flag 4
    #define RIS1_RWF3           (1 << 3)        // Reception Warning Flag 3
    #define RIS1_RWF2           (1 << 2)        // Reception Warning Flag 2
    #define RIS1_RWF1           (1 << 1)        // Reception Warning Flag 1
    #define RIS1_RWF0           (1 << 0)        // Reception Warning Flag 0
#define RIC2                0x0370
    #define RIC2_RFFE           (1 << 31)       // Receive FIFO Full Interrupt Enable
    #define RIC2_QFE17          (1 << 17)       // Receive Queue 17 (Stream) Full Interrupt Enable
    #define RIC2_QFE16          (1 << 16)       // Receive Queue 16 (Stream) Full Interrupt Enable
    #define RIC2_QFE15          (1 << 15)       // Receive Queue 15 (Stream) Full Interrupt Enable
    #define RIC2_QFE14          (1 << 14)       // Receive Queue 14 (Stream) Full Interrupt Enable
    #define RIC2_QFE13          (1 << 13)       // Receive Queue 13 (Stream) Full Interrupt Enable
    #define RIC2_QFE12          (1 << 12)       // Receive Queue 12 (Stream) Full Interrupt Enable
    #define RIC2_QFE11          (1 << 11)       // Receive Queue 11 (Stream) Full Interrupt Enable
    #define RIC2_QFE10          (1 << 10)       // Receive Queue 10 (Stream) Full Interrupt Enable
    #define RIC2_QFE9           (1 << 9)        // Receive Queue 9 (Stream) Full Interrupt Enable
    #define RIC2_QFE8           (1 << 8)        // Receive Queue 8 (Stream) Full Interrupt Enable
    #define RIC2_QFE7           (1 << 7)        // Receive Queue 7 (Stream) Full Interrupt Enable
    #define RIC2_QFE6           (1 << 6)        // Receive Queue 6 (Stream) Full Interrupt Enable
    #define RIC2_QFE5           (1 << 5)        // Receive Queue 5 (Stream) Full Interrupt Enable
    #define RIC2_QFE4           (1 << 4)        // Receive Queue 4 (Stream) Full Interrupt Enable
    #define RIC2_QFE3           (1 << 3)        // Receive Queue 3 (Stream) Full Interrupt Enable
    #define RIC2_QFE2           (1 << 2)        // Receive Queue 2 (Stream) Full Interrupt Enable
    #define RIC2_QFE1           (1 << 1)        // Receive Queue 1 (Stream) Full Interrupt Enable
    #define RIC2_QFE0           (1 << 0)        // Receive Queue 0 (Stream) Full Interrupt Enable
#define RIS2                0x0374
    #define RIS2_RFFF           (1 << 31)       // Receive FIFO Full Interrupt Status
    #define RIS2_QFF17          (1 << 17)       // Receive Queue 17 (Stream) Full Interrupt Status
    #define RIS2_QFF16          (1 << 16)       // Receive Queue 16 (Stream) Full Interrupt Status
    #define RIS2_QFF15          (1 << 15)       // Receive Queue 15 (Stream) Full Interrupt Status
    #define RIS2_QFF14          (1 << 14)       // Receive Queue 14 (Stream) Full Interrupt Status
    #define RIS2_QFF13          (1 << 13)       // Receive Queue 13 (Stream) Full Interrupt Status
    #define RIS2_QFF12          (1 << 12)       // Receive Queue 12 (Stream) Full Interrupt Status
    #define RIS2_QFF11          (1 << 11)       // Receive Queue 11 (Stream) Full Interrupt Status
    #define RIS2_QFF10          (1 << 10)       // Receive Queue 10 (Stream) Full Interrupt Status
    #define RIS2_QFF9           (1 << 9)        // Receive Queue 9 (Stream) Full Interrupt Status
    #define RIS2_QFF8           (1 << 8)        // Receive Queue 8 (Stream) Full Interrupt Status
    #define RIS2_QFF7           (1 << 7)        // Receive Queue 7 (Stream) Full Interrupt Status
    #define RIS2_QFF6           (1 << 6)        // Receive Queue 6 (Stream) Full Interrupt Status
    #define RIS2_QFF5           (1 << 5)        // Receive Queue 5 (Stream) Full Interrupt Status
    #define RIS2_QFF4           (1 << 4)        // Receive Queue 4 (Stream) Full Interrupt Status
    #define RIS2_QFF3           (1 << 3)        // Receive Queue 3 (Stream) Full Interrupt Status
    #define RIS2_QFF2           (1 << 2)        // Receive Queue 2 (Stream) Full Interrupt Status
    #define RIS2_QFF1           (1 << 1)        // Receive Queue 1 (Stream) Full Interrupt Status
    #define RIS2_QFF0           (1 << 0)        // Receive Queue 0 (Stream) Full Interrupt Status
#define TIC                 0x0378      // Transmit Interrupt Control Register
    #define TIC_TDPE3           (1 << 19)       // Transmit Descriptor Processed interrupt Enable 3
    #define TIC_TDPE2           (1 << 18)       // Transmit Descriptor Processed interrupt Enable 2
    #define TIC_TDPE1           (1 << 17)       // Transmit Descriptor Processed interrupt Enable 1
    #define TIC_TDPE0           (1 << 16)       // Transmit Descriptor Processed interrupt Enable 0
    #define TIC_MFWE            (1 << 11)       // MAC status FIFO Warning interrupt Enable
    #define TIC_MFUE            (1 << 10)       // MAC status FIFO Updated interrupt Enable
    #define TIC_TFWE            (1 << 9)        // Time Stamp FIFO Warning Interrupt Enable
    #define TIC_TFUE            (1 << 8)        // Time Stamp FIFO Update Interrupt Enable
    #define TIC_FTE3            (1 << 3)        // Frame Transmitted interrupt Enable 3
    #define TIC_FTE2            (1 << 2)        // Frame Transmitted interrupt Enable 2
    #define TIC_FTE1            (1 << 1)        // Frame Transmitted interrupt Enable 1
    #define TIC_FTE0            (1 << 0)        // Frame Transmitted interrupt Enable 0
#define TIS                 0x037C      // Transmit Interrupt Status Register
    #define TIS_TDPF3           (1 << 19)       // Transmit Descriptor Processed Flag 3
    #define TIS_TDPF2           (1 << 18)       // Transmit Descriptor Processed Flag 2
    #define TIS_TDPF1           (1 << 17)       // Transmit Descriptor Processed Flag 1
    #define TIS_TDPF0           (1 << 16)       // Transmit Descriptor Processed Flag 0
    #define TIS_MFWF            (1 << 11)       // Tx Status FIFO warning level has been reached
    #define TIS_MFUF            (1 << 10)       // Tx Status FIFO has been updated
    #define TIS_TFWF            (1 << 9)        // Time Stamp FIFO has reached the warning level
    #define TIS_TFUF            (1 << 8)        // The time-stamp FIFO has been updated.
    #define TIS_FTF3            (1 << 3)        // Frame Transmitted Flag 3
    #define TIS_FTF2            (1 << 2)        // Frame Transmitted Flag 2
    #define TIS_FTF1            (1 << 1)        // Frame Transmitted Flag 1
    #define TIS_FTF0            (1 << 0)        // Frame Transmitted Flag 0
#define ISS                 0x0380      // Interrupt Summary Status Register
    #define ISS_DPM15           (1 << 31)       // Descriptor Interrupt 15 Mirror
    #define ISS_DPM14           (1 << 30)       // Descriptor Interrupt 14 Mirror
    #define ISS_DPM13           (1 << 29)       // Descriptor Interrupt 13 Mirror
    #define ISS_DPM12           (1 << 28)       // Descriptor Interrupt 12 Mirror
    #define ISS_DPM11           (1 << 27)       // Descriptor Interrupt 11 Mirror
    #define ISS_DPM10           (1 << 26)       // Descriptor Interrupt 10 Mirror
    #define ISS_DPM9            (1 << 25)       // Descriptor Interrupt 9 Mirror
    #define ISS_DPM8            (1 << 24)       // Descriptor Interrupt 8 Mirror
    #define ISS_DPM7            (1 << 23)       // Descriptor Interrupt 7 Mirror
    #define ISS_DPM6            (1 << 22)       // Descriptor Interrupt 6 Mirror
    #define ISS_DPM5            (1 << 21)       // Descriptor Interrupt 5 Mirror
    #define ISS_DPM4            (1 << 20)       // Descriptor Interrupt 4 Mirror
    #define ISS_DPM3            (1 << 19)       // Descriptor Interrupt 3 Mirror
    #define ISS_DPM2            (1 << 18)       // Descriptor Interrupt 2 Mirror
    #define ISS_DPM1            (1 << 17)       // Descriptor Interrupt 1 Mirror
    #define ISS_CGIS            (1 << 13)       // gPTP Interrupt Mirror
    #define ISS_RFWM            (1 << 12)       // Receive FIFO Warning Interrupt Mirror
    #define ISS_MFWM            (1 << 11)       // E-MAC status FIFO Warning Mirror
    #define ISS_MFUM            (1 << 10)       // E-MAC status FIFO Updated Mirror
    #define ISS_TFWM            (1 << 9)        // Time Stamp FIFO Warning Interrupt Mirror
    #define ISS_TFUS            (1 << 8)        // Time Stamp FIFO Update Interrupt
    #define ISS_MS              (1 << 7)        // E-MAC Interrupt Mirror
    #define ISS_ES              (1 << 6)        // Error Interrupt Mirror
    #define ISS_FTS             (1 << 2)        // Frame transmitted Mirror
    #define ISS_RWM             (1 << 1)        // Reception warning Mirror
    #define ISS_FRS             (1 << 0)        // Frame received Mirror
#define CIE                 0x0384      // Common Interrupt Enable Register
    #define CIE_RFFL            (1 << 19)       // Interrupt line B used for this notification; 0: Interrupt line A used for this notification
    #define CIE_RFWL            (1 << 18)       // Interrupt line B used for this notification; 0: Interrupt line A used for this notification
    #define CIE_CL0M            (1 << 17)       // Use queue specific interrupt line 0; 0: Use common interrupt line 0
    #define CIE_RQFM            (1 << 16)       // Use for queue specific interrupt line; 0: Use for error interrupt line
    #define CIE_CTIE            (1 << 8)        // Common Transmit Interrupt Enable
    #define CIE_CRIE            (1 << 0)        // Common Receive Interrupt Enable
#define RIC3                0x0388
    #define RIC3_RDPE17         (1 << 17)       // Receive Descriptor Processed interrupt Enable 17
    #define RIC3_RDPE16         (1 << 16)       // Receive Descriptor Processed interrupt Enable 16
    #define RIC3_RDPE15         (1 << 15)       // Receive Descriptor Processed interrupt Enable 15
    #define RIC3_RDPE14         (1 << 14)       // Receive Descriptor Processed interrupt Enable 14
    #define RIC3_RDPE13         (1 << 13)       // Receive Descriptor Processed interrupt Enable 13
    #define RIC3_RDPE12         (1 << 12)       // Receive Descriptor Processed interrupt Enable 12
    #define RIC3_RDPE11         (1 << 11)       // Receive Descriptor Processed interrupt Enable 11
    #define RIC3_RDPE10         (1 << 10)       // Receive Descriptor Processed interrupt Enable 10
    #define RIC3_RDPE9          (1 << 9)        // Receive Descriptor Processed interrupt Enable 9
    #define RIC3_RDPE8          (1 << 8)        // Receive Descriptor Processed interrupt Enable 8
    #define RIC3_RDPE7          (1 << 7)        // Receive Descriptor Processed interrupt Enable 7
    #define RIC3_RDPE6          (1 << 6)        // Receive Descriptor Processed interrupt Enable 6
    #define RIC3_RDPE5          (1 << 5)        // Receive Descriptor Processed interrupt Enable 5
    #define RIC3_RDPE4          (1 << 4)        // Receive Descriptor Processed interrupt Enable 4
    #define RIC3_RDPE3          (1 << 3)        // Receive Descriptor Processed interrupt Enable 3
    #define RIC3_RDPE2          (1 << 2)        // Receive Descriptor Processed interrupt Enable 2
    #define RIC3_RDPE1          (1 << 1)        // Receive Descriptor Processed interrupt Enable 1
    #define RIC3_RDPE0          (1 << 0)        // Receive Descriptor Processed interrupt Enable 0
#define RIS3                0x038C
    #define RIS3_RDPF17         (1 << 17)       // Receive Descriptor Processed Flag 17
    #define RIS3_RDPF16         (1 << 16)       // Receive Descriptor Processed Flag 16
    #define RIS3_RDPF15         (1 << 15)       // Receive Descriptor Processed Flag 15
    #define RIS3_RDPF14         (1 << 14)       // Receive Descriptor Processed Flag 14
    #define RIS3_RDPF13         (1 << 13)       // Receive Descriptor Processed Flag 13
    #define RIS3_RDPF12         (1 << 12)       // Receive Descriptor Processed Flag 12
    #define RIS3_RDPF11         (1 << 11)       // Receive Descriptor Processed Flag 11
    #define RIS3_RDPF10         (1 << 10)       // Receive Descriptor Processed Flag 10
    #define RIS3_RDPF9          (1 << 9)        // Receive Descriptor Processed Flag 9
    #define RIS3_RDPF8          (1 << 8)        // Receive Descriptor Processed Flag 8
    #define RIS3_RDPF7          (1 << 7)        // Receive Descriptor Processed Flag 7
    #define RIS3_RDPF6          (1 << 6)        // Receive Descriptor Processed Flag 6
    #define RIS3_RDPF5          (1 << 5)        // Receive Descriptor Processed Flag 5
    #define RIS3_RDPF4          (1 << 4)        // Receive Descriptor Processed Flag 4
    #define RIS3_RDPF3          (1 << 3)        // Receive Descriptor Processed Flag 3
    #define RIS3_RDPF2          (1 << 2)        // Receive Descriptor Processed Flag 2
    #define RIS3_RDPF1          (1 << 1)        // Receive Descriptor Processed Flag 1
    #define RIS3_RDPF0          (1 << 0)        // Receive Descriptor Processed Flag 0
#define GCCR                0x0390      // gPTP Configuration Control Register
    #define GCCR_LI_M           (7 << 20)       // Index number of compare unit to be loaded
    #define GCCR_SPC            (1 << 19)       // Periodic comparison value updated by GCCR.LPTC request; 0: Absolute comparison value updated by GCCR.LPTC request
    #define GCCR_PGM            (1 << 16)       // Pulse per second is related to corrected gPTP timer value; 0: Pulse per second is related to gPTP timer value
    #define GCCR_TCSS_M         (3 << 8)        // Timer Capture Source Select
    #define GCCR_TCSS_00        (0 << 8)        // gPTP timer value
    #define GCCR_TCSS_01        (1 << 8)        // Adjusted gPTP timer value
    #define GCCR_TCSS_10        (2 << 8)        // AVTP presentation time
    #define GCCR_LMTT           (1 << 5)        // Maximum Transit Time Configuration Request
    #define GCCR_LPTC           (1 << 4)        // Presentation Time Compare Value Configuration Request
    #define GCCR_LTI            (1 << 3)        // Timer Increment Value Configuration Request
    #define GCCR_LTO            (1 << 2)        // Timer Offset Value Configuration Request
    #define GCCR_TCR_M          (3 << 0)        // Timer Control Request
    #define GCCR_TCR_00         (0 << 0)        // Timer control is not requested
    #define GCCR_TCR_01         (1 << 0)        // gPTP/AVTP presentation time reset
    #define GCCR_TCR_10         (2 << 0)        // Continuous capture of AVTP to GCTt.CTV
    #define GCCR_TCR_11         (3 << 0)        // Captures the value set in the TCSS bit
#define GMTT                0x0394      // gPTP Maximum Transit Time Configuration Register
#define GPTC                0x0398      // gPTP Presentation Time Comparison Register
#define GTI                 0x039C      // gPTP Timer Increment Configuration Register
    #define GTI_TIV         0x0FFFFFFF
#define GTO0                0x03A0      // gPTP Timer Offset Configuration Register
#define GTO1                0x03A4
#define GTO2                0x03A8
#define GIC                 0x03AC      // gPTP Interrupt Control Register
    #define GIC_ATCE15          (1 << 31)       // Avtp Time Captured interrupt Enable 15
    #define GIC_ATCE14          (1 << 30)       // Avtp Time Captured interrupt Enable 14
    #define GIC_ATCE13          (1 << 29)       // Avtp Time Captured interrupt Enable 13
    #define GIC_ATCE12          (1 << 28)       // Avtp Time Captured interrupt Enable 12
    #define GIC_ATCE11          (1 << 27)       // Avtp Time Captured interrupt Enable 11
    #define GIC_ATCE10          (1 << 26)       // Avtp Time Captured interrupt Enable 10
    #define GIC_ATCE9           (1 << 25)       // Avtp Time Captured interrupt Enable 9
    #define GIC_ATCE8           (1 << 24)       // Avtp Time Captured interrupt Enable 8
    #define GIC_ATCE7           (1 << 23)       // Avtp Time Captured interrupt Enable 7
    #define GIC_ATCE6           (1 << 22)       // Avtp Time Captured interrupt Enable 6
    #define GIC_ATCE5           (1 << 21)       // Avtp Time Captured interrupt Enable 5
    #define GIC_ATCE4           (1 << 20)       // Avtp Time Captured interrupt Enable 4
    #define GIC_ATCE3           (1 << 19)       // Avtp Time Captured interrupt Enable 3
    #define GIC_ATCE2           (1 << 18)       // Avtp Time Captured interrupt Enable 2
    #define GIC_ATCE1           (1 << 17)       // Avtp Time Captured interrupt Enable 1
    #define GIC_ATCE0           (1 << 16)       // Avtp Time Captured interrupt Enable 0
    #define GIC_PTME            (1 << 2)        // Presentation Time Match Interrupt Enable
    #define GIC_PTOE            (1 << 1)        // Presentation Time Overrun interrupt Enable
    #define GIC_PTCE            (1 << 0)        // Presentation Time Captured interrupt Enable
#define GIS                 0x03B0      // gPTP Interrupt Status Register
    #define GIS_ATCF15          (1 << 31)       // Avtp Time Capture Flag 15
    #define GIS_ATCF14          (1 << 30)       // Avtp Time Capture Flag 14
    #define GIS_ATCF13          (1 << 29)       // Avtp Time Capture Flag 13
    #define GIS_ATCF12          (1 << 28)       // Avtp Time Capture Flag 12
    #define GIS_ATCF11          (1 << 27)       // Avtp Time Capture Flag 11
    #define GIS_ATCF10          (1 << 26)       // Avtp Time Capture Flag 10
    #define GIS_ATCF9           (1 << 25)       // Avtp Time Capture Flag 9
    #define GIS_ATCF8           (1 << 24)       // Avtp Time Capture Flag 8
    #define GIS_ATCF7           (1 << 23)       // Avtp Time Capture Flag 7
    #define GIS_ATCF6           (1 << 22)       // Avtp Time Capture Flag 6
    #define GIS_ATCF5           (1 << 21)       // Avtp Time Capture Flag 5
    #define GIS_ATCF4           (1 << 20)       // Avtp Time Capture Flag 4
    #define GIS_ATCF3           (1 << 19)       // Avtp Time Capture Flag 3
    #define GIS_ATCF2           (1 << 18)       // Avtp Time Capture Flag 2
    #define GIS_ATCF1           (1 << 17)       // Avtp Time Capture Flag 1
    #define GIS_ATCF0           (1 << 16)       // Avtp Time Capture Flag 0
    #define GIS_PTMF            (1 << 2)        // Presentation Time Match Interrupt Flag
    #define GIS_PTOF            (1 << 1)        // Presentation Time Overrun Flag
    #define GIS_PTCF            (1 << 0)        // Presentation Time Capture Flag
#define GCPT                0x03B4      // Gptp Captured Presentation Time register
#define GCT0                0x03B8      // gPTP Timer Capture Register
#define GCT1                0x03BC
#define GCT2                0x03C0
#define GSR                 0x03C4      // Gptp Status Register
    #define GSR_AFU             (1 << 24)       // Update of AVTP FIFO with value from GPTF.PTFV is ongoing;
                                                    // 0: No update of AVTP FIFO pending
    #define GSR_PCM             (1 << 0)        // Periodic comparison; 0: Single shot comparison
#define GIE                 0x03CC      // Gptp Interrupt Enable register
    #define GIE_ATCS15          (1 << 31)       // Avtp Time Captured interrupt Set 15
    #define GIE_ATCS14          (1 << 30)       // Avtp Time Captured interrupt Set 14
    #define GIE_ATCS13          (1 << 29)       // Avtp Time Captured interrupt Set 13
    #define GIE_ATCS12          (1 << 28)       // Avtp Time Captured interrupt Set 12
    #define GIE_ATCS11          (1 << 27)       // Avtp Time Captured interrupt Set 11
    #define GIE_ATCS10          (1 << 26)       // Avtp Time Captured interrupt Set 10
    #define GIE_ATCS9           (1 << 25)       // Avtp Time Captured interrupt Set 9
    #define GIE_ATCS8           (1 << 24)       // Avtp Time Captured interrupt Set 8
    #define GIE_ATCS7           (1 << 23)       // Avtp Time Captured interrupt Set 7
    #define GIE_ATCS6           (1 << 22)       // Avtp Time Captured interrupt Set 6
    #define GIE_ATCS5           (1 << 21)       // Avtp Time Captured interrupt Set 5
    #define GIE_ATCS4           (1 << 20)       // Avtp Time Captured interrupt Set 4
    #define GIE_ATCS3           (1 << 19)       // Avtp Time Captured interrupt Set 3
    #define GIE_ATCS2           (1 << 18)       // Avtp Time Captured interrupt Set 2
    #define GIE_ATCS1           (1 << 17)       // Avtp Time Captured interrupt Set 1
    #define GIE_ATCS0           (1 << 16)       // Avtp Time Captured interrupt Set 0
    #define GIE_PTMS            (1 << 2)        // Presentation Time Match Interrupt Set
    #define GIE_PTOS            (1 << 1)        // Presentation Time Overrun Interrupt Set
    #define GIE_PTCS            (1 << 0)        // Presentation Time Captured Interrupt Set
#define GID                 0x03D0      // Gptp Interrupt Disable register
    #define GID_ATCD15          (1 << 31)       // Avtp Time Captured interrupt Disable 15
    #define GID_ATCD14          (1 << 30)       // Avtp Time Captured interrupt Disable 14
    #define GID_ATCD13          (1 << 29)       // Avtp Time Captured interrupt Disable 13
    #define GID_ATCD12          (1 << 28)       // Avtp Time Captured interrupt Disable 12
    #define GID_ATCD11          (1 << 27)       // Avtp Time Captured interrupt Disable 11
    #define GID_ATCD10          (1 << 26)       // Avtp Time Captured interrupt Disable 10
    #define GID_ATCD9           (1 << 25)       // Avtp Time Captured interrupt Disable 9
    #define GID_ATCD8           (1 << 24)       // Avtp Time Captured interrupt Disable 8
    #define GID_ATCD7           (1 << 23)       // Avtp Time Captured interrupt Disable 7
    #define GID_ATCD6           (1 << 22)       // Avtp Time Captured interrupt Disable 6
    #define GID_ATCD5           (1 << 21)       // Avtp Time Captured interrupt Disable 5
    #define GID_ATCD4           (1 << 20)       // Avtp Time Captured interrupt Disable 4
    #define GID_ATCD3           (1 << 19)       // Avtp Time Captured interrupt Disable 3
    #define GID_ATCD2           (1 << 18)       // Avtp Time Captured interrupt Disable 2
    #define GID_ATCD1           (1 << 17)       // Avtp Time Captured interrupt Disable 1
    #define GID_ATCD0           (1 << 16)       // Avtp Time Captured interrupt Disable 0
    #define GID_PTMD            (1 << 2)        // Presentation Time Match Interrupt Disable
    #define GID_PTOD            (1 << 1)        // Presentation Time Overrun Interrupt Disable
    #define GID_PTCD            (1 << 0)        // Presentation Time Captured Interrupt Disable
#define GIL                 0x03D4      // Gptp Interrupt Line selection register
    #define GIL_ATCL15          (1 << 31)       // Avtp Time Captured interrupt Line select 15
    #define GIL_ATCL14          (1 << 30)       // Avtp Time Captured interrupt Line select 14
    #define GIL_ATCL13          (1 << 29)       // Avtp Time Captured interrupt Line select 13
    #define GIL_ATCL12          (1 << 28)       // Avtp Time Captured interrupt Line select 12
    #define GIL_ATCL11          (1 << 27)       // Avtp Time Captured interrupt Line select 11
    #define GIL_ATCL10          (1 << 26)       // Avtp Time Captured interrupt Line select 10
    #define GIL_ATCL9           (1 << 25)       // Avtp Time Captured interrupt Line select 9
    #define GIL_ATCL8           (1 << 24)       // Avtp Time Captured interrupt Line select 8
    #define GIL_ATCL7           (1 << 23)       // Avtp Time Captured interrupt Line select 7
    #define GIL_ATCL6           (1 << 22)       // Avtp Time Captured interrupt Line select 6
    #define GIL_ATCL5           (1 << 21)       // Avtp Time Captured interrupt Line select 5
    #define GIL_ATCL4           (1 << 20)       // Avtp Time Captured interrupt Line select 4
    #define GIL_ATCL3           (1 << 19)       // Avtp Time Captured interrupt Line select 3
    #define GIL_ATCL2           (1 << 18)       // Avtp Time Captured interrupt Line select 2
    #define GIL_ATCL1           (1 << 17)       // Avtp Time Captured interrupt Line select 1
    #define GIL_ATCL0           (1 << 16)       // Avtp Time Captured interrupt Line select 0
    #define GIL_PTML            (1 << 2)        // Presentation Time Match Interrupt Line select
    #define GIL_PTOL            (1 << 1)        // Presentation Time Overrun Interrupt Line select
    #define GIL_PTCL            (1 << 0)        // Presentation Time Captured Interrupt Line select
#define GACP                0x03DC      // Gptp Avtp Capture Prescaler register
    #define GACP_ACPN_M         (0xF << 8)      // Prescaler unit number to be configured
    #define GACP_ACPV_M         (0xFF << 0)     // Prescaler configuration of AVTP capture unit i
#define GPTF0               0x03E0      // Gptp Presentation Time Fifo register
#define GPTF1               0x03E4
#define GPTF2               0x03E8
#define GPTF3               0x03EC
#define GCAT0               0x0400      // Gptp Captured Avtp Time register
#define GCAT1               0x0404
#define GCAT2               0x0408
#define GCAT3               0x040C
#define GCAT4               0x0410
#define GCAT5               0x0414
#define GCAT6               0x0418
#define GCAT7               0x041C
#define GCAT8               0x0420
#define GCAT9               0x0424
#define GCAT10              0x0428
#define GCAT11              0x042C
#define GCAT12              0x0430
#define GCAT13              0x0434
#define GCAT14              0x0438
#define GCAT15              0x043C
#define DIL                 0x0440      // Descriptor Interrupt Line selection register
    #define DIL_DPL15           (1 << 15)       // Descriptor Processed interrupt Line select 15
    #define DIL_DPL14           (1 << 14)       // Descriptor Processed interrupt Line select 14
    #define DIL_DPL13           (1 << 13)       // Descriptor Processed interrupt Line select 13
    #define DIL_DPL12           (1 << 12)       // Descriptor Processed interrupt Line select 12
    #define DIL_DPL11           (1 << 11)       // Descriptor Processed interrupt Line select 11
    #define DIL_DPL10           (1 << 10)       // Descriptor Processed interrupt Line select 10
    #define DIL_DPL9            (1 << 9)        // Descriptor Processed interrupt Line select 9
    #define DIL_DPL8            (1 << 8)        // Descriptor Processed interrupt Line select 8
    #define DIL_DPL7            (1 << 7)        // Descriptor Processed interrupt Line select 7
    #define DIL_DPL6            (1 << 6)        // Descriptor Processed interrupt Line select 6
    #define DIL_DPL5            (1 << 5)        // Descriptor Processed interrupt Line select 5
    #define DIL_DPL4            (1 << 4)        // Descriptor Processed interrupt Line select 4
    #define DIL_DPL3            (1 << 3)        // Descriptor Processed interrupt Line select 3
    #define DIL_DPL2            (1 << 2)        // Descriptor Processed interrupt Line select 2
#define EIL                 0x0444      // Error Interrupt Line selection register
    #define EIL_TBFL            (1 << 10)       // Tx-Buffer Full interrupt Line select
    #define EIL_MFFL            (1 << 9)        // E-MAC status FIFO Full interrupt Line select
    #define EIL_TFFL            (1 << 8)        // Timestamp FIFO Full interrupt Line select
    #define EIL_CULL1           (1 << 7)        // CBS Upper Limit reached interrupt Line select 1
    #define EIL_CULL0           (1 << 6)        // CBS Upper Limit reached interrupt Line select 0
    #define EIL_CLL1            (1 << 5)        // CBS Lower Limit reached interrupt Line select 1
    #define EIL_CLL0            (1 << 4)        // CBS Lower Limit reached interrupt Line select 0
    #define EIL_SEL             (1 << 3)        // Separation Error interrupt Line select
    #define EIL_QEL             (1 << 2)        // Queue Error interrupt Line select
    #define EIL_MTEL            (1 << 1)        // E-MAC Transmission Error interrupt Line select
    #define EIL_MREL            (1 << 0)        // E-MAC Reception Error interrupt Line select
#define TIL                 0x0448      // Transmission Interrupt Line selection register
    #define TIL_MFWL            (1 << 11)       // E-MAC status FIFO Warning interrupt Line select
    #define TIL_MFUL            (1 << 10)       // E-MAC status FIFO Updated interrupt Line select
    #define TIL_TFWL            (1 << 9)        // Timestamp FIFO Warning interrupt Line select
    #define TIL_TFUL            (1 << 8)        // Timestamp FIFO Updated interrupt Line select
#define DIE                 0x0450      // Descriptor Interrupt Enable register
    #define DIE_DPS15           (1 << 15)       // Set DIC.DPE15 to 1b
    #define DIE_DPS14           (1 << 14)       // Set DIC.DPE14 to 1b
    #define DIE_DPS13           (1 << 13)       // Set DIC.DPE13 to 1b
    #define DIE_DPS12           (1 << 12)       // Set DIC.DPE12 to 1b
    #define DIE_DPS11           (1 << 11)       // Set DIC.DPE11 to 1b
    #define DIE_DPS10           (1 << 10)       // Set DIC.DPE10 to 1b
    #define DIE_DPS9            (1 << 9)        // Set DIC.DPE9 to 1b
    #define DIE_DPS8            (1 << 8)        // Set DIC.DPE8 to 1b
    #define DIE_DPS7            (1 << 7)        // Set DIC.DPE7 to 1b
    #define DIE_DPS6            (1 << 6)        // Set DIC.DPE6 to 1b
    #define DIE_DPS5            (1 << 5)        // Set DIC.DPE5 to 1b
    #define DIE_DPS4            (1 << 4)        // Set DIC.DPE4 to 1b
    #define DIE_DPS3            (1 << 3)        // Set DIC.DPE3 to 1b
    #define DIE_DPS2            (1 << 2)        // Set DIC.DPE2 to 1b
    #define DIE_DPS1            (1 << 1)        // Set DIC.DPE1 to 1b
#define DID                 0x0454      // Descriptor Interrupt Disable register
    #define DID_DPD15           (1 << 15)       // Set DIC.DPE15 to 0b
    #define DID_DPD14           (1 << 14)       // Set DIC.DPE14 to 0b
    #define DID_DPD13           (1 << 13)       // Set DIC.DPE13 to 0b
    #define DID_DPD12           (1 << 12)       // Set DIC.DPE12 to 0b
    #define DID_DPD11           (1 << 11)       // Set DIC.DPE11 to 0b
    #define DID_DPD10           (1 << 10)       // Set DIC.DPE10 to 0b
    #define DID_DPD9            (1 << 9)        // Set DIC.DPE9 to 0b
    #define DID_DPD8            (1 << 8)        // Set DIC.DPE8 to 0b
    #define DID_DPD7            (1 << 7)        // Set DIC.DPE7 to 0b
    #define DID_DPD6            (1 << 6)        // Set DIC.DPE6 to 0b
    #define DID_DPD5            (1 << 5)        // Set DIC.DPE5 to 0b
    #define DID_DPD4            (1 << 4)        // Set DIC.DPE4 to 0b
    #define DID_DPD3            (1 << 3)        // Set DIC.DPE3 to 0b
    #define DID_DPD2            (1 << 2)        // Set DIC.DPE2 to 0b
    #define DID_DPD1            (1 << 1)        // Set DIC.DPE1 to 0b
#define EIE                 0x0458      // Error Interrupt Enable register
    #define EIE_TBFS            (1 << 10)       // Set EIC.TBFE to 1b
    #define EIE_MFFS            (1 << 9)        // Set EIC.MFFE to 1b
    #define EIE_TFFS            (1 << 8)        // Set EIC.TFFE to 1b
    #define EIE_CULS1           (1 << 7)        // Set EIC.CULE1 to 1b
    #define EIE_CULS0           (1 << 6)        // Set EIC.CULE0 to 1b
    #define EIE_CLLS1           (1 << 5)        // Set EIC.CLLE1 to 1b
    #define EIE_CLLS0           (1 << 4)        // Set EIC.CLLE0 to 1b
    #define EIE_SES             (1 << 3)        // Set EIC.SEE to 1b
    #define EIE_QES             (1 << 2)        // Set EIC.QEE to 1b
    #define EIE_MTES            (1 << 1)        // Set EIC.MTEE to 1b
    #define EIE_MRES            (1 << 0)        // Set EIC.MREE to 1b
#define EID                 0x045C      // Error Interrupt Disable Register
    #define EID_TBFD            (1 << 10)       // Set EIC.TBFE to 0b
    #define EID_MFFD            (1 << 9)        // Set EIC.MFFE to 0b
    #define EID_TFFD            (1 << 8)        // Set EIC.TFFE to 0b
    #define EID_CULD1           (1 << 7)        // Set EIC.CULE1 to 0b
    #define EID_CULD0           (1 << 6)        // Set EIC.CULE0 to 0b
    #define EID_CLLD1           (1 << 5)        // Set EIC.CLLE1 to 0b
    #define EID_CLLD0           (1 << 4)        // Set EIC.CLLE0 to 0b
    #define EID_SED             (1 << 3)        // Set EIC.SEE to 0b
    #define EID_QED             (1 << 2)        // Set EIC.QEE to 0b
    #define EID_MTED            (1 << 1)        // Set EIC.MTEE to 0b
    #define EID_MRED            (1 << 0)        // Set EIC.MREE to 0b
#define RIE0                0x0460      // Reception Interrupt Enable register
    #define RIE0_FRS17          (1 << 17)       // Set RIC0.FRE17 to 1b
    #define RIE0_FRS16          (1 << 16)       // Set RIC0.FRE16 to 1b
    #define RIE0_FRS15          (1 << 15)       // Set RIC0.FRE15 to 1b
    #define RIE0_FRS14          (1 << 14)       // Set RIC0.FRE14 to 1b
    #define RIE0_FRS13          (1 << 13)       // Set RIC0.FRE13 to 1b
    #define RIE0_FRS12          (1 << 12)       // Set RIC0.FRE12 to 1b
    #define RIE0_FRS11          (1 << 11)       // Set RIC0.FRE11 to 1b
    #define RIE0_FRS10          (1 << 10)       // Set RIC0.FRE10 to 1b
    #define RIE0_FRS9           (1 << 9)        // Set RIC0.FRE9 to 1b
    #define RIE0_FRS8           (1 << 8)        // Set RIC0.FRE8 to 1b
    #define RIE0_FRS7           (1 << 7)        // Set RIC0.FRE7 to 1b
    #define RIE0_FRS6           (1 << 6)        // Set RIC0.FRE6 to 1b
    #define RIE0_FRS5           (1 << 5)        // Set RIC0.FRE5 to 1b
    #define RIE0_FRS4           (1 << 4)        // Set RIC0.FRE4 to 1b
    #define RIE0_FRS3           (1 << 3)        // Set RIC0.FRE3 to 1b
    #define RIE0_FRS2           (1 << 2)        // Set RIC0.FRE2 to 1b
    #define RIE0_FRS1           (1 << 1)        // Set RIC0.FRE1 to 1b
    #define RIE0_FRS0           (1 << 0)        // Set RIC0.FRE0 to 1b
#define RID0                0x0464      // Reception Interrupt Disable register
    #define RID0_FRD17          (1 << 17)       // Set RIC0.FRE17 to 0b
    #define RID0_FRD16          (1 << 16)       // Set RIC0.FRE16 to 0b
    #define RID0_FRD15          (1 << 15)       // Set RIC0.FRE15 to 0b
    #define RID0_FRD14          (1 << 14)       // Set RIC0.FRE14 to 0b
    #define RID0_FRD13          (1 << 13)       // Set RIC0.FRE13 to 0b
    #define RID0_FRD12          (1 << 12)       // Set RIC0.FRE12 to 0b
    #define RID0_FRD11          (1 << 11)       // Set RIC0.FRE11 to 0b
    #define RID0_FRD10          (1 << 10)       // Set RIC0.FRE10 to 0b
    #define RID0_FRD9           (1 << 9)        // Set RIC0.FRE9 to 0b
    #define RID0_FRD8           (1 << 8)        // Set RIC0.FRE8 to 0b
    #define RID0_FRD7           (1 << 7)        // Set RIC0.FRE7 to 0b
    #define RID0_FRD6           (1 << 6)        // Set RIC0.FRE6 to 0b
    #define RID0_FRD5           (1 << 5)        // Set RIC0.FRE5 to 0b
    #define RID0_FRD4           (1 << 4)        // Set RIC0.FRE4 to 0b
    #define RID0_FRD3           (1 << 3)        // Set RIC0.FRE3 to 0b
    #define RID0_FRD2           (1 << 2)        // Set RIC0.FRE2 to 0b
    #define RID0_FRD1           (1 << 1)        // Set RIC0.FRE1 to 0b
    #define RID0_FRD0           (1 << 0)        // Set RIC0.FRE0 to 0b
#define RIE1                0x0468      // Reception Interrupt Enable register
    #define RIE1_RFWS           (1 << 31)       // Set RIC1.RFWE to 1b
    #define RIE1_RWS17          (1 << 17)       // Set RIC1.RWE17 to 1b
    #define RIE1_RWS16          (1 << 16)       // Set RIC1.RWE16 to 1b
    #define RIE1_RWS15          (1 << 15)       // Set RIC1.RWE15 to 1b
    #define RIE1_RWS14          (1 << 14)       // Set RIC1.RWE14 to 1b
    #define RIE1_RWS13          (1 << 13)       // Set RIC1.RWE13 to 1b
    #define RIE1_RWS12          (1 << 12)       // Set RIC1.RWE12 to 1b
    #define RIE1_RWS11          (1 << 11)       // Set RIC1.RWE11 to 1b
    #define RIE1_RWS10          (1 << 10)       // Set RIC1.RWE10 to 1b
    #define RIE1_RWS9           (1 << 9)        // Set RIC1.RWE9 to 1b
    #define RIE1_RWS8           (1 << 8)        // Set RIC1.RWE8 to 1b
    #define RIE1_RWS7           (1 << 7)        // Set RIC1.RWE7 to 1b
    #define RIE1_RWS6           (1 << 6)        // Set RIC1.RWE6 to 1b
    #define RIE1_RWS5           (1 << 5)        // Set RIC1.RWE5 to 1b
    #define RIE1_RWS4           (1 << 4)        // Set RIC1.RWE4 to 1b
    #define RIE1_RWS3           (1 << 3)        // Set RIC1.RWE3 to 1b
    #define RIE1_RWS2           (1 << 2)        // Set RIC1.RWE2 to 1b
    #define RIE1_RWS1           (1 << 1)        // Set RIC1.RWE1 to 1b
    #define RIE1_RWS0           (1 << 0)        // Set RIC1.RWE0 to 1b
#define RID1                0x046C      // Reception Interrupt Disable register
    #define RID1_RFWD           (1 << 31)       // Set RIC1.RFWE to 0b
    #define RID1_RWD17          (1 << 17)       // Set RIC1.RWE17 to 0b
    #define RID1_RWD16          (1 << 16)       // Set RIC1.RWE16 to 0b
    #define RID1_RWD15          (1 << 15)       // Set RIC1.RWE15 to 0b
    #define RID1_RWD14          (1 << 14)       // Set RIC1.RWE14 to 0b
    #define RID1_RWD13          (1 << 13)       // Set RIC1.RWE13 to 0b
    #define RID1_RWD12          (1 << 12)       // Set RIC1.RWE12 to 0b
    #define RID1_RWD11          (1 << 11)       // Set RIC1.RWE11 to 0b
    #define RID1_RWD10          (1 << 10)       // Set RIC1.RWE10 to 0b
    #define RID1_RWD9           (1 << 9)        // Set RIC1.RWE9 to 0b
    #define RID1_RWD8           (1 << 8)        // Set RIC1.RWE8 to 0b
    #define RID1_RWD7           (1 << 7)        // Set RIC1.RWE7 to 0b
    #define RID1_RWD6           (1 << 6)        // Set RIC1.RWE6 to 0b
    #define RID1_RWD5           (1 << 5)        // Set RIC1.RWE5 to 0b
    #define RID1_RWD4           (1 << 4)        // Set RIC1.RWE4 to 0b
    #define RID1_RWD3           (1 << 3)        // Set RIC1.RWE3 to 0b
    #define RID1_RWD2           (1 << 2)        // Set RIC1.RWE2 to 0b
    #define RID1_RWD1           (1 << 1)        // Set RIC1.RWE1 to 0b
    #define RID1_RWD0           (1 << 0)        // Set RIC1.RWE0 to 0b
#define RIE2                0x0470      // Reception Interrupt Enable register
    #define RIE2_RFFS           (1 << 31)       // Set RIC2.RFFE to 1b
    #define RIE2_QFS17          (1 << 17)       // Set RIC2.QFE17 to 1b
    #define RIE2_QFS16          (1 << 16)       // Set RIC2.QFE16 to 1b
    #define RIE2_QFS15          (1 << 15)       // Set RIC2.QFE15 to 1b
    #define RIE2_QFS14          (1 << 14)       // Set RIC2.QFE14 to 1b
    #define RIE2_QFS13          (1 << 13)       // Set RIC2.QFE13 to 1b
    #define RIE2_QFS12          (1 << 12)       // Set RIC2.QFE12 to 1b
    #define RIE2_QFS11          (1 << 11)       // Set RIC2.QFE11 to 1b
    #define RIE2_QFS10          (1 << 10)       // Set RIC2.QFE10 to 1b
    #define RIE2_QFS9           (1 << 9)        // Set RIC2.QFE9 to 1b
    #define RIE2_QFS8           (1 << 8)        // Set RIC2.QFE8 to 1b
    #define RIE2_QFS7           (1 << 7)        // Set RIC2.QFE7 to 1b
    #define RIE2_QFS6           (1 << 6)        // Set RIC2.QFE6 to 1b
    #define RIE2_QFS5           (1 << 5)        // Set RIC2.QFE5 to 1b
    #define RIE2_QFS4           (1 << 4)        // Set RIC2.QFE4 to 1b
    #define RIE2_QFS3           (1 << 3)        // Set RIC2.QFE3 to 1b
    #define RIE2_QFS2           (1 << 2)        // Set RIC2.QFE2 to 1b
    #define RIE2_QFS1           (1 << 1)        // Set RIC2.QFE1 to 1b
    #define RIE2_QFS0           (1 << 0)        // Set RIC2.QFE0 to 1b
#define RID2                0x0474      // Reception Interrupt Disable register
    #define RID2_RFFD           (1 << 31)       // Set RIC2.RFFE to 0b
    #define RID2_QFD17          (1 << 17)       // Set RIC2.QFE17 to 0b
    #define RID2_QFD16          (1 << 16)       // Set RIC2.QFE16 to 0b
    #define RID2_QFD15          (1 << 15)       // Set RIC2.QFE15 to 0b
    #define RID2_QFD14          (1 << 14)       // Set RIC2.QFE14 to 0b
    #define RID2_QFD13          (1 << 13)       // Set RIC2.QFE13 to 0b
    #define RID2_QFD12          (1 << 12)       // Set RIC2.QFE12 to 0b
    #define RID2_QFD11          (1 << 11)       // Set RIC2.QFE11 to 0b
    #define RID2_QFD10          (1 << 10)       // Set RIC2.QFE10 to 0b
    #define RID2_QFD9           (1 << 9)        // Set RIC2.QFE9 to 0b
    #define RID2_QFD8           (1 << 8)        // Set RIC2.QFE8 to 0b
    #define RID2_QFD7           (1 << 7)        // Set RIC2.QFE7 to 0b
    #define RID2_QFD6           (1 << 6)        // Set RIC2.QFE6 to 0b
    #define RID2_QFD5           (1 << 5)        // Set RIC2.QFE5 to 0b
    #define RID2_QFD4           (1 << 4)        // Set RIC2.QFE4 to 0b
    #define RID2_QFD3           (1 << 3)        // Set RIC2.QFE3 to 0b
    #define RID2_QFD2           (1 << 2)        // Set RIC2.QFE2 to 0b
    #define RID2_QFD1           (1 << 1)        // Set RIC2.QFE1 to 0b
    #define RID2_QFD0           (1 << 0)        // Set RIC2.QFE0 to 0b
#define TIE                 0x0478      // Transmission Interrupt Enable register
    #define TIE_TDPS3           (1 << 19)       // Set TIC.TDPE3 to 1b
    #define TIE_TDPS2           (1 << 18)       // Set TIC.TDPE2 to 1b
    #define TIE_TDPS1           (1 << 17)       // Set TIC.TDPE1 to 1b
    #define TIE_TDPS0           (1 << 16)       // Set TIC.TDPE0 to 1b
    #define TIE_MFWS            (1 << 11)       // Set TIC.MFWE to 1b
    #define TIE_MFUS            (1 << 10)       // Set TIC.MFUE to 1b
    #define TIE_TFWS            (1 << 9)        // Set TIC.TFWE to 1b
    #define TIE_TFUS            (1 << 8)        // Set TIC.TFUE to 1b
    #define TIE_FTS3            (1 << 3)        // Set TIC.FTE3 to 1b
    #define TIE_FTS2            (1 << 2)        // Set TIC.FTE2 to 1b
    #define TIE_FTS1            (1 << 1)        // Set TIC.FTE1 to 1b
    #define TIE_FTS0            (1 << 0)        // Set TIC.FTE0 to 1b
#define TID                 0x047C      // Transmission Interrupt Disable register
    #define TID_TDPD3           (1 << 19)       // Set TIC.TDPE3 to 0b
    #define TID_TDPD2           (1 << 18)       // Set TIC.TDPE2 to 0b
    #define TID_TDPD1           (1 << 17)       // Set TIC.TDPE1 to 0b
    #define TID_TDPD0           (1 << 16)       // Set TIC.TDPE0 to 0b
    #define TID_MFWD            (1 << 11)       // Set TIC.MFWE to 0b
    #define TID_MFUD            (1 << 10)       // Set TIC.MFUE to 0b
    #define TID_TFWD            (1 << 9)        // Set TIC.TFWE to 0b
    #define TID_TFUD            (1 << 8)        // Set TIC.TFUE to 0b
    #define TID_FTD3            (1 << 3)        // Set TIC.FTE3 to 0b
    #define TID_FTD2            (1 << 2)        // Set TIC.FTE2 to 0b
    #define TID_FTD1            (1 << 1)        // Set TIC.FTE1 to 0b
    #define TID_FTD0            (1 << 0)        // Set TIC.FTE0 to 0b
#define RIE3                0x0488      // Reception Interrupt Enable register
    #define RIE3_RDPS17         (1 << 17)       // Set RIC3.RDPE17 to 1b
    #define RIE3_RDPS16         (1 << 16)       // Set RIC3.RDPE16 to 1b
    #define RIE3_RDPS15         (1 << 15)       // Set RIC3.RDPE15 to 1b
    #define RIE3_RDPS14         (1 << 14)       // Set RIC3.RDPE14 to 1b
    #define RIE3_RDPS13         (1 << 13)       // Set RIC3.RDPE13 to 1b
    #define RIE3_RDPS12         (1 << 12)       // Set RIC3.RDPE12 to 1b
    #define RIE3_RDPS11         (1 << 11)       // Set RIC3.RDPE11 to 1b
    #define RIE3_RDPS10         (1 << 10)       // Set RIC3.RDPE10 to 1b
    #define RIE3_RDPS9          (1 << 9)        // Set RIC3.RDPE9 to 1b
    #define RIE3_RDPS8          (1 << 8)        // Set RIC3.RDPE8 to 1b
    #define RIE3_RDPS7          (1 << 7)        // Set RIC3.RDPE7 to 1b
    #define RIE3_RDPS6          (1 << 6)        // Set RIC3.RDPE6 to 1b
    #define RIE3_RDPS5          (1 << 5)        // Set RIC3.RDPE5 to 1b
    #define RIE3_RDPS4          (1 << 4)        // Set RIC3.RDPE4 to 1b
    #define RIE3_RDPS3          (1 << 3)        // Set RIC3.RDPE3 to 1b
    #define RIE3_RDPS2          (1 << 2)        // Set RIC3.RDPE2 to 1b
    #define RIE3_RDPS1          (1 << 1)        // Set RIC3.RDPE1 to 1b
    #define RIE3_RDPS0          (1 << 0)        // Set RIC3.RDPE0 to 1b
#define RID3                0x048C      // Reception Interrupt Disable register
    #define RID3_RDPD17         (1 << 17)       // Set RIC3.RDPE17 to 0b
    #define RID3_RDPD16         (1 << 16)       // Set RIC3.RDPE16 to 0b
    #define RID3_RDPD15         (1 << 15)       // Set RIC3.RDPE15 to 0b
    #define RID3_RDPD14         (1 << 14)       // Set RIC3.RDPE14 to 0b
    #define RID3_RDPD13         (1 << 13)       // Set RIC3.RDPE13 to 0b
    #define RID3_RDPD12         (1 << 12)       // Set RIC3.RDPE12 to 0b
    #define RID3_RDPD11         (1 << 11)       // Set RIC3.RDPE11 to 0b
    #define RID3_RDPD10         (1 << 10)       // Set RIC3.RDPE10 to 0b
    #define RID3_RDPD9          (1 << 9)        // Set RIC3.RDPE9 to 0b
    #define RID3_RDPD8          (1 << 8)        // Set RIC3.RDPE8 to 0b
    #define RID3_RDPD7          (1 << 7)        // Set RIC3.RDPE7 to 0b
    #define RID3_RDPD6          (1 << 6)        // Set RIC3.RDPE6 to 0b
    #define RID3_RDPD5          (1 << 5)        // Set RIC3.RDPE5 to 0b
    #define RID3_RDPD4          (1 << 4)        // Set RIC3.RDPE4 to 0b
    #define RID3_RDPD3          (1 << 3)        // Set RIC3.RDPE3 to 0b
    #define RID3_RDPD2          (1 << 2)        // Set RIC3.RDPE2 to 0b
    #define RID3_RDPD1          (1 << 1)        // Set RIC3.RDPE1 to 0b
    #define RID3_RDPD0          (1 << 0)        // Set RIC3.RDPE0 to 0b

/* E-MAC registers */
#define ECMR                0x0500      // E-MAC Mode Register
    #define ECMR_TRCCM          (1 << 26)       // Reading from a counter register leads to the register being cleared to 0;
                                                    // 0: Writing to a counter register leads to the register being cleared to 0
    #define ECMR_RCSC           (1 << 23)       // Checksums are automatically calculated
    #define ECMR_DPAD           (1 << 21)       // Padding is not inserted in data for transmission when fewer than 60 bytes are to be transmitted and the data are transmitted without being changed
                                                    // 0: Padding to make up 60 bytes is inserted in data for transmission when fewer than 60 bytes are to be transmitted
    #define ECMR_RZPF           (1 << 20)       // PAUSE Frame Reception with Time = 0
    #define ECMR_ZPF            (1 << 19)       // Transmit Zero PAUSE Frame
    #define ECMR_PFR            (1 << 18)       // PAUSE frames are transferred to the AVB-DMAC.
    #define ECMR_RXF            (1 << 17)       // Flow control for the receiving port is enabled.
    #define ECMR_TXF            (1 << 16)       // Flow control for the transmitting port is enabled.
    #define ECMR_MPDE           (1 << 9)        // Magic Packet detection is enabled.
    #define ECMR_RE             (1 << 6)        // Reception Enable
    #define ECMR_TE             (1 << 5)        // Transmission Enable
    #define ECMR_DM             (1 << 1)        // Full-Duplex operation
    #define ECMR_PRM            (1 << 0)        // Promiscuous Mode
#define RFLR                0x0508      // Receive Frame Length Register
#define ECSR                0x0510      // E-MAC Status Register
    #define ECSR_PFRI           (1 << 4)        // PAUSE Frame retry Interrupt
    #define ECSR_PHYI           (1 << 3)        // PHY interrupt terminal (AVB_PHY_INT) is asserted.
    #define ECSR_LCHNG          (1 << 2)        // The change of Link status signal (AVB_LINK) is detected
    #define ECSR_MPD            (1 << 1)        // A Magic Packet has been detected
    #define ECSR_ICD            (1 << 0)        // PHY-LSI has detected an illegal carrier on the line
#define ECSIPR              0x0518      // E-MAC Interrupt Permission Register
    #define ECSIPR_PFRIM        (1 << 4)        // Interrupts on setting of the PFRI bit is enabled.
    #define ECSIPR_PHYIM        (1 << 3)        // Interrupts on setting of the PHYI bit is enabled.
    #define ECSIPR_LCHNGIP      (1 << 2)        // Interrupts on setting of the LCHNG bit is enabled.
    #define ECSIPR_MPDIP        (1 << 1)        // Interrupts on setting of the MPD bit is enabled
    #define ECSIPR_ICDIP        (1 << 0)        // Interrupt on setting of the ICD bit is enabled.
#define PIR                 0x0520      // PHY Interface Register
    #define PIR_MDI             (1 << 3)        // MII Management Data-In
    #define PIR_MDO             (1 << 2)        // MII Management Data-Out
    #define PIR_MMD             (1 << 1)        // Write direction is specified.
    #define PIR_MDC             (1 << 0)        // MII Management Data Clock
#define PSR                 0x0528      // PHY Status Register
    #define PSR_LMON            (1 << 0)        // The link status signal (AVB_LNK) is at the high level.
#define PIPR                0x052C      // PHY_INT Polarity Register
    #define PIPR_PHYIP          (1 << 0)        // PHY interrupt pin (AVB_PHY-INT) is active high (the high level triggers the interrupt state)
#define APR                 0x0554      // Auto PAUSE frame register
#define MPR                 0x0558      // Manual PAUSE Frame Register
#define PFTCR               0x055C      // PAUSE Frame Transmit Counter
#define PFRCR               0x0560      // PAUSE Frame Receive Counter
#define PFTTLR              0x0564      // PAUSE frame transmit times limit
#define PFTTCR              0x0568      // PAUSE frame transmit times counter
#define GECMR               0x05B0      // E-MAC Mode Register 2
    #define GECMR_SPEED_100     (0 << 0)
    #define GECMR_SPEED_1000    (1 << 0)        // Transfer is at 1000 Mbps; 0: Transfer is at 100 Mbps
#define MAHR                0x05C0      // E-MAC Address High Register
#define MALR                0x05C8      // E-MAC Address Low Register
#define FTTOCR              0x0700      // Frame transmit time-out counter
#define CEFCR               0x0740      // CRC Error Frame Receive Counter Register
#define FRECR               0x0748      // Frame Receive Error Counter Register
#define TSFRCR              0x0750      // Too-Short Frame Receive Counter Register
#define TLFRCR              0x0758      // Too-Long Frame Receive Counter Register
#define RFCR                0x0760      // Residual-Bit Frame Receive Counter Register
#define MAFCR               0x0778      // Multicast Address Frame Receive Counter Register


/* Standard Registers in the PHY */
#define PHY_BCTRL               (0x00)          // Basic Control
    #define PHY_BCTRL_RESET         (1 << 15)       // Reset
    #define PHY_BCTRL_LOOPBACK      (1 << 14)       // Loopback
    // PHY Speed Select. Ignored if Auto Negotiation is enabled
    #define PHY_BCTRL_SPEED_1000    ((1 << 6) & ~(1 << 13))     // 1000Mbps
    #define PHY_BCTRL_SPEED_100     (~(1 << 6) & (1 << 13))     // 100Mbps
    #define PHY_BCTRL_SPEED_10      ~((1 << 6) | (1 << 13))     // 10Mbps
    //
    #define PHY_BCTRL_ANENABLE      (1 << 12)       // Auto-Negotiation Enable
    #define PHY_BCTRL_POWERDOWN     (1 << 11)       // Power-Down
    #define PHY_BCTRL_ISOLATION     (1 << 10)       // Electrical isolation of PHY from RGMII
    #define PHY_BCTRL_ANRESTART     (1 << 9)        // Restart Auto-Negotiation
    #define PHY_BCTRL_DUPLEX        (1 << 8)        // Duplex Mode
#define PHY_STAT                (0x01)          // Basic Status
    #define PHY_STAT_100T4          (1 << 15)       // 100Base-T4
    #define PHY_STAT_100TXFULL      (1 << 14)       // 100Base-TX Full-Duplex
    #define PHY_STAT_100TXHALF      (1 << 13)       // 100Base-TX Half-Duplex
    #define PHY_STAT_10TFULL        (1 << 12)       // 10Base-T Full-Duplex
    #define PHY_STAT_10THALF        (1 << 11)       // 10Base-T Half-Duplex
    #define PHY_STAT_EXT            (1 << 8)        // Extended Status
    #define PHY_STAT_NOPREAM        (1 << 6)        // No Preamble
    #define PHY_STAT_ANCOMPLETE     (1 << 5)        // Auto-Negotiation Complete
    #define PHY_STAT_REMOFAULT      (1 << 4)        // Remote Fault
    #define PHY_STAT_AUNEABI        (1 << 3)        // Auto-Negotiation Ability
    #define PHY_STAT_LINK           (1 << 2)        // Link Status
    #define PHY_STAT_JABBER         (1 << 1)        // Jabber Detect
    #define PHY_STAT_EXTCAPA        (1 << 0)        // Extended Capability
#define PHY_ID1                 (0x02)          // PHY Identifier 1
    #define PHY_ID1_DEFAULT         (0x0022)
#define PHY_ID2                 (0x03)          // PHY Identifier 2
    #define PHY_ID2_DEFAULT         (0x1620)
    #define PHY_ID2_MASK            (0xFFFF)
#define PHY_ANADVERTISE         (0x04)          // Auto-Negotiation Advertisement
    #define PHY_ANA_NEXT            (1 << 15)       // Next page capable
    #define PHY_ANA_REMOFAULT       (1 << 13)       // Remote fault supported
    #define PHY_ANA_NOPAU           (0 << 10)       // No pause
    #define PHY_ANA_SYMPAU          (1 << 10)       // Symmetric pause
    #define PHY_ANA_ASYMPAU         (2 << 10)       // Asymmetric pause (link partner)
    #define PHY_ANA_SAAPAU          (3 << 10)       // Symmetric and asymmetric pause (local device)
    #define PHY_ANA_T4              (1 << 9)        // T4 capable
    #define PHY_ANA_100FDX          (1 << 8)        // 100Mbps full-duplex capable
    #define PHY_ANA_100HDX          (1 << 7)        // 100Mbps half-duplex capable
    #define PHY_ANA_10FDX           (1 << 6)        // 10Mbps full-duplex capable
    #define PHY_ANA_10HDX           (1 << 5)        // 10Mbps half-duplex capable
    #define PHY_ANA_802_3           (1 << 0)        // IEEE 802.3
#define PHY_ANCAPABILITY        (0x05)          // Auto Negotiation Link Partner Ability
    #define PHY_ANLPA_NEXT          (1 << 15)       // Next page capable
    #define PHY_ANLPA_ACK           (1 << 14)       // Acknowledge
    #define PHY_ANLPA_REMOFAULT     (1 << 13)       // Remote fault supported
    #define PHY_ANLPA_NOPAU         (0 << 10)       // No pause
    #define PHY_ANLPA_SYMPAU        (1 << 10)       // Symmetric pause
    #define PHY_ANLPA_ASYMPAU       (2 << 10)       // Asymmetric pause (link partner)
    #define PHY_ANLPA_SAAPAU        (3 << 10)       // Symmetric and asymmetric pause (local device)
    #define PHY_ANLPA_T4CAPA        (1 << 9)        // T4 capable
    #define PHY_ANLPA_100FUCA       (1 << 8)        // 100Mbps full-duplex capable
    #define PHY_ANLPA_100HACA       (1 << 7)        // 100Mbps half-duplex capable
    #define PHY_ANLPA_10FUCA        (1 << 6)        // 10Mbps full-duplex capable
    #define PHY_ANLPA_10HACA        (1 << 5)        // 10Mbps half-duplex capable
#define PHY_ANEXPANSION         (0x06)          // Auto-Negotiation Expansion
    #define PHY_ANE_PDA             (1 << 4)        // Parallel Detection Fault
    #define PHY_ANE_LPNPA           (1 << 3)        // Link partner has next page capability
    #define PHY_ANE_NPA             (1 << 2)        // Local device has next page capability
    #define PHY_ANE_PR              (1 << 1)        // New page received
    #define PHY_ANE_LPANA           (1 << 0)        // Link partner has auto-negotiation capability
#define PHY_ANNEXTPAGE          (0x07)          // Auto-Negotiation Next Page
    #define PHY_ANNP_NP             (1 << 15)       // Additional next pages will follow
    #define PHY_ANNP_MP             (1 << 13)       // Message Page
    #define PHY_ANNP_ACK            (1 << 12)       // Acknowledge2
    #define PHY_ANNP_TOGGLE         (1 << 11)       // Toggle
#define PHY_LNKPARTNEXTPAGE     (0x08)          // Auto-Negotiation Link Partner Next Page Ability
    #define PHY_ANLPNPA_NP          (1 << 15)       // Additional next pages will follow
    #define PHY_ANLPNPA_ACK         (1 << 14)       // Successful receipt of link word
    #define PHY_ANLPNPA_MP          (1 << 13)       // Message Page
    #define PHY_ANLPNPA_ACK2        (1 << 12)       // Able to act on the information
    #define PHY_ANLPNPA_TOGGLE      (1 << 11)       // Toggle
#define PHY_BASE_CON            (0x09)          // 1000Base-T Control
#define PHY_BASE_STA            (0x0A)          // 1000Base-T Status
#define PHY_MMDAC               (0x0D)          // MMD Access – Control
#define PHY_MMDARD              (0x0E)          // MMD Access – Register/Data
#define PHY_EXSTA               (0x0F)          // Extended Status
#define PHY_RELOOP              (0x11)          // Remote Loopback
#define PHY_LMDCD               (0x12)          // LinkMD – Cable Diagnostic
#define PHY_DPS                 (0x13)          // Digital PMA/PCS Status
#define PHY_RXER_CNT            (0x15)          // RXER Counter
#define PHY_INTR                (0x1B)          // Interrupt Control/Status
#define PHY_AMX                 (0x1C)          // Auto MDI/MDI-X
#define PHY_CONTROL             (0x1F)          // PHY Control


/* OTHERS */


//Number of Retries
#define NUM_AUTO_NEGO_RETRIES   (1000)
#define NUM_AVB_RESET_RETRIES   (10)
#define NUM_PHY_WAIT_RETRIES    (10)
#define NUM_AVB_WAIT_SETMODE    (100)


//define from linux
#define BIT(x) (1 << x)

/* The Ethernet AVB descriptor definitions. */
typedef volatile struct {
    volatile uint16_t   ds;     /* Descriptor size */
    volatile uint8_t    cc;     /* Content control MSBs (reserved) */
    volatile uint8_t    die_dt; /* Descriptor interrupt enable and type */
    volatile uint32_t   dptr;   /* Descriptor pointer */
} RAVB_DESC;

#define DPTR_ALIGN  4   /* Required descriptor pointer alignment */

enum DIE_DT {
    /* Frame data */
    DT_FMID     = 0x40,
    DT_FSTART   = 0x50,
    DT_FEND     = 0x60,
    DT_FSINGLE  = 0x70,
    /* Chain control */
    DT_LINK     = 0x80,
    DT_LINKFIX  = 0x90,
    DT_EOS      = 0xa0,
    /* HW/SW arbitration */
    DT_FEMPTY   = 0xc0,
    DT_FEMPTY_IS    = 0xd0,
    DT_FEMPTY_IC    = 0xe0,
    DT_FEMPTY_ND    = 0xf0,
    DT_LEMPTY   = 0x20,
    DT_EEMPTY   = 0x30,
};
#if 0
typedef volatile struct {
    volatile uint16_t   ds_cc;  /* Descriptor size and content control LSBs */
    volatile uint8_t    msc;    /* MAC status code */
    volatile uint8_t    die_dt; /* Descriptor interrupt enable and type */
    volatile uint32_t   dptr;   /* Descpriptor pointer */
} RAVB_RX_DESC;
#endif
typedef volatile struct {
    volatile uint16_t   ds_cc;  /* Descriptor size and content control lower bits */
    volatile uint8_t    msc;    /* MAC status code */
    volatile uint8_t    die_dt; /* Descriptor interrupt enable and type */
    volatile uint32_t   dptr;   /* Descpriptor pointer */
    volatile uint32_t   ts_n;   /* Timestampe nsec */
    volatile uint32_t   ts_sl;  /* Timestamp low */
    volatile uint16_t   ts_sh;  /* Timestamp high */
    volatile uint16_t   res;    /* Reserved bits */
} RAVB_RX_DESC;

enum RX_DS_CC_BIT {
    RX_DS       = 0x0fff, /* Data size */
    RX_TR       = 0x1000, /* Truncation indication */
    RX_EI       = 0x2000, /* Error indication */
    RX_PS       = 0xc000, /* Padding selection */
};

/* E-MAC status code */
enum MSC_BIT {
    MSC_CRC     = 0x01, /* Frame CRC error */
    MSC_RFE     = 0x02, /* Frame reception error (flagged by PHY) */
    MSC_RTSF    = 0x04, /* Frame length error (frame too short) */
    MSC_RTLF    = 0x08, /* Frame length error (frame too long) */
    MSC_FRE     = 0x10, /* Fraction error (not a multiple of 8 bits) */
    MSC_CRL     = 0x20, /* Carrier lost */
    MSC_CEEF    = 0x40, /* Carrier extension error */
    MSC_MC      = 0x80, /* Multicast frame reception */
};

typedef volatile struct {
    volatile uint16_t   ds_tagl;    /* Descriptor size and frame tag LSBs */
    volatile uint8_t    tagh_tsr;   /* Frame tag MSBs and timestamp storage request bit */
    volatile uint8_t    die_dt; /* Descriptor interrupt enable and type */
    volatile uint32_t   dptr;   /* Descpriptor pointer */
} RAVB_TX_DESC;

enum TX_DS_TAGL_BIT {
    TX_DS       = 0x0fff, /* Data size */
    TX_TAGL     = 0xf000, /* Frame tag LSBs */
};

enum TX_TAGH_TSR_BIT {
    TX_TAGH     = 0x3f, /* Frame tag MSBs */
    TX_TSR      = 0x40, /* Timestamp storage request */
};
enum RAVB_QUEUE {
    RAVB_BE = 0,    /* Best Effort Queue */
    RAVB_NC,    /* Network Control Queue */
};

#define DBAT_ENTRY_NUM  22
#define RX_QUEUE_OFFSET 4
#define TX_QUEUE_OFFSET 0
#define RFLR_RFL_MIN    0x05EE  /* Recv Frame length 1518 byte */

#define ALIGN_MASK(x, mask)    (((x) + (mask)) & ~(mask))
#define ALIGN(x, a)            ALIGN_MASK(x, (typeof(x))(a) - 1)

typedef struct {
    nic_config_t        cfg;
    int                 set_flow;
    struct _iopkt_self  *iopkt;
    void                *dll_hdl;
} attach_args_t;

typedef struct {
    struct device       dev;
    struct ethercom     ecom;
    struct mii_data     bsd_mii;
    mdi_t               *mdi;
    int                 set_flow;
    int                 flow_status;
    uint32_t            set_speed;
    int                 set_duplex;
    struct callout      mii_callout;
    nic_config_t        cfg;
    nic_stats_t         stats;
    void                *sdhook;
    int                 iid[2];
    struct _iopkt_inter ient;
    struct _iopkt_self  *iopkt;
    uintptr_t           base;

    volatile uint32_t   iss;    /* Interrupt summary status registor */
    volatile uint32_t   ris0;   /* Receive interrupt status registor 0 */
    volatile uint32_t   ris2;   /* Receive interrupt status registor 2 */
    volatile uint32_t   tis;    /* Transmit interrupt status registor */
    volatile uint32_t   eis;    /* Error interrupt status registor */

    int                 fd;
    uint32_t            desc_bat_dma;
    RAVB_DESC           *desc_bat;

    uint32_t            tx_desc_dma;
    RAVB_TX_DESC        *tx_bd;

    uint32_t            rx_desc_dma;
    RAVB_RX_DESC        *rx_bd;

    int                 tx_cidx;
    int                 tx_pidx;
    int                 rx_idx;

    struct cache_ctrl   cachectl;
    struct mbuf     **tx_pkts;
    struct mbuf     **rx_pkts;
    int             pkts_received;
} ravb_dev_t;

void ravb_update_stats(ravb_dev_t *ravb);
void ravb_clear_stats(ravb_dev_t *ravb);
int ravb_ioctl(struct ifnet *ifp, unsigned long cmd, caddr_t data);

void ravb_reap_tx(ravb_dev_t *ravb);
void ravb_start(struct ifnet *ifp);

void ravb_receive(ravb_dev_t *ravb, struct nw_work_thread *wtp);

int ravb_mediachange(struct ifnet *ifp);
int ravb_phy_init(ravb_dev_t *ravb);
void ravb_phy_fini(ravb_dev_t *ravb);

int ravb_process_interrupt(void *arg, struct nw_work_thread *wtp);
int ravb_dmac_enable_interrupt(void * arg);
const struct sigevent * ravb_dmac_isr(void *arg, int iid);
const struct sigevent * ravb_emac_isr(void *arg, int iid);

#endif // define RAVB_H

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/hardware/devnp/ravb/ravb.h $ $Rev: 813325 $")
#endif
