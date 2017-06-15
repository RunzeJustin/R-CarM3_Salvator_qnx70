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


/*
 * Renesas R-Car processor with ARMv8 Cortex-A57/53 core
 */

#ifndef __ARM_RCAR_H_INCLUDED
#define __ARM_RCAR_H_INCLUDED

#define RCAR_EXT_CLK            16666666        /* External crystal clock */

/*
 * GPIO
 */
#define RCAR_GPIO0_BASE         0xE6050000
#define RCAR_GPIO1_BASE         0xE6051000
#define RCAR_GPIO2_BASE         0xE6052000
#define RCAR_GPIO3_BASE         0xE6053000
#define RCAR_GPIO4_BASE         0xE6054000
#define RCAR_GPIO5_BASE         0xE6055000
#define RCAR_GPIO6_BASE         0xE6055400
#define RCAR_GPIO7_BASE         0xE6055800

 // GPIO Registers, offset from GPIO_BASE
 #define RCAR_GPIO_IOINTSEL     0x0000
 #define RCAR_GPIO_INOUTSEL     0x0004
 #define RCAR_GPIO_OUTDT        0x0008
 #define RCAR_GPIO_INDT         0x000C
 #define RCAR_GPIO_INTDT        0x0010
 #define RCAR_GPIO_INTCLR       0x0014
 #define RCAR_GPIO_INTMSK       0x0018
 #define RCAR_GPIO_MSKCLR       0x001C
 #define RCAR_GPIO_POSNEG       0x0020
 #define RCAR_GPIO_EDGLEVEL     0x0024
 #define RCAR_GPIO_FILONOFF     0x0028
 #define RCAR_GPIO_INTMSKS      0x0038
 #define RCAR_GPIO_MSKCLRS      0x003C
 #define RCAR_GPIO_OUTDTSEL     0x0040
 #define RCAR_GPIO_OUTDTH       0x0044
 #define RCAR_GPIO_OUTDTL       0x0048
 #define RCAR_GPIO_BOTHEDGE     0x004C


/*
 * Pin Function Control
 */
#define RCAR_PFC_BASE               0xE6060000

 /* PFC registers, offset from RCAR_PFC_BASE */
 #define RCAR_PFC_PMMR              0x000
 #define RCAR_PFC_GPSR0             0x100
 #define RCAR_PFC_GPSR1             0x104
 #define RCAR_PFC_GPSR2             0x108
 #define RCAR_PFC_GPSR3             0x10C
 #define RCAR_PFC_GPSR4             0x110
 #define RCAR_PFC_GPSR5             0x114
 #define RCAR_PFC_GPSR6             0x118
 #define RCAR_PFC_GPSR7             0x11C
 #define RCAR_PFC_IPSR0             0x200
 #define RCAR_PFC_IPSR1             0x204
 #define RCAR_PFC_IPSR2             0x208
 #define RCAR_PFC_IPSR3             0x20C
 #define RCAR_PFC_IPSR4             0x210
 #define RCAR_PFC_IPSR5             0x214
 #define RCAR_PFC_IPSR6             0x218
 #define RCAR_PFC_IPSR7             0x21C
 #define RCAR_PFC_IPSR8             0x220
 #define RCAR_PFC_IPSR9             0x224
 #define RCAR_PFC_IPSR10            0x228
 #define RCAR_PFC_IPSR11            0x22C
 #define RCAR_PFC_IPSR12            0x230
 #define RCAR_PFC_IPSR13            0x234
 #define RCAR_PFC_IPSR14            0x238
 #define RCAR_PFC_IPSR15            0x23C
 #define RCAR_PFC_IPSR16            0x240
 #define RCAR_PFC_IPSR17            0x244
 #define RCAR_PFC_DRVCTRL0          0x300
 #define RCAR_PFC_DRVCTRL1          0x304
 #define RCAR_PFC_DRVCTRL2          0x308
 #define RCAR_PFC_DRVCTRL3          0x30C
 #define RCAR_PFC_DRVCTRL4          0x310
 #define RCAR_PFC_DRVCTRL5          0x314
 #define RCAR_PFC_DRVCTRL6          0x318
 #define RCAR_PFC_DRVCTRL7          0x31C
 #define RCAR_PFC_DRVCTRL8          0x320
 #define RCAR_PFC_DRVCTRL9          0x324
 #define RCAR_PFC_DRVCTRL10         0x328
 #define RCAR_PFC_DRVCTRL11         0x32C
 #define RCAR_PFC_DRVCTRL12         0x330
 #define RCAR_PFC_DRVCTRL13         0x334
 #define RCAR_PFC_DRVCTRL14         0x338
 #define RCAR_PFC_DRVCTRL15         0x33C
 #define RCAR_PFC_DRVCTRL16         0x340
 #define RCAR_PFC_DRVCTRL17         0x344
 #define RCAR_PFC_DRVCTRL18         0x348
 #define RCAR_PFC_DRVCTRL19         0x34C
 #define RCAR_PFC_DRVCTRL20         0x350
 #define RCAR_PFC_DRVCTRL21         0x354
 #define RCAR_PFC_DRVCTRL22         0x358
 #define RCAR_PFC_DRVCTRL23         0x35C
 #define RCAR_PFC_DRVCTRL24         0x360
 #define RCAR_PFC_POCCTRL0          0x380
 #define RCAR_PFC_TDSELCTRL0        0x3C0
 #define RCAR_PFC_IOCTRL            0x3E0
 #define RCAR_PFC_FUSEMON0          0x3E4
 #define RCAR_PFC_FUSEMON1          0x3E8        // Fuse Monitor Register 1
 #define RCAR_PFC_PUEN0             0x400
 #define RCAR_PFC_PUEN1             0x404
 #define RCAR_PFC_PUEN2             0x408
 #define RCAR_PFC_PUEN3             0x40C
 #define RCAR_PFC_PUEN4             0x410
 #define RCAR_PFC_PUEN5             0x414
 #define RCAR_PFC_PUEN6             0x418
 #define RCAR_PFC_PUD0              0x440
 #define RCAR_PFC_PUD1              0x444
 #define RCAR_PFC_PUD2              0x448
 #define RCAR_PFC_PUD3              0x44C
 #define RCAR_PFC_PUD4              0x450
 #define RCAR_PFC_PUD5              0x454
 #define RCAR_PFC_PUD6              0x458
 #define RCAR_PFC_MODSEL0           0x500
 #define RCAR_PFC_MODSEL1           0x504
 #define RCAR_PFC_MODSEL2           0x508

 #define RCAR_PFC_PUPR0             0x100
 #define RCAR_PFC_PUPR1             0x104
 #define RCAR_PFC_PUPR2             0x108
 #define RCAR_PFC_PUPR3             0x10C
 #define RCAR_PFC_PUPR4             0x110
 #define RCAR_PFC_PUPR5             0x114
 #define RCAR_PFC_PUPR6             0x118
 #define RCAR_PFC_IOCTL             0x70
 #define RCAR_PFC_IOCTL0            0x60
 #define RCAR_PFC_IOCTL1            0x64
 #define RCAR_PFC_IOCTL4            0x84
 #define RCAR_PFC_IOCTL5            0x88
 #define RCAR_PFC_IOCTL6            0x8C
 #define RCAR_PFC_DDR3GPEN          0x240
 #define RCAR_PFC_DDR3GPOE          0x244
 #define RCAR_PFC_DDR3GPOD          0x248
 #define RCAR_PFC_DDR3GPID          0x24C

/*
 * Clock Pulse Generator
 */
#define RCAR_CPG_BASE               0xE6150000
#define RCAR_CPG_SIZE               0x1000

 /* CPG registers, offset from RCAR_CPG_BASE */
 #define RCAR_CPG_CPGWPCR           0x904       // CPG Write Protect Control Register
 #define RCAR_CPG_CPGWPR            0x900       // CPG Write Protect Register
 #define RCAR_CPG_FRQCRB            0x004       // Frequency Control Register B
 #define RCAR_CPG_MSOCKCR           0x014       // MSIOF clock frequency control register
 #define RCAR_CPG_FRQCRC            0x0E0       // Frequency Control Register C
 #define RCAR_CPG_PLLECR            0x0D0       // PLL Enable Control Register
 #define RCAR_CPG_PLL0CR            0x0D8       // PLL0 Control Register
 #define RCAR_CPG_PLL2CR            0x02C       // PLL2 Control Register
 #define RCAR_CPG_PLL3CR            0x0DC       // PLL2 Control Register
 #define RCAR_CPG_PLL0STPCR         0x0F0       // PLL0 Stop Condition Register
 #define RCAR_CPG_PLL2STPCR         0x0F8       // PLL2 Stop Condition Register
 #define RCAR_CPG_PLL3STPCR         0x0FC       // PLL3 Stop Condition Register
 #define RCAR_CPG_PLL4STPCR         0x1F8       // PLL4 Stop Condition Register

 #define RCAR_CPG_RGXCR             0x0B4       // RGX Control Register
 #define RCAR_CPG_SD0CKCR           0x074       // SDHI 0 Clock Frequency Control Register
 #define RCAR_CPG_SD1CKCR           0x078       // SDHI 1 Clock Frequency Control Register
 #define RCAR_CPG_SD2CKCR           0x268       // SDHI 2 Clock Frequency Control Register
 #define RCAR_CPG_SD3CKCR           0x26C       // SDHI 3 Clock Frequency Control Register
 #define RCAR_CPG_GPUCKCR           0x234       // GPU Clock Frequency Control Register
 #define RCAR_CPG_ADSPCKCR          0x25C       // ADSP Clock Frequency Control Register
 #define RCAR_CPG_SSPCKCR           0x248       // SSP Clock Frequency Control Register
 #define RCAR_CPG_HDMICKCR          0x250       // SSP Clock Frequency Control Register
 #define RCAR_CPG_SSPRSCKCR         0x24C       // SSPRS Clock Frequency Control Register
 #define RCAR_CPG_RCANCKCR          0x270       // RCAN Clock Frequency Control Register
 #define RCAR_CPG_FMMCKCR           0x274       // FMM Clock Frequency Control Register
 #define RCAR_CPG_DVFSCR0           0x058       // DVFS Control Register 0
 #define RCAR_CPG_DVFSCR1           0x05C       // DVFS Control Register 1

 #define RCAR_CPG_MSTPSR0           0x030       // Module Stop Status Register 0
 #define RCAR_CPG_MSTPSR1           0x038       // Module Stop Status Register 1
 #define RCAR_CPG_MSTPSR2           0x040       // Module Stop Status Register 2
 #define RCAR_CPG_MSTPSR3           0x048       // Module Stop Status Register 3
 #define RCAR_CPG_MSTPSR4           0x04C       // Module Stop Status Register 4
 #define RCAR_CPG_MSTPSR5           0x03C       // Module Stop Status Register 5
 #define RCAR_CPG_MSTPSR6           0x1C0       // Module Stop Status Register 6
 #define RCAR_CPG_MSTPSR7           0x1C4       // Module Stop Status Register 7
 #define RCAR_CPG_MSTPSR8           0x9A0       // Module Stop Status Register 8
 #define RCAR_CPG_MSTPSR9           0x9A4       // Module Stop Status Register 9
 #define RCAR_CPG_MSTPSR10          0x9A8       // Module Stop Status Register 10
 #define RCAR_CPG_MSTPSR11          0x9AC       // Module Stop Status Register 11

 #define RCAR_CPG_RMSTPCR0          0x110       // Realtime Module Stop Control Register 0
 #define RCAR_CPG_RMSTPCR1          0x114       // Realtime Module Stop Control Register 1
 #define RCAR_CPG_RMSTPCR2          0x118       // Realtime Module Stop Control Register 2
 #define RCAR_CPG_RMSTPCR3          0x11C       // Realtime Module Stop Control Register 3
 #define RCAR_CPG_RMSTPCR4          0x120       // Realtime Module Stop Control Register 4
 #define RCAR_CPG_RMSTPCR5          0x124       // Realtime Module Stop Control Register 5
 #define RCAR_CPG_RMSTPCR7          0x12C       // Realtime Module Stop Control Register 7
 #define RCAR_CPG_RMSTPCR8          0x980       // Realtime Module Stop Control Register 8
 #define RCAR_CPG_RMSTPCR9          0x984       // Realtime Module Stop Control Register 9
 #define RCAR_CPG_RMSTPCR10         0x988       // Realtime Module Stop Control Register 10
 #define RCAR_CPG_RMSTPCR11         0x98C       // Realtime Module Stop Control Register 11

 #define RCAR_CPG_SMSTPCR0          0x130       // System Module Stop Control Register 0
 #define RCAR_CPG_SMSTPCR1          0x134       // System Module Stop Control Register 1
 #define RCAR_CPG_SMSTPCR2          0x138       // System Module Stop Control Register 2
 #define RCAR_CPG_SMSTPCR3          0x13C       // System Module Stop Control Register 3
 #define RCAR_CPG_SMSTPCR4          0x140       // System Module Stop Control Register 4
 #define RCAR_CPG_SMSTPCR5          0x144       // System Module Stop Control Register 5
 #define RCAR_CPG_SMSTPCR6          0x148       // System Module Stop Control Register 6
 #define RCAR_CPG_SMSTPCR7          0x14C       // System Module Stop Control Register 7
 #define RCAR_CPG_SMSTPCR8          0x990       // System Module Stop Control Register 8
 #define RCAR_CPG_SMSTPCR9          0x994       // System Module Stop Control Register 9
 #define RCAR_CPG_SMSTPCR10         0x998       // System Module Stop Control Register 10
 #define RCAR_CPG_SMSTPCR11         0x99C       // System Module Stop Control Register 11

 #define RCAR_CPG_SRCR0             0x0A0       // Software Reset Register 0
 #define RCAR_CPG_SRCR1             0x0A8       // Software Reset Register 1
 #define RCAR_CPG_SRCR2             0x0B0       // Software Reset Register 2
 #define RCAR_CPG_SRCR3             0x0B8       // Software Reset Register 3
 #define RCAR_CPG_SRCR4             0x0BC       // Software Reset Register 4
 #define RCAR_CPG_SRCR5             0x0C4       // Software Reset Register 5
 #define RCAR_CPG_SRCR6             0x0C8       // Software Reset Register 6
 #define RCAR_CPG_SRCR7             0x1CC       // Software Reset Register 7
 #define RCAR_CPG_SRCR8             0x920       // Software Reset Register 8
 #define RCAR_CPG_SRCR9             0x924       // Software Reset Register 9
 #define RCAR_CPG_SRCR10            0x928       // Software Reset Register 10
 #define RCAR_CPG_SRCR11            0x92C       // Software Reset Register 11

 #define RCAR_CPG_SRSTCLR0          0x940       // Software Reset Clear Register 0
 #define RCAR_CPG_SRSTCLR1          0x944       // Software Reset Clear Register 1
 #define RCAR_CPG_SRSTCLR2          0x948       // Software Reset Clear Register 2
 #define RCAR_CPG_SRSTCLR3          0x94C       // Software Reset Clear Register 3
 #define RCAR_CPG_SRSTCLR4          0x950       // Software Reset Clear Register 4
 #define RCAR_CPG_SRSTCLR5          0x954       // Software Reset Clear Register 5
 #define RCAR_CPG_SRSTCLR6          0x958       // Software Reset Clear Register 6
 #define RCAR_CPG_SRSTCLR7          0x95C       // Software Reset Clear Register 7
 #define RCAR_CPG_SRSTCLR8          0x960       // Software Reset Clear Register 8
 #define RCAR_CPG_SRSTCLR9          0x964       // Software Reset Clear Register 9
 #define RCAR_CPG_SRSTCLR10         0x968       // Software Reset Clear Register 10
 #define RCAR_CPG_SRSTCLR11         0x96C       // Software Reset Clear Register 11

