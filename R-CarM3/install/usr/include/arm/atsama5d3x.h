/*
 * $QNXLicenseC:
 * Copyright 2013, QNX Software Systems.
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


#ifndef _ATSAMA5D3X_H__
#define _ATSAMA5D3X_H__

/* ************************************************************************** */
/*   PERIPHERAL ID DEFINITIONS FOR SAMA5D3x                                   */
/* ************************************************************************** */

#define SAMA5D3X_ID_FIQ     0    /* Advanced Interrupt Controller (FIQ) */
#define SAMA5D3X_ID_SYS     1    /* System Controller Interrupt (SYS) */
#define SAMA5D3X_ID_DBGU    2    /* Debug Unit Interrupt (DBGU) */
#define SAMA5D3X_ID_PIT     3    /* Periodic Interval Timer Interrupt (PIT) */
#define SAMA5D3X_ID_WDT     4    /* Watchdog timer Interrupt (WDT) */
#define SAMA5D3X_ID_SMC     5    /* Multi-bit ECC Interrupt (SMC) */
#define SAMA5D3X_ID_PIOA    6    /* Parallel I/O Controller A (PIOA) */
#define SAMA5D3X_ID_PIOB    7    /* Parallel I/O Controller B (PIOB) */
#define SAMA5D3X_ID_PIOC    8    /* Parallel I/O Controller C (PIOC) */
#define SAMA5D3X_ID_PIOD    9    /* Parallel I/O Controller D (PIOD) */
#define SAMA5D3X_ID_PIOE   10    /* Parallel I/O Controller E (PIOE) */
#define SAMA5D3X_ID_SMD    11    /* SMD Soft Modem (SMD) */
#define SAMA5D3X_ID_USART0 12    /* USART 0 (USART0) */
#define SAMA5D3X_ID_USART1 13    /* USART 1 (USART1) */
#define SAMA5D3X_ID_USART2 14    /* USART 2 (USART2) */
#define SAMA5D3X_ID_USART3 15    /* USART 3 (USART3) */
#define SAMA5D3X_ID_UART0  16    /* UART 0 (UART0) */
#define SAMA5D3X_ID_UART1  17    /* UART 1 (UART1) */
#define SAMA5D3X_ID_TWI0   18    /* Two-Wire Interface 0 (TWI0) */
#define SAMA5D3X_ID_TWI1   19    /* Two-Wire Interface 1 (TWI1) */
#define SAMA5D3X_ID_TWI2   20    /* Two-Wire Interface 2 (TWI2) */
#define SAMA5D3X_ID_HSMCI0 21    /* High Speed Multimedia Card Interface 0 (HSMCI0) */
#define SAMA5D3X_ID_HSMCI1 22    /* High Speed Multimedia Card Interface 1 (HSMCI1) */
#define SAMA5D3X_ID_HSMCI2 23    /* High Speed Multimedia Card Interface 2 (HSMCI2) */
#define SAMA5D3X_ID_SPI0   24    /* Serial Peripheral Interface 0 (SPI0) */
#define SAMA5D3X_ID_SPI1   25    /* Serial Peripheral Interface 1 (SPI1) */
#define SAMA5D3X_ID_TC0    26    /* Timer Counter 0 (ch. 0, 1, 2) (TC0) */
#define SAMA5D3X_ID_TC1    27    /* Timer Counter 1 (ch. 3, 4, 5) (TC1) */
#define SAMA5D3X_ID_PWM    28    /* Pulse Width Modulation Controller (PWM) */
#define SAMA5D3X_ID_ADC    29    /* Touch Screen ADC Controller (ADC) */
#define SAMA5D3X_ID_DMAC0  30    /* DMA Controller 0 (DMAC0) */
#define SAMA5D3X_ID_DMAC1  31    /* DMA Controller 1 (DMAC1) */
#define SAMA5D3X_ID_UHPHS  32    /* USB Host High Speed (UHPHS) */
#define SAMA5D3X_ID_UDPHS  33    /* USB Device High Speed (UDPHS) */
#define SAMA5D3X_ID_GMAC   34    /* Gigabit Ethernet MAC (GMAC) */
#define SAMA5D3X_ID_EMAC   35    /* Ethernet MAC (EMAC) */
#define SAMA5D3X_ID_LCDC   36    /* LCD Controller (LCDC) */
#define SAMA5D3X_ID_ISI    37    /* Image Sensor Interface (ISI) */
#define SAMA5D3X_ID_SSC0   38    /* Synchronous Serial Controller 0 (SSC0) */
#define SAMA5D3X_ID_SSC1   39    /* Synchronous Serial Controller 1 (SSC1) */
#define SAMA5D3X_ID_CAN0   40    /* CAN controller 0 (CAN0) */
#define SAMA5D3X_ID_CAN1   41    /* CAN controller 1 (CAN1) */
#define SAMA5D3X_ID_SHA    42    /* Secure Hash Algorithm (SHA) */
#define SAMA5D3X_ID_AES    43    /* Advanced Encryption Standard (AES) */
#define SAMA5D3X_ID_TDES   44    /* Triple Data EncryptionStandard (TDES) */
#define SAMA5D3X_ID_TRNG   45    /* True Random Number Generator (TRNG) */
#define SAMA5D3X_ID_ARM    46    /* Performance Monitor Unit (ARM) */
#define SAMA5D3X_ID_IRQ    47    /* Advanced Interrupt Controller (IRQ) */
#define SAMA5D3X_ID_FUSE   48    /* Fuse Controller (FUSE) */
#define SAMA5D3X_ID_MPDDRC 49    /* MPDDR controller (MPDDRC) */

#define SAMA5D3X_ID_PERIPH_COUNT 50    /* Number of peripheral IDs */



/* ************************************************************************** */
/*   BASE ADDRESS DEFINITIONS FOR SAMA5D3x                                    */
/* ************************************************************************** */

