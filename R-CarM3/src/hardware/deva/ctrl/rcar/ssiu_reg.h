/*
 * $QNXLicenseC:
 * Copyright 2014, 2016 QNX Software Systems.
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

#ifndef _R_Car_SSIU_REG_H
#define _R_Car_SSIU_REG_H

#include <errno.h>
#include <stdint.h>

/*******/
/* SSI */
/*******/

/*
 * Bit field mappings for SSIx-y_BUSIF_MODE/SSIx_BUSIF_MODE register
 *
 * 21-31:        Not used:  set to 0
 * 20:           SFT_DIR:   bit-shift direction for valid bit position adjustment: default=0 (shift to left)
 * 16,17,18,19:  SFT_NUM:   bit-shift count for valid bit position adjustment: default=0 (no shift)
 * 9-15:         Not used:  set to 0
 * 8:            WORD_SWAP: enable word swap for 16 bit data: default=0 (no word swap)
 * 1-7:          Not used:  set to 0
 * 0:            DMA        only allowed value is 1: DMA access
 */
#define SSI_BUSIF_MODE_SFTDIR_MASK (0x1<<20)
#define SSI_BUSIF_MODE_SFTDIR_LEFT 0
#define SSI_BUSIF_MODE_SFTDIR_RIGHT (0x1<<20)
#define SSI_BUSIF_MODE_SFTNUM_MASK (0xF<<16)
#define SSI_BUSIF_MODE_WORDSWAP_MASK (0x1<<8)
#define SSI_BUSIF_MODE_WORDSWAP_ENABLE (0x1<<8)
#define SSI_BUSIF_MODE_WORDSWAP_DISABLE 0
#define SSI_BUSIF_MODE_DMA_MASK 0x1
#define SSI_BUSIF_MODE_DMA_ENABLE 0x1

/*
 * Bit field mappings for SSIx-y_BUSIF_ADINR/SSIx_BUSIF_ADINR register
 *
 * 21-31:          Not used:  set to 0
 * 16,17,18,19,20: OTBL:  output data bit length: default=0 (24 bits)
 * 4-15:           Not used:  set to 0
 * 0-3:            CHNUM: channel number: default=0 (none)
 */
#define SSI_BUSIF_ADINR_OTBL_MASK (0x1F<<16)
#define SSI_BUSIF_ADINR_OTBL_24BITS 0
#define SSI_BUSIF_ADINR_OTBL_22BITS (0x2<<16)
#define SSI_BUSIF_ADINR_OTBL_20BITS (0x4<<16)
#define SSI_BUSIF_ADINR_OTBL_18BITS (0x6<<16)
#define SSI_BUSIF_ADINR_OTBL_16BITS (0x8<<16)
#define SSI_BUSIF_ADINR_OTBL_8BITS (0x10<<16)
#define SSI_BUSIF_ADINR_CHNUM_MASK 0xF
#define SSI_BUSIF_ADINR_CHNUM_NONE  0
#define SSI_BUSIF_ADINR_CHNUM_ONE   1
#define SSI_BUSIF_ADINR_CHNUM_TWO   2
#define SSI_BUSIF_ADINR_CHNUM_FOUR  4
#define SSI_BUSIF_ADINR_CHNUM_SIX   6
#define SSI_BUSIF_ADINR_CHNUM_EIGHT 8

/*
 * Bit field mappings for SSIx-y_BUSIF_DALIGN/SSIx_BUSIF_DALIGN register
 *
 * 31:      Not used: set to 0
 * 28-30:   PLACE7:   selects input channel to be output as channel 7: default 7
 * 27:      Not used: set to 0
 * 24-26:   PLACE6:   selects input channel to be output as channel 6: default 6
 * 23:      Not used: set to 0
 * 20-22:   PLACE5:   selects input channel to be output as channel 5: default 5
 * 19:      Not used: set to 0
 * 16-18:   PLACE4:   selects input channel to be output as channel 4: default 4
 * 15:      Not used: set to 0
 * 12-14:   PLACE3:   selects input channel to be output as channel 3: default 3
 * 11:      Not used: set to 0
 * 8-10:    PLACE2:   selects input channel to be output as channel 2: default 2
 * 7:       Not used: set to 0
 * 4-6:     PLACE1:   selects input channel to be output as channel 1: default 1
 * 3:       Not used: set to 0
 * 0-2 :    PLACE0:   selects input channel to be output as channel 0: default 0
 * Note:    PLACE2 to PLACE7 do not apply to channels 0-1 to 0-3, 1-1 to 1-3,
 *          2-1 to 2-3, and 9-1 to 9-3, or to non-multi-channel config or to
 *          TDM split mode; defaulted to 0 in these cases, and valid values
 *          for PLACE0 and PLACE1 are 0 and 1 in these cases
 */
#define SSI_BUSIF_DALIGN_PLACE_MASK(i)  (0x7 << (i<<2))

/*
 * Bit field mappings for SSIx-0_MODE/SSIx-MODE register
 *
 * 14-31:   Not used:  set to 0
 * 13:      FS_MODE:   select fs for TDM mode: 0=256fs (stereox4), 1=128fs (monauralx4), default=0
 * 9-12:    Not used:  set to 0
 * 8:       TDM_SPLIT: enables TDM split mode when set to 1, default=0
 * 1-7:     Not used:  set to 0
 * 0:       TDM_EXT:   enables TDM extended mode when set to 1, default=0
 * Note:    For SSI 3,4 FS_MODE and TDM_SPLIT do not apply, these bit fields can not be set to a value
 *          different from default value of 0 for SSI 3,4
 */
#define SSI_MODE_FSMODE_MASK (0x1<<13)
#define SSI_MODE_FSMODE_DISABLE 0
#define SSI_MODE_FSMODE_ENABLE (0x1<<13)
#define SSI_MODE_TDMSPLIT_MASK (0x1<<8)
#define SSI_MODE_TDMSPLIT_DISABLE 0
#define SSI_MODE_TDMSPLIT_ENABLE (0x1<<8)
#define SSI_MODE_TDMEXT_MASK 0x1
#define SSI_MODE_TDMEXT_DISABLE 0
#define SSI_MODE_TDMEXT_ENABLE 0x1

/*
 * Bit field mappings for SSIx-0_CONTROL/SSIx-CONTROL register
 *
 * 13-31:   Not used:  set to 0
 * 12:      START_3:   start/stop subchannel 3: stop=0, start=1, default=0
 * 9-11:    Not used:  set to 0
 * 8:       START_2:   start/stop subchannel 2: stop=0, start=1, default=0
 * 5-7:     Not used:  set to 0
 * 4:       START_1:   start/stop subchannel 1: stop=0, start=1, default=0
 * 1-3:     Not used:  set to 0
 * 0:       START_0:   start/stop subchannel 0, or the actual channel: stop=0, start=1, default=0
 * Note:    For SSI 3,4,5,6,7,8 START_1, START_2, START_3 do not apply, these bit fields can not
 *          be set to a value different from default value of 0 for SSI 3,4,5,6,7,8
 */
#define SSI_CONTROL_START3_MASK (0x1<<12)
#define SSI_CONTROL_START3_STOP 0
#define SSI_CONTROL_START3_START (0x1<<12)
#define SSI_CONTROL_START2_MASK (0x1<<8)
#define SSI_CONTROL_START2_STOP 0
#define SSI_CONTROL_START2_START (0x1<<8)
#define SSI_CONTROL_START1_MASK (0x1<<4)
#define SSI_CONTROL_START1_STOP 0
#define SSI_CONTROL_START1_START (0x1<<4)
#define SSI_CONTROL_START0_MASK 0x1
#define SSI_CONTROL_START0_STOP 0
#define SSI_CONTROL_START0_START 0x1

