/*
 * $QNXLicenseC:
 * Copyright 2014, QNX Software Systems.
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

#include <sys/types.h>
#include <sys/mman.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/neutrino.h>
#include <hw/inout.h>
#include <arm/r-car.h>
#include <hw/dma.h>
#include "hdmi_audio.h"

#define SSI_CHANNEL_NUM         10
#define SSIU_CHANNEL_NUM        10
#define SSIU_SUBCHANNEL_NUM     4

#define SSI_MODULE_SIZE         0x40

/* SSICRn bit */
/*
 * Bit mappings for SSI control register:
 *
 * 29,30,31 NotUsed = 0;
 * 28       DMEN = 0;         DMA Disable
 * 27       UIEN = 0;         Underflow IRQ disable
 * 26       OIEN = 0;         Overflow IRQ disable
 * 25       IIEN = 0;         Idle Mode IRQ disable
 * 24       DIEN = 0;         Data IRQ disable
 * 22,23    CHNL = 0;         Channels
 * 19,20,21 DWL  = 3;         Data Word Length 20 bit
 * 16,17,18 SWL  = 3;         System Word Length 32 bit
 * 15       SCKD = 0;         Serial Clock Output(Slave)
 * 14       SWSD = 0;         Serial WS Output(Slave)
 * 13       SCKP = 1;         Serial Clock Polarity falling edge
 * 12       SWSP = 1;         Serial WS Polarity High for Left
 * 11       SPDP = 0;         Serial Padding Polarity Low
 * 10       SDTA = 1;         Serial Data Alignment Right
 * 9        PDTA = 1;         Parallel Data Alignment Right
 * 8        DEL  = 1;         Serial Data no Delay
 * 7        BREN = 0;         Burst Mode Disable
 * 4,5,6    CKDV = 3;         Serial Oversample Clock Divide Ratio
 * 3        MUEN = 0;         Mute disable
 * 2        CPEN = 0;         Compress mode disable
 * 1        TRMD = 1;         Transmit mode
 * 0        EN   = 0;         SSI Disable
 */