#define SAMA5D3X_NFC_SRAM    0x00200000    /* (NFC_SRAM) Base Address */
#define SAMA5D3X_SMD_BASE    0x00400000    /* (SMD   ) Base Address */
#define SAMA5D3X_AXIMX_BASE  0x00800000    /* (AXIMX ) Base Address */
#define SAMA5D3X_HSMCI0_BASE 0xF0000000    /* (HSMCI0) Base Address */
#define SAMA5D3X_SPI0_BASE   0xF0004000    /* (SPI0  ) Base Address */
#define SAMA5D3X_SSC0_BASE   0xF0008000    /* (SSC0  ) Base Address */
#define SAMA5D3X_CAN0_BASE   0xF000C000    /* (CAN0  ) Base Address */
#define SAMA5D3X_TC0_BASE    0xF0010000    /* (TC0   ) Base Address */
#define SAMA5D3X_TWI0_BASE   0xF0014000    /* (TWI0  ) Base Address */
#define SAMA5D3X_TWI1_BASE   0xF0018000    /* (TWI1  ) Base Address */
#define SAMA5D3X_USART0_BASE 0xF001C000    /* (USART0) Base Address */
#define SAMA5D3X_USART1_BASE 0xF0020000    /* (USART1) Base Address */
#define SAMA5D3X_UART0_BASE  0xF0024000    /* (UART0 ) Base Address */
#define SAMA5D3X_GMAC_BASE   0xF0028000    /* (GMAC  ) Base Address */
#define SAMA5D3X_PWM_BASE    0xF002C000    /* (PWM   ) Base Address */
#define SAMA5D3X_LCDC_BASE   0xF0030000    /* (LCDC  ) Base Address */
#define SAMA5D3X_ISI_BASE    0xF0034000    /* (ISI   ) Base Address */
#define SAMA5D3X_SFR_BASE    0xF0038000    /* (SFR   ) Base Address */
#define SAMA5D3X_HSMCI1_BASE 0xF8000000    /* (HSMCI1) Base Address */
#define SAMA5D3X_HSMCI2_BASE 0xF8004000    /* (HSMCI2) Base Address */
#define SAMA5D3X_SPI1_BASE   0xF8008000    /* (SPI1  ) Base Address */
#define SAMA5D3X_SSC1_BASE   0xF800C000    /* (SSC1  ) Base Address */
#define SAMA5D3X_CAN1_BASE   0xF8010000    /* (CAN1  ) Base Address */
#define SAMA5D3X_TC1_BASE    0xF8014000    /* (TC1   ) Base Address */
#define SAMA5D3X_ADC_BASE    0xF8018000    /* (ADC   ) Base Address */
#define SAMA5D3X_TWI2_BASE   0xF801C000    /* (TWI2  ) Base Address */
#define SAMA5D3X_USART2_BASE 0xF8020000    /* (USART2) Base Address */
#define SAMA5D3X_USART3_BASE 0xF8024000    /* (USART3) Base Address */
#define SAMA5D3X_UART1_BASE  0xF8028000    /* (UART1 ) Base Address */
#define SAMA5D3X_EMAC_BASE   0xF802C000    /* (EMAC  ) Base Address */
#define SAMA5D3X_UDPHS_BASE  0xF8030000    /* (UDPHS ) Base Address */
#define SAMA5D3X_SHA_BASE    0xF8034000    /* (SHA   ) Base Address */
#define SAMA5D3X_AES_BASE    0xF8038000    /* (AES   ) Base Address */
#define SAMA5D3X_TDES_BASE   0xF803C000    /* (TDES  ) Base Address */
#define SAMA5D3X_TRNG_BASE   0xF8040000    /* (TRNG  ) Base Address */
#define SAMA5D3X_SMC_BASE    0xFFFFC000    /* (SMC   ) Base Address */
#define SAMA5D3X_FUSE_BASE   0xFFFFE400    /* (FUSE  ) Base Address */
#define SAMA5D3X_DMAC0_BASE  0xFFFFE600    /* (DMAC0 ) Base Address */
#define SAMA5D3X_DMAC1_BASE  0xFFFFE800    /* (DMAC1 ) Base Address */
#define SAMA5D3X_MPDDRC_BASE 0xFFFFEA00    /* (MPDDRC) Base Address */
#define SAMA5D3X_MATRIX_BASE 0xFFFFEC00    /* (MATRIX) Base Address */
#define SAMA5D3X_DBGU_BASE   0xFFFFEE00    /* (DBGU  ) Base Address */
#define SAMA5D3X_AIC_BASE    0xFFFFF000    /* (AIC   ) Base Address */
#define SAMA5D3X_PIOA_BASE   0xFFFFF200    /* (PIOA  ) Base Address */
#define SAMA5D3X_PIOB_BASE   0xFFFFF400    /* (PIOB  ) Base Address */
#define SAMA5D3X_PIOC_BASE   0xFFFFF600    /* (PIOC  ) Base Address */
#define SAMA5D3X_PIOD_BASE   0xFFFFF800    /* (PIOD  ) Base Address */
#define SAMA5D3X_PIOE_BASE   0xFFFFFA00    /* (PIOE  ) Base Address */
#define SAMA5D3X_PMC_BASE    0xFFFFFC00    /* (PMC   ) Base Address */
#define SAMA5D3X_RSTC_BASE   0xFFFFFE00    /* (RSTC  ) Base Address */
#define SAMA5D3X_SHDWC_BASE  0xFFFFFE10    /* (SHDWC ) Base Address */
#define SAMA5D3X_PIT_BASE    0xFFFFFE30    /* (PIT   ) Base Address */
#define SAMA5D3X_WDT_BASE    0xFFFFFE40    /* (WDT   ) Base Address */
#define SAMA5D3X_SCKC_BASE   0xFFFFFE50    /* (SCKC  ) Base Address */
#define SAMA5D3X_BSC_BASE    0xFFFFFE54    /* (BSC   ) Base Address */
#define SAMA5D3X_GPBR_BASE   0xFFFFFE60    /* (GPBR  ) Base Address */
#define SAMA5D3X_RTC_BASE    0xFFFFFEB0    /* (RTC   ) Base Address */


/* ************************************************************************** */
/*   MEMORY MAPPING DEFINITIONS FOR SAMA5D3x */
/* ************************************************************************** */

#define SAMA5D3X_IRAM0_SIZE 0x10000
#define SAMA5D3X_IRAM1_SIZE 0x10000
#define SAMA5D3X_IROM_SIZE  0x20000
#define SAMA5D3X_IRAM_SIZE  (IRAM0_SIZE+IRAM1_SIZE)

#define SAM5D3X_NFC_SRAM_SIZE 0x2000
#define SAMA5D3X_EBI_MEM_SIZE 0x10000000

#define SAMA5D3X_EBI_CS0_BASE   0x10000000 /**< EBI Chip Select 0 base address */
#define SAMA5D3X_DDR_CS_BASE    0x20000000 /**< DDR Chip Select base address */
#define SAMA5D3X_EBI_CS1_BASE   0x40000000 /**< EBI Chip Select 1 base address */
#define SAMA5D3X_EBI_CS2_BASE   0x50000000 /**< EBI Chip Select 2 base address */
#define SAMA5D3X_EBI_CS3_BASE   0x60000000 /**< EBI Chip Select 3 base address */
#define SAMA5D3X_EBI_NF_BASE    0x60000000 /**< NAND Flash on EBI Chip Select 3 base address */
#define SAMA5D3X_NFC_BASE       0x70000000 /**< NAND Flash Controller Command base address */
#define SAMA5D3X_IROM_BASE      0x00100000 /**< Internal ROM base address */
#define SAMA5D3X_IRAM0_BASE     0x00300000 /**< Internal RAM 0 base address */
#if defined IRAM0_SIZE
#define SAMA5D3X_IRAM1_BASE     (IRAM0_ADDR+IRAM0_SIZE) /**< Internal RAM 1 base address */
#endif
#define SAMA5D3X_UDPHS_RAM_BASE 0x00500000 /**< USB High Speed Device Port RAM base address */
#define SAMA5D3X_UHP_OHCI_BASE  0x00600000 /**< USB Host Port OHCI base address */
#define SAMA5D3X_UHP_EHCI_BASE  0x00700000 /**< USB Host Port EHCI base address */
#define SAMA5D3X_DAP_BASE       0x00900000 /**< Debug Access Port base address */



/* ************************************************************************** */
/*   Peripheral definitions		*/
/* ************************************************************************** */

/* Periodic Interval Timer */
#define SAMA5D3X_PIT_SIZE                    0x10
#define SAMA5D3X_PIT_MR                      0x00
#define SAMA5D3X_PIT_SR                      0x04
#define SAMA5D3X_PIT_PIVR                    0x08
#define SAMA5D3X_PIT_PIIR                    0x0c

/* (DBGU) */
#define SAMA5D3X_DBGU_SIZE                   0x100
#define SAMA5D3X_DBGU_CR                     0x00
#define SAMA5D3X_DBGU_MR                     0x04
#define SAMA5D3X_DBGU_IER                    0x08
#define SAMA5D3X_DBGU_IDR                    0x0c
#define SAMA5D3X_DBGU_IMR                    0x10
#define SAMA5D3X_DBGU_SR                     0x14
#define SAMA5D3X_DBGU_RHR                    0x18
#define SAMA5D3X_DBGU_THR                    0x1c
#define SAMA5D3X_DBGU_BRGR                   0x20
#define SAMA5D3X_DBGU_CIDR                   0x40
#define SAMA5D3X_DBGU_EXID                   0x44
#define SAMA5D3X_DBGU_FNR                    0x48