/*
 * Bit field mappings for SSIx-0_STATUS/SSIx-STATUS register
 *
 * 30-31:   Not used:  read as 0
 * 29:      FCST:      state of WS signal: WS signal stopped=1, default=0
 * 28:      DTST:      state of frequency switching: frequency switching detected=1, default=0
 * 27:      UIRQ:      underflow indication: underflow occured=1, default=0
 * 26:      OIRQ:      overflow indication: overflow occured=1, default=0
 * 25:      IIRQ:      interrupt indication: idle state(?)=1, default=0
 * 24:      DIRQ:      unread data indication: unread data available=1, default=0
 * 16-23:   Not used:  read as 0
 * 15:      UF_3:      buffer underflow in subchannel 3 (applicable to SSI 0,1,2,9)
 * 14:      UF_2:      buffer underflow in subchannel 2 (applicable to SSI 0,1,2,9)
 * 13:      UF_1:      buffer underflow in subchannel 1 (applicable to SSI 0,1,2,9)
 * 12:      UF_0:      buffer underflow in subchannel 0 (applicable to SSI 0,1,2,9)
 * 11:      OF_3:      buffer overflow in subchannel 3 (applicable to SSI 0,1,2,9)
 * 10:      OF_2:      buffer overflow in subchannel 2 (applicable to SSI 0,1,2,9)
 * 9:       OF_1:      buffer overflow in subchannel 1 (applicable to SSI 0,1,2,9)
 * 8:       OF_0:      buffer overflow in subchannel 0 (applicable to SSI 0,1,2,9)
 * 0-7:     Not used:  read as 0
 */
#define SSI_STATUS_FCST_MASK (0x1<<29)
#define SSI_STATUS_DTST_MASK (0x1<<28)
#define SSI_STATUS_UIRQ_MASK (0x1<<27)
#define SSI_STATUS_OIRQ_MASK (0x1<<26)
#define SSI_STATUS_IIRQ_MASK (0x1<<25)
#define SSI_STATUS_DIRQ_MASK (0x1<<24)
#define SSI_STATUS_UF3_MASK (0x1<<15)
#define SSI_STATUS_UF2_MASK (0x1<<14)
#define SSI_STATUS_UF1_MASK (0x1<<13)
#define SSI_STATUS_UF0_MASK (0x1<<12)
#define SSI_STATUS_OF3_MASK (0x1<<11)
#define SSI_STATUS_OF2_MASK (0x1<<10)
#define SSI_STATUS_OF1_MASK (0x1<<9)
#define SSI_STATUS_OF0_MASK (0x1<<8)

/*
 * Bit field mappings for SSIx-0_INT_ENABLE_MAIN/SSIx_INT_ENABLE_MAIN
 * 30-31:   Not used:  set as 0
 * 29:      FCST_IE:   FCST interrupt enable: enable=1, disable=0, default=0
 * 28:      DTST_IE:   DTST interrupt enable: enable=1, disable=0, default=0
 * 27:      UIRQ_IE:   UIRQ interrupt enable: enable=1, disable=0, default=0
 * 26:      OIRQ_IE:   OIRQ interrupt enable: enable=1, disable=0, default=0
 * 25:      IIRQ_IE:   IIRQ interrupt enable: enable=1, disable=0, default=0
 * 24:      DIRQ_IE:   DIRQ interrupt enable: enable=1, disable=0, default=0
 * 16-23:   Not used:  set as 0
 * 15:      UF_3_IE:   UF_3 interrupt enable: enable=1, disable=0, default=0 (applicable to SSI 0,1,2,9)
 * 14:      UF_2_IE:   UF_2 interrupt enable: enable=1, disable=0, default=0 (applicable to SSI 0,1,2,9)
 * 13:      UF_1_IE:   UF_1 interrupt enable: enable=1, disable=0, default=0 (applicable to SSI 0,1,2,9)
 * 12:      UF_0_IE:   UF_0 interrupt enable: enable=1, disable=0, default=0 (applicable to SSI 0,1,2,9)
 * 11:      OF_3_IE:   OF_3 interrupt enable: enable=1, disable=0, default=0 (applicable to SSI 0,1,2,9)
 * 10:      OF_2_IE:   OF_2 interrupt enable: enable=1, disable=0, default=0 (applicable to SSI 0,1,2,9)
 * 9:       OF_1_IE:   OF_1 interrupt enable: enable=1, disable=0, default=0 (applicable to SSI 0,1,2,9)
 * 8:       OF_0_IE:   OF_0 interrupt enable: enable=1, disable=0, default=0 (applicable to SSI 0,1,2,9)
 * 0-7:     Not used:  read as 0
 */
#define SSI_INT_ENABLE_MAIN_FCST_MASK (0x1<<29)
#define SSI_INT_ENABLE_MAIN_FCST_ENABLE (0x1<<29)
#define SSI_INT_ENABLE_MAIN_FCST_DISABLE 0
#define SSI_INT_ENABLE_MAIN_DTST_MASK (0x1<<28)
#define SSI_INT_ENABLE_MAIN_DTST_ENABLE (0x1<<28)
#define SSI_INT_ENABLE_MAIN_DTST_DISABLE 0
#define SSI_INT_ENABLE_MAIN_UIRQ_MASK (0x1<<27)
#define SSI_INT_ENABLE_MAIN_UIRQ_ENABLE (0x1<<27)
#define SSI_INT_ENABLE_MAIN_UIRQ_DISABLE 0
#define SSI_INT_ENABLE_MAIN_OIRQ_MASK (0x1<<26)
#define SSI_INT_ENABLE_MAIN_OIRQ_ENABLE (0x1<<26)
#define SSI_INT_ENABLE_MAIN_OIRQ_DISABLE 0
#define SSI_INT_ENABLE_MAIN_IIRQ_MASK (0x1<<25)
#define SSI_INT_ENABLE_MAIN_IIRQ_ENABLE (0x1<<25)
#define SSI_INT_ENABLE_MAIN_IIRQ_DISABLE 0
#define SSI_INT_ENABLE_MAIN_DIRQ_MASK (0x1<<24)
#define SSI_INT_ENABLE_MAIN_DIRQ_ENABLE (0x1<<24)
#define SSI_INT_ENABLE_MAIN_DIRQ_DISABLE 0
#define SSI_INT_ENABLE_MAIN_UF3_MASK (0x1<<15)
#define SSI_INT_ENABLE_MAIN_UF3_ENABLE (0x1<<15)
#define SSI_INT_ENABLE_MAIN_UF3_DISABLE 0
#define SSI_INT_ENABLE_MAIN_UF2_MASK (0x1<<14)
#define SSI_INT_ENABLE_MAIN_UF2_ENABLE (0x1<<14)
#define SSI_INT_ENABLE_MAIN_UF2_DISABLE 0
#define SSI_INT_ENABLE_MAIN_UF1_MASK (0x1<<13)
#define SSI_INT_ENABLE_MAIN_UF1_ENABLE (0x1<<13)
#define SSI_INT_ENABLE_MAIN_UF1_DISABLE 0
#define SSI_INT_ENABLE_MAIN_UF0_MASK (0x1<<12)
#define SSI_INT_ENABLE_MAIN_UF0_ENABLE (0x1<<12)
#define SSI_INT_ENABLE_MAIN_UF0_DISABLE 0
#define SSI_INT_ENABLE_MAIN_OF3_MASK (0x1<<11)
#define SSI_INT_ENABLE_MAIN_OF3_ENABLE (0x1<<11)
#define SSI_INT_ENABLE_MAIN_OF3_DISABLE 0
#define SSI_INT_ENABLE_MAIN_OF2_MASK (0x1<<10)
#define SSI_INT_ENABLE_MAIN_OF2_ENABLE (0x1<<10)
#define SSI_INT_ENABLE_MAIN_OF2_DISABLE 0
#define SSI_INT_ENABLE_MAIN_OF1_MASK (0x1<<9)
#define SSI_INT_ENABLE_MAIN_OF1_ENABLE (0x1<<9)
#define SSI_INT_ENABLE_MAIN_OF1_DISABLE 0
#define SSI_INT_ENABLE_MAIN_OF0_MASK (0x1<<8)
#define SSI_INT_ENABLE_MAIN_OF0_ENABLE (0x1<<8)
#define SSI_INT_ENABLE_MAIN_OF0_DISABLE 0

