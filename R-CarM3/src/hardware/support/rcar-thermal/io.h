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


#ifndef _IO_H_INCLUDED
#define _IO_H_INCLUDED

#include <sys/resmgr.h>
#include <sys/iomsg.h>
#include <sys/iofunc.h>
#include <arm/renesas_reg.h>
#include <arm/r-car-m3.h>

#define RESMGR_NAME         "thermal"
#define DEV_NAME            "/dev/" RESMGR_NAME     // do not modify

#define VERBOSE_QUIET       0
#define VERBOSE_LEVEL1      1
#define VERBOSE_LEVEL2      2
#define VERBOSE_LEVEL4      4
#define VERBOSE_LEVEL8      8

#define MAX_MSG_SIZE        IOBUF_SIZE              // max size of a message, should be max(IOBUF, devctl_t)
#define IOBUF_SIZE          1024                    // size of io_buffer used to for msg read/write with client
#define ENG                 512                     // Error Code

/* Structure for all THS/CIVM register */
typedef struct
{
    volatile uint32_t THSIRQSTR;    // 0x00 THS Interrupt Status Register
    volatile uint32_t IRQSTR;       // 0x04 Interrupt Status Register
    volatile uint32_t IRQMSK;       // 0x08 Interrupt Mask Register
    volatile uint32_t IRQCTL;       // 0x0C Threshold Edge/Level Register
    volatile uint32_t IRQEN;        // 0x10 Interrupt Enable Register
    volatile uint32_t IRQTEMP1;     // 0x14 Interrupt Temperature 1 Register
    volatile uint32_t IRQTEMP2;     // 0x18 Interrupt Temperature 2 Register
    volatile uint32_t IRQTEMP3;     // 0x1C Interrupt Temperature 3 Register
    volatile uint32_t CTSR;         // 0x20 Control Status Register
    volatile uint32_t TEMP;         // 0x28 Temperature Register
    volatile uint32_t VOLT;         // 0x30 Voltage Register
    volatile uint32_t THCODE_INT1;  // 0x38 Power ON Initial 1 Temperature Register
    volatile uint32_t THCODE_INT2;  // 0x3C Power ON Initial 2 Temperature Register
    volatile uint32_t THCODE_INT3;  // 0x40 Power ON Initial 3 Temperature Register
    volatile uint32_t FTHCODEH;     // 0x48 Fuse THCODE High Temperature Register
    volatile uint32_t FTHCODET;     // 0x4C Fuse THCODE Room Temperature Register
    volatile uint32_t FTHCODEL;     // 0x50 Fuse THCODE Low Temperature Register
    volatile uint32_t FPTATH;       // 0x54 Fuse PTAT High Temperature Register
    volatile uint32_t FPTATT;       // 0x58 Fuse PTAT Room Temperature Register
    volatile uint32_t FPTATL;       // 0x5C Fuse PTAT Low Temperature Register
}ths_civm_t;

/* IRQSTR bit definition */
#define THSCIVM_IRQSTR_TEMPD3_STR   BIT5    // Masking of TEMPD3 interrupt requests
#define THSCIVM_IRQSTR_TEMPD2_STR   BIT4    // Masking of TEMPD2 interrupt requests
#define THSCIVM_IRQSTR_TEMPD1_STR   BIT3    // Masking of TEMPD1 interrupt requests
#define THSCIVM_IRQSTR_TEMP3_STR    BIT2    // Masking of TEMP3 interrupt requests
#define THSCIVM_IRQSTR_TEMP2_STR    BIT1    // Masking of TEMP2 interrupt requests
#define THSCIVM_IRQSTR_TEMP1_STR    BIT0    // Masking of TEMP1 interrupt requests