/* Advanced Interrupt Controller */
#define SAMA5D3X_AIC_SIZE       0x200
#define SAMA5D3X_AIC_SSR		0x00 	// Source Select Register
#define SAMA5D3X_AIC_SMR		0x04	// Source Mode Register
#define SAMA5D3X_AIC_SVR		0x08	// Source Vector Register
#define SAMA5D3X_AIC_IVR		0x10	// Interrupt Vector Register
#define SAMA5D3X_AIC_FVR		0x14	// FIQ Interrupt Vector Register
#define SAMA5D3X_AIC_ISR		0x18	// Interrupt Status Register
#define SAMA5D3X_AIC_IPR0		0x20	// Interrupt Pending Register 0(2)
#define SAMA5D3X_AIC_IPR3		0x2C	// Interrupt Pending Register 3(2)
#define SAMA5D3X_AIC_IMR		0x30	// Interrupt Mask Register
#define SAMA5D3X_AIC_CISR		0x34	// Core Interrupt Status Register
#define SAMA5D3X_AIC_EOICR		0x38	// End of Interrupt Command Register
#define SAMA5D3X_AIC_SPU		0x3C	// Spurious Interrupt Vector Register
#define SAMA5D3X_AIC_IECR		0x40	// Interrupt Enable Command Register
#define SAMA5D3X_AIC_IDCR		0x44	// Interrupt Disable Command Register
#define SAMA5D3X_AIC_ICCR		0x48	// Interrupt Clear Command Register
#define SAMA5D3X_AIC_ISCR		0x4C	// Interrupt Set Command Register
#define SAMA5D3X_AIC_FFER		0x50	// Fast Forcing Enable Register
#define SAMA5D3X_AIC_FFDR		0x54	// Fast Forcing Disable Register
#define SAMA5D3X_AIC_FFSR		0x58	// Fast Forcing Status Register
#define SAMA5D3X_AIC_DCR		0x6C	// Debug Control Register
#define SAMA5D3X_AIC_WPMR		0xE4	// Write Protect Mode Register
#define SAMA5D3X_AIC_WPSR		0xE8	// Write Protect Status Register

#define SAMA5D3X_AIC_WP_KEY		0x41494300	// Write Protect key for AIC


/* Power Management Controller */
#define SAMA5D3X_PMC_SCER		0x0000	// System Clock Enable Register
#define SAMA5D3X_PMC_SCDR		0x0004	// System Clock Disable Register
#define SAMA5D3X_PMC_SCSR		0x0008	// System Clock Status Register
#define SAMA5D3X_PMC_PCER0		0x0010	// Peripheral Clock Enable Register 0
#define SAMA5D3X_PMC_PCDR0		0x0014	// Peripheral Clock Disable Register 0
#define SAMA5D3X_PMC_PCSR0		0x0018	// Peripheral Clock Status Register 0
#define SAMA5D3X_CKGR_UCKR		0x001C	// UTMI Clock Register
#define SAMA5D3X_CKGR_MOR		0x0020	// Main Oscillator Register
#define SAMA5D3X_CKGR_MCFR		0x0024	// Main Clock Frequency Register
#define SAMA5D3X_CKGR_PLLAR		0x0028	// PLLA Register
#define SAMA5D3X_PMC_MCKR		0x0030	// Master Clock Register
#define SAMA5D3X_PMC_USB		0x0038	// USB Clock Register
#define SAMA5D3X_PMC_SMD		0x003C	// Soft Modem Clock Register
#define SAMA5D3X_PMC_PCK0		0x0040	// Programmable Clock 0 Register
#define SAMA5D3X_PMC_PCK1		0x0044	// Programmable Clock 1 Register
#define SAMA5D3X_PMC_PCK2		0x0048	// Programmable Clock 2 Register
#define SAMA5D3X_PMC_IER		0x0060	// Interrupt Enable Register
#define SAMA5D3X_PMC_IDR		0x0064	// Interrupt Disable Register
#define SAMA5D3X_PMC_SR			0x0068	// Status Register
#define SAMA5D3X_PMC_IMR		0x006C	// Interrupt Mask Register
#define SAMA5D3X_PMC_FOCR		0x0078	// Fault Output Clear Register
#define SAMA5D3X_PMC_PLLICPR	0x0080	// PLL Charge Pump Current Register
#define SAMA5D3X_PMC_WPMR		0x00E4	// Write Protect Mode Register
#define SAMA5D3X_PMC_WPSR		0x00E8	// Write Protect Status Register
#define SAMA5D3X_PMC_PCER1		0x0100	// Peripheral Clock Enable Register 1
#define SAMA5D3X_PMC_PCDR1		0x0104	// Peripheral Clock Disable Register 1
#define SAMA5D3X_PMC_PCSR1		0x0108	// Peripheral Clock Status Register 1
#define SAMA5D3X_PMC_PCR		0x010C	// Peripheral Control Register
#define SAMA5D3X_PMC_OCR		0x0110	// Oscillator Calibration Register

#define SAMA5D3X_PMC_WP_KEY		0x504D4300	// Write Protect key for PMC


/* -------- PMC_PCR : (PMC Offset: 0x010C) Peripheral Control Register -------- */
#define PMC_PCR_PID_Pos 		0
#define PMC_PCR_PID_Msk 		(0x3F << PMC_PCR_PID_Pos)
#define PMC_PCR_PID(value) 		((PMC_PCR_PID_Msk & ((value) << PMC_PCR_PID_Pos)))
#define PMC_PCR_CMD 			(1 << 12)
#define PMC_PCR_DIV_Pos 		16
#define PMC_PCR_DIV_Msk 		(3 << PMC_PCR_DIV_Pos)
#define   PMC_PCR_DIV_PERIPH_DIV_MCK 	(0 << 16)
#define   PMC_PCR_DIV_PERIPH_DIV2_MCK 	(1 << 16)
#define   PMC_PCR_DIV_PERIPH_DIV4_MCK 	(2 << 16)
#define   PMC_PCR_DIV_PERIPH_DIV8_MCK 	(3 << 16)
#define PMC_PCR_EN 				(1 << 28)

/* USB clock Definitions */
#define PMC_UCKR_PLLCOUNT		(0x3 << 20)	/* PLL count for USB Clock */
#define PMC_UCKR_UPLLEN			(0x1 << 16)	/* UTMI PLL is enabled */
#define PMC_UCKR_BIASEN			(0x1 << 24)	/* UTMI BIAS is enabled */
#define PMC_SR_LOCKU			0x00000040	/* UPLL lock bit */


