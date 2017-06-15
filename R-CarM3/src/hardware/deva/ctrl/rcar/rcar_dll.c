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

#include <unistd.h>
#include <string.h>

#include "rcar.h"
#include "ssiu.h"
#include "scu.h"
#include "dmac.h"
#include "adg.h"
#include "rcar_support.h"
#include "rcar_rsrc.h"
#include "rcar_mixer.h"

#include "variant.h"
#define CS_MACHINE_LEN (32 + 1)

static inline uint32_t count_set_bits( uint32_t bitmask ) {
    uint32_t count = 0;
    while (bitmask) { bitmask &= (bitmask-1); count++; }
    return count;
}

/* supported_rate_list is a bitmask of RCAR-H2 hardware supported rates */
static const uint32_t supported_rate_list = SND_PCM_RATE_8000 |
                                            SND_PCM_RATE_11025 |
                                            SND_PCM_RATE_16000 |
                                            SND_PCM_RATE_22050 |
                                            SND_PCM_RATE_32000 |
                                            SND_PCM_RATE_44100 |
                                            SND_PCM_RATE_48000;

#define RCAR_NUM_SUPPORTED_RATES (count_set_bits(supported_rate_list))

/* configured_rate_list is a bitmask of configured rates - has to be a subset of supported rates */
static uint32_t configured_rate_list;

static int rcar_set_clock_rate (rcar_context_t * rcar);
static void rcar_register_dump( HW_CONTEXT_T * rcar );

static int32_t rcar_capabilities(HW_CONTEXT_T* rcar, ado_pcm_t *pcm, snd_pcm_channel_info_t* info)
{
    int chn_avail;

    ado_debug( DB_LVL_DRIVER, "rcar : %s", __func__ );

    if( rcar->pcm != pcm ) {
        ado_error( "%s: invalid pcm", __func__ );
        return EINVAL;
    }

    chn_avail = 1;

    info->rates = rcar->playback.pcm_caps.rates;
    info->min_rate = rcar->sample_rate_min;
    info->max_rate = rcar->sample_rate_max;

    if (info->channel == SND_PCM_CHANNEL_PLAYBACK) {
        if ( rcar->playback.subchn ) {
            chn_avail = 0;
        } else if (rcar->sample_rate_min != rcar->sample_rate_max) {

            ado_mutex_lock(&rcar->hw_lock);

            /* Playback and Capture are Rate locked, so adjust rate capabilities
             * if the other side has been aquired.
             */
            if ( rcar->sample_rate ) {
                info->min_rate = info->max_rate = rcar->sample_rate;
                info->rates = ado_pcm_rate2flag(rcar->sample_rate);
            }

            ado_mutex_unlock(&rcar->hw_lock);
        }
    } else if (info->channel == SND_PCM_CHANNEL_CAPTURE) {
        if ( rcar->capture.subchn ) {
            chn_avail = 0;
        } else if (rcar->sample_rate_min != rcar->sample_rate_max) {

            ado_mutex_lock(&rcar->hw_lock);

            /* Playback and Capture are Rate locked, so adjust rate capabilities
             * if the other side has been aquired (in this case rate is non-zero)
             */
            if ( rcar->sample_rate ) {
                info->min_rate = info->max_rate = rcar->sample_rate;
                info->rates = ado_pcm_rate2flag(rcar->sample_rate);
            }

            ado_mutex_unlock(&rcar->hw_lock);
        }
    }

    if (chn_avail == 0) {
        info->formats       = 0;
        info->rates         = 0;
        info->min_rate      = 0;
        info->max_rate      = 0;
        info->min_voices    = 0;
        info->max_voices    = 0;
        info->min_fragment_size = 0;
        info->max_fragment_size = 0;
    }

    return EOK;
}

static int32_t rcar_playback_acquire (HW_CONTEXT_T * rcar, PCM_SUBCHN_CONTEXT_T ** pc,
        ado_pcm_config_t * config, ado_pcm_subchn_t * subchn, uint32_t * why_failed)
{
    int status;

    ado_debug ( DB_LVL_DRIVER, "rcar : %s", __func__ );

    ado_mutex_lock( &rcar->hw_lock );

    if( rcar->playback.subchn ) {
        *why_failed = SND_PCM_PARAMS_NO_CHANNEL;
        ado_mutex_unlock( &rcar->hw_lock );
        ado_error( "%s: no channel available", __func__ );
        return EAGAIN;
    }

    if( rcar->sample_rate_min == rcar->sample_rate_max ) {
        if( config->format.rate != rcar->sample_rate_min ) {
            ado_mutex_unlock (&rcar->hw_lock);
            ado_error( "%s: rate not supported: %d", __func__, config->format.rate );
            return EINVAL;
        }
    } else {
        if( rcar->sample_rate && config->format.rate != rcar->sample_rate ) {
            ado_mutex_unlock (&rcar->hw_lock);
            ado_error( "%s: rate is locked by capture session: locked rate: %d, requested rate: %d",
                       __func__, rcar->sample_rate, config->format.rate );
            return EBUSY;
        }
    }

    rcar->sample_rate = config->format.rate;

    status = rcar_set_clock_rate( rcar );
    if( status != EOK ) {
        ado_mutex_unlock (&rcar->hw_lock);
        ado_error( "%s: failed setting the clock rate", __func__ );
        return status;
    }

    /* Allocate DMA transfer buffer*/

    // added ADO_BUF_CACHE flag to flags used in old driver
    if( ado_pcm_buf_alloc( config, config->dmabuf.size,
                           ADO_SHM_DMA_SAFE | ADO_BUF_CACHE ) == NULL ) {
        ado_mutex_unlock (&rcar->hw_lock);
        ado_error( "%s: failed allocating shared memory", __func__ );
        return ENOMEM;
    }

    ado_debug( DB_LVL_DRIVER, "%s: dmabuf.size = %X ", __func__, config->dmabuf.size );

    if ( !rcar->use_scu ) {
        audio_peripheral_t dest;
        if( rcar->ssi_transfer_mode == SSI_BUSIF_TRANSFER ) {
            dest = AUDIO_PERIPHERAL_SSI_BUSIF(rcar->playback.ssi_chan,0);
        } else {
            dest = AUDIO_PERIPHERAL_SSI(rcar->playback.ssi_chan);
        }
        /* Not to use SCU: DMA transfer from Memory to SSI */
        status = audio_dmac_mp_setup( rcar->playback.dma_context.audiodma_chn,
                                      dest,
                                      config->dmabuf.phys_addr,
                                      config->dmabuf.size );
        if( status != EOK ) {
            ado_error( "%s: failed setting up dmac mp to peripheral %x", __func__, dest );
        }
    } else {
        /* To use SCU: DMA transfer from Memory to SRC */
        status = audio_dmac_mp_setup( rcar->playback.dma_context.audiodma_chn,
                                      AUDIO_PERIPHERAL_SCUSRC(rcar->playback.src_chan),
                                      config->dmabuf.phys_addr,
                                      config->dmabuf.size);

        if( status != EOK ) {
            ado_error( "%s: failed setting up dmac mp to peripheral %x", __func__,
                       AUDIO_PERIPHERAL_SCUSRC(rcar->playback.src_chan) );
        } else {
            /* Setup peripheral-peripheral DMA transfer from SCU-CMD0 to SSI0-0 */
            status = audio_dmac_pp_setup( rcar->playback.dma_context.audiodma_pp_chn,
                                           AUDIO_PERIPHERAL_SCUCMD(rcar->playback.cmd_chan),
                                           AUDIO_PERIPHERAL_SSI_BUSIF(rcar->playback.ssi_chan,0) );
            if( status != EOK ) {
                ado_error( "%s: failed setting up dmac pp between peripherals %x and %x", __func__,
                           AUDIO_PERIPHERAL_SCUCMD(rcar->playback.cmd_chan),
                           AUDIO_PERIPHERAL_SSI_BUSIF(rcar->playback.ssi_chan,0) );
            }
        }
    }

    if( status == EOK ) {
        rcar->playback.subchn = subchn;
        *pc = &rcar->playback;
    } else {
        ado_shm_free (config->dmabuf.addr, config->dmabuf.size, config->dmabuf.name);
        config->dmabuf.addr = NULL;
        rcar->sample_rate = 0;
    }

    ado_mutex_unlock (&rcar->hw_lock);

    return EOK;
}

/* */
/*  Playback release*/
/* */
static int32_t rcar_playback_release (HW_CONTEXT_T * rcar, PCM_SUBCHN_CONTEXT_T * pc,
    ado_pcm_config_t * config)
{
    ado_debug( DB_LVL_DRIVER, "rcar : %s", __func__ );

    ado_mutex_lock (&rcar->hw_lock);

    rcar->playback.subchn = NULL;
    if( !rcar->capture.subchn ) {
        rcar->sample_rate = 0;
    }

    if( config->dmabuf.addr ) {
        (void)ado_shm_free (config->dmabuf.addr, config->dmabuf.size, config->dmabuf.name);
        config->dmabuf.addr = NULL;
    }

    ado_mutex_unlock (&rcar->hw_lock);

    return (EOK);
}