#define SSICR_EN       (1 << 0)        /* module enable*/
#define SSICR_TRMD     (1 << 1)        /* module mode select (0:receive/1:transmit)*/
#define SSICR_CPEN     (1 << 2)        /* compress mode enable*/
#define SSICR_MUEN     (1 << 3)        /* mute enable*/
#define SSICR_CKDV_MSK (7 << 4)        /* serial bit clk mask*/
#define SSICR_CKDV_1   (0 << 4)        /* serial bit clk freq = over sample clk / 1*/
#define SSICR_CKDV_2   (1 << 4)        /* serial bit clk freq = over sample clk / 2*/
#define SSICR_CKDV_4   (2 << 4)        /* serial bit clk freq = over sample clk / 4*/
#define SSICR_CKDV_8   (3 << 4)        /* serial bit clk freq = over sample clk / 8*/
#define SSICR_CKDV_16  (4 << 4)        /* serial bit clk freq = over sample clk / 16*/
#define SSICR_CKDV_6   (5 << 4)        /* serial bit clk freq = over sample clk / 6*/
#define SSICR_CKDV_12  (6 << 4)        /* serial bit clk freq = over sample clk / 12*/
#define SSICR_BREN     (1 << 7)        /* burst mode enable*/
#define SSICR_DEL      (1 << 8)        /* delay of serial data (0:1cycle / 1:none)*/
#define SSICR_PDTA     (1 << 9)        /* parallel data alignment (0:left / 1:right)*/
#define SSICR_SDTA     (1 << 10)       /* serial data alignment (0:left / 1:right)*/
#define SSICR_SPDP     (1 << 11)       /* serial padding polarity (0:low-level / 1:high-level)*/
#define SSICR_SWSP     (1 << 12)       /* serial WS polarity (0:high-level / 1:low-level)*/
#define SSICR_SCKP     (1 << 13)       /* serial bit clk polarity (0:rising-edge / 1:falling-edge)*/
#define SSICR_SWSD     (1 << 14)       /* serial WS direction (0:input(slave mode) / 1:output(master mode))*/
#define SSICR_SCKD     (1 << 15)       /* serial bit clk direction (0:input(slave mode) / 1:output(master mode))*/
#define SSICR_SWL_MSK  (7 << 16)       /* system word bit mask*/
#define SSICR_SWL_8    (0 << 16)       /* system word size = 8bit*/
#define SSICR_SWL_16   (1 << 16)       /* system word size = 16bit*/
#define SSICR_SWL_24   (2 << 16)       /* system word size = 24bit*/
#define SSICR_SWL_32   (3 << 16)       /* system word size = 32bit*/
#define SSICR_SWL_48   (4 << 16)       /* system word size = 48bit*/
#define SSICR_SWL_64   (5 << 16)       /* system word size = 64bit*/
#define SSICR_SWL_128  (6 << 16)       /* system word size = 128bit*/
#define SSICR_SWL_256  (7 << 16)       /* system word size = 256bit*/
#define SSICR_DWL_MSK  (7 << 19)       /* data word bist mask*/
#define SSICR_DWL_8    (0 << 19)       /* data word size = 8bit*/
#define SSICR_DWL_16   (1 << 19)       /* data word size = 16bit*/
#define SSICR_DWL_18   (2 << 19)       /* data word size = 18bit*/
#define SSICR_DWL_20   (3 << 19)       /* data word size = 20bit*/
#define SSICR_DWL_22   (4 << 19)       /* data word size = 22bit*/
#define SSICR_DWL_24   (5 << 19)       /* data word size = 24bit*/
#define SSICR_DWL_32   (6 << 19)       /* data word size = 32bit*/
#define SSICR_CHNL_MSK (3 << 22)       /* channel size bit mask*/
#define SSICR_CHNL_1   (0 << 22)       /* system word = 1 channel*/
#define SSICR_CHNL_2   (1 << 22)       /* system word = 2/4 channel*/
#define SSICR_CHNL_3   (2 << 22)       /* system word = 3/6 channel*/
#define SSICR_CHNL_4   (3 << 22)       /* system word = 4/8 channel*/
#define SSICR_CHNL_16  (1 << 0)        /* system word =  /16 channel*/
#define SSICR_DIEN     (1 << 24)       /* data interrupt enable*/
#define SSICR_IIEN     (1 << 25)       /* idle mode interrupt enable*/
#define SSICR_OIEN     (1 << 26)       /* overflow interrupt enable*/
#define SSICR_UIEN     (1 << 27)       /* underflow interrupt enable*/
#define SSICR_DMEN     (1 << 28)       /* DMA request enable*/
#define SSICR_FORCE    (1 << 31)       /* DMA request enable*/

#define SSI_TX_MASTER     \
    (SSICR_FORCE        | \
     SSICR_DWL_16       | \
     SSICR_SWL_32       | \
     SSICR_SCKD         | \
     SSICR_SWSD         | \
     SSICR_CKDV_4       | \
     SSICR_TRMD         | \
     SSICR_DEL          | \
     SSICR_CHNL_1)

#define SSI_TX_SLAVE      \
    (SSICR_DWL_16       | \
     SSICR_SWL_16       | \
     SSICR_TRMD         | \
     SSICR_CHNL_1)

#define SSI_RX_MASTER     \
    (SSICR_DWL_16       | \
     SSICR_SWL_16       | \
     SSICR_SCKD         | \
     SSICR_SWSD         | \
     SSICR_CKDV_4       | \
     SSICR_CHNL_1)

#define SSI_RX_SLAVE      \
    (SSICR_DWL_16       | \
     SSICR_SWL_16       | \
     SSICR_CHNL_1)

#define SSI_RX1_SLAVE     \
    (SSICR_DWL_16       | \
     SSICR_SWL_32       | \
     SSICR_DEL          | \
     SSICR_CHNL_2)

/* SSIWSRn bit */
#define SSIWSR_WS_MODE (1 << 0)
#define SSIWSR_MONO    (1 << 1)
#define SSIWSR_CONT    (1 << 8)