/* PIO (Pinmux and GPIO controllers) */
#define SAMA5D3X_PIO_PER		0x0000	// PIO Enable Register
#define SAMA5D3X_PIO_PDR		0x0004	// PIO Disable Register
#define SAMA5D3X_PIO_PSR		0x0008	// PIO Status Register
#define SAMA5D3X_PIO_OER		0x0010	// Output Enable Register
#define SAMA5D3X_PIO_ODR		0x0014	// Output Disable Register
#define SAMA5D3X_PIO_OSR		0x0018	// Output Status Register
#define SAMA5D3X_PIO_IFER		0x0020	// Glitch Input Filter Enable Register
#define SAMA5D3X_PIO_IFDR		0x0024	// Glitch Input Filter Disable Register
#define SAMA5D3X_PIO_IFSR		0x0028	// Glitch Input Filter Status Register
#define SAMA5D3X_PIO_SODR		0x0030	// Set Output Data Register
#define SAMA5D3X_PIO_CODR		0x0034	// Clear Output Data Register
#define SAMA5D3X_PIO_ODSR		0x0038	// Output Data Status Register
#define SAMA5D3X_PIO_PDSR		0x003C	// Pin Data Status Register
#define SAMA5D3X_PIO_IER		0x0040	// Interrupt Enable Register
#define SAMA5D3X_PIO_IDR		0x0044	// Interrupt Disable Register
#define SAMA5D3X_PIO_IMR		0x0048	// Interrupt Mask Register
#define SAMA5D3X_PIO_ISR		0x004C	// Interrupt Status Register(4)
#define SAMA5D3X_PIO_MDER		0x0050	// Multi-driver Enable Register
#define SAMA5D3X_PIO_MDDR		0x0054	// Multi-driver Disable Register
#define SAMA5D3X_PIO_MDSR		0x0058	// Multi-driver Status Register
#define SAMA5D3X_PIO_PUDR		0x0060	// Pull-up Disable Register
#define SAMA5D3X_PIO_PUER		0x0064	// Pull-up Enable Register
#define SAMA5D3X_PIO_PUSR		0x0068	// Pad Pull-up Status Register
#define SAMA5D3X_PIO_ABCDSR1	0x0070	// Peripheral Select Register 1
#define SAMA5D3X_PIO_ABCDSR2	0x0074	// Peripheral Select Register 2
#define SAMA5D3X_PIO_IFSCDR		0x0080	// Input Filter Slow Clock Disable Register
#define SAMA5D3X_PIO_IFSCER		0x0084	// Input Filter Slow Clock Enable Register
#define SAMA5D3X_PIO_IFSCSR		0x0088	// Input Filter Slow Clock Status Register
#define SAMA5D3X_PIO_SCDR		0x008C	// Slow Clock Divider Debouncing Register
#define SAMA5D3X_PIO_PPDDR		0x0090	// Pad Pull-down Disable Register
#define SAMA5D3X_PIO_PPDER		0x0094	// Pad Pull-down Enable Register
#define SAMA5D3X_PIO_PPDSR		0x0098	// Pad Pull-down Status Register
#define SAMA5D3X_PIO_OWER		0x00A0	// Output Write Enable
#define SAMA5D3X_PIO_OWDR		0x00A4	// Output Write Disable
#define SAMA5D3X_PIO_OWSR		0x00A8	// Output Write Status Register
#define SAMA5D3X_PIO_AIMER		0x00B0	// Additional Interrupt Modes Enable Register
#define SAMA5D3X_PIO_AIMDR		0x00B4	// Additional Interrupt Modes Disables Register
#define SAMA5D3X_PIO_AIMMR		0x00B8	// Additional Interrupt Modes Mask Register
#define SAMA5D3X_PIO_ESR		0x00C0	// Edge Select Register
#define SAMA5D3X_PIO_LSR		0x00C4	// Level Select Register
#define SAMA5D3X_PIO_ELSR		0x00C8	// Edge/Level Status Register
#define SAMA5D3X_PIO_FELLSR		0x00D0	// Falling Edge/Low Level Select Register
#define SAMA5D3X_PIO_REHLSR		0x00D4	// Rising Edge/ High Level Select Register
#define SAMA5D3X_PIO_FRLHSR		0x00D8	// Fall/Rise - Low/High Status Register
#define SAMA5D3X_PIO_LOCKSR		0x00E0	// Lock Status
#define SAMA5D3X_PIO_WPMR		0x00E4	// Write Protect Mode Register
#define SAMA5D3X_PIO_WPSR		0x00E8	// Write Protect Status Register
#define SAMA5D3X_PIO_SCHMITT	0x0100	// Schmitt Trigger Register
#define SAMA5D3X_PIO_DRIVER1	0x0118	// I/O Drive Register 1
#define SAMA5D3X_PIO_DRIVER2	0x011C	// I/O Drive Register 2
#define SAMA5D3X_PIO_KER		0x0120	// Keypad Controller Enable Register
#define SAMA5D3X_PIO_KRCR		0x0124	// Keypad Controller Row Column Register
#define SAMA5D3X_PIO_KDR		0x0128	// Keypad Controller Debouncing Register
#define SAMA5D3X_PIO_KIER		0x0130	// Keypad Controller Interrupt Enable Register
#define SAMA5D3X_PIO_KIDR		0x0134	// Keypad Controller Interrupt Disable Register
#define SAMA5D3X_PIO_KIMR		0x0138	// Keypad Controller Interrupt Mask Register
#define SAMA5D3X_PIO_KSR		0x013C	// Keypad Controller Status Register
#define SAMA5D3X_PIO_KKPR		0x0140	// Keypad Controller Key Press Register
#define SAMA5D3X_PIO_KKRR		0x0144	// Keypad Controller Key Release Register

#define SAMA5D3X_PIO_WP_KEY		0x50494F00	// Write Protect key for PIO

/* TWI (Two Wire Interface controllers) */
#define SAMA5D3X_TWI_SIZE			0x100
#define SAMA5D3X_TWI_WPROT_MODE		0xE4	// Write Protect Mode Register

#define SAMA5D3X_TWI_WP_KEY			0x54574900	// Write Protect key for TWI

/* SFR (Special Function Registers) */
#define SAMA5D3X_SFR_OHCIICR		0x10 	// OHCI Interrupt Configuration Register
#define SAMA5D3X_SFR_OHCIISR		0x14 	// OHCI Interrupt Status Register
#define SAMA5D3X_SFR_AHB			0x20 	// AHB Configuration Register
#define SAMA5D3X_SFR_BRIDGE			0x24 	// Bridge Configuration Register
#define SAMA5D3X_SFR_SECURE			0x28 	// Security Configuration Register
#define SAMA5D3X_SFR_UTMICKTRIM		0x30 	// UTMI Clock Trimming Register
#define SAMA5D3X_SFR_UTMIHSTRIM		0x34 	// UTMI High Speed Trimming Register
#define SAMA5D3X_SFR_UTMIFSTRIM		0x38 	// UTMI Full Speed Trimming Register
#define SAMA5D3X_SFR_UTMISWAP		0x3C 	// UTMI DP/DM Pin Swapping Register
#define SAMA5D3X_SFR_EBICFG			0x40 	// EBI Configuration Register


/* USART */
#define SAMA5D3X_USART_SIZE			0x100

/* CAN */
#define SAMA5D3X_CAN_SIZE			0x400

/* RTC */
#define SAMA5D3X_RTC_SIZE			0x100
#define SAMA5D3X_RTC_TIME			0x08
#define SAMA5D3X_RTC_CAL			0x0C

/* RSTC (Reset Controller) */
#define SAMA5D3X_RSTC_SIZE			0x10
#define SAMA5D3X_RSTC_CR			0x00
#define SAMA5D3X_RSTC_SR			0x04
#define SAMA5D3X_RSTC_MR			0x08

/* Shutdown Controller */
#define SAMA5D3X_SHDWC_SIZE			0x0c
#define SAMA5D3X_SHDWC_CR			0x00
#define SAMA5D3X_SHDWC_MR			0x04
#define SAMA5D3X_SHDWC_SR			0x08