static int32_t rcar_capture_acquire (HW_CONTEXT_T * rcar, PCM_SUBCHN_CONTEXT_T ** pc,
    ado_pcm_config_t * config, ado_pcm_subchn_t * subchn, uint32_t * why_failed)
{
    int status;

    ado_debug( DB_LVL_DRIVER, "rcar : %s", __func__ );

    ado_mutex_lock (&rcar->hw_lock);

    if( rcar->capture.subchn ) {
        *why_failed = SND_PCM_PARAMS_NO_CHANNEL;
        ado_mutex_unlock( &rcar->hw_lock );
        ado_error( "%s: no channel available", __func__ );
        return EAGAIN;
    }

    if( rcar->sample_rate_min == rcar->sample_rate_max ) {
        if( config->format.rate != rcar->sample_rate_min ) {
            ado_mutex_unlock (&rcar->hw_lock);
            ado_error( "%s: rate not supported: %d", __func__, config->format.rate );
            return EINVAL;
        }
    } else {
        if( rcar->sample_rate && config->format.rate != rcar->sample_rate ) {
            ado_mutex_unlock (&rcar->hw_lock);
            ado_error( "%s: rate is locked by playback session: locked rate: %d, requested rate: %d",
                       __func__, rcar->sample_rate, config->format.rate );
            return EBUSY;
        }
    }

    rcar->sample_rate = config->format.rate;

    status = rcar_set_clock_rate( rcar );
    if( status != EOK ) {
        ado_mutex_unlock (&rcar->hw_lock);
        ado_error( "%s: failed setting the clock rate", __func__ );
        return status;
    }

    /* Allocate DMA transfer buffer*/

    // added ADO_BUF_CACHE flag to flags used in old driver
    if( ado_pcm_buf_alloc( config, config->dmabuf.size,
                           ADO_SHM_DMA_SAFE | ADO_BUF_CACHE ) == NULL ) {
        ado_mutex_unlock (&rcar->hw_lock);
        ado_error( "%s: failed allocating shared memory", __func__ );
        return ENOMEM;
    }

    ado_debug( DB_LVL_DRIVER, "%s: dmabuf.size = %X ", __func__, config->dmabuf.size );

    if ( !rcar->use_scu ) {
        audio_peripheral_t src;
        if( rcar->ssi_transfer_mode == SSI_BUSIF_TRANSFER ) {
            src = AUDIO_PERIPHERAL_SSI_BUSIF(rcar->capture.ssi_chan,0);
        } else {
            src = AUDIO_PERIPHERAL_SSI(rcar->capture.ssi_chan);
        }
        /* Not to use SCU: DMA transfer from SSI to Memory */
        status = audio_dmac_pm_setup( rcar->capture.dma_context.audiodma_chn,
                                      src,
                                      config->dmabuf.phys_addr,
                                      config->dmabuf.size );
        if( status != EOK ) {
            ado_error( "%s: failed setting up dmac pm from peripheral %x", __func__, src );
        }
    } else {
        status = audio_dmac_pm_setup( rcar->capture.dma_context.audiodma_chn,
                                      AUDIO_PERIPHERAL_SCUSRC(rcar->capture.src_chan),
                                      config->dmabuf.phys_addr,
                                      config->dmabuf.size );
        if( status != EOK ) {
            ado_error( "%s: failed setting up dmac pm from peripheral %x", __func__,
                       AUDIO_PERIPHERAL_SCUSRC(rcar->capture.src_chan) );
        } else {
            /* Setup peripheral-peripheral DMA transfer from SSIx-0 to SCU-SRCy */
            status = audio_dmac_pp_setup( rcar->capture.dma_context.audiodma_pp_chn,
                                          AUDIO_PERIPHERAL_SSI_BUSIF(rcar->capture.ssi_chan,0),
                                          AUDIO_PERIPHERAL_SCUSRC(rcar->capture.src_chan) );
            if( status != EOK ) {
                ado_error( "%s: failed setting up dmac pp between peripherals %x and %x", __func__,
                           AUDIO_PERIPHERAL_SSI_BUSIF(rcar->capture.ssi_chan,0),
                           AUDIO_PERIPHERAL_SCUSRC(rcar->capture.src_chan) );
            }
        }
    }

    if( status == EOK ) {
        rcar->capture.subchn = subchn;
        *pc = &rcar->capture;
    } else {
        ado_shm_free( config->dmabuf.addr, config->dmabuf.size, config->dmabuf.name );
        config->dmabuf.addr = NULL;
        rcar->sample_rate = 0;
    }

    ado_mutex_unlock (&rcar->hw_lock);

    return EOK;
}


/* */
/*  Recording release */
/* */
static int32_t rcar_capture_release (HW_CONTEXT_T * rcar, PCM_SUBCHN_CONTEXT_T * pc,
    ado_pcm_config_t * config)
{
    ado_debug (DB_LVL_DRIVER, "rcar : %s", __func__);

    ado_mutex_lock (&rcar->hw_lock);

    rcar->capture.subchn = NULL;
    if( !rcar->playback.subchn ) {
        rcar->sample_rate = 0;
    }

    /* Free DMA transfer buffer*/
    if( config->dmabuf.addr ) {
        ado_shm_free (config->dmabuf.addr, config->dmabuf.size, config->dmabuf.name);
        config->dmabuf.addr = NULL;
    }

    ado_mutex_unlock (&rcar->hw_lock);

    return (EOK);
}

static int32_t rcar_playback_prepare (HW_CONTEXT_T * rcar, PCM_SUBCHN_CONTEXT_T * pc, ado_pcm_config_t * config)
{
    int status;

    ado_debug (DB_LVL_DRIVER, "rcar : %s", __func__);

    /* TBD: why is memory to peripheral DMA set-up repeated here (already done in acquire)? 
	        Answer: If not to re-set-up here, pause/re-start does not work */
    if ( !rcar->use_scu ) {
        audio_peripheral_t dest;
        if( rcar->ssi_transfer_mode == SSI_BUSIF_TRANSFER ) {
            dest = AUDIO_PERIPHERAL_SSI_BUSIF(rcar->playback.ssi_chan,0);
        } else {
            dest = AUDIO_PERIPHERAL_SSI(rcar->playback.ssi_chan);
        }
        /* Not to use SCU: DMA transfer from Memory to SSI */
        status = audio_dmac_mp_setup( rcar->playback.dma_context.audiodma_chn,
                                      dest,
                                      config->dmabuf.phys_addr,
                                      config->dmabuf.size );
        if( status != EOK ) {
            ado_error( "%s: failed setting up dmac mp to peripheral %x", __func__, dest );
        }
    } else {
        /* To use SCU: DMA transfer from Memory to SRC */
        status = audio_dmac_mp_setup( rcar->playback.dma_context.audiodma_chn,
                                      AUDIO_PERIPHERAL_SCUSRC(rcar->playback.src_chan),
                                      config->dmabuf.phys_addr,
                                      config->dmabuf.size );
        if( status != EOK ) {
            ado_error( "%s: failed setting up dmac mp to peripheral %x", __func__,
                       AUDIO_PERIPHERAL_SCUSRC(rcar->playback.src_chan) );
        }
    }

    return status;
}

static int32_t rcar_capture_prepare (HW_CONTEXT_T * rcar, PCM_SUBCHN_CONTEXT_T * pc, ado_pcm_config_t * config)
{
    int status;

    ado_debug (DB_LVL_DRIVER, "rcar : %s", __func__);

    if ( !rcar->use_scu ) {
       audio_peripheral_t src;
        if( rcar->ssi_transfer_mode == SSI_BUSIF_TRANSFER ) {
            src = AUDIO_PERIPHERAL_SSI_BUSIF(rcar->capture.ssi_chan,0);
        } else {
            src = AUDIO_PERIPHERAL_SSI(rcar->capture.ssi_chan);
        }
        /* Not to use SCU: DMA transfer from SSI to Memory */
        status = audio_dmac_pm_setup( rcar->capture.dma_context.audiodma_chn,
                                      src,
                                      config->dmabuf.phys_addr,
                                      config->dmabuf.size );
        if( status != EOK ) {
            ado_error( "%s: failed setting up dmac pm from peripheral %x", __func__, src );
        }
    } else {
        /* To use SCU: DMA transfer from SRC to memory */
        status = audio_dmac_pm_setup( rcar->capture.dma_context.audiodma_chn,
                                      AUDIO_PERIPHERAL_SCUSRC(rcar->capture.src_chan),
                                      config->dmabuf.phys_addr,
                                      config->dmabuf.size );
        if( status != EOK ) {
            ado_error( "%s: failed setting up dmac pm from peripheral %x", __func__,
                       AUDIO_PERIPHERAL_SCUSRC(rcar->capture.src_chan) );
        }
    }

    return status;
}