/*
 * Reset registers
 */
#define RCAR_RESET_BASE             0xE6160000
 #define RCAR_WDTRSTCR              0x54        // Watchdog reset control register
 #define RCAR_MODEMR                0x60        // Mode Monitor Register

/*
 * SYStem Controller
 */
#define RCAR_SYSC_BASE              0xE6180000
 #define RCAR_SYSC_SR               0x00        // Status Register
 #define RCAR_SYSC_ISR              0x04        // Interrupt Status Register
 #define RCAR_SYSC_ISCR             0x08        // Interrupt Status Clear Register
 #define RCAR_SYSC_IER              0x0C        // Interrupt Enable Register
 #define RCAR_SYSC_IMR              0x10        // Interrupt Mask Register
 #define RCAR_SYSC_EERSR            0x20        // External Event Request Status Register
 #define RCAR_SYSC_EERSCR           0x24        // External Event Request Status Clear Register
 #define RCAR_SYSC_EERSER           0x28        // External Event Request Status Enable Register
 #define RCAR_SYSC_EERSR2           0x2C        // External Event Request Status Register 2
 #define RCAR_SYSC_EERSCR2          0x30        // External Event Request Status Clear register 2
 #define RCAR_SYSC_EERSER2          0x34        // External Event Request Status Enable Register 2
 #define RCAR_SYSC_EERSR3           0x40        // External Event Request Status Register 3
 #define RCAR_SYSC_EERSCR3          0x44        // External Event Request Status Clear register 3
 #define RCAR_SYSC_EERSER3          0x48        // External Event Request Status Enable Register 3

 /* CA57 */
 #define RCAR_SYSC_PWRSR0           0x80        // Power status register 0 (CA57)
 #define RCAR_SYSC_PWROFFSR0        0x88        // Power shutoff status register 0 (CA57)
 #define RCAR_SYSC_PWRONSR0         0x90        // Power resume status register 0 (CA57)
 #define RCAR_SYSC_PWRER0           0x94        // Power shutoff/resume error register 0 (CA57)
 #define RCAR_SYSC_PWRPSEU0         0xb8        // Power pseudo shutoff register 0 (CA57)
 #define RCAR_SYSC_PWRISOER0        0xbc        // Power isolation error detection register 0 (CA57)

 /* 3DG */
 #define RCAR_SYSC_PWRSR2           0x100        // Power Status Register
 #define RCAR_SYSC_PWROFFCR2        0x104        // Power Shutoff Control Register
 #define RCAR_SYSC_PWROFFSR2        0x108        // Power Shutoff Status Register
 #define RCAR_SYSC_PWRONCR2         0x10C        // Power Resume Control Register
 #define RCAR_SYSC_PWRONSR2         0x110        // Power Resume Status Register
 #define RCAR_SYSC_PWRER2           0x114        // Power shutoff/resume Error Register
 #define RCAR_SYSC_PWRPSEU2         0x138        // Power shutoff/resume Error Register
 #define RCAR_SYSC_PWRISOER2        0x13C        // Power shutoff/resume Error Register

 /* CA53-SCU */
 #define RCAR_SYSC_PWRSR3           0x140        // Power Status Register
 #define RCAR_SYSC_PWROFFCR3        0x144        // Power Shutoff Control Register
 #define RCAR_SYSC_PWROFFSR3        0x148        // Power Shutoff Status Register
 #define RCAR_SYSC_PWRONCR3         0x14C        // Power Resume Control Register
 #define RCAR_SYSC_PWRONSR3         0x150        // Power Resume Status Register
 #define RCAR_SYSC_PWRER3           0x154        // Power shutoff/resume Error Register
 #define RCAR_SYSC_PWRPSEU3         0x178        // Power shutoff/resume Error Register
 #define RCAR_SYSC_PWRISOER3        0x17C        // Power shutoff/resume Error Register

 /* IMP */
 #define RCAR_SYSC_PWRSR4           0x180        // Power Status Register
 #define RCAR_SYSC_PWROFFCR4        0x184        // Power Shutoff Control Register
 #define RCAR_SYSC_PWROFFSR4        0x188        // Power Shutoff Status Register
 #define RCAR_SYSC_PWRONCR4         0x18C        // Power Resume Control Register
 #define RCAR_SYSC_PWRONSR4         0x190        // Power Resume Status Register
 #define RCAR_SYSC_PWRER4           0x194        // Power shutoff/resume Error Register
 #define RCAR_SYSC_PWRPSEU4         0x1B8        // Power shutoff/resume Error Register
 #define RCAR_SYSC_PWRISOER4        0x1BC        // Power shutoff/resume Error Register

 /* CA57-SCU */
 #define RCAR_SYSC_PWRSR5           0x1C0        // Power Status Register
 #define RCAR_SYSC_PWROFFCR5        0x1C4        // Power Shutoff Control Register
 #define RCAR_SYSC_PWROFFSR5        0x1C8        // Power Shutoff Status Register
 #define RCAR_SYSC_PWRONCR5         0x1CC        // Power Resume Control Register
 #define RCAR_SYSC_PWRONSR5         0x1D0        // Power Resume Status Register
 #define RCAR_SYSC_PWRER5           0x1D4        // Power shutoff/resume Error Register
 #define RCAR_SYSC_PWRPSEU5         0x1F8        // Power shutoff/resume Error Register
 #define RCAR_SYSC_PWRISOER5        0x1FC        // Power shutoff/resume Error Register

 /* CA53-SCU */
 #define RCAR_SYSC_PWRSR6           0x200        // Power Status Register
 #define RCAR_SYSC_PWROFFCR6        0x204        // Power Shutoff Control Register
 #define RCAR_SYSC_PWROFFSR6        0x208        // Power Shutoff Status Register
 #define RCAR_SYSC_PWRONCR6         0x20C        // Power Resume Control Register
 #define RCAR_SYSC_PWRONSR6         0x210        // Power Resume Status Register
 #define RCAR_SYSC_PWRER6           0x214        // Power shutoff/resume Error Register
 #define RCAR_SYSC_PWRPSEU6         0x238        // Power shutoff/resume Error Register
 #define RCAR_SYSC_PWRISOER6        0x23C        // Power shutoff/resume Error Register

 /* CR7 */
 #define RCAR_SYSC_PWRSR7           0x240        // Power Status Register
 #define RCAR_SYSC_PWROFFCR7        0x244        // Power Shutoff Control Register
 #define RCAR_SYSC_PWROFFSR7        0x248        // Power Shutoff Status Register
 #define RCAR_SYSC_PWRONCR7         0x24C        // Power Resume Control Register
 #define RCAR_SYSC_PWRONSR7         0x250        // Power Resume Status Register
 #define RCAR_SYSC_PWRER7           0x254        // Power shutoff/resume Error Register
 #define RCAR_SYSC_PWRPSEU7         0x278        // Power shutoff/resume Error Register
 #define RCAR_SYSC_PWRISOER7        0x27C        // Power shutoff/resume Error Register

 /* A3VP */
 #define RCAR_SYSC_PWRSR8           0x340        // Power Status Register
 #define RCAR_SYSC_PWROFFCR8        0x344        // Power Shutoff Control Register
 #define RCAR_SYSC_PWROFFSR8        0x348        // Power Shutoff Status Register
 #define RCAR_SYSC_PWRONCR8         0x34C        // Power Resume Control Register
 #define RCAR_SYSC_PWRONSR8         0x350        // Power Resume Status Register
 #define RCAR_SYSC_PWRER8           0x354        // Power shutoff/resume Error Register
 #define RCAR_SYSC_PWRPSEU8         0x378        // Power shutoff/resume Error Register
 #define RCAR_SYSC_PWRISOER8        0x37C        // Power shutoff/resume Error Register

 /* A3VC */
 #define RCAR_SYSC_PWRSR9           0x380        // Power Status Register
 #define RCAR_SYSC_PWROFFCR9        0x384        // Power Shutoff Control Register
 #define RCAR_SYSC_PWROFFSR9        0x388        // Power Shutoff Status Register
 #define RCAR_SYSC_PWRONCR9         0x38C        // Power Resume Control Register
 #define RCAR_SYSC_PWRONSR9         0x390        // Power Resume Status Register
 #define RCAR_SYSC_PWRER9           0x394        // Power shutoff/resume Error Register
 #define RCAR_SYSC_PWRPSEU9         0x3B8        // Power shutoff/resume Error Register
 #define RCAR_SYSC_PWRISOER9        0x3BC        // Power shutoff/resume Error Register

 /* A2VC */
 #define RCAR_SYSC_PWRSR10          0x3C0        // Power Status Register
 #define RCAR_SYSC_PWROFFCR10       0x3C4        // Power Shutoff Control Register
 #define RCAR_SYSC_PWROFFSR10       0x3C8        // Power Shutoff Status Register
 #define RCAR_SYSC_PWRONCR10        0x3CC        // Power Resume Control Register
 #define RCAR_SYSC_PWRONSR10        0x3D0        // Power Resume Status Register
 #define RCAR_SYSC_PWRER10          0x3D4        // Power shutoff/resume Error Register
 #define RCAR_SYSC_PWRPSEU10        0x3F8        // Power shutoff/resume Error Register
 #define RCAR_SYSC_PWRISOER10       0x3FC        // Power shutoff/resume Error Register