/* DMAC (DMA Controller) */
#define DMAC_GCFG			0x000	// DMAC Global Configuration Register
#define DMAC_EN				0x004	// DMAC Enable Register
#define DMAC_SREQ			0x008	// DMAC Software Single Request Register
#define DMAC_CREQ			0x00C	// DMAC Software Chunk Transfer Request Register
#define DMAC_LAST			0x010	// DMAC Software Last Transfer Flag Register
#define DMAC_EBCIER			0x018	// DMAC Error, Chained Buffer Transfer Completed Interrupt and Buffer Transfer Completed Interrupt Enable register.
#define DMAC_EBCIDR			0x01C	// DMAC Error, Chained Buffer Transfer Completed Interrupt and Buffer Transfer Completed Interrupt Disable register.
#define DMAC_EBCIMR			0x020	// DMAC Error, Chained Buffer Transfer Completed Interrupt and Buffer transfer completed Mask Register.
#define DMAC_EBCISR			0x024	// DMAC Error, Chained Buffer Transfer Completed Interrupt and Buffer transfer completed Status Register.
#define DMAC_CHER			0x028	// DMAC Channel Handler Enable Register
#define DMAC_CHDR			0x02C	// DMAC Channel Handler Disable Register
#define DMAC_CHSR			0x030	// DMAC Channel Handler Status Register
#define DMAC_SADDR(ch_num)	(0x03C+(ch_num)*(0x28)+(0x0))	// DMAC Channel Source Address Register
#define DMAC_DADDR(ch_num)	(0x03C+(ch_num)*(0x28)+(0x4))	// DMAC Channel Destination Address Register
#define DMAC_DSCR(ch_num)	(0x03C+(ch_num)*(0x28)+(0x8))	// DMAC Channel Descriptor Address Register
#define DMAC_CTRLA(ch_num)	(0x03C+(ch_num)*(0x28)+(0xC))	// DMAC Channel Control A Register
#define DMAC_CTRLB(ch_num)	(0x03C+(ch_num)*(0x28)+(0x10))	// DMAC Channel Control B Register
#define DMAC_CFG(ch_num)	(0x03C+(ch_num)*(0x28)+(0x14))	// DMAC Channel Configuration Register
#define DMAC_SPIP(ch_num)	(0x03C+(ch_num)*(0x28)+(0x18))	// DMAC Channel Source Picture-in-Picture Configuration Register
#define DMAC_DPIP(ch_num)	(0x03C+(ch_num)*(0x28)+(0x1C))	// DMAC Channel Destination Picture-in-Picture Configuration Register
#define DMAC_WPMR			0x1E4	// DMAC Write Protect Mode Register
#define DMAC_WPSR			0x1E8	// DMAC Write Protect Status Register

#define DMAC_WP_KEY		0x444D4100		// Write protect control key



/* LCDC (Display Controller) */
#define SAMA5D3X_LCDC_SIZE		0x2000