/* SSISRn bit */
#define SSISR_IDST             (1 << 0)
#define SSISR_SWNO             (1 << 1)
#define SSISR_CHNO_0           (0 << 2)
#define SSISR_CHNO_1           (1 << 2)
#define SSISR_CHNO_2           (2 << 2)
#define SSISR_CHNO_3           (3 << 2)
#define SSISR_DIRQ             (1 << 24)
#define SSISR_IIRQ             (1 << 25)
#define SSISR_OIRQ             (1 << 26)
#define SSISR_UIRQ             (1 << 27)
#define SSISR_DMRQ             (1 << 28)

#define SSI_TRANFER_IND_YES          1
#define SSI_TRANFER_IND_NO           0

#define SSI_BUS_FORMAT_I2S           1
#define SSI_BUS_FORMAT_TDM           2
#define SSI_BUS_FORMAT_MONO          3

#define SSI_BUS_FORMAT_TDM_NONE      0
#define SSI_BUS_FORMAT_TDM_EXT       1
#define SSI_BUS_FORMAT_TDM_16CH      2
#define SSI_BUS_FORMAT_TDM_SPLIT     4
#define SSI_BUS_FORMAT_TDM_SPLIT_EXT 8

#define SSI_DATA_ALIGN_LEFT          1
#define SSI_DATA_ALIGN_RIGHT         2

#define SSI_IND_MASTER               1
#define SSI_IND_SLAVE                2

#define SSI_PIN_MODE_IND             0
#define SSI_PIN_MODE_COMMON_BOTH_SLAVE      1
#define SSI_PIN_MODE_COMMON_ONE_MASTER      2

#define DEFAULT_RATE                 44100
#define DMA_BUFFSIZE                 0x1000

/* structure of SSI registers*/
typedef struct
{
    volatile uint32_t   cr;         /* Control register*/
    volatile uint32_t   sr;         /* Status register*/
    volatile uint32_t   tdr;        /* Transmit data register*/
    volatile uint32_t   rdr;        /* Receive data register*/
    volatile uint32_t   dummy[4];
    volatile uint32_t   wsr;
    volatile uint32_t   fmr;
    volatile uint32_t   fsr;
    volatile uint32_t   dummy1[1];
    volatile uint32_t   cre;
    volatile uint32_t   filler[3];
} ssi_regs_t;

typedef struct
{
    volatile uint32_t   busif_mode;
    volatile uint32_t   busif_adinr;
    volatile uint32_t   busif_dalign;
    volatile uint32_t   mode;
    volatile uint32_t   control;
    volatile uint32_t   status;
    volatile uint32_t   int_enable_main;
    volatile uint32_t   dummy;
} ssiu_regs_t;

typedef struct
{
    volatile uint32_t   busif_mode;
    volatile uint32_t   busif_adinr;
    volatile uint32_t   busif_dalign;
    volatile uint32_t   dummy[5];
} ssiu2_regs_t;


typedef struct
{
    volatile uint32_t   busif_dalign2;
    volatile uint32_t   mode2;
    volatile uint32_t   dummy;
    volatile uint32_t   status2;
    volatile uint32_t   enable_main2;
    volatile uint32_t   dummy2[4];
} ssiu_ext_regs_t;

typedef struct
{
    volatile uint32_t   mode0;
    volatile uint32_t   mode1;
    volatile uint32_t   mode2;
    volatile uint32_t   mode3;
    volatile uint32_t   control;
    volatile uint32_t   control2;
    volatile uint32_t   dummy[(0x40 - 0x14) / 4 - 1];
    volatile uint32_t   status0;
    volatile uint32_t   status1;
    volatile uint32_t   status2;
    volatile uint32_t   status3;
    volatile uint32_t   int_enable0;
    volatile uint32_t   int_enable1;
    volatile uint32_t   int_enable2;
    volatile uint32_t   int_enable3;
    volatile uint32_t   dummy2[(0x80 - 0x5C) / 4 - 1];
    volatile uint32_t   status4;
    volatile uint32_t   status5;
    volatile uint32_t   status6;
    volatile uint32_t   status7;
    volatile uint32_t   int_enable4;
    volatile uint32_t   int_enable5;
    volatile uint32_t   int_enable6;
    volatile uint32_t   int_enable7;
    volatile uint32_t   dummy3[(0x9E0 - 0x89C) / 4 - 1];
    volatile uint32_t   hdmi0_sel;
    volatile uint32_t   hdmi1_sel;
} ssiu_common_regs_t;