/*
 * Watchdog Timer
 */
#define RCAR_RWDT_BASE              0xE6020000  // RCLK Watchdog Timer
#define RCAR_SWDT_BASE              0xE6030000  // Secure Watchdog Timer
 #define RCAR_WDT_CNT               0x00        // Count Register
 #define RCAR_WDT_CSRA              0x04        // Control/Status Register A
 #define RCAR_WDT_CSRB              0x08        // Control/Status Register B

/*
 * SCU
 */
#define RCAR_SCU_BASE               0xEC500000
#define RCAR_SCU_SIZE               0x1000

/*
 * SSIU
 */
#define RCAR_SSIU_BASE              0xEC540000
 #define RCAR_SSIU_MODE0            0x800
 #define RCAR_SSIU_MODE1            0x804
 #define RCAR_SSIU_MODE2            0x808
 #define RCAR_SSIU_MODE3            0x80C
 #define RCAR_SSIU_CONTROL          0x810


/*
 * SSI
 */
#define RCAR_SSI_BASE               0xEC541000
 #define RCAR_SSI_CR                0x00        // Control Register
 #define RCAR_SSI_SR                0x04        // Status Register
 #define RCAR_SSI_TDR               0x08        // Transmit Data Register
 #define RCAR_SSI_RDR               0x0C        // Receive Data Register
 #define RCAR_SSI_WSR               0x20        // WS Mode Register
 #define RCAR_SSI_FMR               0x24        // FS Mode Register
 #define RCAR_SSI_FSR               0x28        // FS Status Register

/*
 * GIC
 */
#define RCAR_GIC_BASE               0xF1000000
#define RCAR_GIC_CPU_BASE           0xF1020000
#define RCAR_GIC_DIST_BASE          0xF1010000

/*
 * INTC
 */
#define RCAR_IRQC0_BASE             0xE61C0000
#define RCAR_IRQC1_BASE             0xE61C0200
 #define RCAR_INTREQ_STS0           0x000       /* R */
 #define RCAR_INTEN_STS0            0x004       /* R/WC1 */
 #define RCAR_INTEN_SET0            0x008       /* W */
 #define RCAR_INTREQ_STS1           0x010       /* R */
 #define RCAR_INTEN_STS1            0x014       /* R/WC1 */
 #define RCAR_INTEN_SET1            0x018       /* W */
 #define RCAR_DETECT_STATUS         0x100       /* R/WC1 */
 #define RCAR_CONFIG_00             0x180       /* R/W */


/*
 * SDHI
 */
#define RCAR_SDHI0_BASE             0xEE100000  // SDHI0 Base
#define RCAR_SDHI1_BASE             0xEE120000  // SDHI1 Base
#define RCAR_SDHI2_BASE             0xEE140000  // SDHI2 Base
#define RCAR_SDHI3_BASE             0xEE160000  // SDHI3 Base
#define RCAR_SDHI_SIZE              0x2000

#define RCAR_SDHI0_POC_MASK         0x3F << 0
#define RCAR_SDHI1_POC_MASK         0x3F << 6
#define RCAR_SDHI2_POC_MASK         0x7F << 12
#define RCAR_SDHI3_POC_MASK         0x7FFF << 19

#define RCAR_SDHI0_VDD_PORT         2
#define RCAR_SDHI3_VDD_PORT         15

#define RCAR_SDHI0_VDDQVA_PORT      1
#define RCAR_SDHI1_VDDQVA_PORT      3
#define RCAR_SDHI2_VDDQVA_PORT      9
#define RCAR_SDHI3_VDDQVA_PORT      14

/*
 * MMCIF
 */
#define RCAR_MMCIF0_BASE            0xEE140000
#define RCAR_MMCIF1_BASE            0xEE160000
#define RCAR_MMCIF_SIZE             0x2000

/*
 * SYSDMAC
 */
#define RCAR_SYSDMAC0_BASE          0xE6700000
#define RCAR_SYSDMAC1_BASE          0xE7300000
#define RCAR_SYSDMAC2_BASE          0xE7310000

 /* SYSDMAC global registers, offset from BASE */
 #define RCAR_SYSDMAC_DMAISTA       0x0020      // DMA interrupt status register
 #define RCAR_SYSDMAC_DMASEC        0x0030      // DMA secure control register
 #define RCAR_SYSDMAC_DMAOR         0x0060      // DMA operation register
 #define RCAR_SYSDMAC_DMACHCLR      0x0080      // DMA channel clear register
 #define RCAR_SYSDMAC_DMADPSEC      0x00A0      // DPRAM secure control register

/* SYSDMAC register offset */
#define RCAR_SYSDMAC_REGS           0x8000
#define RCAR_SYSDMAC_REGSIZE        0x80

 /* SYSDMAC registers, offset from BASE + SYSDMAC_REG */
 #define RCAR_SYSDMAC_DMASAR        0x00
 #define RCAR_SYSDMAC_DMADAR        0x04
 #define RCAR_SYSDMAC_DMATCR        0x08
 #define RCAR_SYSDMAC_DMACHCR       0x0C
 #define RCAR_SYSDMAC_DMAFIXSAR     0x10
 #define RCAR_SYSDMAC_DMAFIXDAR     0x14
 #define RCAR_SYSDMAC_DMATCRB       0x18
 #define RCAR_SYSDMAC_DMACHCRB      0x1C
 #define RCAR_SYSDMAC_DMASART       0x20
 #define RCAR_SYSDMAC_DMADART       0x24
 #define RCAR_SYSDMAC_DMATSR        0x28
 #define RCAR_SYSDMAC_DMACHCRT      0x2C
 #define RCAR_SYSDMAC_DMATSRB       0x38
 #define RCAR_SYSDMAC_DMARS         0x40
 #define RCAR_SYSDMAC_DMABUFCR      0x48
 #define RCAR_SYSDMAC_DMADPBASE     0x50
 #define RCAR_SYSDMAC_DMADPCR       0x54
 #define RCAR_SYSDMAC_DMAFIXDPBASE  0x60

/*
 * AUDIODMAC
 */
#define RCAR_AUDIODMAC0_BASE        0xEC700000
#define RCAR_AUDIODMAC1_BASE        0xEC720000

/*
 * AUDIODMACPP
 */
#define RCAR_AUDIODMACPP_BASE       0xEC740000
#define RCAR_AUDIODMACPP_SIZE       0x1000
 #define RCAR_PDMASAR(x)            (0x20 + 0x10 * (x))
 #define RCAR_PDMADAR(x)            (0x24 + 0x10 * (x))
 #define RCAR_PDMACHAR(x)           (0x28 + 0x10 * (x))

/*
 * I2C
 */
#define RCAR_I2C0_BASE              0xE6508000
#define RCAR_I2C1_BASE              0xE6518000
#define RCAR_I2C2_BASE              0xE6530000
#define RCAR_I2C3_BASE              0xE6540000
#define RCAR_I2C4_BASE              0xE6520000
#define RCAR_I2C5_BASE              0xE6528000

#define RCAR_I2C_SIZE               0x100

 /* I2C registers, offset from I2C base */
 #define RCAR_I2C_ICSCR             0x00        // Slave Control Register
 #define RCAR_I2C_ICMCR             0x04        // Master Control Register
 #define RCAR_I2C_ICSSR             0x08        // Slave Status Register
 #define RCAR_I2C_ICMSR             0x0C        // Master Status Register
 #define RCAR_I2C_ICSIER            0x10        // Slave Interrupt Enable Register
 #define RCAR_I2C_ICMIER            0x14        // Master Interrupt Enable Register
 #define RCAR_I2C_ICCCR             0x18        // Clock Control Register
 #define RCAR_I2C_ICSAR             0x1C        // Slave Address Register
 #define RCAR_I2C_ICMAR             0x20        // Master Address Register
 #define RCAR_I2C_ICRXD             0x24        // Receive Data Register
 #define RCAR_I2C_ICTXD             0x24        // Transmit Data Register
 #define RCAR_I2C_ICCCR2            0x28
 #define RCAR_I2C_ICMPR             0x2C
 #define RCAR_I2C_ICHPR             0x30
 #define RCAR_I2C_ICLPR             0x34

  /* ICSCR bit definition */
  #define RCAR_ICSCR_SDBS           (1 << 3)
  #define RCAR_ICSCR_SIE            (1 << 2)
  #define RCAR_ICSCR_GCAE           (1 << 1)
  #define RCAR_ICSCR_FNA            (1 << 0)

  /* ICSSR bit definition */
  #define RCAR_ICSSR_GCAR           (1 << 6)
  #define RCAR_ICSSR_STM            (1 << 5)
  #define RCAR_ICSSR_SSR            (1 << 4)
  #define RCAR_ICSSR_SDE            (1 << 3)
  #define RCAR_ICSSR_SDT            (1 << 2)
  #define RCAR_ICSSR_SDR            (1 << 1)
  #define RCAR_ICSSR_SAR            (1 << 0)

  /* ICSIER bit definition */
  #define RCAR_ICSIER_SSRE          (1 << 4)
  #define RCAR_ICSIER_SDEE          (1 << 3)
  #define RCAR_ICSIER_SDTE          (1 << 2)
  #define RCAR_ICSIER_SDRE          (1 << 1)
  #define RCAR_ICSIER_SARE          (1 << 0)

  /* ICMCR bit definition */
  #define RCAR_ICMCR_MDBS           (1 << 7)
  #define RCAR_ICMCR_FSCL           (1 << 6)
  #define RCAR_ICMCR_FSDA           (1 << 5)
  #define RCAR_ICMCR_OBPC           (1 << 4)
  #define RCAR_ICMCR_MIE            (1 << 3)
  #define RCAR_ICMCR_TSBE           (1 << 2)
  #define RCAR_ICMCR_FSB            (1 << 1)
  #define RCAR_ICMCR_ESG            (1 << 0)

  /* ICMSR bit definition */
  #define RCAR_ICMSR_MNR            (1 << 6)
  #define RCAR_ICMSR_MAL            (1 << 5)
  #define RCAR_ICMSR_MST            (1 << 4)
  #define RCAR_ICMSR_MDE            (1 << 3)
  #define RCAR_ICMSR_MDT            (1 << 2)
  #define RCAR_ICMSR_MDR            (1 << 1)
  #define RCAR_ICMSR_MAT            (1 << 0)

  /* ICMIER bit definition */
  #define RCAR_ICMIER_MNRE          (1 << 6)
  #define RCAR_ICMIER_MALE          (1 << 5)
  #define RCAR_ICMIER_MSTE          (1 << 4)
  #define RCAR_ICMIER_MDEE          (1 << 3)
  #define RCAR_ICMIER_MDTE          (1 << 2)
  #define RCAR_ICMIER_MDRE          (1 << 1)
  #define RCAR_ICMIER_MATE          (1 << 0)

  /* ICMAR bit definition */
  #define RCAR_ICMAR_STM1           (1 << 0)

  /* ICCCR bit definition */
  #define RCAR_ICCCR_SCGD(x)        ((x) << 3)
  #define RCAR_ICCCR_CDF(x)         ((x) & 0x7)