/*
 * Bit field mappings for SSI_MODE0 register
 *
 * 26-31: Not used:      set to 0
 * 25:    IND_WORD_SWAP9 Word order swap for SSI9 - word order not swapped=0, word order swapped=1, default=0
 * 24:    IND_WORD_SWAP8 Word order swap for SSI8 - word order not swapped=0, word order swapped=1, default=0
 * 23:    IND_WORD_SWAP7 Word order swap for SSI7 - word order not swapped=0, word order swapped=1, default=0
 * 22:    IND_WORD_SWAP6 Word order swap for SSI6 - word order not swapped=0, word order swapped=1, default=0
 * 21:    IND_WORD_SWAP5 Word order swap for SSI5 - word order not swapped=0, word order swapped=1, default=0
 * 20:    IND_WORD_SWAP4 Word order swap for SSI4 - word order not swapped=0, word order swapped=1, default=0
 * 19:    IND_WORD_SWAP3 Word order swap for SSI3 - word order not swapped=0, word order swapped=1, default=0
 * 18:    IND_WORD_SWAP2 Word order swap for SSI2 - word order not swapped=0, word order swapped=1, default=0
 * 17:    IND_WORD_SWAP3 Word order swap for SSI1 - word order not swapped=0, word order swapped=1, default=0
 * 16:    IND_WORD_SWAP2 Word order swap for SSI0 - word order not swapped=0, word order swapped=1, default=0
 * 10-15: Not used:      set to 0
 * 9:     IND9           Independent SSI9 transfer - indep SSI9 transfer=1, no indep SSI9 transfer=0, default=0
 * 8:     IND8           Independent SSI8 transfer - indep SSI8 transfer=1, no indep SSI8 transfer=0, default=0
 * 7:     IND7           Independent SSI8 transfer - indep SSI7 transfer=1, no indep SSI7 transfer=0, default=0
 * 6:     IND6           Independent SSI6 transfer - indep SSI6 transfer=1, no indep SSI6 transfer=0, default=0
 * 5:     IND5           Independent SSI5 transfer - indep SSI5 transfer=1, no indep SSI5 transfer=0, default=0
 * 4:     IND4           Independent SSI4 transfer - indep SSI4 transfer=1, no indep SSI4 transfer=0, default=0
 * 3:     IND3           Independent SSI3 transfer - indep SSI3 transfer=1, no indep SSI3 transfer=0, default=0
 * 2:     IND2           Independent SSI2 transfer - indep SSI2 transfer=1, no indep SSI2 transfer=0, default=0
 * 1:     IND1           Independent SSI1 transfer - indep SSI1 transfer=1, no indep SSI1 transfer=0, default=0
 * 0:     IND0           Independent SSI0 transfer - indep SSI0 transfer=1, no indep SSI0 transfer=0, default=0
 */
#define SSI_MODE0_INDWORDSWAP_MASK(s)     (0x1<<(25+s))
#define SSI_MODE0_INDWORDSWAP_DISABLE(s)  0
#define SSI_MODE0_INDWORDSWAP_ENABLE(s)   (0x1<<(25+s))
#define SSI_MODE0_IND_MASK(s)             (0x1<<s)
#define SSI_MODE0_IND_DISABLE(s)          0
#define SSI_MODE0_IND_ENABLE(s)           (0x1<<s)

/*
 * Bit field mappings for SSI_MODE1 register
 *
 * 21-31: Not used:      set to 0
 * 20:    SSI34_SYNC:    SSI 3,4 sync mode: synchronized=1, not synchronized=0, default=0
 * 18-19: Not used:      set to 0
 * 16-17: SSI4_PIN:      Selects the connections of SSI_SCK4 and SSI_WS4 pins:
 *                       SSI 3,4 use their pins independently: 0 (default)
 *                       SSI 3,4 use the SSI 3 pins, both modules are slaves: 1
 *                       SSI 3,4 use the SSI 3 pins, SSI 3 is master, SSI 4 is slave: 2
 * 5-15:  Not used:      set to 0
 * 4:     SSI012_3MOD:   SSI 0,1,2 3-module mode: SSI 0,1,2 use in 3-module (6-channel) mode=1, default=0
 * 2,3:   SSI2_PIN:      Selects the connections of SSI_SCK2 and SSI_WS2 pins:
 *                       SSI 0,2 use their pins independently: 0 (default)
 *                       SSI 0,2 use the SSI 0 pins, both modules are slaves: 1
 *                       SSI 0,2 use the SSI 0 pins, SSI 0 is master, SSI 2 is slave: 2
 * 1,0:   SSI1_PIN:      Selects the connections of SSI_SCK1 and SSI_WS1 pins:
 *                       SSI 0,1 use their pins independently: 0 (default)
 *                       SSI 0,1 use the SSI 0 pins, both modules are slaves: 1
 *                       SSI 0,1 use the SSI 0 pins, SSI 0 is master, SSI 2 is slave: 2
 *
 */
#define SSI_MODE1_SSI34SYNC_MASK                (0x1<<20)
#define SSI_MODE1_SSI34SYNC_ENABLE              (0x1<<20)
#define SSI_MODE1_SSI34SYNC_DISABLE             0
#define SSI_MODE1_SSI4PIN_MASK                  (0x3<<16)
#define SSI_MODE1_SSI4PIN_SSI34_INDEPENDENT     0
#define SSI_MODE1_SSI4PIN_SSI34_BOTHSLAVE       (0x1<<16)
#define SSI_MODE1_SSI4PIN_SSI34_MASTERSLAVE     (0x2<<16)
#define SSI_MODE1_SSI012THREEMODULE_MASK        (0x1<<4)
#define SSI_MODE1_SSI012THREEMODULE_ENABLE      (0x1<<4)
#define SSI_MODE1_SSI012THREEMODULE_DISABLE     0
#define SSI_MODE1_SSI2PIN_MASK                  (0x3<<2)
#define SSI_MODE1_SSI2PIN_SSI02_INDEPENDENT     0
#define SSI_MODE1_SSI2PIN_SSI02_BOTHSLAVE       (0x1<<2)
#define SSI_MODE1_SSI2PIN_SSI02_MASTERSLAVE     (0x2<<2)
#define SSI_MODE1_SSI1PIN_MASK                  0x3
#define SSI_MODE1_SSI1PIN_SSI01_INDEPENDENT     0
#define SSI_MODE1_SSI1PIN_SSI01_BOTHSLAVE       0x1
#define SSI_MODE1_SSI1PIN_SSI01_MASTERSLAVE     0x2

/*
 * Bit field mappings for SSI_MODE2 register
 *
 * 5-31:  Not used:      set to 0
 * 4:     SSI0129_4MOD:  SSI 0,1,2 3-module mode: SSI 0,1,2,9 use in 4-module (8-channel) mode=1, default=0
 * 3:     Not used:      set to 0
 * 0,1,2: SSI9_PIN:      Selects the connections of SSI_SCK9 and SSI_WS9 pins:
 *                       SSI 0,9 and 3,9 use their pins independently: 0 (default)
 *                       SSI 0,9 use the SSI 0 pins, both modules are slaves: 1
 *                       SSI 0,9 use the SSI 0 pins, SSI 0 is master, SSI 9 is slave: 2
 *                       SSI 3,9 use the SSI 3 pins, both modules are slaves: 5
 *                       SSI 3,9 use the SSI 3 pins, SSI 3 is master, SSI 9 is slave: 6
 */
#define SSI_MODE2_SSI0129FOURMODULE_MASK        (0x1<<4)
#define SSI_MODE2_SSI0129FOURMODULE_ENABLE      (0x1<<4)
#define SSI_MODE2_SSI0129FOURMODULE_DISABLE     0
#define SSI_MODE2_SSI9PIN_MASK                  0x7
#define SSI_MODE2_SSI9PIN_SSI09_39_INDEPENDENT  0
#define SSI_MODE2_SSI9PIN_SSI09_BOTHSLAVE       0x1
#define SSI_MODE2_SSI9PIN_SSI09_MASTERSLAVE     0x2
#define SSI_MODE2_SSI9PIN_SSI39_BOTHSLAVE       0x5
#define SSI_MODE2_SSI9PIN_SSI39_MASTERSLAVE     0x6

/*
 * Bit field mappings for SSI_MODE3 register
 *
 * 3-31:  Not used:      set to 0
 * 0,1:   SSI3_PIN:      Selects the connections of SSI_SCK3 and SSI_WS3 pins:
 *                       SSI 0,3 use their pins independently: 0 (default)
 *                       SSI 0,3 use the SSI 0 pins, both modules are slaves: 1
 *                       SSI 0,3 use the SSI 0 pins, SSI 0 is master, SSI 3 is slave: 2
 */
#define SSI_MODE3_SSI3PIN_MASK                  0x3
#define SSI_MODE3_SSI3PIN_SSI03_INDEPENDENT     0
#define SSI_MODE3_SSI3PIN_SSI03_BOTHSLAVE       0x1
#define SSI_MODE3_SSI3PIN_SSI03_MASTERSLAVE     0x2