#define RCAR_SSIU(c, s) ((c) * (SSIU_SUBCHANNEL_NUM) + s)

static ssi_regs_t            *ssi_regs = MAP_FAILED;
static ssiu_regs_t           *ssiu_regs = MAP_FAILED;
static ssiu2_regs_t          *ssiu2_regs = MAP_FAILED;
static ssiu_ext_regs_t       *ssiu_ext_regs = MAP_FAILED;
static ssiu_common_regs_t    *ssiu_com_regs = MAP_FAILED;

/*
 * Below code is to configure Audio path :
 * ADV7482 ----> SSI4 ----> Audio-DMAC-PP ----> SSI3 ----> HDMI
 */

int ssi_init()
{
    /* SSI registers */
    ssi_regs = (ssi_regs_t *)mmap_device_io (SSI_CHANNEL_NUM * SSI_MODULE_SIZE, RCAR_SSI_BASE);
    if (ssi_regs == MAP_FAILED )
    {
        printf ("%s : SSI registers map failed\n", __FUNCTION__);
        return -1;
    }

    /* SSIU registers */
    ssiu_regs = (ssiu_regs_t *)mmap_device_io (0x1000, RCAR_SSIU_BASE);
    if (ssiu_regs == MAP_FAILED )
    {
        printf ("%s: SSIU map failed\n", __FUNCTION__);
        return -1;
    }
    ssiu2_regs    = (ssiu2_regs_t *)((uintptr_t)ssiu_regs + 0x500);
    ssiu_com_regs = (ssiu_common_regs_t *)((uintptr_t)ssiu_regs + 0x800);
    ssiu_ext_regs = (ssiu_ext_regs_t *)((uintptr_t)ssiu_regs + 0xA08);

    return 0;
}

void ssi_deinit()
{
    munmap_device_io((uintptr_t)ssi_regs, SSI_CHANNEL_NUM * SSI_MODULE_SIZE);
    ssi_regs = MAP_FAILED;

    munmap_device_io((uintptr_t )ssiu_regs, 0x1000);
    ssiu_regs     = MAP_FAILED;
    ssiu2_regs    = MAP_FAILED;
    ssiu_com_regs = MAP_FAILED;
    ssiu_ext_regs = MAP_FAILED;
}

static int ssi_control_register_set(int channel, uint32_t cr)
{
    if ((channel < 0) || (channel >= SSI_CHANNEL_NUM))
    {
        printf("%s : SSI%d is not supported\n", __FUNCTION__, channel);
        return -1;
    }

    ssi_regs[channel].cr = cr;

    return 0;
}


static int ssi_control2_register_set(int channel, uint32_t cr)
{
    if ((channel < 0) || (channel >= SSI_CHANNEL_NUM))
    {
        printf("%s : SSI%d is not supported\n", __FUNCTION__, channel);
        return -1;
    }

    ssi_regs[channel].cre = cr;

    return 0;
}

static int ssi_control_register_get(int channel, uint32_t *cr_val)
{
    if ((channel < 0) || (channel >= SSI_CHANNEL_NUM))
    {
        printf("%s : SSI%d is not supported\n", __FUNCTION__, channel);
        return -1;
    }

    *cr_val = ssi_regs[channel].cr;

    return 0;
}

static int ssi_status_register_get(int channel, uint32_t * sr_val)
{
    if ((channel < 0) || (channel >= SSI_CHANNEL_NUM))
    {
        printf("%s: SSI%d is not supported\n", __FUNCTION__, channel);
        return -1;
    }

    *sr_val = ssi_regs[channel].sr;

    return 0;
}

static int ssi_ws_register_set(int channel, uint32_t wsr)
{
    if ((channel < 0) || (channel >= SSI_CHANNEL_NUM))
    {
        printf("%s : SSI%d is not supported\n", __FUNCTION__, channel);
        return -1;
    }

    ssi_regs[channel].wsr = wsr;

    return 0;
}