/*
 * IIC
 */
#define RCAR_IIC0_BASE              0xE6500000
#define RCAR_IIC1_BASE              0xE6510000
#define RCAR_IIC2_BASE              0xE6520000
#define RCAR_IIC3_BASE              0xE60B0000

#define RCAR_IIC_SIZE               0x100

 /* IIC registers, offset from base */
 #define RCAR_IIC_ICDR              0x0000      // Data Register
 #define RCAR_IIC_ICCR              0x0004      // Control Register
 #define RCAR_IIC_ICSR              0x0008      // Status Register
 #define RCAR_IIC_ICIC              0x000C      // Interrupt Control Register
 #define RCAR_IIC_ICCL              0x0010      // Clock Control Register Low
 #define RCAR_IIC_ICCH              0x0014      // Clock Control Register High

  /* ICCR bit definition */
  #define   RCAR_ICCR_ICE           (1 << 7)    // IIC Interface Enable
  #define   RCAR_ICCR_RACK          (1 << 6)    // Receive Acknowledge
  #define   RCAR_ICCR_MTM           (1 << 4)    // Master Transmit Mode
  #define   RCAR_ICCR_MRM           (0 << 4)    // Master Receive Mode
  #define   RCAR_ICCR_BBSY          (1 << 2)    // Bus Busy
  #define   RCAR_ICCR_SCP           (1 << 0)    // START/STOP Condition Prohibit

  /* ICSR bit definition */
  #define   RCAR_ICSR_DTE           (1 << 0)    // Data Transmit Enable
  #define   RCAR_ICSR_WAIT          (1 << 1)    // Wait
  #define   RCAR_ICSR_TACK          (1 << 2)    // Transmit Acknowledge
  #define   RCAR_ICSR_AL            (1 << 3)    // Arbitration Lost
  #define   RCAR_ICSR_BUSY          (1 << 4)    // Transmit Busy

  /* ICIC bit definition */
  #define   RCAR_ICIC_DTE           (1 << 0)    // Data Transmit Enable
  #define   RCAR_ICIC_WAIT          (1 << 1)    // Wait
  #define   RCAR_ICIC_TACK          (1 << 2)    // Transmit Acknowledge
  #define   RCAR_ICIC_AL            (1 << 3)    // Arbitration Lost

/*
 * Clock Synchronized Serial Interface with FIFO
 */
#define RCAR_MSIOF0_BASE            0xE6E20000
#define RCAR_MSIOF1_BASE            0xE6E10000
#define RCAR_MSIOF2_BASE            0xE6E00000
#define RCAR_MSIOF3_BASE            0xE6C90000

#define RCAR_MSIOF_SIZE             0x100

 /* MSIOF registers, offset from base */
 #define RCAR_MSIOF_TMDR1           0x00
 #define RCAR_MSIOF_TMDR2           0x04
 #define RCAR_MSIOF_TMDR3           0x08
 #define RCAR_MSIOF_RMDR1           0x10
 #define RCAR_MSIOF_RMDR2           0x14
 #define RCAR_MSIOF_RMDR3           0x18
 #define RCAR_MSIOF_TSCR            0x20
 #define RCAR_MSIOF_RSCR            0x22
 #define RCAR_MSIOF_CTR             0x28
 #define RCAR_MSIOF_FCTR            0x30
 #define RCAR_MSIOF_STR             0x40
 #define RCAR_MSIOF_IER             0x44
 #define RCAR_MSIOF_TFDR            0x50
 #define RCAR_MSIOF_RFDR            0x60


/*
 * SCIF
 */
#define RCAR_SCIF0_BASE             0xE6E60000
#define RCAR_SCIF1_BASE             0xE6E68000
#define RCAR_SCIF2_BASE             0xE6E88000
#define RCAR_SCIF3_BASE             0xE6EA8000
#define RCAR_SCIF4_BASE             0xE6EE0000
#define RCAR_SCIF5_BASE             0xE6EE8000

#define RCAR_SCIF_SIZE              0x100

#define RCAR_SCIF_FIFO_SIZE         16

 /* SCIF registers, offset from SCIF_BASE */
 #define RCAR_SCIF_SCSMR            0x00
 #define RCAR_SCIF_SCBRR            0x04
 #define RCAR_SCIF_SCSCR            0x08
 #define RCAR_SCIF_SCFTDR           0x0C
 #define RCAR_SCIF_SCFSR            0x10
 #define RCAR_SCIF_SCFRDR           0x14
 #define RCAR_SCIF_SCFCR            0x18
 #define RCAR_SCIF_SCFDR            0x1C
 #define RCAR_SCIF_SCSPTR           0x20
 #define RCAR_SCIF_SCLSR            0x24
 #define RCAR_SCIF_BRG_DL           0x30
 #define RCAR_SCIF_BRG_CKS          0x34

 /* SCSMR bit definition */
 #define RCAR_SCIF_SCSMR_CHR        (1 << 6)
 #define RCAR_SCIF_SCSMR_PE         (1 << 5)
 #define RCAR_SCIF_SCSMR_OE         (1 << 4)
 #define RCAR_SCIF_SCSMR_STOP       (1 << 3)
 #define RCAR_SCIF_SCSMR_CKS_0      (0 << 0)
 #define RCAR_SCIF_SCSMR_CKS_4      (1 << 0)
 #define RCAR_SCIF_SCSMR_CKS_16     (2 << 0)
 #define RCAR_SCIF_SCSMR_CKS_64     (3 << 0)

  /* SCSCR bit definition */
  #define RCAR_SCIF_SCSCR_TEIE      (1 << 11)
  #define RCAR_SCIF_SCSCR_TIE       (1 << 7)
  #define RCAR_SCIF_SCSCR_RIE       (1 << 6)
  #define RCAR_SCIF_SCSCR_TE        (1 << 5)
  #define RCAR_SCIF_SCSCR_RE        (1 << 4)
  #define RCAR_SCIF_SCSCR_REIE      (1 << 3)
  #define RCAR_SCIF_SCSCR_CKE_MASK  (3 << 0)

  /* SCFSR bit definition */
  #define RCAR_SCIF_SCFSR_PERF(x)   (((x) >> 12) & 0xF)
  #define RCAR_SCIF_SCFSR_FERF(x)   (((x) >> 8) & 0xF)
  #define RCAR_SCIF_SCFSR_ER        (1 << 7)
  #define RCAR_SCIF_SCFSR_TEND      (1 << 6)
  #define RCAR_SCIF_SCFSR_TDFE      (1 << 5)
  #define RCAR_SCIF_SCFSR_BRK       (1 << 4)
  #define RCAR_SCIF_SCFSR_FER       (1 << 3)
  #define RCAR_SCIF_SCFSR_PER       (1 << 2)
  #define RCAR_SCIF_SCFSR_RDF       (1 << 1)
  #define RCAR_SCIF_SCFSR_DR        (1 << 0)

  /* SCFCR bit definition */
  #define RCAR_SCIF_SCFCR_RTRG_1    (0 << 6)
  #define RCAR_SCIF_SCFCR_RTRG_4    (1 << 6)
  #define RCAR_SCIF_SCFCR_RTRG_8    (2 << 6)
  #define RCAR_SCIF_SCFCR_RTRG_14   (3 << 6)
  #define RCAR_SCIF_SCFCR_TTRG_0    (3 << 4)
  #define RCAR_SCIF_SCFCR_TTRG_2    (2 << 4)
  #define RCAR_SCIF_SCFCR_TTRG_4    (1 << 4)
  #define RCAR_SCIF_SCFCR_TTRG_8    (0 << 4)
  #define RCAR_SCIF_SCFCR_MCE       (1 << 3)
  #define RCAR_SCIF_SCFCR_TFRST     (1 << 2)
  #define RCAR_SCIF_SCFCR_RFRST	    (1 << 1)
  #define RCAR_SCIF_SCFCR_LOOP      (1 << 0)

  /* SCFDR bit definition */
  #define RCAR_SCIF_SCFDR_TX(x)     (((x) >> 8) & 0x1F)
  #define RCAR_SCIF_SCFDR_RX(x)     ((x) & 0x1F)

  /* SCSPTR bit definition */
  #define RCAR_SCIF_SCSPTR_RTSIO    (1 << 7)
  #define RCAR_SCIF_SCSPTR_RTSDT    (1 << 6)
  #define RCAR_SCIF_SCSPTR_CTSIO    (1 << 5)
  #define RCAR_SCIF_SCSPTR_CTSDT    (1 << 4)
  #define RCAR_SCIF_SCSPTR_SPB2IO   (1 << 1)
  #define RCAR_SCIF_SCSPTR_SPB2DT   (1 << 0)

  /* SCLSR bit definition */
  #define RCAR_SCIF_SCLSR_ORER      (1 << 0)


/*
* RPC (SPI)
*/
#define RCAR_RPC_BASE				0xEE200000
#define RCAR_RPC_SIZE				0x8C
#define RCAR_RPC_BUFFER_BASE		0xEE208000
#define RCAR_RPC_BUFFER_SIZE		0xFF

/* RPC registers, offset from RCAR_RPC_BASE */
#define RCAR_RPC_CMNCR			0x00	/* Common control register */
#define RCAR_RPC_SSLDR			0x04	/* SSL delay register  */
#define RCAR_RPC_DRCR			0x0C	/* Data read control register */
#define RCAR_RPC_DRCMR			0x10	/* Data read command setting register */
#define RCAR_RPC_DREAR			0x14	/* Data read extended address setting register */
#define RCAR_RPC_DROPR			0x18	/* Data read option setting register */
#define RCAR_RPC_DRENR			0x1C	/* Data read enable setting register */
#define RCAR_RPC_SMCR			0x20	/* Manual mode control register */
#define RCAR_RPC_SMCMR			0x24	/* Manual mode command setting register */
#define RCAR_RPC_SMADR			0x28	/* Manual mode address setting register */
#define RCAR_RPC_SMOPR			0x2C	/* Manual mode option setting register */
#define RCAR_RPC_SMENR			0x30	/* Manual mode enable setting register */
#define RCAR_RPC_SMRDR0			0x38	/* Manual mode read data register 0 */
#define RCAR_RPC_SMRDR1			0x3C	/* Manual mode read data register 1 */
#define RCAR_RPC_SMWDR0			0x40	/* Manual mode write data register 0 */
#define RCAR_RPC_SMWDR1			0x44	/* Manual mode write data register 1 */
#define RCAR_RPC_CMNSR			0x48	/* Common status register */
#define RCAR_RPC_DRDMCR			0x58	/* Data read dummy cycle setting register */
#define RCAR_RPC_DRDRENR		0x5C	/* Data read DDR enable register */
#define RCAR_RPC_SMDMCR			0x60	/* Manual mode dummy cycle setting register */
#define RCAR_RPC_SMDRENR		0x64	/* Manual mode DDR enable register */
#define RCAR_RPC_PHYCNT			0x7C	/* PHY control register */
#define RCAR_RPC_OFFSET1		0x80	/*  */
#define RCAR_RPC_PHYINT			0x88	/* PHY interrupt register */

/* CMNCR BIT Definitions */
#define RCAR_RPC_CMNCR_MD			(1 << 31)		/* Operating Mode Switch. 0: External space address read mode, 1: Manual mode */
#define RCAR_RPC_CMNCR_BSZ_CLEAR	~(0x3 << 0)
#define RCAR_RPC_CMNCR_BSZ_4BIT		(0x0 << 0)		/* Serial flash memory x 1 */
#define RCAR_RPC_CMNCR_BSZ_8BIT		(0x1 << 0)		/* Serial flash memory x 2 or HyperFlash or 8-bit serial flash memory x 1 */

