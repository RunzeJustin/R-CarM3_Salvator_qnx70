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

#ifndef _R_Car_SSIU_H
#define _R_Car_SSIU_H

#include <stdint.h>
#include <errno.h>
#include "ssiu_reg.h"

typedef enum
{
    SSI_CHANNEL_0,
    SSI_CHANNEL_1,
    SSI_CHANNEL_2,
    SSI_CHANNEL_3,
    SSI_CHANNEL_4,
    SSI_CHANNEL_5,
    SSI_CHANNEL_6,
    SSI_CHANNEL_7,
    SSI_CHANNEL_8,
    SSI_CHANNEL_9,
    SSI_CHANNEL_NUM
} ssi_channel_t;

typedef enum
{
    SSI_SUB_CHANNEL_0,
    SSI_SUB_CHANNEL_1,
    SSI_SUB_CHANNEL_2,
    SSI_SUB_CHANNEL_3,
    SSI_SUB_CHANNEL_NUM
} ssi_sub_channel_t;

typedef enum
{
    SSI_OP_MODE_MONO,
    SSI_OP_MODE_STEREO,
    SSI_OP_MODE_MULTICH,
    SSI_OP_MODE_TDM,
    SSI_OP_MODE_TDMEXT,
    SSI_OP_MODE_TDMSPLIT_4XMONO,
    SSI_OP_MODE_TDMSPLIT_4XSTEREO
} ssi_op_mode_t;

typedef enum
{
    SSI_ALL_SLAVE = 0,      /* all SSIs controlled by this driver instance are configured as slave */
    SSI_MASTER_SLAVE        /* one of the SSIs controlled by this driver instance is configured as master */
} ssi_masterslave_mode_t;

typedef enum
{
    SSI_INDEPENDENT_START = 0, /* independent start via SSICR.EN */
    SSI_SYNC_SSI0129_START,    /* synchronized start of SSI0,1,2,9 via SSI_CONTROL.SSI0129 */
    SSI_SYNC_SSI34_START       /* synchronized start of SSI3,4 via SSI_CONTROL.SSI34 */
} ssi_start_mode_t;

typedef enum
{
    SSI_INDEPENDENT_TRANSFER,  /* independent transfer not involving the SSIU BUSIF */
    SSI_BUSIF_TRANSFER         /* transfer involving the SSIU BUSIF */
} ssi_transfer_mode_t;

typedef enum
{
    SSI_INDEPENDENT_PINS, // SCK and WS pins are independent
    SSI_SHARED_PINS       // SCK and WS pins are shared across SSIs
} ssi_pin_mode_t;

/* Used to configure the details of a particular protocol used for streaming audio */
typedef struct
{
    ssi_bit_clk_pol_t clk_pol;
    ssi_ws_pol_t ws_pol;
    ssi_bit_delay_t bit_delay;
    ssi_sys_word_len_t sys_word_length;   // number of bits in a system word
    ssi_data_word_len_t data_word_length;  // number of bits in a data word
    ssi_padding_pol_t padding_pol;  // serial padding polarity
    ssi_serial_data_align_t serial_data_alignment; // order of data and padding bits
} ssi_config_t;

typedef struct
{
    uint32_t dma;
    uint32_t underflow;
    uint32_t overflow;
    uint32_t idle_mode;
    uint32_t data;
} ssi_interrupt_t;

int ssiu_init(void);
int ssiu_deinit(void);

/* one SSI channel set-up for independent transfer:
   ssi_channel - SSI channel to use for the transfer
   master_ssi_channel - if set to SSI_CHANNEL_NUM, use slave mode
                      - if set same as ssi_channel, use master mode without pin sharing
                      - else configure pin sharing between ssi_channel and master_ssi_channel
                        with master_ssi_channel configured as master
   is_transmit        - indicates whether to configure the SSI channel as transmit(1) or receive (0)
   op_mode            - operation mode to use
   transfer_mode      - this method is intended for configuring independent transfer for the SSI
                        channel, however if the SCU is used for this channel, transfer_mode has
                        to be set for BUSIF transfer
   bitnum             - num bits per sample
   voicenum           - num voices
   voicenum_ext       - num voices on the external serial sound interface (SSI) bus, different
                        from voicenum only when extended TDM mode is used
   config             - audio format config */