static int ssi_control_register_setup(uint32_t *cr_val, uint32_t *cr2_val, int data_bits_num, int ws_bits_num, int channel_size, int data_align, int bus_format)
{
    uint32_t tmp1 = 0, tmp2 = 0;

    switch (data_bits_num)
    {
        case 8:
            tmp1 |= SSICR_DWL_8;
            break;
        case 16:
            tmp1 |= SSICR_DWL_16;
            break;
        case 18:
            tmp1 |= SSICR_DWL_18;
            break;
        case 20:
            tmp1 |= SSICR_DWL_20;
            break;
        case 22:
            tmp1 |= SSICR_DWL_22;
            break;
        case 24:
            tmp1 |= SSICR_DWL_24;
            break;
        case 32:
            tmp1 |= SSICR_DWL_32;
            break;
        default:
            tmp1 |= SSICR_DWL_16;
            break;
    }
    if(bus_format == SSI_BUS_FORMAT_I2S)
    {
        switch (channel_size)
        {
            case 1:
                tmp1 |= SSICR_CHNL_1;
                break;
            case 2:
                tmp1 |= SSICR_CHNL_2;
                break;
            case 3:
                tmp1 |= SSICR_CHNL_3;
                break;
            case 4:
                tmp1 |= SSICR_CHNL_4;
                break;
            default:
                tmp1 |= SSICR_CHNL_1;
                break;
        }
    } //TDM mode
    else
    {
        switch (channel_size)
        {
            case 4:
                tmp1 |= SSICR_CHNL_2;
                break;
            case 6:
                tmp1 |= SSICR_CHNL_3;
                break;
            case 8:
                tmp1 |= SSICR_CHNL_4;
                break;
            case 16:
                tmp2 |= SSICR_CHNL_16;
                break;
            default:
                tmp1 |= SSICR_CHNL_2;
                break;
        }
    }

    if (ws_bits_num == 8)
        tmp1 |= SSICR_SWL_8;
    else if (ws_bits_num <= 16)
        tmp1 |= SSICR_SWL_16;
    else if (ws_bits_num <= 24)
        tmp1 |= SSICR_SWL_24;
    else if (ws_bits_num <= 32)
        tmp1 |= SSICR_SWL_32;
    else if (ws_bits_num <= 48)
        tmp1 |= SSICR_SWL_48;
    else if (ws_bits_num <= 64)
        tmp1 |= SSICR_SWL_64;
    else if (ws_bits_num <= 128)
        tmp1 |= SSICR_SWL_128;
    else if (ws_bits_num <= 256)
        tmp1 |= SSICR_SWL_256;
    else
        tmp1 |= SSICR_SWL_16;

    if(data_align == SSI_DATA_ALIGN_RIGHT)
    {
        tmp1 |= SSICR_SDTA;
    }

    *cr_val  |= tmp1;
    *cr2_val |= tmp2;

    return 0;
}

int      rx_ssi_channel = 4;
int      tx_ssi_channel = 3;
uint32_t hdmi_backup_val;
int      hdmi_chan = 0;