/* SSLDR BIT Definitions */
#define RCAR_RPC_SSLDR_SPNDL_CLEAR	~(0x7 << 16)	/* Next Access Delay */
#define RCAR_RPC_SSLDR_SPNDL(x)		(x << 16)		/* x = 0 -> 7 */
#define RCAR_RPC_SSLDR_SLNDL_CLEAR	~(0x7 << 8)		/* QSPIn_SSL Negation Delay */
#define RCAR_RPC_SSLDR_SLNDL(x)		(x << 8)		/* x = 0 -> 7 */
#define RCAR_RPC_SSLDR_SCKDL_CLEAR	~(0x7 << 0)		/* Clock Delay */
#define RCAR_RPC_SSLDR_SCKDL(x)		(x << 0)		/* x = 0 -> 7 */

/* DRCR BIT Definitions */
#define RCAR_RPC_DRCR_SSLN			(1 << 24)		/* QSPIn_SSL Negation */
#define RCAR_RPC_DRCR_RBURST_CLEAR	~(0x1F << 16)	/* Read Data Burst Length */
#define RCAR_RPC_DRCR_RBURST(x)		(x << 16)		/* x = 0 -> 31 */
#define RCAR_RPC_DRCR_RBE			(1 << 8)		/* Read Burst. 0: Normal read, 1: Burst read */

/* DRCMR BIT Definition */
#define RCAR_RPC_DRCMR_CMD_CLEAR	~(0xFF << 16)	/* Command */
#define RCAR_RPC_DRCMR_CMD(x)		(x << 16)		/* x = 0x00 -> 0xFF */
#define RCAR_RPC_DRCMR_OCMD_CLEAR	~(0xFF << 0)	/* Optional command */
#define RCAR_RPC_DRCMR_OCMD(x)		(x << 0)		/* x = 0x00 -> 0xFF */

/* DREAR BIT Definitions */
#define RCAR_RPC_DREAR_EAV_CLEAR	~(0xFF << 16)	/* 32-Bit Extended Upper Address Fixed Value */
#define RCAR_RPC_DREAR_EAV(x)		(x << 16)		/* x = 0x00 -> 0xFF */
#define RCAR_RPC_DREAR_EAC_CLEAR	~(0x7 << 0)		/* 32-Bit Extended External Address Valid Range */
#define RCAR_RPC_DREAR_EAC_24_0		(0x0 << 0)		/* External address bits [24:0] enabled */
#define RCAR_RPC_DREAR_EAC_25_0		(0x1 << 0)		/* External address bits [25:0] enabled */

/* DRENR BIT Definitions */
#define RCAR_RPC_DRENR_CDB_CLEAR	~(0x3 << 30)	/* Command Bit Size */
#define RCAR_RPC_DRENR_CDB_1BIT		(0x0 << 30)
#define RCAR_RPC_DRENR_CDB_2BITS	(0x1 << 30)
#define RCAR_RPC_DRENR_CDB_4BITS	(0x2 << 30)
#define RCAR_RPC_DRENR_OCBD_CLEAR	~(0x3 << 28)	/* Optional Command Bit Size */
#define RCAR_RPC_DRENR_OCBD_1BIT	(0x0 << 28)
#define RCAR_RPC_DRENR_OCBD_2BITS	(0x1 << 28)
#define RCAR_RPC_DRENR_OCBD_4BITS	(0x2 << 28)
#define RCAR_RPC_DRENR_ADB_CLEAR	~(0x3 << 24)	/* Address Bit Size */
#define RCAR_RPC_DRENR_ADB_1BIT		(0x0 << 24)
#define RCAR_RPC_DRENR_ADB_2BITS	(0x1 << 24)
#define RCAR_RPC_DRENR_ADB_4BITS	(0x2 << 24)
#define RCAR_RPC_DRENR_OPDB_CLEAR	~(0x3 << 20)	/* Optional Data Bit Size */
#define RCAR_RPC_DRENR_OPDB_1BIT	(0x0 << 20)
#define RCAR_RPC_DRENR_OPDB_2BITS	(0x1 << 20)
#define RCAR_RPC_DRENR_OPDB_4BITS	(0x2 << 20)
#define RCAR_RPC_DRENR_SPIDB_CLEAR	~(0x3 << 16)	/* Transfer Data Bit Size */
#define RCAR_RPC_DRENR_SPIDB_1BIT	(0x0 << 16)
#define RCAR_RPC_DRENR_SPIDB_2BITS	(0x1 << 16)
#define RCAR_RPC_DRENR_SPIDB_4BITS	(0x2 << 16)
#define RCAR_RPC_DRENR_DME			(1 << 15)		/* Dummy Cycle Enable */
#define RCAR_RPC_DRENR_CDE			(1 << 14)		/* Command Enable */
#define RCAR_RPC_DRENR_OCDE			(1 << 12)		/* Optional Command Enable */
#define RCAR_RPC_DRENR_ADE_CLEAR	~(0xF << 8)		/* Address Enable. ADR[31:0] is output */
#define RCAR_RPC_DRENR_ADE_DISABLE	(0x0 << 8)
#define RCAR_RPC_DRENR_ADE_24_1		(0x7 << 8)		/* ADR[24:1] is output */
#define RCAR_RPC_DRENR_ADE_32_1		(0xF << 8)		/* ADR[32:1] is output */
#define RCAR_RPC_DRENR_OPDE_CLEAR	~(0xF << 4)		/* Optional Data Enable. OPD3, OPD2, OPD1, and OPD0 are output. */
#define RCAR_RPC_DRENR_OPDE_DISABLE	(0x0 << 4)
#define RCAR_RPC_DRENR_OPDE_OPD3	(0x8 << 4)		/* OPD3 is output */
#define RCAR_RPC_DRENR_OPDE_OPD32	(0xC << 4)		/* OPD3 and OPD2 are output */
#define RCAR_RPC_DRENR_OPDE_OPD321	(0xE << 4)		/* OPD3, OPD2 and OPD1 are output */
#define RCAR_RPC_DRENR_OPDE_OPD3210	(0xF << 4)		/* OPD3, OPD2, OPD1 and OPD0 are output */

/* SMCR BIT Definition */
#define RCAR_RPC_SMCR_SSLKP			(1 << 8)		/* QSPIn_SSL Signal Level */
#define RCAR_RPC_SMCR_SPIRE			(1 << 2)		/* Data Read Enable */
#define RCAR_RPC_SMCR_SPIWE			(1 << 1)		/* Data Write Enable */
#define RCAR_RPC_SMCR_SPIE			(1 << 0)		/* SPI Data Transfer Enable */

/* SMCMR BIT Definition */
#define RCAR_RPC_SMCMR_CMD_CLEAR	~(0xFF << 16)	/* Command */
#define RCAR_RPC_SMCMR_CMD(x)		(x << 16)		/* x = 0x00 -> 0xFF */
#define RCAR_RPC_SMCMR_OCMD_CLEAR	~(0xFF << 0)	/* Optional command */
#define RCAR_RPC_SMCMR_OCMD(x)		(x << 0)		/* x = 0x00 -> 0xFF */

/* SMENR BIT Definition */
#define RCAR_RPC_SMENR_CDB_CLEAR	~(0x3 << 30)	/* Command Bit Size */
#define RCAR_RPC_SMENR_CDB_1BIT		(0x0 << 30)
#define RCAR_RPC_SMENR_CDB_2BITS	(0x1 << 30)
#define RCAR_RPC_SMENR_CDB_4BITS	(0x2 << 30)
#define RCAR_RPC_SMENR_OCBD_CLEAR	~(0x3 << 28)	/* Optional Command Bit Size */
#define RCAR_RPC_SMENR_OCBD_1BIT	(0x0 << 28)
#define RCAR_RPC_SMENR_OCBD_2BITS	(0x1 << 28)
#define RCAR_RPC_SMENR_OCBD_4BITS	(0x2 << 28)
#define RCAR_RPC_SMENR_ADB_CLEAR	~(0x3 << 24)	/* Address Bit Size */
#define RCAR_RPC_SMENR_ADB_1BIT		(0x0 << 24)
#define RCAR_RPC_SMENR_ADB_2BITS	(0x1 << 24)
#define RCAR_RPC_SMENR_ADB_4BITS	(0x2 << 24)
#define RCAR_RPC_SMENR_OPDB_CLEAR	~(0x3 << 20)	/* Optional Data Bit Size */
#define RCAR_RPC_SMENR_OPDB_1BIT	(0x0 << 20)
#define RCAR_RPC_SMENR_OPDB_2BITS	(0x1 << 20)
#define RCAR_RPC_SMENR_OPDB_4BITS	(0x2 << 20)
#define RCAR_RPC_SMENR_SPIDB_CLEAR	~(0x3 << 16)	/* Transfer Data Bit Size */
#define RCAR_RPC_SMENR_SPIDB_1BIT	(0x0 << 16)
#define RCAR_RPC_SMENR_SPIDB_2BITS	(0x1 << 16)
#define RCAR_RPC_SMENR_SPIDB_4BITS	(0x2 << 16)
#define RCAR_RPC_SMENR_DME			(1 << 15)		/* Dummy Cycle Enable */
#define RCAR_RPC_SMENR_CDE			(1 << 14)		/* Command Enable */
#define RCAR_RPC_SMENR_OCDE			(1 << 12)		/* Optional Command Enable */
#define RCAR_RPC_SMENR_ADE_CLEAR	~(0xF << 8)		/* Address Enable. ADR[31:0] is output */
#define RCAR_RPC_SMENR_ADE_DISABLE	(0x0 << 8)			/* Output disabled */
#define RCAR_RPC_SMENR_ADE_23_16	(0x4 << 8)			/* ADR[23:16] is output */
#define RCAR_RPC_SMENR_ADE_23_8		(0x6 << 8)			/* ADR[23:8] is output */
#define RCAR_RPC_SMENR_ADE_23_0		(0x7 << 8)			/* ADR[23:0] is output */
#define RCAR_RPC_SMENR_ADE_31_0		(0xF << 8)			/* ADR[31:0] is output */
#define RCAR_RPC_SMENR_OPDE_CLEAR	~(0xF << 4)		/* Optional Data Enable. OPD3, OPD2, OPD1, and OPD0 are output. */
#define RCAR_RPC_SMENR_OPDE_DISABLE	(0x0 << 4)
#define RCAR_RPC_SMENR_OPDE_OPD3	(0x8 << 4)			/* OPD3 is output */
#define RCAR_RPC_SMENR_OPDE_OPD32	(0xC << 4)			/* OPD3 and OPD2 are output */
#define RCAR_RPC_SMENR_OPDE_OPD321	(0xE << 4)			/* OPD3, OPD2 and OPD1 are output */
#define RCAR_RPC_SMENR_OPDE_OPD3210	(0xF << 4)			/* OPD3, OPD2, OPD1 and OPD0 are output */
#define RCAR_RPC_SMENR_SPIDE_CLEAR	~(0xF << 0)		/* Transfer Data Enable */
#define RCAR_RPC_SMENR_SPIDE_DIS	(0x0 << 0)			/* Not transferred */
#define RCAR_RPC_SMENR_SPIDE_16BITS	(0x8 << 0)			/* 16 bits transferred */
#define RCAR_RPC_SMENR_SPIDE_32BITS	(0xC << 0)			/* 32 bits transferred */
#define RCAR_RPC_SMENR_SPIDE_64BITS	(0xF << 0)			/* 64 bits transferred */

/* CMNSR BIT Definition */
#define RCAR_RPC_CMNSR_SSLF			(1<<1)			/* QSPIn_SSL Pin State Monitor. 0: negated, 1: asserted */
#define RCAR_RPC_CMNSR_TEND			(1<<0)			/* Transfer End Flag. 0: in progress, 1: ended */


/* DRDMCR BIT Definition */
#define RCAR_RPC_DRDMCR_DMDB_CLEAR	~(0x3 << 16)	/* Dummy Cycle Bit Size */
#define RCAR_RPC_DRDMCR_DMDB_1BIT	(0x0 << 16)
#define RCAR_RPC_DRDMCR_DMDB_2BITS	(0x1 << 16)
#define RCAR_RPC_DRDMCR_DMDB_4BITS	(0x2 << 16)
#define RCAR_RPC_DRDMCR_DMCYC_CLEAR	~(0x1F << 0)	/* Number of Dummy Cycles Setting */
#define RCAR_RPC_DRDMCR_DMCYC(x)	(x << 0)		/* x = 0 -> 19 */