static int32_t rcar_playback_trigger (HW_CONTEXT_T * rcar, PCM_SUBCHN_CONTEXT_T * pc, uint32_t cmd)
{
    ado_debug (DB_LVL_DRIVER, "rcar : %s", __func__);

    if( pc->subchn != rcar->playback.subchn ) {
        ado_debug( DB_LVL_DRIVER, "%s: unknown subchn", __func__ );
        return EINVAL;
    }

    if( cmd == ADO_PCM_TRIGGER_GO ) {
        ado_debug( DB_LVL_DRIVER, "%s: ADO_PCM_TRIGGER_GO", __func__ );

        /* Start Audio-DMAC */
        ado_debug( DB_LVL_DRIVER, "%s: Start Audio DMAC", __func__ );
        audio_dmac_start( rcar->playback.dma_context.audiodma_chn );

        if( rcar->use_scu ) {
            /* Start Peripheral-Peripheral DMAC */
            ado_debug( DB_LVL_DRIVER, "%s: Start Audio DMAC PP", __func__ );
            audio_dmac_pp_start( rcar->playback.dma_context.audiodma_pp_chn );
        }

        if( rcar->ssi_start_mode == SSI_SYNC_SSI0129_START ) {
            /* Multichannel SSI */

            /* start the individual SSIs */
            ado_debug( DB_LVL_DRIVER, "%s: Start SSI 0,1,2 (no CR.EN)", __func__ );
            ssi_start( SSI_CHANNEL_0, SSI_SYNC_SSI0129_START );
            ssi_start( SSI_CHANNEL_1, SSI_SYNC_SSI0129_START );
            ssi_start( SSI_CHANNEL_2, SSI_SYNC_SSI0129_START );
            if( rcar->voices == 8 ) {
                ado_debug( DB_LVL_DRIVER, "%s: Start SSI 9 (no CR.EN)", __func__ );
                ssi_start( SSI_CHANNEL_9, SSI_SYNC_SSI0129_START );
            }

            /* start in a synchronized fashion the SSI0129 or SSI012 channels */
            ado_debug( DB_LVL_DRIVER, "%s: Synchronized start of SSI 0,1,2(,9)", __func__ );
            ssiu_start( SSI_SYNC_SSI0129_START );
        } else if( rcar->ssi_start_mode == SSI_SYNC_SSI34_START ) {
            /* SSI 3,4 configured for synchronized start */

            /* start the individual SSIs */
            ado_debug( DB_LVL_DRIVER, "%s: Start SSI 3,4 (no CR.EN)", __func__ );
            ssi_start( SSI_CHANNEL_3, SSI_SYNC_SSI34_START );
            ssi_start( SSI_CHANNEL_4, SSI_SYNC_SSI34_START );

            /* start in a synchronized fashion the SSI34 */
            ado_debug( DB_LVL_DRIVER, "%s: Synchronized start of SSI 3,4", __func__ );
            ssiu_start( SSI_SYNC_SSI34_START );
        } else {
            /* start SSIx as independent SSI */
            ado_debug( DB_LVL_DRIVER, "%s: Start SSI %d (CR.EN)", __func__,
                       rcar->playback.ssi_chan );
            ssi_start( rcar->playback.ssi_chan, SSI_INDEPENDENT_START );
        }

        /* start applicable busif if required */
        if( rcar->ssi_transfer_mode == SSI_BUSIF_TRANSFER ) {
            /* start busif SSIx-0 */
            ado_debug( DB_LVL_DRIVER, "%s: Start BUSIF for SSI %d subchan 0", __func__,
                       rcar->playback.ssi_chan );
            ssiu_busif_start(rcar->playback.ssi_chan, 0);

            /* start busif SSIx-1,2,3 if in TDM split mode */
            if( rcar->ssi_op_mode == SSI_OP_MODE_TDMSPLIT_4XMONO ||
                rcar->ssi_op_mode == SSI_OP_MODE_TDMSPLIT_4XSTEREO ) {
                ado_debug( DB_LVL_DRIVER, "%s: Start BUSIF for SSI %d subchan 1,2,3", __func__,
                           rcar->playback.ssi_chan );
                ssiu_busif_start(rcar->playback.ssi_chan, 1);
                ssiu_busif_start(rcar->playback.ssi_chan, 2);
                ssiu_busif_start(rcar->playback.ssi_chan, 3);
            }
        }

        if( rcar->use_scu ) {
            /* start src */
            ado_debug( DB_LVL_DRIVER, "%s: Start SRC %d", __func__,
                       rcar->playback.src_chan );
            scu_src_start(rcar->playback.src_chan);

            /* start cmd */
            ado_debug( DB_LVL_DRIVER, "%s: Start CMD %d", __func__,
                       rcar->playback.cmd_chan );
            scu_cmd_start(rcar->playback.cmd_chan);
        }

        ado_debug (DB_LVL_DRIVER, "%s: ADO_PCM_TRIGGER_START complete", __func__);

    } else if (cmd == ADO_PCM_TRIGGER_STOP) {
        ado_debug (DB_LVL_DRIVER, "%s: ADO_PCM_TRIGGER_STOP", __func__);

        /* DMA request disable*/
        ado_debug( DB_LVL_DRIVER, "%s: Stop Audio DMAC", __func__ );
        audio_dmac_stop(rcar->playback.dma_context.audiodma_chn);

        if (rcar->use_scu == 1) {
            /* Stop Peripheral-Peripheral DMAC */
            ado_debug( DB_LVL_DRIVER, "%s: Stop Audio DMAC PP", __func__ );
            audio_dmac_pp_stop(rcar->playback.dma_context.audiodma_pp_chn);

            /* stop src0 */
            ado_debug( DB_LVL_DRIVER, "%s: Stop SRC %d", __func__,
                       rcar->playback.src_chan );
            scu_src_stop(rcar->playback.src_chan);

            /* stop cmd0 */
            ado_debug( DB_LVL_DRIVER, "%s: Stop CMD %d", __func__,
                       rcar->playback.cmd_chan );
            scu_cmd_stop(rcar->playback.cmd_chan);
        }

        /* stop applicable busif if required */
        if( rcar->ssi_transfer_mode == SSI_BUSIF_TRANSFER ) {
            /* stop busif SSIx-0 */
            ado_debug( DB_LVL_DRIVER, "%s: Stop BUSIF for SSI %d subchan 0", __func__,
                       rcar->playback.ssi_chan );
            ssiu_busif_stop(rcar->playback.ssi_chan, 0);

            /* stop busif SSIx-1,2,3 if in TDM split mode */
            if( rcar->ssi_op_mode == SSI_OP_MODE_TDMSPLIT_4XMONO ||
                rcar->ssi_op_mode == SSI_OP_MODE_TDMSPLIT_4XSTEREO ) {
                ado_debug( DB_LVL_DRIVER, "%s: Stop BUSIF for SSI %d subchan 1,2,3", __func__,
                           rcar->playback.ssi_chan );
                ssiu_busif_stop(rcar->playback.ssi_chan, 1);
                ssiu_busif_stop(rcar->playback.ssi_chan, 2);
                ssiu_busif_stop(rcar->playback.ssi_chan, 3);
            }
        }
        if( rcar->ssi_start_mode == SSI_SYNC_SSI0129_START ) {
            /* Multichannel SSI */
            ado_debug( DB_LVL_DRIVER, "%s: Synchronized stop of SSI 0,1,2(,9)", __func__ );
            ssiu_stop( SSI_SYNC_SSI0129_START );

            /* stop the individual SSIs */
            ado_debug( DB_LVL_DRIVER, "%s: Stop SSI 0,1,2 (no CR.EN)", __func__ );
            ssi_stop( SSI_CHANNEL_0 );
            ssi_stop( SSI_CHANNEL_1 );
            ssi_stop( SSI_CHANNEL_2 );
            if( rcar->voices == 8 ) {
                ado_debug( DB_LVL_DRIVER, "%s: Stop SSI 9 (no CR.EN)", __func__ );
                ssi_stop( SSI_CHANNEL_9 );
            }
        } else if( rcar->ssi_start_mode == SSI_SYNC_SSI34_START ) {
            /* SSI 3,4 configured for synchronized start */
            ado_debug( DB_LVL_DRIVER, "%s: Synchronized stop of SSI 3,4", __func__ );
            ssiu_stop( SSI_SYNC_SSI34_START );

            /* stop the individual SSIs */
            ado_debug( DB_LVL_DRIVER, "%s: Stop SSI 3,4 (no CR.EN)", __func__ );
            ssi_stop( SSI_CHANNEL_3 );
            ssi_stop( SSI_CHANNEL_4 );
        } else {
            /* stop SSIx */
            ado_debug( DB_LVL_DRIVER, "%s: Stop SSI %d (CR.EN)", __func__,
                       rcar->playback.ssi_chan );
            ssi_stop( rcar->playback.ssi_chan );
        }

        /* Wait for idle mode*/
        delay(1);
        ado_debug( DB_LVL_DRIVER, "%s: Waiting for IDST clear on SSI %d", __func__,
                   rcar->playback.ssi_chan );
        ssi_wait_status_clear(rcar->playback.ssi_chan, SSISR_IDST_MASK);

        ado_debug (DB_LVL_DRIVER, "%s: ADO_PCM_TRIGGER_STOP complete", __func__);
    }

    if( rcar->debug ) {
        rcar_register_dump( rcar );
    }

    return EOK;
}

static int32_t rcar_capture_trigger (HW_CONTEXT_T * rcar, PCM_SUBCHN_CONTEXT_T * pc, uint32_t cmd)
{
    ado_debug (DB_LVL_DRIVER, "rcar : %s", __func__);

    if( pc->subchn != rcar->capture.subchn ) {
        ado_debug (DB_LVL_DRIVER, "%s: unknown subchn", __func__);
        return EINVAL;
    }

    if (cmd == ADO_PCM_TRIGGER_GO) {
        ado_debug (DB_LVL_DRIVER, "%s: ADO_PCM_TRIGGER_GO", __func__);

        /* Start Audio-DMAC */
        ado_debug( DB_LVL_DRIVER, "%s: Start Audio DMAC", __func__ );
        audio_dmac_start(rcar->capture.dma_context.audiodma_chn);

        if ( rcar->use_scu ) {
            /* Start Peripheral-Peripheral DMAC */
            ado_debug( DB_LVL_DRIVER, "%s: Start Audio DMAC PP", __func__ );
            audio_dmac_pp_start(rcar->capture.dma_context.audiodma_pp_chn);

            /* start SRC */
            ado_debug( DB_LVL_DRIVER, "%s: Start SRC %d", __func__, rcar->capture.src_chan );
            scu_src_start(rcar->capture.src_chan);
        }

        /* start applicable busif if required */
        if( rcar->ssi_transfer_mode == SSI_BUSIF_TRANSFER &&
            rcar->ssi_op_mode != SSI_OP_MODE_TDMSPLIT_4XMONO &&
            rcar->ssi_op_mode != SSI_OP_MODE_TDMSPLIT_4XSTEREO ) {
            /* start busif SSIx-0 */
            ado_debug( DB_LVL_DRIVER, "%s: Start BUSIF for SSI %d subchan 0", __func__,
                       rcar->capture.ssi_chan );
            ssiu_busif_start(rcar->capture.ssi_chan, 0);
        }

        if( rcar->ssi_start_mode == SSI_SYNC_SSI34_START ) {
            /* SSI 3,4 configured for synchronized start */

            /* start the individual SSIs */
            ado_debug( DB_LVL_DRIVER, "%s: Start SSI 3,4 (no CR.EN)", __func__ );
            ssi_start( SSI_CHANNEL_3, SSI_SYNC_SSI34_START );
            ssi_start( SSI_CHANNEL_4, SSI_SYNC_SSI34_START );

            /* start in a synchronized fashion the SSI34 */
            ado_debug( DB_LVL_DRIVER, "%s: Synchronized start of SSI 3,4", __func__ );
            ssiu_start( SSI_SYNC_SSI34_START );
        } else {
            /* start SSIx as independent SSI */
            ado_debug( DB_LVL_DRIVER, "%s: Start SSI %d (CR.EN)", __func__, rcar->capture.ssi_chan );
            ssi_start( rcar->capture.ssi_chan, SSI_INDEPENDENT_START );
        }

        /* start busif SSIx-1,2,3 if in TDM split mode, after starting the SSI channel */
        if( rcar->ssi_op_mode == SSI_OP_MODE_TDMSPLIT_4XMONO ||
            rcar->ssi_op_mode == SSI_OP_MODE_TDMSPLIT_4XSTEREO ) {
            ado_debug( DB_LVL_DRIVER, "%s: Start BUSIF for SSI %d subchan 0,1,2,3", __func__,
                       rcar->capture.ssi_chan );
            ssiu_busif_start(rcar->capture.ssi_chan, 0);
            ssiu_busif_start(rcar->capture.ssi_chan, 1);
            ssiu_busif_start(rcar->capture.ssi_chan, 2);
            ssiu_busif_start(rcar->capture.ssi_chan, 3);
        }
    } else if (cmd == ADO_PCM_TRIGGER_STOP) {
        ado_debug (DB_LVL_DRIVER, "%s: ADO_PCM_TRIGGER_STOP", __func__);

        /* DMA request disable */
        ado_debug( DB_LVL_DRIVER, "%s: Stop Audio DMAC", __func__ );
        audio_dmac_stop(rcar->capture.dma_context.audiodma_chn);

        if (rcar->use_scu) {
            /* stop pp dmac */
            ado_debug( DB_LVL_DRIVER, "%s: Stop Audio DMAC PP", __func__ );
            audio_dmac_pp_stop(rcar->capture.dma_context.audiodma_pp_chn);

            /* stop src */
            ado_debug( DB_LVL_DRIVER, "%s: Stop SRC %d", __func__, rcar->capture.src_chan );
            scu_src_stop(rcar->capture.src_chan);
        }

        /* stop applicable busif if required */
        if( rcar->ssi_transfer_mode == SSI_BUSIF_TRANSFER ) {
            /* stop busif SSIx-0 */
            ado_debug( DB_LVL_DRIVER, "%s: Stop BUSIF for SSI %d subchan 0", __func__,
                       rcar->capture.ssi_chan );
            ssiu_busif_stop(rcar->capture.ssi_chan, 0);

            /* stop busif SSIx-1,2,3 if in TDM split mode */
            if( rcar->ssi_op_mode == SSI_OP_MODE_TDMSPLIT_4XMONO ||
                rcar->ssi_op_mode == SSI_OP_MODE_TDMSPLIT_4XSTEREO ) {
                ado_debug( DB_LVL_DRIVER, "%s: Stop BUSIF for SSI %d subchan 1,2,3", __func__,
                           rcar->capture.ssi_chan );
                ssiu_busif_stop(rcar->playback.ssi_chan, 1);
                ssiu_busif_stop(rcar->playback.ssi_chan, 2);
                ssiu_busif_stop(rcar->playback.ssi_chan, 3);
            }
        }

        if( rcar->ssi_start_mode == SSI_SYNC_SSI34_START ) {
            /* SSI 3,4 configured for synchronized start */

            /* stop SSI34 */
            ado_debug( DB_LVL_DRIVER, "%s: Synchronized stop of SSI 3,4", __func__ );
            ssiu_stop( SSI_SYNC_SSI34_START );

            /* stop the individual SSIs */
            ado_debug( DB_LVL_DRIVER, "%s: Stop SSI 3,4 (no CR.EN)", __func__ );
            ssi_stop( SSI_CHANNEL_3 );
            ssi_stop( SSI_CHANNEL_4 );
        } else {
            /* stop SSIx */
            ado_debug( DB_LVL_DRIVER, "%s: Stop SSI %d (CR.EN)", __func__, rcar->capture.ssi_chan );
            ssi_stop(rcar->capture.ssi_chan);
        }

        /* Wait for idle mode*/
        delay(1);
        ado_debug( DB_LVL_DRIVER, "%s: Waiting for DIRQ clear on SSI %d", __func__,
                   rcar->capture.ssi_chan );
        ssi_wait_status_clear(rcar->capture.ssi_chan, SSISR_DIRQ_MASK);
    }

    if( rcar->debug ) {
        rcar_register_dump( rcar );
    }

    return EOK;
}