/*
 * Bit field mappings for SSI_CONTROL register
 *
 * 5-31:  Not used:      set to 0
 * 4:     SSI34:         SSI 3,4 simultaneous transfer enable/disable: enable/start=1, disable/stop=0, default=0
 * 1-3:   Not used:      set to 0
 * 0:     SSI0129        SSI 0,1,2 or SSI 0,1,2,9 simultaneous transfer enable/disable: enable/start=1, disable/stop=0, default=0
 *
 */
#define SSI_CONTROL_SSI34_MASK       (0x1<<4)
#define SSI_CONTROL_SSI34_ENABLE     (0x1<<4)
#define SSI_CONTROL_SSI34_DISABLE    0
#define SSI_CONTROL_SSI0129_MASK     0x1
#define SSI_CONTROL_SSI0129_ENABLE   0x1
#define SSI_CONTROL_SSI0129_DISABLE  0

/*
 * Bit field mappings for SSI_SYSTEM_STATUS0 register
 *
 * 12-31:  Not used:     set to 0
 * 11:     OF2-3:        Buffer overflow state of the SSI2-3_BUSIF buffer: overflow occured=1, normal=0
 * 10:     OF2-2:        Buffer overflow state of the SSI2-2_BUSIF buffer: overflow occured=1, normal=0
 * 9:      OF2-1:        Buffer overflow state of the SSI2-1_BUSIF buffer: overflow occured=1, normal=0
 * 8:      OF2-0:        Buffer overflow state of the SSI2-0_BUSIF buffer: overflow occured=1, normal=0
 * 7:      OF1-3:        Buffer overflow state of the SSI1-3_BUSIF buffer: overflow occured=1, normal=0
 * 6:      OF1-2:        Buffer overflow state of the SSI1-2_BUSIF buffer: overflow occured=1, normal=0
 * 5:      OF1-1:        Buffer overflow state of the SSI1-1_BUSIF buffer: overflow occured=1, normal=0
 * 4:      OF1-0:        Buffer overflow state of the SSI1-0_BUSIF buffer: overflow occured=1, normal=0
 * 3:      OF0-3:        Buffer overflow state of the SSI0-3_BUSIF buffer: overflow occured=1, normal=0
 * 2:      OF0-2:        Buffer overflow state of the SSI0-2_BUSIF buffer: overflow occured=1, normal=0
 * 1:      OF0-1:        Buffer overflow state of the SSI0-1_BUSIF buffer: overflow occured=1, normal=0
 * 0:      OF0-0:        Buffer overflow state of the SSI0-0_BUSIF buffer: overflow occured=1, normal=0
 *
 */
#define SSI_SYSTEM_STATUS0_OF23_MASK (0x1<<11)
#define SSI_SYSTEM_STATUS0_OF22_MASK (0x1<<10)
#define SSI_SYSTEM_STATUS0_OF21_MASK (0x1<<9)
#define SSI_SYSTEM_STATUS0_OF20_MASK (0x1<<8)
#define SSI_SYSTEM_STATUS0_OF13_MASK (0x1<<7)
#define SSI_SYSTEM_STATUS0_OF12_MASK (0x1<<6)
#define SSI_SYSTEM_STATUS0_OF11_MASK (0x1<<5)
#define SSI_SYSTEM_STATUS0_OF10_MASK (0x1<<4)
#define SSI_SYSTEM_STATUS0_OF03_MASK (0x1<<3)
#define SSI_SYSTEM_STATUS0_OF02_MASK (0x1<<2)
#define SSI_SYSTEM_STATUS0_OF01_MASK (0x1<<1)
#define SSI_SYSTEM_STATUS0_OF00_MASK 0x1

/*
 * Bit field mappings for SSI_SYSTEM_STATUS1 register
 *
 * 8-31:   Not used:     set to 0
 * 7:      OF9-3:        Buffer overflow state of the SSI1-3_BUSIF buffer: overflow occured=1, normal=0
 * 6:      OF9-2:        Buffer overflow state of the SSI1-2_BUSIF buffer: overflow occured=1, normal=0
 * 5:      OF9-1:        Buffer overflow state of the SSI1-1_BUSIF buffer: overflow occured=1, normal=0
 * 4:      OF9-0:        Buffer overflow state of the SSI1-0_BUSIF buffer: overflow occured=1, normal=0
 * 0-3:    Not used:     set to 0
 *
 */
#define SSI_SYSTEM_STATUS1_OF93_MASK (0x1<<7)
#define SSI_SYSTEM_STATUS1_OF92_MASK (0x1<<6)
#define SSI_SYSTEM_STATUS1_OF91_MASK (0x1<<5)
#define SSI_SYSTEM_STATUS1_OF90_MASK (0x1<<4)

/*
 * Bit field mappings for SSI_SYSTEM_STATUS2 register
 *
 * 12-31:  Not used:     set to 0
 * 11:     UF2-3:        Buffer underflow state of the SSI2-3_BUSIF buffer: underflow occured=1, normal=0
 * 10:     UF2-2:        Buffer underflow state of the SSI2-2_BUSIF buffer: underflow occured=1, normal=0
 * 9:      UF2-1:        Buffer underflow state of the SSI2-1_BUSIF buffer: underflow occured=1, normal=0
 * 8:      UF2-0:        Buffer underflow state of the SSI2-0_BUSIF buffer: underflow occured=1, normal=0
 * 7:      UF1-3:        Buffer underflow state of the SSI1-3_BUSIF buffer: underflow occured=1, normal=0
 * 6:      UF1-2:        Buffer underflow state of the SSI1-2_BUSIF buffer: underflow occured=1, normal=0
 * 5:      UF1-1:        Buffer underflow state of the SSI1-1_BUSIF buffer: underflow occured=1, normal=0
 * 4:      UF1-0:        Buffer underflow state of the SSI1-0_BUSIF buffer: underflow occured=1, normal=0
 * 3:      UF0-3:        Buffer underflow state of the SSI0-3_BUSIF buffer: underflow occured=1, normal=0
 * 2:      UF0-2:        Buffer underflow state of the SSI0-2_BUSIF buffer: underflow occured=1, normal=0
 * 1:      UF0-1:        Buffer underflow state of the SSI0-1_BUSIF buffer: underflow occured=1, normal=0
 * 0:      UF0-0:        Buffer underflow state of the SSI0-0_BUSIF buffer: underflow occured=1, normal=0
 *
 */
#define SSI_SYSTEM_STATUS2_UF23_MASK (0x1<<11)
#define SSI_SYSTEM_STATUS2_UF22_MASK (0x1<<10)
#define SSI_SYSTEM_STATUS2_UF21_MASK (0x1<<9)
#define SSI_SYSTEM_STATUS2_UF20_MASK (0x1<<8)
#define SSI_SYSTEM_STATUS2_UF13_MASK (0x1<<7)
#define SSI_SYSTEM_STATUS2_UF12_MASK (0x1<<6)
#define SSI_SYSTEM_STATUS2_UF11_MASK (0x1<<5)
#define SSI_SYSTEM_STATUS2_UF10_MASK (0x1<<4)
#define SSI_SYSTEM_STATUS2_UF03_MASK (0x1<<3)
#define SSI_SYSTEM_STATUS2_UF02_MASK (0x1<<2)
#define SSI_SYSTEM_STATUS2_UF01_MASK (0x1<<1)
#define SSI_SYSTEM_STATUS2_UF00_MASK 0x1

/*
 * Bit field mappings for SSI_SYSTEM_STATUS3 register
 *
 * 8-31:   Not used:     set to 0
 * 7:      OF9-3:        Buffer underflow state of the SSI1-3_BUSIF buffer: underflow occured=1, normal=0
 * 6:      OF9-2:        Buffer underflow state of the SSI1-2_BUSIF buffer: underflow occured=1, normal=0
 * 5:      OF9-1:        Buffer underflow state of the SSI1-1_BUSIF buffer: underflow occured=1, normal=0
 * 4:      OF9-0:        Buffer underflow state of the SSI1-0_BUSIF buffer: underflow occured=1, normal=0
 * 0-3:    Not used:     set to 0
 *
 */
#define SSI_SYSTEM_STATUS3_UF93_MASK (0x1<<7)
#define SSI_SYSTEM_STATUS3_UF92_MASK (0x1<<6)
#define SSI_SYSTEM_STATUS3_UF91_MASK (0x1<<5)
#define SSI_SYSTEM_STATUS3_UF90_MASK (0x1<<4)