#define LCDC_LCDCFG0	0x0000		// LCD Controller Configuration Register 0
#define LCDC_LCDCFG1	0x0004		// LCD Controller Configuration Register 1
#define LCDC_LCDCFG2	0x0008		// LCD Controller Configuration Register 2
#define LCDC_LCDCFG3	0x000C		// LCD Controller Configuration Register 3
#define LCDC_LCDCFG4	0x0010		// LCD Controller Configuration Register 4
#define LCDC_LCDCFG5	0x0014		// LCD Controller Configuration Register 5
#define LCDC_LCDCFG6	0x0018		// LCD Controller Configuration Register 6
#define LCDC_LCDEN		0x0020		// LCD Controller Enable Register
#define LCDC_LCDDIS		0x0024		// LCD Controller Disable Register
#define LCDC_LCDSR		0x0028		// LCD Controller Status Register
#define LCDC_LCDIER		0x002C		// LCD Controller Interrupt Enable Register
#define LCDC_LCDIDR		0x0030		// LCD Controller Interrupt Disable Register
#define LCDC_LCDIMR		0x0034		// LCD Controller Interrupt Mask Register
#define LCDC_LCDISR		0x0038		// LCD Controller Interrupt Status Register
#define LCDC_BASECHER	0x0040		// Base Layer Channel Enable Register
#define LCDC_BASECHDR	0x0044		// Base Layer Channel Disable Register
#define LCDC_BASECHSR	0x0048		// Base Layer Channel Status Register
#define LCDC_BASEIER	0x004C		// Base Layer Interrupt Enable Register
#define LCDC_BASEIDR	0x0050		// Base Layer Interrupt Disabled Register
#define LCDC_BASEIMR	0x0054		// Base Layer Interrupt Mask Register
#define LCDC_BASEISR	0x0058		// Base Layer Interrupt status Register
#define LCDC_BASEHEAD	0x005C		// Base DMA Head Register
#define LCDC_BASEADDR	0x0060		// Base DMA Address Register
#define LCDC_BASECTRL	0x0064		// Base DMA Control Register
#define LCDC_BASENEXT	0x0068		// Base DMA Next Register
#define LCDC_BASECFG0	0x006C		// Base Configuration register 0
#define LCDC_BASECFG1	0x0070		// Base Configuration register 1
#define LCDC_BASECFG2	0x0074		// Base Configuration register 2
#define LCDC_BASECFG3	0x0078		// Base Configuration register 3
#define LCDC_BASECFG4	0x007C		// Base Configuration register 4
#define LCDC_BASECFG5	0x0080		// Base Configuration register 5
#define LCDC_BASECFG6	0x0084		// Base Configuration register 6
#define LCDC_OVR1CHER	0x0140		// Overlay 1 Channel Enable Register
#define LCDC_OVR1CHDR	0x0144		// Overlay 1 Channel Disable Register
#define LCDC_OVR1CHSR	0x0148		// Overlay 1 Channel Status Register
#define LCDC_OVR1IER	0x014C		// Overlay 1 Interrupt Enable Register
#define LCDC_OVR1IDR	0x0150		// Overlay 1 Interrupt Disable Register
#define LCDC_OVR1IMR	0x0154		// Overlay 1 Interrupt Mask Register
#define LCDC_OVR1ISR	0x0158		// Overlay 1 Interrupt Status Register
#define LCDC_OVR1HEAD	0x015C		// Overlay 1 DMA Head Register
#define LCDC_OVR1ADDR	0x0160		// Overlay 1 DMA Address Register
#define LCDC_OVR1CTRL	0x0164		// Overlay1 DMA Control Register
#define LCDC_OVR1NEXT	0x0168		// Overlay1 DMA Next Register
#define LCDC_OVR1CFG0	0x016C		// Overlay 1 Configuration 0 Register
#define LCDC_OVR1CFG1	0x0170		// Overlay 1 Configuration 1 Register
#define LCDC_OVR1CFG2	0x0174		// Overlay 1 Configuration 2 Register
#define LCDC_OVR1CFG3	0x0178		// Overlay 1 Configuration 3 Register
#define LCDC_OVR1CFG4	0x017C		// Overlay 1 Configuration 4 Register
#define LCDC_OVR1CFG5	0x0180		// Overlay 1 Configuration 5 Register
#define LCDC_OVR1CFG6	0x0184		// Overlay 1 Configuration 6 Register
#define LCDC_OVR1CFG7	0x0188		// Overlay 1 Configuration 7 Register
#define LCDC_OVR1CFG8	0x018C		// Overlay 1 Configuration 8Register
#define LCDC_OVR1CFG9	0x0190		// Overlay 1 Configuration 9 Register
#define LCDC_OVR2CHER	0x0240		// Overlay 2 Channel Enable Register
#define LCDC_OVR2CHDR	0x0244		// Overlay 2 Channel Disable Register
#define LCDC_OVR2CHSR	0x0248		// Overlay 2 Channel Status Register
#define LCDC_OVR2IER	0x024C		// Overlay 2 Interrupt Enable Register
#define LCDC_OVR2IDR	0x0250		// Overlay 2 Interrupt Disable Register
#define LCDC_OVR2IMR	0x0254		// Overlay 2 Interrupt Mask Register
#define LCDC_OVR2ISR	0x0258		// Overlay 2 Interrupt status Register
#define LCDC_OVR2HEAD	0x025C		// Overlay 2 DMA Head Register
#define LCDC_OVR2ADDR	0x0260		// Overlay 2 DMA Address Register
#define LCDC_OVR2CTRL	0x0264		// Overlay 2 DMA Control Register
#define LCDC_OVR2NEXT	0x0268		// Overlay 2 DMA Next Register
#define LCDC_OVR2CFG0	0x026C		// Overlay 2 Configuration 0 Register
#define LCDC_OVR2CFG1	0x0270		// Overlay 2 Configuration 1 Register
#define LCDC_OVR2CFG2	0x0274		// Overlay 2 Configuration 2 Register
#define LCDC_OVR2CFG3	0x0278		// Overlay 2 Configuration 3 Register
#define LCDC_OVR2CFG4	0x027C		// Overlay 2 Configuration 4 Register
#define LCDC_OVR2CFG5	0x0280		// Overlay 2 Configuration 5 Register
#define LCDC_OVR2CFG6	0x0284		// Overlay 2 Configuration 6 Register
#define LCDC_OVR2CFG7	0x0288		// Overlay 2 Configuration 7 Register
#define LCDC_OVR2CFG8	0x028C		// Overlay 2 Configuration 8 Register
#define LCDC_OVR2CFG9	0x0290		// Overlay 2 Configuration 9 Register
#define LCDC_HEOCHER	0x0340		// High-End Overlay Channel Enable Register
#define LCDC_HEOCHDR	0x0344		// High-End Overlay Channel Disable Register
#define LCDC_HEOCHSR	0x0348		// High-End Overlay Channel Status Register
#define LCDC_HEOIER		0x034C		// High-End Overlay Interrupt Enable Register
#define LCDC_HEOIDR		0x0350		// High-End Overlay Interrupt Disable Register
#define LCDC_HEOIMR		0x0354		// High-End Overlay Interrupt Mask Register
#define LCDC_HEOISR		0x0358		// High-End Overlay Interrupt Status Register
#define LCDC_HEOHEAD	0x035C		// High-End Overlay DMA Head Register
#define LCDC_HEOADDR	0x0360		// High-End Overlay DMA Address Register
#define LCDC_HEOCTRL	0x0364		// High-End Overlay DMA Control Register
#define LCDC_HEONEXT	0x0368		// High-End Overlay DMA Next Register
#define LCDC_HEOUHEAD	0x036C		// High-End Overlay U DMA Head Register
#define LCDC_HEOUADDR	0x0370		// High-End Overlay U DMA Address Register
#define LCDC_HEOUCTRL	0x0374		// High-End Overlay U DMA control Register
#define LCDC_HEOUNEXT	0x0378		// High-End Overlay U DMA Next Register
#define LCDC_HEOVHEAD	0x037C		// High-End Overlay V DMA Head Register
#define LCDC_HEOVADDR	0x0380		// High-End Overlay V DMA Address Register
#define LCDC_HEOVCTRL	0x0384		// High-End Overlay V DMA control Register
#define LCDC_HEOVNEXT	0x0388		// High-End Overlay VDMA Next Register
#define LCDC_HEOCFG0	0x038C		// High-End Overlay Configuration Register 0
#define LCDC_HEOCFG1	0x0390		// High-End Overlay Configuration Register 1
#define LCDC_HEOCFG2	0x0394		// High-End Overlay Configuration Register 2
#define LCDC_HEOCFG3	0x0398		// High-End Overlay Configuration Register 3
#define LCDC_HEOCFG4	0x039C		// High-End Overlay Configuration Register 4
#define LCDC_HEOCFG5	0x03A0		// High-End Overlay Configuration Register 5
#define LCDC_HEOCFG6	0x03A4		// High-End Overlay Configuration Register 6
#define LCDC_HEOCFG7	0x03A8		// High-End Overlay Configuration Register 7
#define LCDC_HEOCFG8	0x03AC		// High-End Overlay Configuration Register 8
#define LCDC_HEOCFG9	0x03B0		// High-End Overlay Configuration Register 9
#define LCDC_HEOCFG10	0x03B4		// High-End Overlay Configuration Register 10
#define LCDC_HEOCFG11	0x03B8		// High-End Overlay Configuration Register 11
#define LCDC_HEOCFG12	0x03BC		// High-End Overlay Configuration Register 12
#define LCDC_HEOCFG13	0x03C0		// High-End Overlay Configuration Register 13
#define LCDC_HEOCFG14	0x03C4		// High-End Overlay Configuration Register 14
#define LCDC_HEOCFG15	0x03C8		// High-End Overlay Configuration Register 15
#define LCDC_HEOCFG16	0x03CC		// High-End Overlay Configuration Register 16
#define LCDC_HEOCFG17	0x03D0		// High-End Overlay Configuration Register 17
#define LCDC_HEOCFG18	0x03D4		// High-End Overlay Configuration Register 18
#define LCDC_HEOCFG19	0x03D8		// High-End Overlay Configuration Register 19
#define LCDC_HEOCFG20	0x03DC		// High-End Overlay Configuration Register 20
#define LCDC_HEOCFG21	0x03E0		// High-End Overlay Configuration Register 21
#define LCDC_HEOCFG22	0x03E4		// High-End Overlay Configuration Register 22
#define LCDC_HEOCFG23	0x03E8		// High-End Overlay Configuration Register 23
#define LCDC_HEOCFG24	0x03EC		// High-End Overlay Configuration Register 24
#define LCDC_HEOCFG25	0x03F0		// High-End Overlay Configuration Register 25
#define LCDC_HEOCFG26	0x03F4		// High-End Overlay Configuration Register 26
#define LCDC_HEOCFG27	0x03F8		// High-End Overlay Configuration Register 27
#define LCDC_HEOCFG28	0x03FC		// High-End Overlay Configuration Register 28
#define LCDC_HEOCFG29	0x0400		// High-End Overlay Configuration Register 29
#define LCDC_HEOCFG30	0x0404		// High-End Overlay Configuration Register 30
#define LCDC_HEOCFG31	0x0408		// High-End Overlay Configuration Register 31
#define LCDC_HEOCFG32	0x040C		// High-End Overlay Configuration Register 32
#define LCDC_HEOCFG33	0x0410		// High-End Overlay Configuration Register 33
#define LCDC_HEOCFG34	0x0414		// High-End Overlay Configuration Register 34
#define LCDC_HEOCFG35	0x0418		// High-End Overlay Configuration Register 35
#define LCDC_HEOCFG36	0x041C		// High-End Overlay Configuration Register 36
#define LCDC_HEOCFG37	0x0420		// High-End Overlay Configuration Register 37
#define LCDC_HEOCFG38	0x0424		// High-End Overlay Configuration Register 38
#define LCDC_HEOCFG39	0x0428		// High-End Overlay Configuration Register 39
#define LCDC_HEOCFG40	0x042C		// High-End Overlay Configuration Register 40
#define LCDC_HEOCFG41	0x0430		// High-End Overlay Configuration Register 41
#define LCDC_HCRCHER	0x0440		// Hardware Cursor Channel Enable Register
#define LCDC_HCRCHDR	0x0444		// Hardware Cursor Channel disable Register
#define LCDC_HCRCHSR	0x0448		// Hardware Cursor Channel Status Register
#define LCDC_HCRIER		0x044C		// Hardware Cursor Interrupt Enable Register
#define LCDC_HCRIDR		0x0450		// Hardware Cursor Interrupt Disable Register
#define LCDC_HCRIMR		0x0454		// Hardware Cursor Interrupt Mask Register
#define LCDC_HCRISR		0x0458		// Hardware Cursor Interrupt Status Register
#define LCDC_HCRHEAD	0x045C		// Hardware Cursor DMA Head Register
#define LCDC_HCRADDR	0x0460		// Hardware cursor DMA Address Register
#define LCDC_HCRCTRL	0x0464		// Hardware Cursor DMA Control Register
#define LCDC_HCRNEXT	0x0468		// Hardware Cursor DMA NExt Register
#define LCDC_HCRCFG0	0x046C		// Hardware Cursor Configuration 0 Register
#define LCDC_HCRCFG1	0x0470		// Hardware Cursor Configuration 1 Register
#define LCDC_HCRCFG2	0x0474		// Hardware Cursor Configuration 2 Register
#define LCDC_HCRCFG3	0x0478		// Hardware Cursor Configuration 3 Register
#define LCDC_HCRCFG4	0x047C		// Hardware Cursor Configuration 4 Register
#define LCDC_HCRCFG6	0x0484		// Hardware Cursor Configuration 6 Register
#define LCDC_HCRCFG7	0x0488		// Hardware Cursor Configuration 7 Register
#define LCDC_HCRCFG8	0x048C		// Hardware Cursor Configuration 8 Register
#define LCDC_HCRCFG9	0x0490		// Hardware Cursor Configuration 9 Register
#define LCDC_PPCHER		0x0540		// Post Processing Channel Enable Register
#define LCDC_PPCHDR		0x0544		// Post Processing Channel Disable Register
#define LCDC_PPCHSR		0x0548		// Post Processing Channel Status Register
#define LCDC_PPIER		0x054C		// Post Processing Interrupt Enable Register
#define LCDC_PPIDR		0x0550		// Post Processing Interrupt Disable Register
#define LCDC_PPIMR		0x0554		// Post Processing Interrupt Mask Register
#define LCDC_PPISR		0x0558		// Post Processing Interrupt Status Register
#define LCDC_PPHEAD		0x055C		// Post Processing Head Register
#define LCDC_PPADDR		0x0560		// Post Processing Address Register
#define LCDC_PPCTRL		0x0564		// Post Processing Control Register
#define LCDC_PPNEXT		0x0568		// Post Processing Next Register
#define LCDC_PPCFG0		0x056C		// Post Processing Configuration Register 0
#define LCDC_PPCFG1		0x0570		// Post Processing Configuration Register 1
#define LCDC_PPCFG2		0x0574		// Post Processing Configuration Register 2
#define LCDC_PPCFG3		0x0578		// Post Processing Configuration Register 3
#define LCDC_PPCFG4		0x057C		// Post Processing Configuration Register 4
#define LCDC_PPCFG5		0x0580		// Post Processing Configuration Register 5