static uint32_t rcar_position
(
    HW_CONTEXT_T * rcar,
    PCM_SUBCHN_CONTEXT_T * pc,
    ado_pcm_config_t * config,
    uint32_t * hw_buffer_level
)
{
    uint32_t pos = 0;

    if( hw_buffer_level ) {
        *hw_buffer_level = 0;
    }

    ado_mutex_lock (&rcar->hw_lock);

    audio_dmac_count_register_get(pc->dma_context.audiodma_chn, &pos);

    ado_mutex_unlock (&rcar->hw_lock);

    ado_debug (DB_LVL_DRIVER, "%s: position=%x", __func__, pos);

    return (config->dmabuf.size - pos);
}

static void rcar_playback_interrupt (HW_CONTEXT_T * rcar, int32_t irq)
{
    ado_debug (DB_LVL_INTERRUPT, "%s: irq=%d", __func__, irq);

    /* Clear Interrupt status */
    audio_dmac_cleanup(rcar->playback.dma_context.audiodma_chn);

    /* Signal to io-audio (DMA transfer was completed) */
    dma_interrupt(rcar->playback.subchn);
}

static void rcar_capture_interrupt (HW_CONTEXT_T * rcar, int32_t irq)
{
    ado_debug (DB_LVL_INTERRUPT, "%s: irq=%d", __func__, irq);

    /* Clear Interrupt status */
    audio_dmac_cleanup(rcar->capture.dma_context.audiodma_chn);

    /* Signal to io-audio (DMA transfer was completed) */
    dma_interrupt(rcar->capture.subchn);
}

static void rcar_parse_version(char * str)
{
    if( strstr( str, "H2") || strstr( str, "h2") ){
        rcar_version_set( RCAR_VERSION_H2 );
    } else if ( strstr(str, "M2" ) || strstr( str, "m2") ){
        rcar_version_set( RCAR_VERSION_M2 );
    } else if ( strstr(str, "E2" ) || strstr( str, "e2") ){
        rcar_version_set( RCAR_VERSION_E2 );
    } else if ( strstr(str, "V2" ) || strstr( str, "v2") ){
        rcar_version_set( RCAR_VERSION_V2 );
    } else if ( strstr(str, "H3" ) || strstr( str, "h3") ){
        rcar_version_set( RCAR_VERSION_H3 );
	} else if ( strstr(str, "M3" ) || strstr( str, "m3") ){
        rcar_version_set( RCAR_VERSION_M3 );
    } else {
        ado_debug( DB_LVL_DRIVER, "%s: version not set", __func__ );
    }
}