int ssi_setup()
{
    int data_bits_num = 16;
    int ws_bits_num = 32;
    int channel_size = 1;
    int data_align = SSI_DATA_ALIGN_RIGHT;
    int bus_format = SSI_BUS_FORMAT_I2S;
    uint32_t cr, cr2 = 0, ws = 0, tmp = 0;

    /* Setup master transmitter */
    ssi_control_register_setup(&tmp, &cr2, data_bits_num, ws_bits_num, channel_size, data_align, bus_format);
    cr  = (SSI_TX_MASTER & ~(SSICR_SWL_MSK | SSICR_DWL_MSK | SSICR_CHNL_MSK | SSICR_SCKD | SSICR_SWSD)) | tmp;

    if(bus_format == SSI_BUS_FORMAT_TDM)
    {
        ws |= SSIWSR_WS_MODE;
    }
    cr |= SSICR_SCKD | SSICR_SWSD; // master mode
    ws |= SSIWSR_CONT;
    ssi_control_register_set(tx_ssi_channel, cr);
    ssi_control2_register_set(tx_ssi_channel, cr2);
    ssi_ws_register_set(tx_ssi_channel, ws);

    /* Set slave receiver */
    tmp = 0;
    ws = 0;
    ssi_control_register_setup(&tmp, &cr2, data_bits_num, ws_bits_num, channel_size, data_align, bus_format);
    cr = (SSI_RX_SLAVE & ~(SSICR_SWL_MSK | SSICR_DWL_MSK | SSICR_CHNL_MSK | SSICR_SCKD | SSICR_SWSD)) | tmp;

    if(bus_format == SSI_BUS_FORMAT_TDM) {
        ws |= SSIWSR_WS_MODE;
    }

    ssi_control_register_set(rx_ssi_channel, cr);
    ssi_control2_register_set(rx_ssi_channel, cr2);
    ssi_ws_register_set(rx_ssi_channel, ws);

    /* set SSIU */
    ssiu_regs[RCAR_SSIU(tx_ssi_channel, 0)].busif_adinr =((24 - data_bits_num) << 16) //output: 24 data bits
                                                        | (2 << 0); //2 channels
    ssiu_regs[RCAR_SSIU(tx_ssi_channel, 0)].mode = 0;
    ssiu_ext_regs[tx_ssi_channel].mode2 = 0;

    ssiu_regs[RCAR_SSIU(rx_ssi_channel, 0)].busif_adinr =((24 - data_bits_num) << 16) //output: 24 data bits
                                                        | (2 << 0); //2 channels
    ssiu_regs[RCAR_SSIU(rx_ssi_channel, 0)].mode = 0;
    ssiu_ext_regs[rx_ssi_channel].mode2 = 0;

    ssiu_com_regs->mode0 &= ~((1 << tx_ssi_channel) | (1 << rx_ssi_channel)); //SSI independent not performed
    ssiu_com_regs->mode1 &= ~(3 << 16); //SSI3 and SSI4 use their own pins independently

    tmp = (tx_ssi_channel << 0)     // SSI_SCK and SSI_WS to HDMI
         |(tx_ssi_channel << 16)    // SSI_SDATA to HDMI
         |(0xFFF << 20);        // Fix at 0

    if(hdmi_chan == 0)
    {
        hdmi_backup_val = ssiu_com_regs->hdmi0_sel;
        ssiu_com_regs->hdmi0_sel = tmp;
    }
    else
    {
        hdmi_backup_val = ssiu_com_regs->hdmi1_sel;
        ssiu_com_regs->hdmi1_sel = tmp;
    }

    return 0;
}

int ssi_clean()
{
    ssi_control_register_set(tx_ssi_channel, 0);
    ssi_control_register_set(rx_ssi_channel, 0);

    return 0;
}

int ssi_start()
{
    uint32_t cr = 0;

    ssiu_regs[RCAR_SSIU(tx_ssi_channel, 0)].control |= 1 << 0;
    ssiu_regs[RCAR_SSIU(rx_ssi_channel, 0)].control |= 1 << 0;

    ssi_control_register_get(tx_ssi_channel, &cr);
    cr |= SSICR_EN | SSICR_DMEN | SSICR_UIEN | SSICR_OIEN;
    ssi_control_register_set(tx_ssi_channel, cr);

    ssi_control_register_get(rx_ssi_channel, &cr);
    cr |= SSICR_EN | SSICR_DMEN | SSICR_UIEN | SSICR_OIEN;
    ssi_control_register_set(rx_ssi_channel, cr);

    return 0;
}

int ssi_idle_wait(int channel, uint32_t bit)
{
    uint32_t sr = 0;
    int      i, timeout = 1000;

    for (i = 0; i < timeout; i++)
    {
        ssi_status_register_get(channel, &sr);
        if (sr & bit)
        {
            return (0);
        }
        nanospin_ns(100);
    }

    return (-1);
}