/*
 * Bit field mappings for SSI_SYSTEM_INTENABLE0 register
 *
 * 12-31:  Not used:     set to 0
 * 11:     OF2-3_IE:     OF2-3 interrupt enable: enable=1, disable=0, default=0
 * 10:     OF2-2_IE:     OF2-2 interrupt enable: enable=1, disable=0, default=0
 * 9:      OF2-1_IE:     OF2-1 interrupt enable: enable=1, disable=0, default=0
 * 8:      OF2-0_IE:     OF2-0 interrupt enable: enable=1, disable=0, default=0
 * 7:      OF1-3_IE:     OF1-3 interrupt enable: enable=1, disable=0, default=0
 * 6:      OF1-2_IE:     OF1-2 interrupt enable: enable=1, disable=0, default=0
 * 5:      OF1-1_IE:     OF1-1 interrupt enable: enable=1, disable=0, default=0
 * 4:      OF1-0_IE:     OF1-0 interrupt enable: enable=1, disable=0, default=0
 * 3:      OF0-3_IE:     OF0-3 interrupt enable: enable=1, disable=0, default=0
 * 2:      OF0-2_IE:     OF0-2 interrupt enable: enable=1, disable=0, default=0
 * 1:      OF0-1_IE:     OF0-1 interrupt enable: enable=1, disable=0, default=0
 * 0:      OF0-0_IE:     OF0-0 interrupt enable: enable=1, disable=0, default=0
 *
 */
#define SSI_SYSTEM_INTENABLE0_OF23_MASK    (0x1<<11)
#define SSI_SYSTEM_INTENABLE0_OF23_ENABLE  (0x1<<11)
#define SSI_SYSTEM_INTENABLE0_OF23_DISABLE 0
#define SSI_SYSTEM_INTENABLE0_OF22_MASK    (0x1<<10)
#define SSI_SYSTEM_INTENABLE0_OF22_ENABLE  (0x1<<10)
#define SSI_SYSTEM_INTENABLE0_OF22_DISABLE 0
#define SSI_SYSTEM_INTENABLE0_OF21_MASK    (0x1<<9)
#define SSI_SYSTEM_INTENABLE0_OF21_ENABLE  (0x1<<9)
#define SSI_SYSTEM_INTENABLE0_OF21_DISABLE 0
#define SSI_SYSTEM_INTENABLE0_OF20_MASK    (0x1<<8)
#define SSI_SYSTEM_INTENABLE0_OF20_ENABLE  (0x1<<8)
#define SSI_SYSTEM_INTENABLE0_OF20_DISABLE 0
#define SSI_SYSTEM_INTENABLE0_OF13_MASK    (0x1<<7)
#define SSI_SYSTEM_INTENABLE0_OF13_ENABLE  (0x1<<7)
#define SSI_SYSTEM_INTENABLE0_OF13_DISABLE 0
#define SSI_SYSTEM_INTENABLE0_OF12_MASK    (0x1<<6)
#define SSI_SYSTEM_INTENABLE0_OF12_ENABLE  (0x1<<6)
#define SSI_SYSTEM_INTENABLE0_OF12_DISABLE 0
#define SSI_SYSTEM_INTENABLE0_OF11_MASK    (0x1<<5)
#define SSI_SYSTEM_INTENABLE0_OF11_ENABLE  (0x1<<5)
#define SSI_SYSTEM_INTENABLE0_OF11_DISABLE 0
#define SSI_SYSTEM_INTENABLE0_OF10_MASK    (0x1<<4)
#define SSI_SYSTEM_INTENABLE0_OF10_ENABLE  (0x1<<4)
#define SSI_SYSTEM_INTENABLE0_OF10_DISABLE 0
#define SSI_SYSTEM_INTENABLE0_OF03_MASK    (0x1<<3)
#define SSI_SYSTEM_INTENABLE0_OF03_ENABLE  (0x1<<3)
#define SSI_SYSTEM_INTENABLE0_OF03_DISABLE 0
#define SSI_SYSTEM_INTENABLE0_OF02_MASK    (0x1<<2)
#define SSI_SYSTEM_INTENABLE0_OF02_ENABLE  (0x1<<2)
#define SSI_SYSTEM_INTENABLE0_OF02_DISABLE 0
#define SSI_SYSTEM_INTENABLE0_OF01_MASK    (0x1<<1)
#define SSI_SYSTEM_INTENABLE0_OF01_ENABLE  (0x1<<1)
#define SSI_SYSTEM_INTENABLE0_OF01_DISABLE 0
#define SSI_SYSTEM_INTENABLE0_OF00_MASK    0x1
#define SSI_SYSTEM_INTENABLE0_OF00_ENABLE  0x1
#define SSI_SYSTEM_INTENABLE0_OF00_DISABLE 0

/*
 * Bit field mappings for SSI_SYSTEM_INTENABLE1 register
 *
 * 8-31:   Not used:     set to 0
 * 7:      OF9-3_IE:     OF9-3 interrupt enable: enable=1, disable=0, default=0
 * 6:      OF9-2_IE:     OF9-2 interrupt enable: enable=1, disable=0, default=0
 * 5:      OF9-1_IE:     OF9-1 interrupt enable: enable=1, disable=0, default=0
 * 4:      OF9-0_IE:     OF9-0 interrupt enable: enable=1, disable=0, default=0
 * 0-3:    Not used:     set to 0
 *
 */
#define SSI_SYSTEM_INTENABLE1_OF93_MASK     (0x1<<7)
#define SSI_SYSTEM_INTENABLE1_OF93_ENABLE   (0x1<<7)
#define SSI_SYSTEM_INTENABLE1_OF93_DISABLE  0
#define SSI_SYSTEM_INTENABLE1_OF92_MASK     (0x1<<6)
#define SSI_SYSTEM_INTENABLE1_OF92_ENABLE   (0x1<<6)
#define SSI_SYSTEM_INTENABLE1_OF92_DISABLE  0
#define SSI_SYSTEM_INTENABLE1_OF91_MASK     (0x1<<5)
#define SSI_SYSTEM_INTENABLE1_OF91_ENABLE   (0x1<<5)
#define SSI_SYSTEM_INTENABLE1_OF91_DISABLE  0
#define SSI_SYSTEM_INTENABLE1_OF90_MASK     (0x1<<4)
#define SSI_SYSTEM_INTENABLE1_OF90_ENABLE   (0x1<<4)
#define SSI_SYSTEM_INTENABLE1_OF90_DISABLE  0

/*
 * Bit field mappings for SSI_SYSTEM_INTENABLE2 register
 *
 * 12-31:  Not used:     set to 0
 * 11:     UF2-3_IE:     UF2-3 interrupt enable: enable=1, disable=0, default=0
 * 10:     UF2-2_IE:     UF2-2 interrupt enable: enable=1, disable=0, default=0
 * 9:      UF2-1_IE:     UF2-1 interrupt enable: enable=1, disable=0, default=0
 * 8:      UF2-0_IE:     UF2-0 interrupt enable: enable=1, disable=0, default=0
 * 7:      UF1-3_IE:     UF1-3 interrupt enable: enable=1, disable=0, default=0
 * 6:      UF1-2_IE:     UF1-2 interrupt enable: enable=1, disable=0, default=0
 * 5:      UF1-1_IE:     UF1-1 interrupt enable: enable=1, disable=0, default=0
 * 4:      UF1-0_IE:     UF1-0 interrupt enable: enable=1, disable=0, default=0
 * 3:      UF0-3_IE:     UF0-3 interrupt enable: enable=1, disable=0, default=0
 * 2:      UF0-2_IE:     UF0-2 interrupt enable: enable=1, disable=0, default=0
 * 1:      UF0-1_IE:     UF0-1 interrupt enable: enable=1, disable=0, default=0
 * 0:      UF0-0_IE:     UF0-0 interrupt enable: enable=1, disable=0, default=0
 *
 */