static int rcar_parse_commandline (rcar_context_t * rcar, char *args)
{
    int      ret = EOK;
    int      opt = 0;
    char     *value;
    int      numvalue = 0;
    uint32_t use_tx = 0;
    uint32_t use_rx = 0;
    uint32_t min_idx, max_idx;
    char     cs_machine_str[CS_MACHINE_LEN];
    char     *opts[] = {
        "tx_ssi",           // 0 - e.g. tx_ssi=0, tx_ssi=0129, enumerates the SSI indexes used for transmit
        "rx_ssi",           // 1 - e.g. rx_ssi=1, specifies the SSI index used for receive
        "voices",           // 2 - e.g. voices=8, voices=2, specifies number of voices used
        "master",           // 3 - no params, refers to one of the used SSIs indicated by tx_ssi and rx_ssi being
                            //     configured as master
        "sync_start",       // 4 - no params, refers to synchronized start of SSI3,4
        "tdm_ext",          // 5 - no params, refers to the use of extended TDM mode
        "scu",              // 6 - no params, refers to the use of the SCU module
        "mlp",              // 7 - no params, refers to the use of the MLP port
        "dtcp",             // 8 - no params, refers to the use of DTCP
        "sample_size",      // 9
        "clk_pol",          // 10
        "fsync_pol",        // 11
        "bit_delay",        // 12
        "sample_rate",      // 13
        "sample_rate_list", // 14
        "slot_size",        // 15
        "ver",              // 16
        "debug",            // 17
        NULL
    };

    ado_debug (DB_LVL_DRIVER, "rcar : %s", __func__);

    rcar->ssi_masterslave_mode             = DEFAULT_SSI_MASTERSLAVE_MODE;  /* see variant.h for default */
    rcar->master_ssi_channel               = SSI_CHANNEL_NUM;
    rcar->ssi_start_mode                   = DEFAULT_SSI_START_MODE;        /* see variant.h for default */
    rcar->ssi_transfer_mode                = DEFAULT_SSI_TRANSFER_MODE;     /* see variant.h for default */
    rcar->ssi_op_mode                      = DEFAULT_SSI_OP_MODE;           /* see variant.h for default */
    rcar->use_scu                          = DEFAULT_USE_SCU;               /* see variant.h for default */
    rcar->use_mlp                          = 0;                             /* by default, disable mlp */
    rcar->sample_rate_min                  = SAMPLE_RATE_MIN;
    rcar->sample_rate_max                  = SAMPLE_RATE_MAX;
    rcar->sample_rate                      = 0;                             /* 0 if no channel acquired, actual rate when a channel is acquired */
    rcar->slot_size                        = DEFAULT_SLOT_SIZE;             /* see variant.h for default */
    rcar->sample_size                      = 16;                            /* by default, 16 bit */
    rcar->voices                           = DEFAULT_VOICES;                /* see variant.h for default */
    rcar->playback.ssi_chan                = DEFAULT_SSI_CHANNEL_PLAYBACK;  /* see variant.h for default */
    rcar->capture.ssi_chan                 = DEFAULT_SSI_CHANNEL_CAPTURE;   /* see variant.h for default */
    rcar->ssi_config.clk_pol               = SSI_BIT_CLK_POL_RISING;
    rcar->ssi_config.ws_pol                = SSI_WS_POL_0;
    rcar->ssi_config.bit_delay             = SSI_BIT_DELAY_ONE;
    rcar->ssi_config.padding_pol           = SSI_PADDING_POL_LOW;
    rcar->ssi_config.serial_data_alignment = SSI_SER_DATA_ALIGN_DATA_FIRST;
    rcar->ssi_config.sys_word_length       = SSI_SYS_WORD_LEN_16BIT_STEREO;
    rcar->ssi_config.data_word_length      = SSI_DATA_WORD_LEN_16BIT;
    rcar->debug                            = 0;                             /* by default, no register dumps */

    /* Detect R-Car version based on confstr */
    if( confstr(_CS_MACHINE, cs_machine_str, CS_MACHINE_LEN) > 0 ) {
        ado_debug( DB_LVL_DRIVER, "%s: version detected=%s", __func__, cs_machine_str );

        rcar_parse_version(cs_machine_str);
    }

    /* Sets options to each values */
    while (args != NULL && args[0] != 0) {

        switch ((opt = getsubopt (&args, opts, &value))) {
        case 0: // "tx_ssi"
            if( value != NULL ) {
                if( !strcmp(value, "0129") ) {
                    if( rcar->ssi_op_mode == SSI_OP_MODE_TDMEXT ) {
                        ado_error( "%s: SSI 0,1,2,9 not supported in TDM EXT mode", __func__ );
                        return EINVAL;
                    }
                    if( !rcar_ssi_supported( 0 ) || !rcar_ssi_supported( 1 ) ||
                        !rcar_ssi_supported( 2 ) || !rcar_ssi_supported( 9 ) ) {
                        ado_error( "%s: Some of SSI 0,1,2,9 not supported", __func__ );
                        return EINVAL;
                    }
                    rcar->ssi_op_mode = SSI_OP_MODE_MULTICH;
                    rcar->voices = 8;
                    // set playback ssi_chan as 0, the use of multiple channels is specified by the op mode
                    // and playback voices
                    rcar->playback.ssi_chan = SSI_CHANNEL_0;
                    rcar->ssi_start_mode = SSI_SYNC_SSI0129_START;
                    rcar->ssi_transfer_mode = SSI_BUSIF_TRANSFER;
                    ado_debug( DB_LVL_DRIVER, "%s: tx uses multiple SSI 0129", __func__ );
                } else if ( !strcmp(value, "012") ) {
                    if( rcar->ssi_op_mode == SSI_OP_MODE_TDMEXT ) {
                        ado_error( "%s: SSI 0,1,2 not supported in TDM EXT mode", __func__ );
                        return EINVAL;
                    }
                    if( !rcar_ssi_supported( 0 ) || !rcar_ssi_supported( 1 ) ||
                        !rcar_ssi_supported( 2 ) ) {
                        ado_error("%s: Some of SSI 0,1,2 not supported", __func__);
                        return EINVAL;
                    }
                    rcar->ssi_op_mode = SSI_OP_MODE_MULTICH;
                    rcar->voices = 6;
                    // set playback ssi_chan as 0, the use of multiple channels is specified by the op mode
                    rcar->playback.ssi_chan = SSI_CHANNEL_0;
                    rcar->ssi_start_mode = SSI_SYNC_SSI0129_START;
                    rcar->ssi_transfer_mode = SSI_BUSIF_TRANSFER;
                    ado_debug( DB_LVL_DRIVER, "%s: tx uses multiple SSI 0,1,2", __func__ );
                } else {
                    numvalue = strtol( value, NULL, 0 );
                    if( rcar_ssi_supported( numvalue ) ) {
                        rcar->playback.ssi_chan = numvalue;
                        ado_debug( DB_LVL_DRIVER, "%s: tx uses SSI %d", __func__, rcar->playback.ssi_chan );
                    } else {
                        ado_error( "%s: SSI %d not supported", __func__, numvalue );
                        return EINVAL;
                    }
                }
            }
            break;
        case 1: // "rx_ssi"
            if( value != NULL ) {
                numvalue = strtol( value, NULL, 0 );
                if( rcar_ssi_supported( numvalue ) ) {
                    rcar->capture.ssi_chan = numvalue;
                    ado_debug( DB_LVL_DRIVER, "%s: rx uses SSI %d", __func__, rcar->capture.ssi_chan );
                } else {
                    ado_error( "%s: SSI %d not supported", __func__, numvalue );
                    return EINVAL;
                }
            }
            break;
        case 2: // "voices"
            if( value != NULL && rcar->voices == 0 ) {
                numvalue = strtol( value, NULL, 0 );
                if( numvalue == 1 || numvalue == 2 || numvalue == 4 ||
                    numvalue == 6 || numvalue == 8 ) {
                    rcar->voices = numvalue;
                }
                ado_debug( DB_LVL_DRIVER, "rcar : voices %d", rcar->voices );
            }
            break;
        case 3: // "master"
            rcar->ssi_masterslave_mode = SSI_MASTER_SLAVE;
            break;
        case 4: // "sync_start"
            rcar->ssi_start_mode = SSI_SYNC_SSI34_START;
            break;
        case 5: // "tdm_ext"
            if( rcar->ssi_op_mode != SSI_OP_MODE_MULTICH ) {
                rcar->ssi_op_mode = SSI_OP_MODE_TDMEXT;
                rcar->ssi_transfer_mode = SSI_BUSIF_TRANSFER;
            } else {
                ado_error( "%s: SSI 0,1,2(,9) not supported in TDM EXT mode", __func__ );
                return EINVAL;
            }
            break;
        case 6: // "scu"
            if( rcar_src_get_supported_range(&min_idx, &max_idx) == EOK ) {
                rcar->use_scu = 1;
                rcar->ssi_transfer_mode = SSI_BUSIF_TRANSFER;
            } else {
                ado_error( "%s: SCU not supported", __func__ );
            }
            break;
        case 7: // "mlp"
            if( rcar_mlm_get_supported_range(&min_idx, &max_idx) ) {
                rcar->use_mlp = 1;
                rcar->ssi_transfer_mode = SSI_BUSIF_TRANSFER;
            } else {
                ado_error( "%s: MLP/MLM not supported", __func__ );
            }
            break;
        case 8: // "dtcp"
            if( rcar_dtcp_get_supported_range(&min_idx, &max_idx) ) {
                rcar->use_dtcp = 1;
                rcar->ssi_transfer_mode = SSI_BUSIF_TRANSFER;
            } else {
                ado_error( "%s: DTCP not supported", __func__ );
            }
            break;
        case 9: // "sample_size"
            if( value != NULL ) {
                numvalue = strtol( value, NULL, 0 );
                if( numvalue == 16 || numvalue == 24 || numvalue == 32 ) {
                    rcar->sample_size = numvalue;
                } else {
                    ado_error( "%s: Invalid sample size: %d", __func__, numvalue );
                    return EINVAL;
                }
            }
            break;
        case 10: // "clk_pol"
            if( value != NULL ) {
                numvalue = strtol( value, NULL, 0 );

                if( numvalue == 0 ) {
                    rcar->ssi_config.clk_pol = SSI_BIT_CLK_POL_RISING;
                } else if( numvalue == 1 ) {
                    rcar->ssi_config.clk_pol = SSI_BIT_CLK_POL_FALLING;
                } else {
                    ado_error( "%s: Invalid clk pol: %d", __func__, numvalue );
                    return EINVAL;
                }
                ado_debug( DB_LVL_DRIVER, "%s: Set clock polarity %s", __func__,
                           rcar->ssi_config.clk_pol ? "FALLING" : "RISING" );
            }
            break;
        case 11: // "ws_pol"
            if( value != NULL ) {
                numvalue = strtol( value, NULL, 0 );

                if( numvalue == 0 ) {
                    rcar->ssi_config.ws_pol = SSI_WS_POL_0;
                } else if ( numvalue == 1 ) {
                    rcar->ssi_config.ws_pol = SSI_WS_POL_1;
                } else {
                    ado_error( "%s: Invalid ws pol: %d", __func__, numvalue );
                    return EINVAL;
                }
                ado_debug( DB_LVL_DRIVER, "%s: Set WS polarity %d", __func__, rcar->ssi_config.ws_pol );
            }
            break;
        case 12: // "bit_delay"
            if( value != NULL ) {
                numvalue = strtol( value, NULL, 0 );

                if( numvalue == 0 ) {
                    rcar->ssi_config.bit_delay = SSI_BIT_DELAY_NONE;
                } else if ( numvalue == 1 ) {
                    rcar->ssi_config.bit_delay = SSI_BIT_DELAY_ONE;
                } else {
                    ado_error( "%s: Invalid bit delay: %d", __func__, numvalue );
                    return EINVAL;
                }
                ado_debug (DB_LVL_DRIVER, "%s: Set bit delay %s", __func__,
                          rcar->ssi_config.bit_delay ? "NONE" : "ONE");
            }
            break;
        case 13: // "sample_rate" - min and max sample rate, separated by ':'
            if (value != NULL) {
                uint32_t rates[2];
                uint32_t n = 0;
                uint32_t rate_flag_min;
                uint32_t rate_flag_max;
                uint32_t rate_flag;

                while (value && n < 2) {
                    if (n > 0) value++;  // skip over separator
                    rates[n++] = strtoul(value, &value, 0);
                    value = strchr(value, ':');  // find next separator
                }

                if( n == 0 ) {
                    ado_error( "%s: No valid rate in sample_rate: %s", __func__, value );
                    return EINVAL;
                }

                if( ( rate_flag_min = ado_pcm_rate2flag(rates[0]) ) == 0 ) {
                    ado_error( "%s: Invalid min_rate in sample_rate: %d", __func__, rates[0] );
                    return EINVAL;
                }

                if( ( rate_flag_min & supported_rate_list ) == 0 ) {
                    ado_error( "%s: Unsupported min_rate in sample_rate: %d", __func__, rates[0] );
                    return EINVAL;
                }

                if ( rates[1] < rates[0] ||
                     ( rate_flag_max = ado_pcm_rate2flag(rates[1]) ) == 0 ) {
                    ado_error( "%s: Invalid max_rate in sample_rate: %d", __func__, rates[1] );
                    return EINVAL;
                }

                if( ( rate_flag_max & supported_rate_list ) == 0 ) {
                    ado_error( "%s: Unsupported max_rate in sample_rate: %d", __func__, rates[0] );
                    return EINVAL;
                }

                rcar->sample_rate_min = rates[0];
                rcar->sample_rate_max = (n > 1) ? rates[1] : rcar->sample_rate_min;

                configured_rate_list = rate_flag_min;
                for( rate_flag = rate_flag_min; rate_flag <= rate_flag_max; rate_flag <<= 1 ) {
                    if( rate_flag & supported_rate_list ) {
                        configured_rate_list |= rate_flag;
                    }
                }

                ado_debug( DB_LVL_DRIVER, "%s: sample rate min %d, max %d, rate_list %x", __func__,
                           rcar->sample_rate_min, rcar->sample_rate_max, configured_rate_list );
            }
            break;
        case 14: // "sample_rate_list" - all supported sample rates, separated by ':'
            if (value != NULL) {
                uint32_t rates[RCAR_NUM_SUPPORTED_RATES];
                uint32_t n = 0, i = 0;
                uint32_t rate_flag;

                while (value && n < sizeof(rates)/sizeof(uint32_t)) {
                    if (n > 0) value++;  // skip over separator
                    rates[n++] = strtoul(value, &value, 0);
                    value = strchr(value, ':');  // find next separator
                }

                if( n == 0 ) {
                    ado_error( "%s: No valid rate in sample_rate_list: %s", __func__, value );
                    return EINVAL;
                }

                rcar->sample_rate_min = rates[0];
                rcar->sample_rate_max = rates[0];

                for( i = 0; i < n; i++ ) {
                    if( ( rate_flag = ado_pcm_rate2flag(rates[i]) ) == 0 ) {
                        ado_error( "%s: Invalid rate in sample_rate_list: %d", __func__, rates[i] );
                        return EINVAL;
                    }
                    if( ( rate_flag & supported_rate_list ) == 0 ) {
                        ado_error( "%s: Unsupported rate in sample_rate_list: %d", __func__, rates[i] );
                        return EINVAL;
                    }
                    configured_rate_list |= rate_flag;
                    if( rates[i] < rcar->sample_rate_min ) {
                        rcar->sample_rate_min = rates[i];
                    } else if( rates[i] > rcar->sample_rate_max ) {
                        rcar->sample_rate_max = rates[i];
                    }
                }

                ado_debug( DB_LVL_DRIVER, "%s: sample rate min %d, max %d, rate_list %x", __func__,
                           rcar->sample_rate_min, rcar->sample_rate_max, configured_rate_list );
            }
            break;
        case 15: // "slot_size"
            if( value != NULL ) {
                numvalue = strtol( value, NULL, 0 );
                if( numvalue == 16 || numvalue == 32 ) {
                    rcar->slot_size = numvalue;
                }
                ado_debug( DB_LVL_DRIVER, "%s: slot_size %d", __func__, rcar->slot_size );
            }
            break;
        case 16: // "ver"
            if( value != NULL ) {
                rcar_parse_version(value);
                ado_debug( DB_LVL_DRIVER, "%s: ver %s", __func__, value );
            }
            break;
        case 17: // "debug"
            rcar->debug = 1;
            ado_debug( DB_LVL_DRIVER, "%s: Debug mode is on", __func__ );
        }
    }

    use_tx = ( rcar->playback.ssi_chan != SSI_CHANNEL_NUM ? 1 : 0 );
    use_rx = ( rcar->capture.ssi_chan != SSI_CHANNEL_NUM ? 1 : 0 );

    /* when adding MLP support change the below validation to check also for MLP use */
    /* when using MLP both transmit and receive could use the MLP port */
    if( !use_tx && !use_rx ) {
        ado_error( "%s: No SSI specified for either transmit or receive", __func__ );
        return EINVAL;
    }

    ret = EOK;

    /* the only reason to configure a transmit and receive SSI in the same driver
       instance is pin sharing between the SSIs used for receive and transmit;
       pin sharing occurs for SSI pairs {0,1}, {0,2}, {0,3}, {0,9}, {3,9}, {3,4}, {7,8} */
    if( use_tx && use_rx ) {
        switch( rcar->playback.ssi_chan ) {
            case SSI_CHANNEL_0:
                if( rcar->capture.ssi_chan != SSI_CHANNEL_1 &&
                    rcar->capture.ssi_chan != SSI_CHANNEL_2 &&
                    rcar->capture.ssi_chan != SSI_CHANNEL_3 &&
                    rcar->capture.ssi_chan != SSI_CHANNEL_9 ) {
                    ret = EINVAL;
                }
                if( rcar->ssi_masterslave_mode == SSI_MASTER_SLAVE ) {
                    rcar->master_ssi_channel = SSI_CHANNEL_0;
                }
                break;
            case SSI_CHANNEL_1:
            case SSI_CHANNEL_2:
                if( rcar->capture.ssi_chan != SSI_CHANNEL_0 ) {
                    ret = EINVAL;
                }
                if( rcar->ssi_masterslave_mode == SSI_MASTER_SLAVE ) {
                    rcar->master_ssi_channel = SSI_CHANNEL_0;
                }
                break;
            case SSI_CHANNEL_3:
                if( rcar->capture.ssi_chan != SSI_CHANNEL_0 &&
                    rcar->capture.ssi_chan != SSI_CHANNEL_4 &&
                    rcar->capture.ssi_chan != SSI_CHANNEL_9 ) {
                    ret = EINVAL;
                }
                if( rcar->ssi_masterslave_mode == SSI_MASTER_SLAVE ) {
                    if( rcar->capture.ssi_chan == SSI_CHANNEL_0 ) {
                        rcar->master_ssi_channel = SSI_CHANNEL_0;
                    } else {
                        rcar->master_ssi_channel = SSI_CHANNEL_3;
                    }
                }
                break;
            case SSI_CHANNEL_4:
                if( rcar->capture.ssi_chan != SSI_CHANNEL_3 ) {
                    ret = EINVAL;
                }
                if( rcar->ssi_masterslave_mode == SSI_MASTER_SLAVE ) {
                    rcar->master_ssi_channel = SSI_CHANNEL_3;
                }
                break;
            case SSI_CHANNEL_9:
                if( rcar->capture.ssi_chan != SSI_CHANNEL_0 &&
                    rcar->capture.ssi_chan != SSI_CHANNEL_3 ) {
                    ret = EINVAL;
                }
                if( rcar->ssi_masterslave_mode == SSI_MASTER_SLAVE ) {
                    if( rcar->capture.ssi_chan == SSI_CHANNEL_0 ) {
                        rcar->master_ssi_channel = SSI_CHANNEL_0;
                    } else {
                        rcar->master_ssi_channel = SSI_CHANNEL_3;
                    }
                }
                break;
            case SSI_CHANNEL_7:
                if( rcar->capture.ssi_chan != SSI_CHANNEL_8 ) {
                    ret = EINVAL;
                }
                if( rcar->ssi_masterslave_mode == SSI_MASTER_SLAVE ) {
                    rcar->master_ssi_channel = SSI_CHANNEL_7;
                }
                break;
            case SSI_CHANNEL_8:
                if( rcar->capture.ssi_chan != SSI_CHANNEL_7 ) {
                    ret = EINVAL;
                }
                if( rcar->ssi_masterslave_mode == SSI_MASTER_SLAVE ) {
                    rcar->master_ssi_channel = SSI_CHANNEL_7;
                }
                break;
        }
        if( ret == EINVAL ) {
            ado_error( "%s: SSI %d and %d can not be configured in pin sharing mode", __func__,
                       rcar->playback.ssi_chan, rcar->capture.ssi_chan );
            return ret;
        }
    }

    switch( rcar->slot_size ) {
        case 16:
            rcar->ssi_config.sys_word_length = (rcar->ssi_op_mode == SSI_OP_MODE_MONO)? SSI_SYS_WORD_LEN_16BIT_MONO:SSI_SYS_WORD_LEN_16BIT_STEREO;
            break;
        case 32:
            rcar->ssi_config.sys_word_length = (rcar->ssi_op_mode == SSI_OP_MODE_MONO)? SSI_SYS_WORD_LEN_32BIT_MONO:SSI_SYS_WORD_LEN_32BIT_STEREO;
            break;
        default:
            ado_error( "%s: Unsupported slot size %d", rcar->slot_size );
    }

    if( rcar->ssi_op_mode == SSI_OP_MODE_MULTICH && use_rx ) {
        ado_error( "%s: SSI 012(9) and %d can not be configured in the same driver instance", __func__,
                   rcar->capture.ssi_chan);
        return EINVAL;
    }

    if( rcar->voices == 0 ) {
        if( rcar->ssi_op_mode == SSI_OP_MODE_TDMEXT ) {
            ado_error( "%s: Number of voices needs to be specified for TDM extended mode", __func__ );
            return EINVAL;
        } else {
            rcar->voices = 2;
        }
    }

    /* don't support the split TDM modes at this time, as exact use case/routing is unclear */
    /* fill in the remaining voices and op mode information based on the existing information */
    rcar->ssi_voices = rcar->voices;
    if( rcar->voices == 6 || rcar->voices == 8 ) {
        if( rcar->ssi_op_mode == SSI_OP_MODE_TDMEXT ) {
            rcar->ssi_voices = ( rcar->voices == 6 ? 8 : 6 );
        } else if( rcar->ssi_op_mode != SSI_OP_MODE_MULTICH ) {
            rcar->ssi_op_mode = SSI_OP_MODE_TDM;
        }
    } else {
        if( rcar->ssi_op_mode == SSI_OP_MODE_TDMEXT ) {
            ado_error( "%s: Number of voices can be only 6 or 8 for TDM extended mode", __func__ );
            return EINVAL;
        }
        if( rcar->ssi_op_mode == SSI_OP_MODE_MULTICH ) {
            ado_error( "%s: Number of voices can be only 6 or 8 for multi channel mode", __func__ );
            return EINVAL;
        }
        if( rcar->voices == 1 ) {
            rcar->ssi_op_mode = SSI_OP_MODE_MONO;
        }
    }

    /* finally check if the requested SSI channels support the calculated op mode */
    if( rcar->ssi_op_mode == SSI_OP_MODE_TDMEXT ) {
        if( use_tx && !rcar_ssi_tdmext_supported( rcar->playback.ssi_chan ) ) {
            ado_error("%s: SSI %d does not support the TDM EXT mode", __func__, rcar->playback.ssi_chan);
            return EINVAL;
        }
        if( use_rx && !rcar_ssi_tdmext_supported( rcar->capture.ssi_chan ) ) {
            ado_error("%s: SSI %d does not support the TDM EXT mode", __func__, rcar->capture.ssi_chan);
            return EINVAL;
        }
    }

    return EOK;
}