int ssi_stop()
{
    uint32_t cr = 0;

    ssi_control_register_get(rx_ssi_channel, &cr);
    cr &= ~SSICR_DMEN;
    ssi_control_register_set(rx_ssi_channel, cr);

    ssi_idle_wait(rx_ssi_channel, SSISR_DIRQ);

    cr &=~SSICR_EN;
    cr |= SSICR_IIEN;
    ssi_control_register_set(rx_ssi_channel, cr);

    ssi_control_register_get(tx_ssi_channel, &cr);
    cr &= ~SSICR_DMEN;
    ssi_control_register_set(tx_ssi_channel, cr);

    ssi_idle_wait(tx_ssi_channel, SSISR_DIRQ);

    cr &=~SSICR_EN;
    cr |= SSICR_IIEN;
    ssi_control_register_set(tx_ssi_channel, cr);

    ssiu_regs[RCAR_SSIU(tx_ssi_channel, 0)].control &= ~(1 << 0);
    ssiu_regs[RCAR_SSIU(rx_ssi_channel, 0)].control &= ~(1 << 0);

    return 0;
}

extern int get_audioppdmafuncs(dma_functions_t *functable, int tabsize);

static dma_functions_t  audioppdmafuncs;
static void             *audioppdma_chn;

int audio_dmac_pp_init()
{
    if (get_audioppdmafuncs (&audioppdmafuncs, sizeof (dma_functions_t)) == -1)
    {
        printf("%s : failed to get audio pp DMA lib functions\n", __FUNCTION__);
        return -1;
    }

    audioppdmafuncs.init(NULL);

    audioppdma_chn = audioppdmafuncs.channel_attach(NULL, NULL, NULL, 0, DMA_ATTACH_ANY_CHANNEL);
    if (audioppdma_chn == NULL)
    {
        printf("%s : Unable to attach to audio pp DMA Channel\n", __FUNCTION__);
        return -1;
    }

    return 0;
}

int audio_dmac_pp_setup()
{
    dma_transfer_t  tinfo;
    dma_addr_t      saddr, daddr;
    uint32_t dmac_src_add = 0xEC404000; //SSI4-0
    uint32_t dmac_src_req = 0x0D;
    uint32_t dmac_dst_add = 0xEC403000; //SSI3-0
    uint32_t dmac_dst_req = 0x0C;

    if (!audioppdma_chn)
    {
        return -1;
    }

    memset(&tinfo, 0, sizeof(tinfo));

    /* Setup peripheral-peripheral DMA transfer from SSI3-0 to SSI4-0 */
    saddr.paddr     = dmac_src_add;
    tinfo.src_addrs = &saddr;
    daddr.paddr     = dmac_dst_add;
    tinfo.dst_addrs = &daddr;
    tinfo.req_id    = (dmac_src_req << 24) | (dmac_dst_req << 16);

    audioppdmafuncs.setup_xfer (audioppdma_chn, &tinfo);

    return 0;
}

int audio_dmac_pp_start()
{
    if (!audioppdma_chn)
    {
        return -1;
    }
    /* Start Peripheral-Peripheral DMAC */
    audioppdmafuncs.xfer_start(audioppdma_chn);

    return 0;
}

int audio_dmac_pp_stop()
{
    if (!audioppdma_chn)
    {
        return -1;
    }
    /* Start Peripheral-Peripheral DMAC */
    audioppdmafuncs.xfer_abort(audioppdma_chn);

    return 0;
}

void audio_dmac_pp_deinit()
{
    if (audioppdma_chn)
    {
        audioppdmafuncs.channel_release (audioppdma_chn);
    }
}

void audio_setup(int screen_idx)
{
    if(screen_idx == 1)
        hdmi_chan = 0;
    else if(screen_idx == 2)
        hdmi_chan = 1;
    else {
        hdmi_chan = -1;
        return;
    }
    
    ssi_init();
    audio_dmac_pp_init();
    ssi_setup();
    audio_dmac_pp_setup();
}

void audio_start()
{
    if(hdmi_chan < 0)
        return;
    
    audio_dmac_pp_start();
    ssi_start();
}

void audio_stop()
{
    if(hdmi_chan < 0)
        return;
    
    ssi_stop();
    audio_dmac_pp_stop();
}

void audio_deinit()
{
    if(hdmi_chan < 0)
        return;
    
    if(hdmi_chan == 0) {
        ssiu_com_regs->hdmi0_sel = hdmi_backup_val;
    }
    else {
        ssiu_com_regs->hdmi1_sel = hdmi_backup_val;
    }
    ssi_deinit();
    audio_dmac_pp_deinit();
}

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL:$ $Rev:$")
#endif