#define SSI_SYSTEM_INTENABLE2_UF23_MASK    (0x1<<11)
#define SSI_SYSTEM_INTENABLE2_UF23_ENABLE  (0x1<<11)
#define SSI_SYSTEM_INTENABLE2_UF23_DISABLE 0
#define SSI_SYSTEM_INTENABLE2_UF22_MASK    (0x1<<10)
#define SSI_SYSTEM_INTENABLE2_UF22_ENABLE  (0x1<<10)
#define SSI_SYSTEM_INTENABLE2_UF22_DISABLE 0
#define SSI_SYSTEM_INTENABLE2_UF21_MASK    (0x1<<9)
#define SSI_SYSTEM_INTENABLE2_UF21_ENABLE  (0x1<<9)
#define SSI_SYSTEM_INTENABLE2_UF21_DISABLE 0
#define SSI_SYSTEM_INTENABLE2_UF20_MASK    (0x1<<8)
#define SSI_SYSTEM_INTENABLE2_UF20_ENABLE  (0x1<<8)
#define SSI_SYSTEM_INTENABLE2_UF20_DISABLE 0
#define SSI_SYSTEM_INTENABLE2_UF13_MASK    (0x1<<7)
#define SSI_SYSTEM_INTENABLE2_UF13_ENABLE  (0x1<<7)
#define SSI_SYSTEM_INTENABLE2_UF13_DISABLE 0
#define SSI_SYSTEM_INTENABLE2_UF12_MASK    (0x1<<6)
#define SSI_SYSTEM_INTENABLE2_UF12_ENABLE  (0x1<<6)
#define SSI_SYSTEM_INTENABLE2_UF12_DISABLE 0
#define SSI_SYSTEM_INTENABLE2_UF11_MASK    (0x1<<5)
#define SSI_SYSTEM_INTENABLE2_UF11_ENABLE  (0x1<<5)
#define SSI_SYSTEM_INTENABLE2_UF11_DISABLE 0
#define SSI_SYSTEM_INTENABLE2_UF10_MASK    (0x1<<4)
#define SSI_SYSTEM_INTENABLE2_UF10_ENABLE  (0x1<<4)
#define SSI_SYSTEM_INTENABLE2_UF10_DISABLE 0
#define SSI_SYSTEM_INTENABLE2_UF03_MASK    (0x1<<3)
#define SSI_SYSTEM_INTENABLE2_UF03_ENABLE  (0x1<<3)
#define SSI_SYSTEM_INTENABLE2_UF03_DISABLE 0
#define SSI_SYSTEM_INTENABLE2_UF02_MASK    (0x1<<2)
#define SSI_SYSTEM_INTENABLE2_UF02_ENABLE  (0x1<<2)
#define SSI_SYSTEM_INTENABLE2_UF02_DISABLE 0
#define SSI_SYSTEM_INTENABLE2_UF01_MASK    (0x1<<1)
#define SSI_SYSTEM_INTENABLE2_UF01_ENABLE  (0x1<<1)
#define SSI_SYSTEM_INTENABLE2_UF01_DISABLE 0
#define SSI_SYSTEM_INTENABLE2_UF00_MASK    0x1
#define SSI_SYSTEM_INTENABLE2_UF00_ENABLE  0x1
#define SSI_SYSTEM_INTENABLE2_UF00_DISABLE 0

/*
 * Bit field mappings for SSI_SYSTEM_INTENABLE3 register
 *
 * 8-31:   Not used:     set to 0
 * 7:      UF9-3_IE:     UF9-3 interrupt enable: enable=1, disable=0, default=0
 * 6:      UF9-2_IE:     UF9-2 interrupt enable: enable=1, disable=0, default=0
 * 5:      UF9-1_IE:     UF9-1 interrupt enable: enable=1, disable=0, default=0
 * 4:      UF9-0_IE:     UF9-0 interrupt enable: enable=1, disable=0, default=0
 * 0-3:    Not used:     set to 0
 *
 */
#define SSI_SYSTEM_INTENABLE3_UF93_MASK    (0x1<<7)
#define SSI_SYSTEM_INTENABLE3_UF93_ENABLE  (0x1<<7)
#define SSI_SYSTEM_INTENABLE3_UF93_DISABLE 0
#define SSI_SYSTEM_INTENABLE3_UF92_MASK    (0x1<<6)
#define SSI_SYSTEM_INTENABLE3_UF92_ENABLE  (0x1<<6)
#define SSI_SYSTEM_INTENABLE3_UF92_DISABLE 0
#define SSI_SYSTEM_INTENABLE3_UF91_MASK    (0x1<<5)
#define SSI_SYSTEM_INTENABLE3_UF91_ENABLE  (0x1<<5)
#define SSI_SYSTEM_INTENABLE3_UF91_DISABLE 0
#define SSI_SYSTEM_INTENABLE3_UF90_MASK    (0x1<<4)
#define SSI_SYSTEM_INTENABLE3_UF90_ENABLE  (0x1<<4)
#define SSI_SYSTEM_INTENABLE3_UF90_DISABLE 0

/*
 * Enum values that relate to SSICRx register
 */

// Defines on what edge of the bit clk data is sampled
typedef enum
{
    SSI_BIT_CLK_POL_RISING = 0,
    SSI_BIT_CLK_POL_FALLING
} ssi_bit_clk_pol_t;

// Defines on what edge of the bit clk data is sampled
typedef enum
{
    SSI_WS_POL_0 = 0,
    SSI_WS_POL_1
} ssi_ws_pol_t;

typedef enum
{
    SSI_BIT_DELAY_ONE = 0,
    SSI_BIT_DELAY_NONE
} ssi_bit_delay_t;

typedef enum
{
    /* mono values */
    SSI_SYS_WORD_LEN_16BIT_MONO = 0,
    SSI_SYS_WORD_LEN_32BIT_MONO,
    SSI_SYS_WORD_LEN_48BIT_MONO,
    SSI_SYS_WORD_LEN_64BIT_MONO,
    SSI_SYS_WORD_LEN_96BIT_MONO,
    SSI_SYS_WORD_LEN_128BIT_MONO,
    SSI_SYS_WORD_LEN_256BIT_MONO,
    SSI_SYS_WORD_LEN_512BIT_MONO,
    /* stereo/multi channel values */
    SSI_SYS_WORD_LEN_8BIT_STEREO = 0,
    SSI_SYS_WORD_LEN_16BIT_STEREO,
    SSI_SYS_WORD_LEN_24BIT_STEREO,
    SSI_SYS_WORD_LEN_32BIT_STEREO,
    SSI_SYS_WORD_LEN_48BIT_STEREO,
    SSI_SYS_WORD_LEN_64BIT_STEREO,
    SSI_SYS_WORD_LEN_128BIT_STEREO,
    SSI_SYS_WORD_LEN_256BIT_STEREO
} ssi_sys_word_len_t;

typedef enum
{
   SSI_DATA_WORD_LEN_8BIT = 0,
   SSI_DATA_WORD_LEN_16BIT,
   SSI_DATA_WORD_LEN_18BIT,
   SSI_DATA_WORD_LEN_20BIT,
   SSI_DATA_WORD_LEN_22BIT,
   SSI_DATA_WORD_LEN_24BIT,
   SSI_DATA_WORD_LEN_32BIT
} ssi_data_word_len_t;

typedef enum
{
   SSI_PARALLEL_DATA_ALIGN_LEFT = 0,
   SSI_PARALLEL_DATA_ALIGN_RIGHT
} ssi_parallel_data_align_t;

typedef enum
{
   SSI_SER_DATA_ALIGN_DATA_FIRST = 0,
   SSI_SER_DATA_ALIGN_PADDING_FIRST = 1
} ssi_serial_data_align_t;

typedef enum
{
   SSI_PADDING_POL_LOW = 0,
   SSI_PADDING_POL_HIGH
} ssi_padding_pol_t;

/*
 * Bit field mappings for SSICRx register
 *
 * 31:       FORCE      Fixed to the value 1; default value=1
 * 29,30:    Not Used   Set to 0
 * 28:       DMEN       DMA enable setting - enable=1, disable=0, default=0
 * 27:       UIEN       Underflow IRQ enable setting - enable=1, disable=0, default=0
 * 26:       OIEN       Overflow IRQ enable setting - enable=1, disable=0, default=0
 * 25:       IIEN       Idle Mode IRQ enable setting - enable=1, disable=0, default=0
 * 24:       DIEN       Data IRQ enable setting - enable=1, disable=0, default=0
 * 22,23:    CHNL       Channels
 * 19,20,21: DWL        Data Word Length, default value=3 (32 bits)
 * 16,17,18: SWL        System Word Length, default value=3 (32 bits)
 * 15:       SCKD       Serial Clock Direction - SCK is input/slave mode=0, WS is output/master mode=1, default=0
 * 14:       SWSD       Serial WS Direction - WS is input/slave mode=0, WS is output/master mode=1, default=0
 * 13:       SCKP       Serial Clock Polarity - rising edge=0, falling edge=1, default=0
 * 12:       SWSP       Serial WS Polarity - meaning of 0,1 values is context dependent
 * 11:       SPDP       Serial Padding Polarity - padding bits are zero/low=0, one/high=1, default=0
 * 10:       SDTA       Serial Data Alignment - serial data followed by padding bits=0,
 *                                              padding bits followed by serial data=1, default=0
 * 9:        PDTA       Parallel Data Alignment - left alignment=0, right alignment=1, default=0
 * 8:        DEL        Serial Data Delay - no delay=0, 1 clock cycle delay=1, default=0
 * 7:        Not used   Set to 0
 * 4,5,6:    CKDV       Serial Oversample Clock Divide Ratio, default=3 (CKDV_8)
 * 3:        MUEN       Mute setting - mute=1, unmute=0, default=0
 * 2:        Not used   Set to 0
 * 1:        TRMD       Transmit/receive mode, transmit=1, receive=0, default=1
 * 0:        EN         SSI Enable setting - enable=1, disable=0, default=0
 */