static void rcar_register_dump( HW_CONTEXT_T * rcar )
{
    uint32_t use_tx = 0, use_rx = 0;

    ado_debug (DB_LVL_DRIVER, "rcar : %s", __func__);

    ssiu_common_register_dump();

    use_tx = rcar->playback.ssi_chan == SSI_CHANNEL_NUM ? 0 : 1;
    use_rx = rcar->capture.ssi_chan == SSI_CHANNEL_NUM ? 0 : 1;

    /* reserve the required resources */
    if( use_tx ) {
        if (rcar->ssi_op_mode == SSI_OP_MODE_MULTICH) { //Multiple SSI
            ssiu_ssi_register_dump( SSI_CHANNEL_0 );
            ssiu_ssi_register_dump( SSI_CHANNEL_1 );
            ssiu_ssi_register_dump( SSI_CHANNEL_2 );
            if( rcar->voices == 8 ) {
                ssiu_ssi_register_dump( SSI_CHANNEL_9 );
            }
        } else {
            ssiu_ssi_register_dump( rcar->playback.ssi_chan );
        }
    }
    if( use_rx ) {
        ssiu_ssi_register_dump( rcar->capture.ssi_chan );
    }
    if (rcar->use_scu) {
        if( use_tx ) {
            scu_src_register_dump( rcar->playback.src_chan );
            scu_cmd_register_dump( rcar->playback.cmd_chan );
            scu_dvc_register_dump( rcar->playback.cmd_chan );
        }
        if( use_rx ) {
            scu_src_register_dump( rcar->capture.src_chan );
        }
    }

    adg_register_dump();

    rcar_mixer_register_dump();
}

static int rcar_init_cleanup( HW_CONTEXT_T * rcar )
{
    uint32_t use_tx = rcar->playback.ssi_chan == SSI_CHANNEL_NUM ? 0 : 1;
    uint32_t use_rx = rcar->capture.ssi_chan == SSI_CHANNEL_NUM ? 0 : 1;

    ado_debug (DB_LVL_DRIVER, "rcar : %s", __func__);

    /* perform SSIU cleanup */
    if( use_tx && use_rx ) {
        ssiu_2channel_duplex_cleanup( rcar->playback.ssi_chan,
                                      rcar->capture.ssi_chan );
    } else if( use_tx ) {
        if (rcar->ssi_op_mode == SSI_OP_MODE_MULTICH) {
            if( rcar->voices == 8 ) {
                ssiu_4channel_transmit_cleanup();
            } else {
                ssiu_3channel_transmit_cleanup();
            }
        } else { //Single SSI
            ssiu_1channel_cleanup( rcar->playback.ssi_chan,
                                   rcar->master_ssi_channel );
        }
    } else if( use_rx ) {
        ssiu_1channel_cleanup( rcar->capture.ssi_chan,
                               rcar->master_ssi_channel );
    }

    /* release the reserved SSI resources */
    if( use_tx ) {
        if (rcar->ssi_op_mode == SSI_OP_MODE_MULTICH) { //Multiple SSI
            rcar_release_ssi( SSI_CHANNEL_0, SSI_CHANNEL_2 );
            if( rcar->voices == 8 ) {
                rcar_release_ssi( SSI_CHANNEL_9, SSI_CHANNEL_9 );
            }
        } else {
            rcar_release_ssi( rcar->playback.ssi_chan, rcar->playback.ssi_chan );
        }
        rcar->playback.ssi_chan = SSI_CHANNEL_NUM;
    }

    if( use_rx ) {

        /* perform SSIU cleanup - TBA */

        rcar_release_ssi( rcar->capture.ssi_chan, rcar->capture.ssi_chan );
        rcar->capture.ssi_chan = SSI_CHANNEL_NUM;
    }

    /* clean-up and release the reserved SCU resources */
    if (rcar->use_scu) {
        if( use_tx ) {
            /* just in case, stop the SRC and CMD */
            scu_src_stop(rcar->playback.src_chan);
            scu_cmd_stop(rcar->playback.cmd_chan);

            scu_src_cleanup(rcar->playback.src_chan);
            scu_dvc_cleanup(rcar->playback.cmd_chan);

            rcar_release_src( rcar->playback.src_chan );
            rcar_release_cmd( rcar->playback.cmd_chan );
            rcar->playback.src_chan = SCU_SRC_CHANNEL_NUM;
            rcar->playback.cmd_chan = SCU_CMD_CHANNEL_NUM;
        }
        if( use_rx ) {
            /* just in case, stop the SRC */
            scu_src_stop(rcar->capture.src_chan);

            scu_src_cleanup(rcar->capture.src_chan);

            rcar_release_src( rcar->capture.src_chan );
            rcar->capture.src_chan = SCU_SRC_CHANNEL_NUM;
        }
    }
    return EOK;
}