/* DRDRENR BIT Definition */
#define RCAR_RPC_DRDRENR_HYPE_CLEAR	~(0x7 << 12)	/* HyperFlash or 8-bit serial flash in DDR mode Enable */
#define RCAR_RPC_DRDRENR_HYPE		(0x5 << 12)			/* HyperFlash or 8-bit serial flash in DDR mode */
#define RCAR_RPC_DRDRENR_HYPE_SPI	(0x0 << 12)			/* SPI flash mode */
#define RCAR_RPC_DRDRENR_ADDRE		(1 << 8)		/* Address DDR Enable */
#define RCAR_RPC_DRDRENR_OPDRE		(1 << 4)		/* Option Data DDR Enable */
#define RCAR_RPC_DRDRENR_DRDRE		(1 << 0)		/* Data Dead DDR Enable */

/* SMDMCR BIT Definitions */
#define RCAR_RPC_SMDMCR_DMDB_CLEAR	~(0x3 << 16)	/* Dummy Cycle Bit Size */
#define RCAR_RPC_SMDMCR_DMDB_1BIT	(0x0 << 16)
#define RCAR_RPC_SMDMCR_DMDB_2BITS	(0x1 << 16)
#define RCAR_RPC_SMDMCR_DMDB_4BITS	(0x2 << 16)
#define RCAR_RPC_SMDMCR_DMCYC_CLEAR	~(0x1F << 0)	/* Number of Dummy Cycles Setting */
#define RCAR_RPC_SMDMCR_DMCYC(x)	(x << 0)		/* x = 0 -> 19 */

/* SMDRENR BIT Definitions */
#define RCAR_RPC_SMDRENR_HYPE_CLEAR	~(0x7 << 12)	/* HyperFlash or 8-bit serial flash in DDR mode Enable */
#define RCAR_RPC_SMDRENR_HYPE		(0x5 << 12)			/* HyperFlash or 8-bit serial flash in DDR mode */
#define RCAR_RPC_SMDRENR_HYPE_SPI	(0x0 << 12)			/* SPI flash mode */
#define RCAR_RPC_SMDRENR_ADDRE		(1 << 8)		/* Address DDR Enable */
#define RCAR_RPC_SMDRENR_OPDRE		(1 << 4)		/* Option Data DDR Enable */
#define RCAR_RPC_SMDRENR_SPIDRE		(1 << 0)		/* Data Dead DDR Enable */

/* PHYCNT BIT Definitions */
#define RCAR_RPC_PHYCNT_CAL			(1 << 31)		/* PHY Calibration. 0: not executed, 1: executed */
#define RCAR_RPC_PHYCNT_OCTA		~(0x3 << 22)	/* 8-bit serial flash alignment */
#define RCAR_RPC_PHYCNT_EXDS		(1 << 21)		/* External Data Strobe. 0: Not use, 1: use*/
#define RCAR_RPC_PHYCNT_OCT			(1 << 20)		/* 8-bit serial flash DDR protocol mode */
#define RCAR_RPC_PHYCNT_WBUF2		(1 << 4)		/* Write Buffer Enable2 */
#define RCAR_RPC_PHYCNT_WBUF		(1 << 2)		/* Write Buffer Enable */
#define RCAR_RPC_PHYCNT_PHYMEM_HYPE	(0x3 << 0)		/* Device Selection. HyperFlash */

/* PHYINT BIT Definitions */
#define RCAR_RPC_PHYINT_RSTEN		(1<<18)			/* RPC_RESET# Pin Enable */
#define RCAR_RPC_PHYINT_WPEN		(1<<17)			/* RPC_WP# Pin Enable */
#define RCAR_RPC_PHYINT_INTEN		(1<<16)			/* RPC_INT# Pin Enable */
#define RCAR_RPC_PHYINT_RSTVAL		(1<<2)			/* RPC_RESET# Pin Output Value */
#define RCAR_RPC_PHYINT_WPVAL		(1<<1)			/* RPC_WP# Pin Output Value */
#define RCAR_RPC_PHYINT_INT			(1<<0)			/* Interrupt Status */

/*
 * USB 2.0
 */
#define RCAR_USB0_BASE              0xEE080000  // USB2.0 ch0
#define RCAR_USB1_BASE              0xEE0A0000  // USB2.0 ch1
#define RCAR_USB2_BASE              0xEE0C0000  // USB2.0 ch2

#define RCAR_PCI_CONF_EHCI          0x10100     // Offset to PCI configuration Register


/* Register offset */
/* AHB bridge registers */
#define RCAR_USB_AHB_INT_ENABLE     0x200
#define RCAR_USB_AHB_USBCTR         0x20C

#define RCAR_USB_AHB_PLL_RST        (1 << 1)
#define RCAR_USB_AHB_USBH_INTBEN    (1 << 2)
#define RCAR_USB_AHB_USBH_INTAEN    (1 << 1)

/* Core defined registers */
#define RCAR_USB_CORE_SPD_RSM_TIMSET     0x30C
#define RCAR_USB_CORE_OC_TIMSET          0x310

/*
 * HSUSB
 */
#define RCAR_HSUSB_BASE             0xE6590000
#define RCAR_HSUSB_REG_LPSTS        (RCAR_HSUSB_BASE + 0x0102)   /* 16bit */
#define RCAR_HSUSB_SUSPM            0x4000
#define RCAR_HSUSB_SUSPM_NORMAL     0x4000

#define RCAR_HSUSB_REG_UGCTRL2      (RCAR_HSUSB_BASE + 0x0184)   /* 32bit */
#define RCAR_HSUSB_USB0SEL          0x00000030
#define RCAR_HSUSB_USB0SEL_EHCI     0x00000010

#define RCAR_HSUSB_SYSCFG           0x0000
#define RCAR_HSUSB_INTENB0          0x0030
#define RCAR_HSUSB_UGCTRL           0x0180
#define RCAR_HSUSB_UGCTRL2          0x0184

/*
 * USB 3.0
 */
#define RCAR_USB30_BASE             0xEE000000  // USB3.0 ch0
#define RCAR_USB31_BASE             0xEE040000  // USB3.0 ch1
#define USB30_SIZE                  0xBFF

/*** Register Offset ***/
#define RCAR_USB3_CLASSCODE         0x204
#define RCAR_USB3_RELEASE_NUMBER    0x210

#define RCAR_USB3_INT_ENA           0x224   /* Interrupt Enable */
#define RCAR_USB3_DL_CTRL           0x250   /* FW Download Control & Status */
#define RCAR_USB3_FW_DATA0          0x258   /* FW Data0 */

#define RCAR_USB3_LCLK              0xa44   /* LCLK Select */
#define RCAR_USB3_CONF1             0xa48   /* USB3.0 Configuration1 */
#define RCAR_USB3_CONF2             0xa5c   /* USB3.0 Configuration2 */
#define RCAR_USB3_CONF3             0xaa8   /* USB3.0 Configuration3 */
#define RCAR_USB3_RX_POL            0xab0   /* USB3.0 RX Polarity */
#define RCAR_USB3_TX_POL            0xab8   /* USB3.0 TX Polarity */

/*** Register Settings ***/
/* Interrupt Enable */
#define RCAR_USB3_INT_XHC_ENA       0x00000001
#define RCAR_USB3_INT_PME_ENA       0x00000002
#define RCAR_USB3_INT_HSE_ENA       0x00000004
#define RCAR_USB3_INT_LTM_ENA       0x00000008
#define RCAR_USB3_INT_SMI_ENA       0x00000010
#define RCAR_USB3_INT_ENA_VAL       (RCAR_USB3_INT_XHC_ENA | RCAR_USB3_INT_PME_ENA | RCAR_USB3_INT_HSE_ENA)

/* FW Download Control & Status */
#define RCAR_USB3_DL_CTRL_ENABLE        0x00000001
#define RCAR_USB3_DL_CTRL_FW_SUCCESS    0x00000010
#define RCAR_USB3_DL_CTRL_FW_SET_DATA0  0x00000100

/* LCLK Select */
#define RCAR_USB3_LCLK_ENA_VAL      0x01030001

/* USB3.0 Configuration */
#define RCAR_USB3_CONF1_VAL         0x00030204
#define RCAR_USB3_CONF2_VAL         0x00030300
#define RCAR_USB3_CONF3_VAL         0x13802007

/* USB3.0 Polarity */
#define BIT(nr)                     (1UL << (nr))
#define RCAR_USB3_RX_POL_VAL        BIT(21)
#define RCAR_USB3_TX_POL_VAL        BIT(4)

/*
 * DBSC4 Controller
 */
#define RCAR_DBSC4_BASE             0xE6790000
 #define RCAR_SDRAM_0               0x40000000
 #define RCAR_SDRAM_1               0x500000000
 #define RCAR_SDRAM_2               0x600000000
 #define RCAR_SDRAM_3               0x700000000