#define SSICR_EN_MASK             0x1
#define SSICR_EN_ENABLE           0x1
#define SSICR_EN_DISABLE          0
#define SSICR_TRMD_MASK           (0x1<<1)
#define SSICR_TRMD_TX_MODE        (0x1<<1)
#define SSICR_TRMD_RX_MODE        0
#define SSICR_MUEN_MASK           (0x1<<3)
#define SSICR_MUEN_MUTED          (0x1<<3)
#define SSICR_MUEN_UNMUTED        0
#define SSICR_CKDV_MASK           (0x7<<4)
#define SSICR_CKDV_1              0
#define SSICR_CKDV_2              (0x1<<4)
#define SSICR_CKDV_4              (0x2<<4)
#define SSICR_CKDV_8              (0x3<<4)
#define SSICR_CKDV_16             (0x4<<4)
#define SSICR_CKDV_6              (0x5<<4)
#define SSICR_CKDV_12             (0x6<<4)
#define SSICR_DEL_MASK            (0x1<<8)
#define SSICR_DEL_1CYCLE          (SSI_BIT_DELAY_ONE<<8)
#define SSICR_DEL_NONE            (SSI_BIT_DELAY_NONE<<8)
#define SSICR_PDTA_MASK           (0x1<<9)
#define SSICR_PDTA_LEFT           (SSI_PARALLEL_DATA_ALIGN_LEFT<<9)
#define SSICR_PDTA_RIGHT          (SSI_PARALLEL_DATA_ALIGN_RIGHT<<9)
#define SSICR_SDTA_MASK           (0x1<<10)
#define SSICR_SDTA_SDATA_FIRST    (SSI_SER_DATA_ALIGN_DATA_FIRST<<10)
#define SSICR_SDTA_PADDING_FIRST  (SSI_SER_DATA_ALIGN_PADDING_FIRST<<10)
#define SSICR_SPDP_MASK           (0x1<<11)
#define SSICR_SPDP_PADDING_LOW    (SSI_PADDING_POL_LOW<<11)
#define SSICR_SPDP_PADDING_HIGH   (SSI_PADDING_POL_HIGH<<11)
#define SSICR_SWSP_MASK           (0x1<<12)
#define SSICR_SWSP_0              (SSI_WS_POL_0<<12)  /* meaning of SWSP is context dependent */
#define SSICR_SWSP_1              (SSI_WS_POL_1<<12)  /* meaning of SWSP is context dependent */
#define SSICR_SCKP_MASK           (0x1<<13)
#define SSICR_SCKP_RISING         (SSI_BIT_CLK_POL_RISING<<13)
#define SSICR_SCKP_FALLING        (SSI_BIT_CLK_POL_FALLING<<13)
#define SSICR_SWSD_MASK           (0x1<<14)
#define SSICR_SWSD_INPUT          0
#define SSICR_SWSD_OUTPUT         (0x1<<14)
#define SSICR_SCKD_MASK           (0x1<<15)
#define SSICR_SCKD_INPUT          0
#define SSICR_SCKD_OUTPUT         (0x1<<15)
#define SSICR_SWL_MASK            (0x7<<16)
#define SSICR_SWL_8BIT            (SSI_SYS_WORD_LEN_8BIT_STEREO<<16)
#define SSICR_SWL_16BIT           (SSI_SYS_WORD_LEN_16BIT_STEREO<<16)
#define SSICR_SWL_24BIT           (SSI_SYS_WORD_LEN_24BIT_STEREO<<16)
#define SSICR_SWL_32BIT           (SSI_SYS_WORD_LEN_32BIT_STEREO<<16)
#define SSICR_SWL_48BIT           (SSI_SYS_WORD_LEN_48BIT_STEREO<<16)
#define SSICR_SWL_64BIT           (SSI_SYS_WORD_LEN_64BIT_STEREO<<16)
#define SSICR_SWL_128BIT          (SSI_SYS_WORD_LEN_128BIT_STEREO<<16)
#define SSICR_SWL_256BIT          (SSI_SYS_WORD_LEN_256BIT_STEREO<<16)
#define SSICR_SWL_16BIT_MONO      (SSI_SYS_WORD_LEN_16BIT_MONO<<16)
#define SSICR_SWL_32BIT_MONO      (SSI_SYS_WORD_LEN_32BIT_MONO<<16)
#define SSICR_SWL_48BIT_MONO      (SSI_SYS_WORD_LEN_48BIT_MONO<<16)
#define SSICR_SWL_64BIT_MONO      (SSI_SYS_WORD_LEN_64BIT_MONO<<16)
#define SSICR_SWL_96BIT_MONO      (SSI_SYS_WORD_LEN_96BIT_MONO<<16)
#define SSICR_SWL_128BIT_MONO     (SSI_SYS_WORD_LEN_128BIT_MONO<<16)
#define SSICR_SWL_256BIT_MONO     (SSI_SYS_WORD_LEN_256BIT_MONO<<16)
#define SSICR_SWL_512BIT_MONO     (SSI_SYS_WORD_LEN_512BIT_MONO<<16)
#define SSICR_DWL_MASK            (0x7<<19)
#define SSICR_DWL_8BIT            0
#define SSICR_DWL_16BIT           (0x1<<19)
#define SSICR_DWL_18BIT           (0x2<<19)
#define SSICR_DWL_20BIT           (0x3<<19)
#define SSICR_DWL_22BIT           (0x4<<19)
#define SSICR_DWL_24BIT           (0x5<<19)
#define SSICR_DWL_32BIT           (0x6<<19)
#define SSICR_CHNL_MASK           (0x3<<22)
#define SSICR_CHNL_1              0         /* system word=1 channel, or mono frame=1 system word */
#define SSICR_CHNL_2              (0x1<<22) /* system word=2 channel, or TDM frame=4 system words */
#define SSICR_CHNL_3              (0x2<<22) /* system word=3 channel, or TDM frame=6 system words */
#define SSICR_CHNL_4              (0x3<<22) /* system word=4 channel, or TDM frame=8 system words */
#define SSICR_DIEN_MASK           (0x1<<24)
#define SSICR_DIEN_DISABLE        0
#define SSICR_DIEN_ENABLE         (0x1<<24)
#define SSICR_IIEN_MASK           (0x1<<25)
#define SSICR_IIEN_DISABLE        0
#define SSICR_IIEN_ENABLE         (0x1<<25)
#define SSICR_OIEN_MASK           (0x1<<26)
#define SSICR_OIEN_DISABLE        0
#define SSICR_OIEN_ENABLE         (0x1<<26)
#define SSICR_UIEN_MASK           (0x1<<27)
#define SSICR_UIEN_DISABLE        0
#define SSICR_UIEN_ENABLE         (0x1<<27)
#define SSICR_DMEN_MASK           (0x1<<28)
#define SSICR_DMEN_DISABLE        0
#define SSICR_DMEN_ENABLE         (0x1<<28)
#define SSICR_FORCE_MASK          (0x1<<31)
#define SSICR_FORCE_FIXED         (0x1<<31)