/* IRQMSK bit definition */
#define THSCIVM_IRQMSK_TEMPD3_MSK   BIT5    // Masking of TEMPD3 interrupt requests
#define THSCIVM_IRQMSK_TEMPD2_MSK   BIT4    // Masking of TEMPD2 interrupt requests
#define THSCIVM_IRQMSK_TEMPD1_MSK   BIT3    // Masking of TEMPD1 interrupt requests
#define THSCIVM_IRQMSK_TEMP3_MSK    BIT2    // Masking of TEMP3 interrupt requests
#define THSCIVM_IRQMSK_TEMP2_MSK    BIT1    // Masking of TEMP2 interrupt requests
#define THSCIVM_IRQMSK_TEMP1_MSK    BIT0    // Masking of TEMP1 interrupt requests

/*CTSR bit definition*/
#define THSCIVM_CTSR_IRQ3           BIT31   // interrupt signal that is detected by IRQTEMP3
#define THSCIVM_CTSR_IRQ2           BIT30   // interrupt signal that is detected by IRQTEMP2
#define THSCIVM_CTSR_IRQ1           BIT29   // interrupt signal that is detected by IRQTEMP1
#define THSCIVM_CTSR_THCHOPOV       BIT28   // overflow of TSC counter
#define THSCIVM_CTSR_PONSEQSTOP     BIT27   // end flag of power-on mode
#define THSCIVM_CTSR_THTMP3         BIT26   // detection of capturing the third temperature
#define THSCIVM_CTSR_THTMP2         BIT25   // detection of capturing the second temperature
#define THSCIVM_CTSR_THTMP1         BIT24   // detection of capturing the first temperature
#define THSCIVM_CTSR_VMCNTOV_MASK   (3<<22) // detection of an overflow of the counters for chip internal voltage monitor
#define THSCIVM_CTSR_VMCNTOV_0      (0<<22) // Normal operation
#define THSCIVM_CTSR_VMCNTOV_1      (1<<22) // Counter 0 has overflowed
#define THSCIVM_CTSR_VMCNTOV_2      (2<<22) // Counter 1 has overflowed
#define THSCIVM_CTSR_VMCNTOV_3      (3<<22) // Both counters 0 and 1 have overflowed
#define THSCIVM_CTSR_THCNTOV_MASK   (3<<20) // Detection of an overflow of the counters for the thermal sensor
#define THSCIVM_CTSR_THCNTOV_0      (0<<20) // Normal operation
#define THSCIVM_CTSR_THCNTOV_1      (1<<20) // Counter 0 has overflowed
#define THSCIVM_CTSR_THCNTOV_2      (2<<20) // Counter 1 has overflowed
#define THSCIVM_CTSR_THCNTOV_3      (3<<20) // Both counters 0 and 1 have overflowed
#define THSCIVM_CTSR_THFAIL1        BIT17   // all TEMP_CODE [11:0] bits are 1
#define THSCIVM_CTSR_THFAIL0        BIT16   // all TEMP_CODE [11:0] bits are 0
#define THSCIVM_CTSR_PONM           BIT8    // Mode switching signal
#define THSCIVM_CTSR_AOUT           BIT7    // Enabling/disabling external output of THS
#define THSCIVM_CTSR_THBGR          BIT5    // Enabling/disabling of THS
#define THSCIVM_CTSR_VMEN           BIT4    // Enabling/disabling of the CIVM
#define THSCIVM_CTSR_VMST           BIT1    // Enabling/disabling of the A/D converter for the CIVM
#define THSCIVM_CTSR_THSST          BIT0    // Enabling/disabling of the A/D converter for the THS


#define CTEMP_MIN_TEMP              -40
#define CTEMP_MAX_TEMP              125


int io_devctl(resmgr_context_t *ctp, io_devctl_t *msg, RESMGR_OCB_T *ocb);
int thermal_slogf(const int Verbose, const char *fmt, ...);


int ths_civm_get_temperature(uint16_t *temperature);
int ths_civm_get_voltage(uint16_t *voltage);

#endif /* _IO_H_INCLUDED */

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/hardware/support/rcar-thermal/io.h $ $Rev: 811478 $")
#endif