// Start of each register set
#define LCDC_LAYER_BASE			LCDC_BASECHER
#define LCDC_LAYER_OVERLAY1		LCDC_OVR1CHER
#define LCDC_LAYER_OVERLAY2		LCDC_OVR2CHER
#define LCDC_LAYER_HIGHPERF		LCDC_HEOCHER
#define LCDC_LAYER_CURSOR		LCDC_HCRCHER
#define LCDC_LAYER_POSTPROC		LCDC_PPCHER

// Common layers at the start of each layer block
#define LCDC_LAYER_CHER	0x00		// Channel Enable Register
#define LCDC_LAYER_CHDR	0x04		// Channel Disable Register
#define LCDC_LAYER_CHSR	0x08		// Channel Status Register
#define LCDC_LAYER_IER	0x0C		// Interrupt Enable Register
#define LCDC_LAYER_IDR	0x10		// Interrupt Disabled Register
#define LCDC_LAYER_IMR	0x14		// Interrupt Mask Register
#define LCDC_LAYER_ISR	0x18		// Interrupt status Register
#define LCDC_LAYER_HEAD	0x1C		// DMA Head Register
#define LCDC_LAYER_ADDR	0x20		// DMA Address Register
#define LCDC_LAYER_CTRL	0x24		// DMA Control Register
#define LCDC_LAYER_NEXT	0x28		// DMA Next Register

// Overlay 1 and Overlay 2 share the same register layout
#define LCDC_OVRCFG0	0x2C		// Overlay 1 Configuration 0 Register
#define LCDC_OVRCFG1	0x30		// Overlay 1 Configuration 1 Register
#define LCDC_OVRCFG2	0x34		// Overlay 1 Configuration 2 Register
#define LCDC_OVRCFG3	0x38		// Overlay 1 Configuration 3 Register
#define LCDC_OVRCFG4	0x3C		// Overlay 1 Configuration 4 Register
#define LCDC_OVRCFG5	0x40		// Overlay 1 Configuration 5 Register
#define LCDC_OVRCFG6	0x44		// Overlay 1 Configuration 6 Register
#define LCDC_OVRCFG7	0x48		// Overlay 1 Configuration 7 Register
#define LCDC_OVRCFG8	0x4C		// Overlay 1 Configuration 8 Register
#define LCDC_OVRCFG9	0x50		// Overlay 1 Configuration 9 Register

/*
 * SMC controller (NAND etc)
 */
#define SAMA5D3X_SMC_SIZE 0x700
#define SRAM0_SIZE 1200
#define SMC_CFG		0x00
	#define PS512	(0 << 0)
	#define PS1024	(1 << 0)
	#define PS2048	(2 << 0)
	#define PS4096	(3 << 0)
	#define PS8192	(4 << 0)
	#define WSPARE_ON	(1<<8)
	#define WSPARE_OFF	(0<<8)
	#define RSPARE_ON	(1<<9)
	#define RSPARE_OFF	(0<<9)
	#define EDGE_RISE	(0<<12)
	#define EDGE_FALL	(1<<12)
	#define RBEDGE_LEVEL (0<<13)
	#define RBEDGE_TRANS (1<<13)
	#define DTOCYC(n) (((n)&0xf)<<16)

	#define DTOMUL(n) (((n)&0x7)<<20)
		#define X1 0
		#define X16 1
		#define X128 2
		#define X256 3
		#define X1024 4
		#define X4096 5
		#define X65536 6
		#define X1048576 7
	#define CFC_SPARE_SIZE(n) (((((n)/4)-1)&0x7f)<<24)

#define SMC_CTRL	0x04
	#define NAND_ENABLE 0x1
	#define NAND_DISABLE 0x2

#define SMC_SR		0x08
	/* Masks to use on status and interrupt registers */
	#define SMCSTS	(1<<0)
	#define RB_RISE (1<<4)
	#define RB_FALL (1<<5)
	#define NFCBUSY (1<<8)
	#define NFCWR_ST (1<<11)
	#define NFCSID (7<<12)
	#define SMC_XFRDONE (1<<16)
	#define CMDDONE (1<<17)
	#define SMC_DTOE (1<<20)
	#define UNDEF (1<<21)
	#define AWB (1<<22)
	#define NFCASE (1<<23)
	#define RB_EDGE0 (1<<24)
	#define INVALID_BIT (1<<31) /* not defined in docs. Used by software */
	#define SMC_ERRORS (NFCASE | AWB | UNDEF | SMC_DTOE)

#define SMC_IER		0x0C
#define SMC_IDR		0x10
#define SMC_IMR		0x14
#define SMC_ADDR	0x18
#define SMC_BANK	0x1c
// reserved 0x020-0x06c
#define SMC_PME_CCFG	0x70
	#define BCH_ERR2 (0)
	#define BCH_ERR4 (1)
	#define BCH_ERR8 (2)
	#define BCH_ERR12 (3)
	#define BCH_ERR24 (4)
	#define SECTOR_512 ((0) << 4)
	#define SECTOR_1024 ((1) << 4)
	#define PAGESIZE_1SEC ((0) << 8)
	#define PAGESIZE_2SEC ((1) << 8)
	#define PAGESIZE_4SEC ((2) << 8)
	#define PAGESIZE_8SEC ((3) << 8)
	#define ECC_NAND_RD ((0) << 12)
	#define ECC_NAND_WR ((1) << 12)
	#define SPARE_PROTECTED ((1) << 16)
	#define SPARE_NOT_PROTECTED ((0) << 16)
	#define AUTO_ENABLE ((1) << 20)
#define SMC_PMECCSAREA	0x74
#define SMC_PMECCSADDR	0x78
#define SMC_PMECCEADDR	0x7C
// 0x80 Reserved
#define SMC_PMECCTRL	0x84
	#define ECC_RESET (1<<0)
	#define ECC_DATA (1<<1)
	#define ECC_USER (1<<2)
	#define ECC_ENABLE (1<<4)
	#define ECC_DISABLE (1<<5)