/*
 * Bit field mappings for SSISRx register
 *
 * 29-31:   Not used
 * 28:      DMRQ:      DMA Request Status Flag; DMA request active=1; inactive=0, default=0
 * 27:      UIRQ:      Underflow Interrupt Status Flag; underflow condition present=1, absent=0, default=0
 * 26:      OIRQ:      Overflow Interrupt Status Flag; overflow condition present=1, absent=0, default=0
 * 25:      IIRQ:      Idle Mode Interrupt Status Flag; idle state detected=1, not detected=0, default=0
 * 24:      DIRQ:      Data Interrupt Status Flag; data available to be read, or data needs to be written=1, otherwise=0, default=0
 * 4-23:    Not used
 * 2,3:     CHNO:      Current channel number that is being written to or read from (not applicable for 8 or 16 bit data)
 * 1:       SWNO:      Current system word number that is being written or read (not applicable for 8 or 16 bit data)
 * 0:       IDST:      Idle Mode Status Flag: serial bus activity stopped=1; otherwise=0; default=0
 *
 */
#define SSISR_IDST_MASK     0x11
#define SSISR_SWNO_MASK     (0x1<<1)
#define SSISR_SWNO_0        0
#define SSISR_SWNO_1        (0x1<<1)
#define SSISR_CHNO_MASK     (0x3<<2)
#define SSISR_CHNO_0        0
#define SSISR_CHNO_1        (0x1<<2)
#define SSISR_CHNO_2        (0x2<<2)
#define SSISR_CHNO_3        (0x3<<2)
#define SSISR_DIRQ_MASK     (0x1<<24)
#define SSISR_IIRQ_MASK     (0x1<<25)
#define SSISR_OIRQ_MASK     (0x1<<26)
#define SSISR_UIRQ_MASK     (0x1<<27)
#define SSISR_DMRQ_MASK     (0x1<<28)


/*
 * Bit field mappings for SSIWSRx register
 *
 * 21-31:   Not used     Set to 0
 * 16-20:   WIDTH:       Sync Pulse Width in clock cycles (1 to 31), 0 if TDM mode is used
 * 9-15:    Not used     Set to 0
 * 8:       CONT:        WS Continue function: enabled=1, disabled=0, default=0
 * 2-7:     Not used     Set to 0
 * 1:       MONO:        TDM/Monaural Format: TDM=0, Mono=1, default=0
 * 0:       WS_MODE:     WS Mode: Stereo, multichannel=0, TDM, Monaural=1, default=0
 *
 */
#define SSIWSR_WSMODE_MASK            0x1
#define SSIWSR_WSMODE_STEREO_MULTICH  0
#define SSIWSR_WSMODE_TDM_MONO        0x1
#define SSIWSR_MONO_MASK              (0x1<<1)
#define SSIWSR_MONO_TDM               0
#define SSIWSR_MONO_MONO              (0x1<<1)
#define SSIWSR_CONT_MASK              (0x1<<8)
#define SSIWSR_CONT_DISABLED          0
#define SSIWSR_CONT_ENABLED           (0x1<<8)
#define SSIWSR_WIDTH_MASK             (0x1F<<16)

/*
 * Bit field mappings for SSIFMRx register
 *
 * 22-31:   Not used     Set to 0
 * 16-20:   DTCT         Frequency Switching Detection Range
 * 6-15:    Not used     Set to 0
 * 4,5:     CTDV         BUs clock division ratio
 * 1-3:     Not used     Set to 0
 * 0:       FSEN         Frequency Switching Detection Enable - enale=1, disable=0, default=0
 *
 */
#define SSIFMR_DTCT_MASK     (0x1F<<16)
#define SSIFMR_CTDV_MASK     (0x3<<4)
#define SSIFMR_FSEN_MASK     0x1
#define SSIFMR_FSEN_DISABLE  0
#define SSIFMR_FSEN_ENABLE   0x1

/*
 * Bit field mappings for SSIFSRx register
 *
 * 16-31:   Not used     Read as 0
 * 15:      FCST         WS Stopped Status Flag - WS signal stopped=1, otherwise=0
 * 14:      DTST         Frequency Switching Detection Status - switching detected=1, otherwise=0
 * 12,13:   Not used     Read as 0
 * 0-11:    FCNT         Frequency Count Monitor
 *
 */
#define SSIFSR_FCST_MASK    (0x1<<15)
#define SSIFSR_DTST_MASK    (0x1<<14)
#define SSIFSR_FCNT_MASK    0xFFF

/* structure used for memory mapped SSI registers*/
typedef struct
{
    volatile uint32_t   cr;         /* Control register - SSICRx, x=0..9 */
    volatile uint32_t   sr;         /* Status register - SSISRx, x=0..9 */
    volatile uint32_t   tdr;        /* Transmit data register - SSITDRx, x=0..9 */
    volatile uint32_t   rdr;        /* Receive data register - SSIRDRx, x=0..9 */
    volatile uint32_t   dummy1[4];
    volatile uint32_t   wsr;        /* WS mode register - SSIWSRx, x=0..9 */
    volatile uint32_t   fmr;        /* FS mode register - SSIFMRx, x=0..9 */
    volatile uint32_t   fsr;        /* FS status register - SSIFSRx, x=0..9 */
    volatile uint32_t   dummy2[5];
} ssi_reg_t;

/* structures used for memory mapped SSIU registers*/
typedef struct
{
    volatile uint32_t   busif_mode;  // SSIx-y_BUSIF_MODE for x=0,1,2,9 and y=0,1,2,3 or SSIx_BUSIF_MODE for x=3,4,5,6,7,8
    volatile uint32_t   busif_adinr; // SSIx-y_BUSIF_ADINR for x=0,1,2,9 and y=0,1,2,3 or SSIx_BUSIF_ADINR for x=3,4,5,6,7,8
    volatile uint32_t   busif_dalign; // SSIx-y_BUSIF_DALIGN for x=0,1,2,9 and y=0,1,2,3 or SSIx_BUSIF_DALIGN for x=3,4,5,6,7,8
} ssiu_busif_reg_t;

typedef struct
{
    volatile uint32_t   mode; // SSIx-0_MODE for x=0,1,2,9 or SSIx_MODE for x=3,4
    volatile uint32_t   control; // SSIx-0_CONTROL for x=0,1,2,9 or SSIx_CONTROL for x=3,4,5,6,7,8
    volatile uint32_t   status; // SSIx-0_STATUS for x=0,1,2,9 or SSIx_STATUS for x=3,4,5,6,7,8
    volatile uint32_t   int_enable_main; // SSIx-0_INT_ENABLE_MAIN for x=0,1,2,9 or SSIx_INT_ENABLE_MAIN for x=3,4,5,6,7,8
} ssiu_ssi_reg_t;

typedef struct
{
    ssiu_busif_reg_t   busif;
    ssiu_ssi_reg_t     ssi;
    volatile uint32_t  dummy;
} ssiu_reg_t;

typedef struct
{
    volatile uint32_t   mode0; // SSI_MODE0
    volatile uint32_t   mode1; // SSI_MODE1
    volatile uint32_t   mode2; // SSI_MODE2
    volatile uint32_t   mode3; // SSI_MODE3
    volatile uint32_t   control; // SSI_CONTROL
    volatile uint32_t   dummy[11];
    volatile uint32_t   system_status0; // SSI_SYSTEM_STATUS0
    volatile uint32_t   system_status1; // SSI_SYSTEM_STATUS1
    volatile uint32_t   system_status2; // SSI_SYSTEM_STATUS2
    volatile uint32_t   system_status3; // SSI_SYSTEM_STATUS3
    volatile uint32_t   system_int_enable0; // SSI_SYSTEM_INT_ENABLE0
    volatile uint32_t   system_int_enable1; // SSI_SYSTEM_INT_ENABLE1
    volatile uint32_t   system_int_enable2; // SSI_SYSTEM_INT_ENABLE2
    volatile uint32_t   system_int_enable3; // SSI_SYSTEM_INT_ENABLE3
} ssiu_common_reg_t;

ssi_reg_t* get_ssi_reg(uint32_t ssi_idx);
ssiu_busif_reg_t* get_ssiu_busif_reg(uint32_t ssi_idx, uint32_t ssi_subchan_idx);
ssiu_ssi_reg_t* get_ssiu_ssi_reg(uint32_t ssi_idx);
ssiu_common_reg_t* get_ssiu_common_reg(void);

int ssiu_mem_map(void);
int ssiu_mem_unmap(void);

#define SSI(s) get_ssi_reg(s)
#define SSIU_BUSIF(s,u) get_ssiu_busif_reg(s,u)
#define SSIU_SSI(s) get_ssiu_ssi_reg(s)
#define SSIU_COMMON get_ssiu_common_reg()

#endif /* _R_Car_SSIU_REG_H */

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/hardware/deva/ctrl/rcar/ssiu_reg.h $ $Rev: 812827 $")
#endif