/* DBSC4 registers, offset from DBSC4_BASE */

 #define RCAR_DBSC4_DBSYSCONF1      0x0004
 #define RCAR_DBSC4_DBPHYCONF0      0x0010
 #define RCAR_DBSC4_DBKIND          0x0020
 #define RCAR_DBSC4_DBMEMCONF_0_0   0x0030
 #define RCAR_DBSC4_DBMEMCONF_0_1   0x0034
 #define RCAR_DBSC4_DBMEMCONF_1_0   0x0040
 #define RCAR_DBSC4_DBMEMCONF_1_1   0x0044
 #define RCAR_DBSC4_DBMEMCONF_2_0   0x0050
 #define RCAR_DBSC4_DBMEMCONF_2_1   0x0054
 #define RCAR_DBSC4_DBMEMCONF_3_0   0x0060
 #define RCAR_DBSC4_DBMEMCONF_3_1   0x0064

 #define RCAR_DBSC4_DBMEMCONF_0_2   0x0038
 #define RCAR_DBSC4_DBMEMCONF_0_3   0x003C
 #define RCAR_DBSC4_DBMEMCONF_1_2   0x0048
 #define RCAR_DBSC4_DBMEMCONF_1_3   0x004C
 #define RCAR_DBSC4_DBMEMCONF_2_2   0x0058
 #define RCAR_DBSC4_DBMEMCONF_2_3   0x005C
 #define RCAR_DBSC4_DBMEMCONF_3_2   0x0068
 #define RCAR_DBSC4_DBMEMCONF_3_3   0x006C


 #define RCAR_DBSC4_DBSTATE0        0x0108

 #define RCAR_DBSC4_DBACEN          0x0200
 #define RCAR_DBSC4_DBRFEN          0x0204
 #define RCAR_DBSC4_DBCMD           0x0208

 #define RCAR_DBSC4_DBWAIT          0x0210	//wait DBCMD 1=busy, 0=ready

 #define RCAR_DBSC4_DBTR0           0x0300
 #define RCAR_DBSC4_DBTR1           0x0304
 #define RCAR_DBSC4_DBTR3           0x030C
 #define RCAR_DBSC4_DBTR4           0x0310
 #define RCAR_DBSC4_DBTR5           0x0314
 #define RCAR_DBSC4_DBTR6           0x0318
 #define RCAR_DBSC4_DBTR7           0x031C
 #define RCAR_DBSC4_DBTR8           0x0320
 #define RCAR_DBSC4_DBTR9           0x0324
 #define RCAR_DBSC4_DBTR10          0x0328
 #define RCAR_DBSC4_DBTR11          0x032C
 #define RCAR_DBSC4_DBTR12          0x0330
 #define RCAR_DBSC4_DBTR13          0x0334
 #define RCAR_DBSC4_DBTR14          0x0338
 #define RCAR_DBSC4_DBTR15          0x033C
 #define RCAR_DBSC4_DBTR16          0x0340
 #define RCAR_DBSC4_DBTR17          0x0344
 #define RCAR_DBSC4_DBTR18          0x0348
 #define RCAR_DBSC4_DBTR19          0x034C
 #define RCAR_DBSC4_DBTR20          0x0350
 #define RCAR_DBSC4_DBTR21          0x0354
 #define RCAR_DBSC4_DBTR22          0x0358


 #define RCAR_DBSC4_DBRFCNF1        0x0414
 #define RCAR_DBSC4_DBRFCNF2        0x0418

 #define RCAR_DBSC4_DBRNK0          0x0430
 #define RCAR_DBSC4_DBRNK1          0x0434
 #define RCAR_DBSC4_DBRNK2          0x0438
 #define RCAR_DBSC4_DBRNK3          0x043C
 #define RCAR_DBSC4_DBRNK4          0x0440
 #define RCAR_DBSC4_DBRNK5          0x0444
 #define RCAR_DBSC4_DBRNK6          0x0448

 #define RCAR_DBSC4_DBADJ0          0x0500
 #define RCAR_DBSC4_DBADJ2          0x0508

 #define RCAR_DBSC4_DBDFIPMSTRCNF   0x0520

 #define RCAR_DBSC4_DBPDLK_0        0x0620
 #define RCAR_DBSC4_DBPDLK_1        0x0660
 #define RCAR_DBSC4_DBPDLK_2        0x06a0
 #define RCAR_DBSC4_DBPDLK_3        0x06e0


 #define RCAR_DBSC4_INITCOMP_0      0x0600
 #define RCAR_DBSC4_INITCOMP_1      0x0640
 #define RCAR_DBSC4_INITCOMP_2      0x0680
 #define RCAR_DBSC4_INITCOMP_3      0x06C0


 #define RCAR_DBSC4_DBDFICNT_0      0x0604
 #define RCAR_DBSC4_DBDFICNT_1      0x0644
 #define RCAR_DBSC4_DBDFICNT_2      0x0684
 #define RCAR_DBSC4_DBDFICNT_3      0x06C4

 #define RCAR_DBSC4_DBPDCNT0_0      0x0610
 #define RCAR_DBSC4_DBPDCNT0_1      0x0650
 #define RCAR_DBSC4_DBPDCNT0_2      0x0690
 #define RCAR_DBSC4_DBPDCNT0_3      0x06D0

 #define RCAR_DBSC4_DBPDCNT_0       0x061C
 #define RCAR_DBSC4_DBPDCNT_1       0x065C
 #define RCAR_DBSC4_DBPDCNT_2       0x069C
 #define RCAR_DBSC4_DBPDCNT_3       0x06DC

 #define RCAR_DBSC4_DBPDRGA_0       0x0624
 #define RCAR_DBSC4_DBPDRGD_0       0x0628
 #define RCAR_DBSC4_DBPDRGA_1       0x0664
 #define RCAR_DBSC4_DBPDRGD_1       0x0668
 #define RCAR_DBSC4_DBPDRGA_2       0x06A4
 #define RCAR_DBSC4_DBPDRGD_2       0x06A8
 #define RCAR_DBSC4_DBPDRGA_3       0x06E4
 #define RCAR_DBSC4_DBPDRGD_3       0x06E8

 #define RCAR_DBSC4_DBBUS0CNF0      0x0800
 #define RCAR_DBSC4_DBBUS0CNF1      0x0804

 #define RCAR_DBSC4_DBCAM0CNF0      0x0900	//CAM Unit Configuration Register 0
 #define RCAR_DBSC4_DBCAM0CNF1      0x0904	//CAM Unit Configuration Register 1
 #define RCAR_DBSC4_DBCAM0CNF2      0x0908	//CAM Unit Configuration Register 2
 #define RCAR_DBSC4_DBCAM0CNF3      0x090C	//CAM Unit Configuration Register 3
 #define RCAR_DBSC4_DBSCHCNT0       0x1000	//Scheduler Unit Operation Setting Register 0
 #define RCAR_DBSC4_DBSCHCNT1       0x1004	//Scheduler Unit Operation Setting Register 1
 #define RCAR_DBSC4_DBSCHSZ0        0x1010	//Size Miss Scheduling Setting Register 0
 #define RCAR_DBSC4_DBSCHRW0        0x1020	//Read/Write Scheduling Setting Register 0
 #define RCAR_DBSC4_DBSCHRW1        0x1024	//Read/Write Scheduling Setting Register 1


 #define RCAR_DBSC4_DBSCHQOS_4_0    0x1070
 #define RCAR_DBSC4_DBSCHQOS_4_1    0x1074
 #define RCAR_DBSC4_DBSCHQOS_4_2    0x1078
 #define RCAR_DBSC4_DBSCHQOS_4_3    0x107C
 #define RCAR_DBSC4_DBSCHQOS_9_0    0x10C0
 #define RCAR_DBSC4_DBSCHQOS_9_1    0x10C4
 #define RCAR_DBSC4_DBSCHQOS_9_2    0x10C8
 #define RCAR_DBSC4_DBSCHQOS_9_3    0x10CC
 #define RCAR_DBSC4_DBSCHQOS_13_0   0x1100
 #define RCAR_DBSC4_DBSCHQOS_13_1   0x1104
 #define RCAR_DBSC4_DBSCHQOS_13_2   0x1108
 #define RCAR_DBSC4_DBSCHQOS_13_3   0x110C
 #define RCAR_DBSC4_DBSCHQOS_14_0   0x1110
 #define RCAR_DBSC4_DBSCHQOS_14_1   0x1114
 #define RCAR_DBSC4_DBSCHQOS_14_2   0x1118
 #define RCAR_DBSC4_DBSCHQOS_14_3   0x111C
 #define RCAR_DBSC4_DBSCHQOS_15_0   0x1120
 #define RCAR_DBSC4_DBSCHQOS_15_1   0x1124
 #define RCAR_DBSC4_DBSCHQOS_15_2   0x1128
 #define RCAR_DBSC4_DBSCHQOS_15_3   0x112C

 #define RCAR_DBSC4_SCFCTST0        0x1700	//Schedule timing setting register 0
 #define RCAR_DBSC4_SCFCTST1        0x1708	//Schedule timing setting register 1
 #define RCAR_DBSC4_SCFCTST2        0x170C	//Schedule timing setting register 2

 #define RCAR_DBSC4_PLL_LOCK_0      0x4054
 #define RCAR_DBSC4_PLL_LOCK_1      0x4154
 #define RCAR_DBSC4_PLL_LOCK_2      0x4254
 #define RCAR_DBSC4_PLL_LOCK_3      0x4354

 #define RCAR_DBSC4_FREQ_CHG_ACK_0  0x0618
 #define RCAR_DBSC4_FREQ_CHG_ACK_1  0x0658
 #define RCAR_DBSC4_FREQ_CHG_ACK_2  0x0698
 #define RCAR_DBSC4_FREQ_CHG_ACK_3  0x06D8

 #define RCAR_DBSC4_DFI_FREQ_0      0x0614
 #define RCAR_DBSC4_DFI_FREQ_1      0x0654
 #define RCAR_DBSC4_DFI_FREQ_2      0x0694
 #define RCAR_DBSC4_DFI_FREQ_3      0x06D4

/*
 * Thermal Sensor
 */
#define RCAR_THS_TSC0               0xE6190000
#define RCAR_THS_TSC1               0xE6198000
#define RCAR_THS_TSC2               0xE61A0000
#define RCAR_THS_TSC3               0xE61A8000

/*
 * Audio Clock Generator
 */
#define RCAR_ADG_BASE               0xEC5A0000

 #define RCAR_ADG_BRRA              0x00
 #define RCAR_ADG_BRRB              0x04
 #define RCAR_ADG_SSICKR            0x08
 #define RCAR_ADG_CKSEL0            0x0C
 #define RCAR_ADG_CKSEL1            0x10
 #define RCAR_ADG_CKSEL2            0x14
 #define RCAR_ADG_TIM_EN            0x30
 #define RCAR_ADG_SRCIN_TIMSEL0     0x34
 #define RCAR_ADG_SRCIN_TIMSEL1     0x38
 #define RCAR_ADG_SRCIN_TIMSEL2     0x3C
 #define RCAR_ADG_SRCIN_TIMSEL3     0x40
 #define RCAR_ADG_SRCIN_TIMSEL4     0x44
 #define RCAR_ADG_SRCOUT_TIMSEL0    0x48
 #define RCAR_ADG_SRCOUT_TIMSEL1    0x4C
 #define RCAR_ADG_SRCOUT_TIMSEL2    0x50
 #define RCAR_ADG_SRCOUT_TIMSEL3    0x54
 #define RCAR_ADG_SRCOUT_TIMSEL4    0x58
 #define RCAR_ADG_CMDOUT_TIMSEL     0x5C
 #define RCAR_ADG_DTCP_TIMSEL       0x64
 #define RCAR_ADG_ADSP_TIMSEL1      0x80
 #define RCAR_ADG_ADSP_TIMSEL2      0x84
 #define RCAR_ADG_ADSP_TIMSEL3      0x88
 #define RCAR_ADG_SSICKR1           0x100
 #define RCAR_ADG_AVB_SYNC_SEL0     0x104
 #define RCAR_ADG_AVB_SYNC_SEL1     0x10C
 #define RCAR_ADG_AVB_SYNC_SEL2     0x110
 #define RCAR_ADG_AVB_SYNC_DIV0	    0x114
 #define RCAR_ADG_AVB_SYNC_DIV1	    0x118
 #define RCAR_ADG_AVB_CLK_DIV0	    0x11C
 #define RCAR_ADG_AVB_CLK_DIV1	    0x120
 #define RCAR_ADG_AVB_CLK_DIV2	    0x124
 #define RCAR_ADG_AVB_CLK_DIV3	    0x128
 #define RCAR_ADG_AVB_CLK_DIV4	    0x12C
 #define RCAR_ADG_AVB_CLK_DIV5	    0x130
 #define RCAR_ADG_AVB_CLK_DIV6	    0x134
 #define RCAR_ADG_AVB_CLK_DIV7	    0x138
 #define RCAR_ADG_AVB_CLK_CONFIG    0x13C
 #define RCAR_ADG_AVB_CLK_STATUS    0x140

/*
 * Video Capture
 */
#define RCAR_VIN0_BASE				0xE6EF0000
#define RCAR_VIN4_BASE				0xE6EF4000
#define RCAR_VIN_SIZE				0x1000

 #define RCAR_VIN_MC				0x0000
 #define RCAR_VIN_MS				0x0004
 #define RCAR_VIN_FC				0x0008
 #define RCAR_VIN_SLPRC				0x000C
 #define RCAR_VIN_ELPRC				0x0010
 #define RCAR_VIN_SPPRC				0x0014
 #define RCAR_VIN_EPPRC				0x0018
 #define RCAR_VIN_CSI_IFMD			0x0020
 #define RCAR_VIN_IS				0x002C
 #define RCAR_VIN_LC				0x003C
 #define RCAR_VIN_IE				0x0040
 #define RCAR_VIN_INTS				0x0044
 #define RCAR_VIN_SI				0x0048
 #define RCAR_VIN_DMR				0x0058
 #define RCAR_VIN_DMR2				0x005C
 #define RCAR_VIN_UVAOF				0x0060
 #define RCAR_VIN_UDS_CTRL			0x0080
 #define RCAR_VIN_UDS_SCALE			0x0084
 #define RCAR_VIN_UDS_PASS_BW		0x0090
 #define RCAR_VIN_UDS_IPC			0x0098
 #define RCAR_VIN_UDS_CLIPSIZE		0x00A4
 #define RCAR_VIN_LUTP				0x0100
 #define RCAR_VIN_LUTD				0x0104
 #define RCAR_VIN_MB(x)				0x0030 + 0x04*x
 #define RCAR_VIN_CSCC(x)			0x0064 + 0x04*x
 #define RCAR_VIN_YCCR(x)			0x0228 + 0x04*x
 #define RCAR_VIN_CBCCR(x)			0x0234 + 0x04*x
 #define RCAR_VIN_CRCCR(x)			0x0240 + 0x04*x
 #define RCAR_VIN_CSCE(x)			0x0300 + 0x04*x