int rcar_init( HW_CONTEXT_T * hw )
{
    int status = EOK;
    int i;

    rcar_context_t* rcar = (rcar_context_t*) hw;

    uint32_t use_tx = 0, use_rx = 0;

    use_tx = rcar->playback.ssi_chan == SSI_CHANNEL_NUM ? 0 : 1;
    use_rx = rcar->capture.ssi_chan == SSI_CHANNEL_NUM ? 0 : 1;

    ado_debug (DB_LVL_DRIVER, "rcar : %s", __func__);

    /* seed the resources if not already done */
    rcar_create_resources();

    /* reserve the required resources */
    if( use_tx ) {
        if (rcar->ssi_op_mode == SSI_OP_MODE_MULTICH) { //Multiple SSI
            if( rcar_reserve_ssi( SSI_CHANNEL_0, SSI_CHANNEL_2 ) != EOK ) {
                //rcar_init_cleanup(); /* nothing to clean up yet */
                ado_error("%s: failed reserving SSI 0,1,2", __func__);
                return EAGAIN;
            }
            if( rcar->voices == 8 &&
                rcar_reserve_ssi( SSI_CHANNEL_9, SSI_CHANNEL_9 ) != EOK ) {
                ado_error("%s: failed reserving SSI 9", __func__);
                rcar_init_cleanup( rcar );
                return EAGAIN;
            }
        } else {
            if( rcar_reserve_ssi( rcar->playback.ssi_chan, rcar->playback.ssi_chan ) != EOK ) {
                //rcar_init_cleanup(); /* nothing to clean up yet */
                ado_error("%s: failed reserving SSI %d", __func__, rcar->playback.ssi_chan);
                return EAGAIN;
            }
        }
    }
    if( use_rx ) {
        if( rcar_reserve_ssi( rcar->capture.ssi_chan, rcar->capture.ssi_chan ) != EOK ) {
            ado_error("%s: failed reserving SSI %d", __func__, rcar->capture.ssi_chan);
            rcar_init_cleanup( rcar );
            return EAGAIN;
        }
    }
    if (rcar->use_scu) {
        if( use_tx ) {
            if( rcar_reserve_src( rcar->voices > 2 ? 1 : 0, 0, 1,
                                  &rcar->playback.src_chan ) != EOK ||
                rcar_reserve_cmd( &rcar->playback.cmd_chan ) != EOK ) {
                ado_error("%s: failed reserving playback SRC", __func__);
                rcar_init_cleanup( rcar );
                return EAGAIN;
            }
        }
        if( use_rx ) {
            if( rcar_reserve_src( rcar->voices > 2 ? 1 : 0, 0, 1,
                                  &rcar->capture.src_chan ) != EOK ) {
                ado_error("%s: failed reserving capture SRC", __func__);
                rcar_init_cleanup( rcar );
                return EAGAIN;
            }
        }
    }

    /* set-up the SSIU */
    if( use_tx && use_rx ) {
        /* set-up for duplex - playback and capture */
        status = ssiu_2channel_duplex_setup( rcar->playback.ssi_chan,
                                             rcar->capture.ssi_chan,
                                             rcar->ssi_op_mode,
                                             rcar->ssi_masterslave_mode,
                                             rcar->ssi_transfer_mode,
                                             rcar->ssi_start_mode,
                                             rcar->sample_size,
                                             rcar->voices,
                                             rcar->ssi_voices,
                                             &rcar->ssi_config );
    } else if( use_tx ) {
        /* set-up for playback */
        if (rcar->ssi_op_mode == SSI_OP_MODE_MULTICH) { //Multiple SSI
            if( rcar->voices == 8 ) {
                status = ssiu_4channel_transmit_setup(rcar->ssi_masterslave_mode, 16, &rcar->ssi_config);
            } else {
                status = ssiu_3channel_transmit_setup(rcar->ssi_masterslave_mode, 16, &rcar->ssi_config);
            }
        } else { //Single SSI
            status = ssiu_1channel_setup( rcar->playback.ssi_chan,
                                          rcar->master_ssi_channel,
                                          1, /* transmit */
                                          rcar->ssi_op_mode,
                                          rcar->ssi_transfer_mode,
                                          rcar->sample_size,
                                          rcar->voices,
                                          rcar->ssi_voices,
                                          &rcar->ssi_config );
        }
    } else if( use_rx ) {
        status = ssiu_1channel_setup( rcar->capture.ssi_chan,
                                      rcar->master_ssi_channel,
                                      0, /* receive */
                                      rcar->ssi_op_mode,
                                      rcar->ssi_transfer_mode,
                                      rcar->sample_size,
                                      rcar->voices,
                                      rcar->ssi_voices,
                                      &rcar->ssi_config );
    }

    if( status != EOK ) {
        ado_error("%s: failed setting up SSIU", __func__);
        rcar_init_cleanup( rcar );
        return status;
    }

    /* Set-up the SCU */
    if (rcar->use_scu == 1) {
        if( use_tx ) {
            /* Initialize the DVC volume variables to 0dB = 0x100000 for all channels */
            for( i = 0; i < sizeof(rcar->playback.dvc_volume)/sizeof(rcar->playback.dvc_volume[0]); i++ ) {
                rcar->playback.dvc_volume[i] = 0x100000;
            }

            /* Setup syncronous SRC0 */
            status = scu_src_setup( rcar->playback.src_chan,
                                    1,
                                    0,
                                    rcar->sample_size,
                                    rcar->voices,
                                    rcar->sample_rate_max,
                                    SAMPLE_RATE_SRC );
            /* use SRC0 -> CMD0 */
            if( status == EOK ) {
                status = scu_cmd_setup( rcar->playback.cmd_chan,
                                        rcar->playback.src_chan );
            }

            /* setup DVC0 */
            if( status == EOK ) {
                status = scu_dvc_setup( rcar->playback.cmd_chan,
                                        rcar->sample_size,
                                        rcar->voices,
                                        rcar->playback.dvc_volume );
            }
        }
        if( use_rx ) {
            /* Setup SRC */
            status = scu_src_setup( rcar->capture.src_chan,
                                     0,
                                     1,
                                     rcar->sample_size,
                                     rcar->voices,
                                     SAMPLE_RATE_SRC,
                                     rcar->sample_rate_max );
        }
    }

    if( status != EOK ) {
        ado_error("%s: failed setting up SCU", __func__);
        rcar_init_cleanup( rcar );
        return status;
    }

    status = rcar_set_clock_rate( rcar );

    if( status != EOK ) {
        ado_error("%s: failed setting up the clock rate", __func__);
        rcar_init_cleanup( rcar );
        return status;
    }

    if( rcar->debug ) {
        rcar_register_dump( rcar );
    }

    return EOK;
}

static int rcar_set_clock_rate( rcar_context_t * rcar )
{
    int ret = EOK;
    uint32_t ssi_divisor = 0;
    uint32_t adg_divisor = 0;

    uint32_t sample_rate;
    uint32_t ssi_sample_rate;

    uint32_t adg_clk = AUDIO_CLKA;

    ado_debug( DB_LVL_DRIVER, "rcar : %s : sample_rate=%d, sample_rate_max=%d", __func__,
               rcar->sample_rate, rcar->sample_rate_max );

    if( !rcar->sample_rate ) {
        sample_rate = rcar->sample_rate_max;
    } else {
        sample_rate = rcar->sample_rate;
    }
    if( rcar->use_scu ) {
        ssi_sample_rate = SAMPLE_RATE_SRC;
    } else {
        ssi_sample_rate = sample_rate;
    }
    ado_debug( DB_LVL_DRIVER, "rcar : %s : ssi_sample_rate=%d", __func__, ssi_sample_rate );
    if(RCAR_VERSION_M3 == rcar_version_get()) {
        // MCLK: AUDIO_CLKA = 22.5792 MHz, AUDIO_CLKB = 24.5760 MHz
        // Divisor = mclk/sys_word_length/voices/ssi_sample_rate
        switch( ssi_sample_rate ) {
            case 44100:
                adg_clk = AUDIO_CLKA;
                ssi_divisor = 8;
                adg_divisor = 1;
                break;
            case 48000:
                adg_clk = AUDIO_CLKB;
                ssi_divisor = 8;
                adg_divisor = 1;
                break;
            case 88200:
                adg_clk = AUDIO_CLKA;
                ssi_divisor = 4;
                adg_divisor = 1;
                break;
            case 96000:
                adg_clk = AUDIO_CLKB;
                ssi_divisor = 4;
                adg_divisor = 1;
                break;
            case 176400:
                adg_clk = AUDIO_CLKA;
                ssi_divisor = 2;
                adg_divisor = 1;
                break;
            case 192000:
                adg_clk = AUDIO_CLKB;
                ssi_divisor = 2;
                adg_divisor = 1;
                break;
            default:
                ado_error( "%s: clock rate %d not supported", __func__, ssi_sample_rate );
                return EINVAL;
        }
    } else {
        switch( ssi_sample_rate ) {
            // MCLK (AUDIO_CLKA) = 12.288 MHZ
            // Divisors in two different audio blocks (ADG and SSI) must be used to
            // achieve the desired range of clock rates. A second, more subtle
            // restriction dictates that the SSI divisor must be a value other than
            // '1' if a continuous frame clock (i.e. even without data streaming) is
            // configured.
            // The number of bits in a frame also indirectly affects the clock rates
            // and thus the selection of divisors.
            case 8000:
                if (rcar->slot_size == 32) {
                    // MCLK / 24 = 512 KHz
                    ssi_divisor = 6;
                    adg_divisor = 4;
                } else {
                    // MCLK / 48 = 256 KHz
                    ssi_divisor = 6;
                    adg_divisor = 8;
                }
                break;

            case 16000:
                if (rcar->slot_size == 32) {
                    // MCLK / 12 = 1.024 MHz
                    ssi_divisor = 12;
                    adg_divisor = 1;
                } else {
                    // MCLK / 24 = 512 KHz
                    ssi_divisor = 12;
                    adg_divisor = 2;
                }
                break;

            case 32000:
               if (rcar->slot_size == 32) {
                    // MCLK / 6 = 2.048 MHz
                    ssi_divisor = 6;
                    adg_divisor = 1;
                } else {
                    // MCLK / 12 = 1.024 MHz
                    ssi_divisor = 12;
                    adg_divisor = 1;
                }
                break;

            case 48000:
               if (rcar->slot_size == 32) {
                    // MCLK / 4 = 3.072 MHz
                    ssi_divisor = 4;
                    adg_divisor = 1;
                } else {
                    // MCLK / 8 = 1.536 MHz
                    ssi_divisor = 8;
                    adg_divisor = 1;
                }
                break;
            case 0:
                ado_error( "%s: clock rate not initialized", __func__ );
                return EINVAL;
            default:
                ado_error( "%s: clock rate %d not supported", __func__, ssi_sample_rate );
                return EINVAL;
        }
    }

    ado_debug( DB_LVL_DRIVER, "%s: Set SSI divisor to %d, ADG divisor to %d", __func__,
               ssi_divisor, adg_divisor );

    if( rcar->ssi_op_mode == SSI_OP_MODE_MULTICH ) {
        ret = ssi_set_divisor( SSI_CHANNEL_0, ssi_divisor );
        if( ret == EOK ) ret = ssi_set_divisor( SSI_CHANNEL_1, ssi_divisor );
        if( ret == EOK ) ret = ssi_set_divisor( SSI_CHANNEL_2, ssi_divisor );
        if( ret == EOK && rcar->voices == 8 ) {
            ret = ssi_set_divisor( SSI_CHANNEL_9, ssi_divisor );
        }
    } else {
        if( rcar->playback.ssi_chan != SSI_CHANNEL_NUM ) {
            ret = ssi_set_divisor( rcar->playback.ssi_chan, ssi_divisor );
        }
        if( ret == EOK && rcar->capture.ssi_chan != SSI_CHANNEL_NUM ) {
            ret = ssi_set_divisor( rcar->capture.ssi_chan, ssi_divisor );
        }
    }
    if (ret != EOK) {
        ado_error( "%s: failed setting the SSI divisor", __func__ );
        return ret;
    }

    if( rcar->playback.ssi_chan != SSI_CHANNEL_NUM ) {
        ret = adg_set_clk(rcar->playback.ssi_chan, adg_clk);
        if (ret != EOK) {
            ado_error( "%s: failed setting the ADG Clock Select for SSI %d", __func__ , rcar->playback.ssi_chan );
            return ret;
        }
        ret = adg_set_divisor(rcar->playback.ssi_chan, adg_divisor);
        if (ret != EOK) {
            ado_error( "%s: failed setting the ADG divisor for SSI %d", __func__, rcar->playback.ssi_chan );
            return ret;
        }
    }

    if( rcar->capture.ssi_chan != SSI_CHANNEL_NUM ) {
        ret = adg_set_clk(rcar->capture.ssi_chan, adg_clk);
        if (ret != EOK) {
            ado_error( "%s: failed setting the ADG Clock Select for SSI %d", __func__ , rcar->playback.ssi_chan );
            return ret;
        }
        ret = adg_set_divisor(rcar->capture.ssi_chan, adg_divisor);
        if (ret != EOK) {
            ado_error( "%s: failed setting the ADG divisor for SSI %d", __func__, rcar->capture.ssi_chan );
            return ret;
        }
    }

    /* set the mixer/codec clock rate if applicable */
    ret = rcar_mixer_set_clock_rate( ssi_sample_rate );
    if (ret != EOK) {
        ado_error( "%s: failed setting the mixer clock rate", __func__ );
        return ret;
    }

    if( rcar->use_scu ) {
        if( rcar->playback.ssi_chan != SSI_CHANNEL_NUM ) {
            /* Re-setup SRC */
            ret = scu_src_setup( rcar->playback.src_chan,
                                 1,
                                 0,
                                 rcar->sample_size,
                                 rcar->voices,
                                 sample_rate,
                                 ssi_sample_rate );
            /* Re-setup DVC  */
            if( ret == EOK ) {
                scu_dvc_setup( rcar->playback.cmd_chan,
                               rcar->sample_size,
                               rcar->voices,
                               rcar->playback.dvc_volume );
            }
        }
        if( rcar->capture.ssi_chan != SSI_CHANNEL_NUM ) {
            /* Re-configure scu-src with new sample rate */
            ret = scu_src_setup( rcar->capture.src_chan,
                                 0,
                                 1,
                                 rcar->sample_size,
                                 rcar->voices,
                                 ssi_sample_rate,
                                 sample_rate );
        }

        if( ret != EOK) {
            ado_error( "%s: failed setting the SCU rate", __func__ );
        }
    }

    return EOK;
}