int ssiu_1channel_setup
(
    uint32_t ssi_channel,
    uint32_t master_ssi_channel,
    uint32_t is_transmit,
    ssi_op_mode_t op_mode,
    ssi_transfer_mode_t transfer_mode,
    uint32_t bitnum,
    uint32_t voicenum,
    uint32_t voicenum_ext,
    ssi_config_t* config
);

/* one SSI channel independent transfer clean-up */
int ssiu_1channel_cleanup
(
    uint32_t ssi_channel,
    uint32_t master_ssi_channel
);

/* two SSI channels set-up for duplex transfer in pin sharing mode:
   ssi_channel_tx     - SSI channel to use for transmit
   master_ssi_channel - SSI channel to use for receive
   op_mode            - operation mode to use
   masterslave_mode   - shows whether one of the two SSI channels is configured as master
   transfer_mode      - needed?
   bitnum             - num bits per sample
   voicenum           - num voices
   voicenum_ext       - num voices on the external serial sound interface (SSI) bus, different
                        from voicenum only when extended TDM mode is used
   config             - audio format config */
int ssiu_2channel_duplex_setup
(
    uint32_t ssi_channel_tx,
    uint32_t ssi_channel_rx,
    ssi_op_mode_t op_mode,
    ssi_masterslave_mode_t masterslave_mode,
    ssi_transfer_mode_t transfer_mode,
    ssi_start_mode_t start_mode,
    uint32_t bitnum,
    uint32_t voicenum,
    uint32_t voicenum_ext,
    ssi_config_t* config
);

/* two SSI channel in pin sharing mode clean-up */
int ssiu_2channel_duplex_cleanup
(
    uint32_t ssi_channel_tx,
    uint32_t ssi_channel_rx
);

/* 3-channel transmit set-up: 3 SSIs grouped together for multi-channel transmit
   master_slave_mode - shows whether one of the two SSI channels is configured as master
   bitnum             - num bits per sample
   config             - audio format config */
int ssiu_3channel_transmit_setup
(
    ssi_masterslave_mode_t master_slave_mode,
    uint32_t bitnum,
    ssi_config_t* config
);

/* 3-channel multi channel transmit clean-up */
int ssiu_3channel_transmit_cleanup(void);

/* 4-channel transmit set-up: 4 SSIs grouped together for multi-channel transmit
   master_slave_mode - shows whether one of the two SSI channels is configured as master
   bitnum             - num bits per sample
   config             - audio format config */
int ssiu_4channel_transmit_setup
(
    ssi_masterslave_mode_t master_slave_mode,
    uint32_t bitnum,
    ssi_config_t* config
);

/* 4-channel multi channel transmit clean-up */
int ssiu_4channel_transmit_cleanup(void);

/* start SSI operation - this includes enabling the DMA and interrupts and setting the
   SSICR.EN bit; in case of SSI3,4 or SSI0,1,2,9 synchronized start, the setting of the
   SSICR.EN bit is not included */
int ssi_start(uint32_t ssi_channel, ssi_start_mode_t start_mode);
/* stop SSI operation */
int ssi_stop(uint32_t ssi_channel);
/* wait for status bit to clear while stopping SSI operation */
int ssi_wait_status_clear(uint32_t ssi_channel, uint32_t bitmask);

/* synchronized start for SSI0,1,2 or SSI0,1,2,9 or SSI3,4 */
int ssiu_start(ssi_start_mode_t start_mode);

/* synchronized stop for SSI0,1,2 or SSI0,1,2,9 or SSI3,4 */
int ssiu_stop(ssi_start_mode_t start_mode);

/* start busif for a given SSI channel and subchannel */
int ssiu_busif_start(uint32_t ssi_channel, uint32_t ssi_sub_channel);

/* stop busif for a given SSI channel and subchannel */
int ssiu_busif_stop(uint32_t ssi_channel, uint32_t ssi_sub_channel);

/* cleanup busif for a given SSI channel, all subchannels */
int ssiu_busif_cleanup( uint32_t ssi_channel );

/* configure SSI clock divisor for specific SSI channel */
int ssi_set_divisor(uint32_t ssi_channel, uint32_t divisor);

/* register dump functions */
void ssiu_common_register_dump(void);
void ssiu_ssi_register_dump(uint32_t ssi_channel);

#endif /* _R_Car_SSIU_H */

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/hardware/deva/ctrl/rcar/ssiu.h $ $Rev: 804482 $")
#endif