/*
 * Camera Serial Interface 2
 */
#define RCAR_CSI40_BASE				0xFEAA0000
#define RCAR_CSI20_BASE				0xFEA80000
#define RCAR_CSI2_SIZE				0x10000

#define RCAR_CSI2_TREF				0x00
#define RCAR_CSI2_SRST				0x04
#define RCAR_CSI2_PHYCNT			0x08
#define RCAR_CSI2_CHKSUM			0x0C
#define RCAR_CSI2_VCDT				0x10
#define RCAR_CSI2_VCDT2				0x14
#define RCAR_CSI2_FRDT				0x18
#define RCAR_CSI2_FLD				0x1C
#define RCAR_CSI2_ASTBY				0x20
#define RCAR_CSI2_LNGDT0			0x28
#define RCAR_CSI2_LNGDT1			0x2C
#define RCAR_CSI2_INTEN				0x30
#define RCAR_CSI2_INTCLOSE			0x34
#define RCAR_CSI2_INTSTATE			0x38
#define RCAR_CSI2_INTERRSTATE		0x3C
#define RCAR_CSI2_SHPDAT			0x40
#define RCAR_CSI2_SHPCNT			0x44
#define RCAR_CSI2_LINKCNT			0x48
#define RCAR_CSI2_LSWAP				0x4C
#define RCAR_CSI2_PHTC				0x58
#define RCAR_CSI2_PHYPLL			0x68
#define RCAR_CSI2_PHEERM			0x74
#define RCAR_CSI2_PHCLM				0x78
#define RCAR_CSI2_PHDLM				0x7C

/*
 * L2 Cache
 */
#define RCAR_L2CPL310_BASE          0xF0100000

/*
 * LBSC
 */
#define RCAR_LBSC_BASE              0xFEC00000

 #define RCAR_LBSC_CS0CTRL          0x200
 #define RCAR_LBSC_CS1CTRL          0x204
 #define RCAR_LBSC_ECS0CTRL         0x208
 #define RCAR_LBSC_ECS1CTRL         0x20C
 #define RCAR_LBSC_CSWCR0           0x230
 #define RCAR_LBSC_CSWCR1           0x234
 #define RCAR_LBSC_ECSWCR0          0x238
 #define RCAR_LBSC_ECSWCR1          0x23C
 #define RCAR_LBSC_CSPWCR0          0x280
 #define RCAR_LBSC_CSPWCR1          0x284
 #define RCAR_LBSC_ECSPWCR0         0x288
 #define RCAR_LBSC_ECSPWCR1         0x28C
 #define RCAR_LBSC_EXWTSYNC         0x2A0
 #define RCAR_LBSC_CS1GDST          0x2C0
 #define RCAR_LBSC_ECS0GDST         0x2C4
 #define RCAR_LBSC_ECS1GDST         0x2C8
 #define RCAR_LBSC_ATACSCTRL        0x380

/*
 * Product Register (PRR)
 */
#define RCAR_PRODUCT_REGISTER       0xFFF00044
#define RCAR_PRODUCT_ID(x)          ((x & 0x7F00) >> 8)
 #define PRODUCT_ID_RCAR_H3         0x4F
 #define PRODUCT_ID_RCAR_M3         0x52
#define RCAR_PRODUCT_REV(x)         (x & 0xFF)
 #define PRODUCT_REV_1_0            0x00
 #define PRODUCT_REV_1_1            0x01
 #define PRODUCT_REV_2_0            0x10
 #define PRODUCT_REV_3_0            0x20

/***************************
 * Interrupt ID
 ************************* */
#define RCAR_INTCSYS_IRQ0           (  0 + 32)
#define RCAR_INTCSYS_IRQ1           (  1 + 32)
#define RCAR_INTCSYS_IRQ2           (  2 + 32)
#define RCAR_INTCSYS_IRQ3           (  3 + 32)
#define RCAR_INTCSYS_GPIO0          (  4 + 32)
#define RCAR_INTCSYS_GPIO1          (  5 + 32)
#define RCAR_INTCSYS_GPIO2          (  6 + 32)
#define RCAR_INTCSYS_GPIO3          (  7 + 32)
#define RCAR_INTCSYS_GPIO4          (  8 + 32)
#define RCAR_INTCSYS_GPIO5          (  9 + 32)
#define RCAR_INTCSYS_SCIF4          ( 16 + 32)
#define RCAR_INTCSYS_SCIF5          ( 17 + 32)
#define RCAR_INTCSYS_I2C4           ( 19 + 32)
#define RCAR_INTCSYS_I2C5           ( 20 + 32)
#define RCAR_INTCSYS_SCIF3          ( 23 + 32)
#define RCAR_INTCSYS_THERMAL0       ( 67 + 32)
#define RCAR_INTCSYS_THERMAL1       ( 68 + 32)
#define RCAR_INTCSYS_THERMAL2       ( 69 + 32)
#define RCAR_INTCSYS_SCIF0          (152 + 32)
#define RCAR_INTCSYS_SCIF1          (153 + 32)
#define RCAR_INTCSYS_HSCIF0         (154 + 32)
#define RCAR_INTCSYS_HSCIF1         (155 + 32)
#define RCAR_INTCSYS_HSCIF2         (144 + 32)
#define RCAR_INTCSYS_HSCIF3         (145 + 32)
#define RCAR_INTCSYS_HSCIF4         (146 + 32)
#define RCAR_INTCSYS_MSIOF0         (156 + 32)
#define RCAR_INTCSYS_MSIOF1         (157 + 32)
#define RCAR_INTCSYS_MSIOF2         (158 + 32)
#define RCAR_INTCSYS_MSIOF3         (159 + 32)
#define RCAR_INTCSYS_SCIF2          (164 + 32)
#define RCAR_INTCSYS_SDHI0          (165 + 32)
#define RCAR_INTCSYS_SDHI1          (166 + 32)
#define RCAR_INTCSYS_SDHI2          (167 + 32)
#define RCAR_INTCSYS_SDHI3          (168 + 32)
#define RCAR_INTCSYS_MMC0           (169 + 32)
#define RCAR_INTCSYS_MMC1           (170 + 32)
#define RCAR_INTCSYS_IIC3           (173 + 32)
#define RCAR_INTCSYS_VIN4           (174 + 32)
#define RCAR_INTCSYS_VIN5           (175 + 32)
#define RCAR_INTCSYS_VIN6           (176 + 32)
#define RCAR_INTCSYS_QSPI           (184 + 32)
#define RCAR_INTCSYS_VIN0           (188 + 32)
#define RCAR_INTCSYS_VIN1           (189 + 32)
#define RCAR_INTCSYS_VIN2           (190 + 32)
#define RCAR_INTCSYS_VIN3           (191 + 32)
#define RCAR_INTCSYS_I2C2           (286 + 32)
#define RCAR_INTCSYS_I2C0           (287 + 32)
#define RCAR_INTCSYS_I2C1           (288 + 32)
#define RCAR_INTCSYS_I2C3           (290 + 32)
#define RCAR_INTCSYS_DMASDHI0       (291 + 32)
#define RCAR_INTCSYS_DMASDHI1       (292 + 32)
#define RCAR_INTCSYS_DMASDHI2       (293 + 32)
#define RCAR_INTCSYS_DMASDHI3       (294 + 32)

/*
 * DMA request ID
 */
#define RCAR_DREQ_SCIFA0_TXI        0x21
#define RCAR_DREQ_SCIFA0_RXI        0x22
#define RCAR_DREQ_SCIFA1_TXI        0x25
#define RCAR_DREQ_SCIFA1_RXI        0x26
#define RCAR_DREQ_SCIFA2_TXI        0x27
#define RCAR_DREQ_SCIFA2_RXI        0x28
#define RCAR_DREQ_SCIFB0_TXI        0x3D
#define RCAR_DREQ_SCIFB0_RXI        0x3E
#define RCAR_DREQ_SCIFB1_TXI        0x19
#define RCAR_DREQ_SCIFB1_RXI        0x1A
#define RCAR_DREQ_SCIFB2_TXI        0x1D
#define RCAR_DREQ_SCIFB2_RXI        0x1E
#define RCAR_DREQ_HSCIF0_TXI        0x39
#define RCAR_DREQ_HSCIF0_RXI        0x3A
#define RCAR_DREQ_HSCIF1_TXI        0x4D
#define RCAR_DREQ_HSCIF1_RXI        0x4E
#define RCAR_DREQ_SCIF0_TXI         0x29
#define RCAR_DREQ_SCIF0_RXI         0x2A
#define RCAR_DREQ_SCIF1_TXI         0x2D
#define RCAR_DREQ_SCIF1_RXI         0x2E
#define RCAR_DREQ_MSIOF0_TXI        0x81
#define RCAR_DREQ_MSIOF0_RXI        0x82
#define RCAR_DREQ_MSIOF1_TXI        0x85
#define RCAR_DREQ_MSIOF1_RXI        0x86
#define RCAR_DREQ_MSIOF2_TXI        0x41
#define RCAR_DREQ_MSIOF2_RXI        0x42
#define RCAR_DREQ_MSIOF3_TXI        0x45
#define RCAR_DREQ_MSIOF3_RXI        0x46
#define RCAR_DREQ_QSPI_TXI          0x17
#define RCAR_DREQ_QSPI_RXI          0x18
#define RCAR_DREQ_SIM_TXI           0xA1
#define RCAR_DREQ_SIM_RXI           0xA2
#define RCAR_DREQ_I2C0_TXI          0x61
#define RCAR_DREQ_I2C0_RXI          0x62
#define RCAR_DREQ_I2C1_TXI          0x65
#define RCAR_DREQ_I2C1_RXI          0x66
#define RCAR_DREQ_I2C2_TXI          0x69
#define RCAR_DREQ_I2C2_RXI          0x6A
#define RCAR_DREQ_IIC_DVFS_TXI      0x77
#define RCAR_DREQ_IIC_DVFS_RXI      0x78
#define RCAR_DREQ_SDHI0_TXI         0xCD
#define RCAR_DREQ_SDHI0_RXI         0xCE
#define RCAR_DREQ_SDHI1_TXI         0xC9
#define RCAR_DREQ_SDHI1_RXI         0xCA
#define RCAR_DREQ_SDHI2_TXI         0xC1
#define RCAR_DREQ_SDHI2_RXI         0xC2
#define RCAR_DREQ_SDHI2_C2_TXI      0xC5
#define RCAR_DREQ_SDHI2_C2_RXI      0xC6
#define RCAR_DREQ_SDHI3_TXI         0xD3
#define RCAR_DREQ_SDHI3_RXI         0xD4
#define RCAR_DREQ_SDHI3_C2_TXI      0xDF
#define RCAR_DREQ_SDHI3_C2_RXI      0xDE
#define RCAR_DREQ_TPU0_TXI          0xF1
#define RCAR_DREQ_TSIF0_RXI         0xEA
#define RCAR_DREQ_TSIF1_RXI         0xF0
#define RCAR_DREQ_AXISTATR_RXI      0xA6
#define RCAR_DREQ_AXISTATR0_RXI     0xAC
#define RCAR_DREQ_AXISTATR1_RXI     0xAA
#define RCAR_DREQ_AXISTAT2_RXI      0xA8
#define RCAR_DREQ_AXISTAT3C_RXI     0xA4
#define RCAR_DREQ_MMCIF0_TXI        0xD1
#define RCAR_DREQ_MMCIF0_RXI        0xD2
#define RCAR_DREQ_MMCIF1_TXI        0xE1
#define RCAR_DREQ_MMCIF1_RXI        0xE2
#define RCAR_DREQ_AXSTM_RXI         0xAE


#endif /* __ARM_RCAR_H_INCLUDED */

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/hardware/startup/lib/public/arm/r-car-m3.h $ $Rev: 814384 $")
#endif