ado_dll_version_t ctrl_version;
void ctrl_version (int *major, int *minor, char *date)
{
    *major = ADO_MAJOR_VERSION;
    *minor = 1;
    date = __DATE__;
}

static void ctrl_init_cleanup(rcar_context_t * rcar)
{
    ado_debug (DB_LVL_DRIVER, "rcar : %s", __func__);

    ado_mutex_destroy (&rcar->hw_lock);

    ssiu_deinit();
    scu_deinit();
    adg_deinit();
    audio_dmac_deinit(&rcar->playback.dma_context, &rcar->capture.dma_context);

    ado_free (rcar);

    rcar = NULL;
}

/* */
/*  Initialize */
/* */

int ctrl_init (HW_CONTEXT_T ** hw_context, ado_card_t * card, char *args)
{
    rcar_context_t *rcar;
    int status;

    ado_debug (DB_LVL_DRIVER, "rcar : CTRL_DLL_INIT");

    if ((rcar = (rcar_context_t *) ado_calloc (1, sizeof (rcar_context_t))) == NULL) {
        ado_error ("rcar %s: Unable to allocate memory (%s)", __func__, strerror (errno));
        return ENOMEM;
    }

    memset(rcar, 0, sizeof(rcar_context_t));

    *hw_context = rcar;

    if ((status = rcar_parse_commandline (rcar, args)) != EOK) {
        ado_free (rcar);
        return status;
    }

    ado_card_set_shortname (card, "rcar");
    ado_card_set_longname (card, "rcar", RCAR_SSI_BASE);

    /* Map Common SSIU base register */
    if ((status = ssiu_init()) != EOK) {
        ado_error ("rcar %s: SSIU init failed", __func__);
        ado_free (rcar);
        rcar = NULL;
        return status;
    }

    /* Map SCU */
    if ((status = scu_init()) != EOK) {
        ado_error ("rcar %s: SCU init failed", __func__);
        ssiu_deinit();
        ado_free (rcar);
        rcar = NULL;
        return status;
    }

    /* Map ADG */
    if ((status = adg_init()) != EOK) {
        ado_error ("rcar %s: ADG init failed", __func__);
        ssiu_deinit();
        scu_deinit();
        ado_free (rcar);
        rcar = NULL;
        return status;
    }

    /* Map DMAC */
    if ((status = audio_dmac_init(&rcar->playback.dma_context, &rcar->capture.dma_context)) != EOK) {
        ado_error ("rcar %s: Audio DMAC init failed (%s)", __func__, strerror (errno));
        ssiu_deinit();
        scu_deinit();
        adg_deinit();
        ado_free (rcar);
        rcar = NULL;
        return status;
    }

    ado_mutex_init (&rcar->hw_lock);

    if( (status = rcar_init(rcar)) != EOK ) {
        ado_error ("rcar %s: RCAR hw init failed", __func__);
        ctrl_init_cleanup(rcar);
        return status;
    }

    ado_debug (DB_LVL_DRIVER, "%s: Attaching interrupts - playback: %d, capture %d", __func__,
               rcar->playback.dma_context.audiodma_irq, rcar->capture.dma_context.audiodma_irq);

    if ((status = ado_attach_interrupt (card, rcar->playback.dma_context.audiodma_irq, rcar_playback_interrupt, rcar)) != EOK ||
        (status = ado_attach_interrupt (card, rcar->capture.dma_context.audiodma_irq, rcar_capture_interrupt, rcar)) != EOK) {
        ado_error ("rcar %s: Unable to attach interrupt (%s)", __func__, strerror (errno));
        ctrl_init_cleanup(rcar);
        return status;
    }

    rcar->playback.pcm_caps.chn_flags = SND_PCM_CHNINFO_BLOCK | SND_PCM_CHNINFO_STREAM |
        SND_PCM_CHNINFO_INTERLEAVE | SND_PCM_CHNINFO_BLOCK_TRANSFER |
        SND_PCM_CHNINFO_MMAP | SND_PCM_CHNINFO_MMAP_VALID;

    if( rcar->sample_size == 16 ) {
        rcar->playback.pcm_caps.formats = SND_PCM_FMT_S16_LE;
    } else if( rcar->sample_size == 24 ) {
        rcar->playback.pcm_caps.formats = SND_PCM_FMT_S24_LE;
    } else if( rcar->sample_size == 32 ) {
        rcar->playback.pcm_caps.formats = SND_PCM_FMT_S32_LE;
    }

    if( configured_rate_list ) {
        rcar->playback.pcm_caps.rates = configured_rate_list;
    } else {
        rcar->playback.pcm_caps.rates = supported_rate_list;
    }
    rcar->playback.pcm_caps.min_rate = rcar->sample_rate_min;
    rcar->playback.pcm_caps.max_rate = rcar->sample_rate_max;
    rcar->playback.pcm_caps.min_voices = rcar->voices;
    rcar->playback.pcm_caps.max_voices = rcar->voices;
    rcar->playback.pcm_caps.min_fragsize = 64;
    rcar->playback.pcm_caps.max_fragsize = 64 * 1024;
    rcar->playback.pcm_caps.max_frags = 2;

    /* Set capabilities of recording */
    memcpy (&rcar->capture.pcm_caps, &rcar->playback.pcm_caps,
        sizeof (rcar->playback.pcm_caps));
    rcar->capture.pcm_caps.max_voices = rcar->voices;

    /* Set functions for playback */
    rcar->playback.pcm_funcs.aquire = rcar_playback_acquire;
    rcar->playback.pcm_funcs.release = rcar_playback_release;
    rcar->playback.pcm_funcs.prepare = rcar_playback_prepare;
    rcar->playback.pcm_funcs.trigger = rcar_playback_trigger;
#ifdef RCAR_6_6_0
    rcar->playback.pcm_funcs.position3 = rcar_position;
    rcar->playback.pcm_funcs.capabilities2 = rcar_capabilities;
#else
    rcar->playback.pcm_funcs.position = rcar_position;
    rcar->playback.pcm_funcs.capabilities = rcar_capabilities;
#endif

    /* Set functions for recording */
    rcar->capture.pcm_funcs.aquire = rcar_capture_acquire;
    rcar->capture.pcm_funcs.release = rcar_capture_release;
    rcar->capture.pcm_funcs.prepare = rcar_capture_prepare;
    rcar->capture.pcm_funcs.trigger = rcar_capture_trigger;
#ifdef RCAR_6_6_0
    rcar->capture.pcm_funcs.position3 = rcar_position;
    rcar->capture.pcm_funcs.capabilities2 = rcar_capabilities;
#else
    rcar->capture.pcm_funcs.position = rcar_position;
    rcar->capture.pcm_funcs.capabilities = rcar_capabilities;
#endif

    if( (status = rcar_mixer_init(card, rcar)) != EOK ) {
        ado_error ("rcar %s: Unable to create a mixer", __func__);
        ctrl_init_cleanup(rcar);
        return status;
    }

    /* Create a PCM audio device */
    if( (status = ado_pcm_create (card, "R-Car SSI", 0, "rcar",
            1, &rcar->playback.pcm_caps, &rcar->playback.pcm_funcs,
            1, &rcar->capture.pcm_caps, &rcar->capture.pcm_funcs, rcar->mixer, &rcar->pcm)) != EOK ) {
        ado_error ("rcar %s: Unable to create pcm devices (%s)", __func__, strerror (errno));
        ctrl_init_cleanup(rcar);
        return status;
    }

    codec_set_default_group( rcar->pcm, rcar->mixer, ADO_PCM_CHANNEL_PLAYBACK, 0 );
    codec_set_default_group( rcar->pcm, rcar->mixer, ADO_PCM_CHANNEL_CAPTURE, 0 );

    ado_debug (DB_LVL_DRIVER, "RCar initialization complete.....");

    return EOK;
}

ado_ctrl_dll_destroy_t ctrl_destroy;
int ctrl_destroy (HW_CONTEXT_T * rcar)
{
    ado_debug (DB_LVL_DRIVER, "rcar : CTRL_DLL_DESTROY");

    ctrl_init_cleanup(rcar);
    rcar = NULL;

    return EOK;
}

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/hardware/deva/ctrl/rcar/rcar_dll.c $ $Rev: 812827 $")
#endif