#define SMC_PMECCSR		0x88
	#define ECC_BUSY (1)
#define SMC_PMECCIER	0x8C
#define SMC_PMECCIDR	0x90
#define SMC_PMECCIMR	0x94
#define SMC_PMECCISR	0x98
// reserved 0x9C-0xAC
// Redundnacy registers: SEC = 0-?, N=0-10
#define SMC_PMECC_RED(SEC,N) (0x0B0 +((SEC)*0x40)+((N)*4))
// Redundnacy registers: SEC = 0-?, N=0-11
#define SMC_PMECC_REM(SEC,N)	(0x2B0 +((SEC)*0x40)+((N)*4))
// reserved 0x4a0-0x4fc
#define SMC_ELCFG	0x500
#define SMC_ELPRIM	0x504
#define SMC_ELEN	0x508
#define SMC_ELDIS	0x50C
#define SMC_ELSR	0x510
#define SMC_ELIER	0x514
#define SMC_ELIDR	0x518
#define SMC_ELIMR	0x51C
#define SMC_ELISR	0x520
// reserved 0x524 - 0x52c
// SIGMA registers 0-24
#define SMC_SIGMA(N) (0x528 +(N)*4)
// Error location 0-24
#define SMC_ERROR(N) (0x58C +(N)*4)
// reserved 0x5ec - 0x5fc
// Setup Chip Select = 0-3
#define SMC_SETUP(CS)	(0x600 + ((CS)*0x14))
#define SMC_PULSE(CS)	(0x604 + ((CS)*0x14))
#define SMC_CYCLE(CS)	(0x608 + ((CS)*0x14))
#define SMC_TIMINGS(CS)	(0x60C + ((CS)*0x14))
#define SMC_MODE(CS)	(0x610 + ((CS)*0x14))
#define SMC_OCMS		0x6A0
#define SMC_KEY1		0x6A4
#define SMC_KEY2		0x6A8
// reserved 0x6ac - 0x6e0
#define SMC_WPCR		0x6E4
	#define SMC_UNLOCK 0x534D4300
	#define SMC_LOCK 0x534D4301

#define SMC_WPSR		0x6E8

/* For creating NAND Command parameters */
#define NAND_CMD1(n) (((n)&0xff)<<2)
#define NAND_CMD2(n) (((n)&0xff)<<10)
#define NAND_VCMD2 (1<<18)
#define NAND_ACYCLE(n) (((n)&0x7)<<19)
#define NAND_CSID(n) (((n)&0x7)<<22)
#define NAND_DATAEN (1<<25)
#define NAND_NO_DATA (0<<25)
#define NAND_NFCRD (0<<26)
#define NAND_NFCWR (1<<26)
#define NAND_NFC_BUSY (1<<27)

/*
 * CAN Controller register offsets
 */
#define CAN_MR 0x00
	#define CAN_MR_CANEN (1<<0)
	#define CAN_MR_LPB (1<<1)
	#define CAN_MR_ABM (1<<2)
	#define CAN_MR_OVL (1<<3)
	#define CAN_MR_TEOF (1<<4)
	#define CAN_MR_TTM (1<<5)
	#define CAN_MR_TIMFRZ (1<<6)
	#define CAN_MR_DRPT (1<<7)
#define CAN_IER 0x04
#define CAN_IDR 0x08
#define CAN_IMR 0x0C
	#define CAN_IR_ERRA (1<<16)
	#define CAN_IR_WARN (1<<17)
	#define CAN_IR_ERRP (1<<18)
	#define CAN_IR_BOFF (1<<19)
	#define CAN_IR_SLEEP (1<<20)
	#define CAN_IR_WAKEUP (1<<21)
	#define CAN_IR_TOVF (1<<22)
	#define CAN_IR_TSTP (1<<23)
	#define CAN_IR_CERR (1<<24)
	#define CAN_IR_SERR (1<<25)
	#define CAN_IR_AERR (1<<26)
	#define CAN_IR_FERR (1<<27)
	#define CAN_IR_BERR (1<<28)
	#define CAN_ERROR_INTERRUPTS (CAN_IR_CERR | CAN_IR_SERR | CAN_IR_BERR | CAN_IR_FERR | CAN_IR_AERR)
	#define CAN_INFO_INTERRUPTS (CAN_IR_ERRA | CAN_IR_WARN | CAN_IR_ERRP | CAN_IR_BOFF)
#define CAN_SR 0x10
	#define CAN_SR_ERRA (1<<16)
	#define CAN_SR_WARN (1<<17)
	#define CAN_SR_ERRP (1<<18)
	#define CAN_SR_BOFF (1<<19)
	#define CAN_SR_SLEEP (1<<20)
	#define CAN_SR_WAKEUP (1<<21)
	#define CAN_SR_TOVF (1<<22)
	#define CAN_SR_TSTP (1<<23)
	#define CAN_SR_CERR (1<<24)
	#define CAN_SR_SERR (1<<25)
	#define CAN_SR_AERR (1<<26)
	#define CAN_SR_FERR (1<<27)
	#define CAN_SR_BERR (1<<28)
	#define CAN_SR_RBSY (1<<29)
	#define CAN_SR_TBSY (1<<30)
	#define CAN_SR_OVLSY (1<<31)
	#define CAN_BUSY (CAN_SR_OVLSY | CAN_SR_TBSY | CAN_SR_RBSY)
#define CAN_BR 0x14
#define CAN_TIM 0x18
#define CAN_TIMESTP 0x1C
#define CAN_ECR 0x20
#define CAN_TCR 0x24
#define CAN_ACR 0x28
#define CAN_WPMR 0xE4
	#define CAN_DISABLE_WP 0x43414E00
	#define CAN_ENSABLE_WP 0x43414E01
#define CAN_WPSR 0xE8
/* Message mode registers */
#define CAN_MMR(x) (0x200 + ((x)*0x20))
	#define CAN_DIS 0
	#define CAN_RX	1
	#define CAN_RX_OVR	2
	#define CAN_TX	3
	#define CAN_CONS	4
	#define CAN_PROD	5
	#define CAN_MB_TYPE_SHIFT 24
	#define CAN_MB_TYPE_MASK 0x7
	#define CAN_MOT(x) (((x) & CAN_MB_TYPE_MASK) << CAN_MB_TYPE_SHIFT)
	#define CAN_PRIOR(x) (((x) & 0xf)<<16)

/* Message Acceptance Mask registers */
#define CAN_MAM(x) (0x204 + ((x)*0x20))
	#define CAN_ID_A (0<<29)
	#define CAN_ID_B (1<<29)
	#define CAN_MASK_A (0x3ff << 18)
	#define CAN_MASK_B (0x1fffffff)
/* Message ID registers */
#define CAN_MID(x) (0x208 + ((x)*0x20))
	#define CAN_A_ID(x) ((x)<<18)
	#define CAN_B_ID(x) (x)
/* Message Family ID registers */
#define CAN_MFID(x) (0x20C + ((x)*0x20))
/* Message Status register */
#define CAN_MSR(x) (0x210 + ((x)*0x20))
	#define CAN_MSR_MRDY (1<<23)
/* Message Data Low register */
#define CAN_MDL(x) (0x214 + ((x)*0x20))
/* Message Data High register */
#define CAN_MDH(x) (0x218 + ((x)*0x20))
/* Message Control register */
#define CAN_MCR(x) (0x21C + ((x)*0x20))
	#define CAN_MCR_MDLC(x) (((x)&0xf)<<16)
	#define CAN_MCR_MRTR (1<<20)
	#define CAN_MCR_MACR (1<<22)
	#define CAN_MCR_MTCR (1<<23)


#endif /* #ifndef _ATSAMA5D3X_H__ */




#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/hardware/startup/lib/public/arm/atsama5d3x.h $ $Rev: 798438 $")
#endif
