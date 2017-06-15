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

struct ak4613_context;
#define MIXER_CONTEXT_T struct ak4613_context

#include "rcar_mixer.h"

extern void cs2000_set(uint8_t enable);
extern void cs2000_dump();

typedef struct ak4613_context
{
    ado_mixer_t             *mixer;
    HW_CONTEXT_T            *hwc;
} ak4613_context_t;

#include "scu.h"
#include "ak4613.h"

static int32_t pcm_devices[1] = { 0 };

/* Kind of channel */
static snd_mixer_voice_t    stereo_voices[2] = {
    {SND_MIXER_VOICE_LEFT,  0}, // left channel
    {SND_MIXER_VOICE_RIGHT, 0}  // right channel
};

/* Output volume range */
static struct snd_mixer_element_volume1_range   ak4613_output_range[2] = {
    {0, AK4613_MAX_DIGITAL_VOL, -12700, 0},    // min, max, min_dB, max_dB (SPEAKER)
    {0, AK4613_MAX_DIGITAL_VOL, -12700, 0}     // min, max, min_dB, max_dB (SPEAKER)
};


static  int32_t
ak4613_master_vol_control (MIXER_CONTEXT_T * ak4613, ado_mixer_delement_t * element, uint8_t set,
    uint32_t * vol, void *instance_data)
{
    uint8_t left_vol, right_vol;
    int32_t altered = 0;

    /* get output volume from  volume */
    ak4613_output_vol_get( &left_vol, &right_vol );

    if (set)
    {
        /* compare value of vol with CODEC */
        altered = (vol[0] != left_vol || vol[1] != right_vol);

        left_vol = vol[0] & 0xFF;
        right_vol = vol[1] & 0xFF;

        ado_debug( DB_LVL_DRIVER, "rcar : %s: setting AK4613 output volume to %x:%x", __func__, left_vol, right_vol );
        /* set output volume to CODEC */
        ak4613_output_vol_set( left_vol, right_vol );
    }
    else /* read volume */
    {
        vol[0] = left_vol;
        vol[1] = right_vol;
    }

    return altered;
}

/* Required for compatibility with Audioman
 * This switch is called by audio manager to ask deva to send the current HW status, i.e., whether headset is connected
 */
static  int32_t
rcar_audioman_refresh_set(MIXER_CONTEXT_T * hw_ctx, ado_dswitch_t * dswitch, snd_switch_t * cswitch,
                            void *instance_data)
{
    return EOK;
}

static int32_t
rcar_audioman_refresh_get(MIXER_CONTEXT_T * hw_ctx, ado_dswitch_t * dswitch, snd_switch_t * cswitch,
                            void *instance_data)
{
    /* Always return disabled as this switch does not maintain state */
    cswitch->type = SND_SW_TYPE_BOOLEAN;
    cswitch->value.enable = 0;
    return 0;
}

/*
 *  build_ak4613_mixer
 */
static  int32_t
build_ak4613_mixer(MIXER_CONTEXT_T * ak4613)
{
    int     error = 0;
    ado_mixer_delement_t *pre_elem, *vol_elem, *elem = NULL;

    ado_debug (DB_LVL_DRIVER, "AK4613: build_ak4613_mixer");

    /* ################# */
    /* the OUTPUT GROUPS */
    /* ################# */
    if (!error && (elem = ado_mixer_element_pcm1 (ak4613->mixer, "DAC PCM",
                SND_MIXER_ETYPE_PLAYBACK1, 1, &pcm_devices[0])) == NULL)
        error++;

    pre_elem = elem;

    if (!error && (vol_elem = ado_mixer_element_volume1 (ak4613->mixer, "DAC Volume",
                2, ak4613_output_range, ak4613_master_vol_control, NULL, NULL)) == NULL)
    error++;

    /* route pcm to volume */
    if (!error && ado_mixer_element_route_add(ak4613->mixer, pre_elem, vol_elem) != 0)
        error++;

    pre_elem = vol_elem;

    if (!error && (elem = ado_mixer_element_io (ak4613->mixer, "DAC Output",
                SND_MIXER_ETYPE_OUTPUT, 0, 2, stereo_voices)) == NULL)
        error++;

    if (!error && ado_mixer_element_route_add (ak4613->mixer, pre_elem, elem) != 0)
        error++;

    if (!error && ado_mixer_playback_group_create(ak4613->mixer, SND_MIXER_MASTER_OUT,
                SND_MIXER_CHN_MASK_STEREO, vol_elem, NULL) == NULL)
        error++;

    /* ################ */
    /* the INPUT GROUPS */
    /* ################ */
    if (!error && (elem = ado_mixer_element_io(ak4613->mixer, "PCM In",
                SND_MIXER_ETYPE_INPUT, 0, 2, stereo_voices)) == NULL)
        error++;

    pre_elem = elem;

    if (!error && (elem = ado_mixer_element_pcm1 (ak4613->mixer, SND_MIXER_ELEMENT_CAPTURE,
                SND_MIXER_ETYPE_CAPTURE1, 1, &pcm_devices[0])) == NULL)
        error++;

    if (!error && ado_mixer_element_route_add (ak4613->mixer, pre_elem, elem) != 0)
        error++;

    if (!error && ado_mixer_capture_group_create (ak4613->mixer, SND_MIXER_GRP_IGAIN,
                SND_MIXER_CHN_MASK_STEREO, NULL, NULL, NULL, NULL) == NULL)
        error++;

    if (!error) {
        if (ado_mixer_switch_new(ak4613->mixer, "Audioman Refresh", SND_SW_TYPE_BOOLEAN, 0,
                (void*)rcar_audioman_refresh_get,
                (void *)rcar_audioman_refresh_set, NULL, NULL) == NULL) {
            return ENOMEM;
        }
        return EOK;
    }

    return ENOMEM;
}

void
codec_set_default_group( ado_pcm_t *pcm, ado_mixer_t *mixer, int channel, int index )
{
    switch (channel)
    {
        case ADO_PCM_CHANNEL_PLAYBACK:
            ado_pcm_chn_mixer (pcm, ADO_PCM_CHANNEL_PLAYBACK, mixer,
                ado_mixer_find_element (mixer, SND_MIXER_ETYPE_PLAYBACK1,
                    SND_MIXER_ELEMENT_PLAYBACK, index), ado_mixer_find_group (mixer,
                    SND_MIXER_MASTER_OUT, index));
            break;
        case ADO_PCM_CHANNEL_CAPTURE:
            ado_pcm_chn_mixer (pcm, ADO_PCM_CHANNEL_CAPTURE, mixer,
                ado_mixer_find_element (mixer, SND_MIXER_ETYPE_CAPTURE1,
                    SND_MIXER_ELEMENT_CAPTURE, index), ado_mixer_find_group (mixer,
                    SND_MIXER_GRP_IGAIN, index));
            break;
        default:
            break;
    }
}

//ado_mixer_reset_t rcar_mixer_reset;
static int32_t rcar_mixer_reset (MIXER_CONTEXT_T *ak4613)
{
    ado_debug (DB_LVL_MIXER, "%s: resetting AK4613", __func__);
    return ak4613_reset();
}

//ado_mixer_destroy_t rcar_mixer_destroy;
static int32_t rcar_mixer_destroy (MIXER_CONTEXT_T *ak4613)
{
    ado_debug (DB_LVL_MIXER, "rcar : MIXER DESTROY", __func__);
    ado_free(ak4613);
    ak4613_close_i2c_fd();
    return EOK;
}


/*
 * create & init the mixer
 */
int rcar_mixer_init (ado_card_t * card, HW_CONTEXT_T * hwc)
{
    ak4613_context_t    *ak4613;
    int                 status;

    ado_debug (DB_LVL_MIXER, "%s", __func__);

    if ((ak4613 = (ak4613_context_t *) ado_calloc (1, sizeof (ak4613_context_t))) == NULL)
    {
        ado_error ("%s: failed allocating memory %s", __func__, strerror (errno));
        return ENOMEM;
    }

    if( (status = ak4613_open_i2c_fd()) != EOK ) {
        ado_free (ak4613);
        return status;
    }

    if ((status = ado_mixer_create (card, "ak4613", &hwc->mixer, ak4613)) != EOK)
    {
        ado_free(ak4613);
        ak4613_close_i2c_fd();
        return status;
    }

    ak4613->mixer = hwc->mixer;
    ak4613->hwc = hwc;

    if ((status = build_ak4613_mixer(ak4613)) != EOK)
    {
        ado_error("%s: Failed to build AK4613 mixer");
        ado_free(ak4613);
        ak4613_close_i2c_fd();
        return status;
    }

    if ((status = rcar_mixer_reset(ak4613)) != EOK)
    {
        ado_error("%s: Failed to reset AK4613");
        ado_free(ak4613);
        ak4613_close_i2c_fd();
        return status;
    }

    ado_mixer_set_reset_func (ak4613->mixer, rcar_mixer_reset);
    ado_mixer_set_destroy_func (ak4613->mixer, rcar_mixer_destroy);

    return (0);
}

int rcar_mixer_set_clock_rate( uint32_t sample_rate )
{
    ado_debug( DB_LVL_DRIVER, "%s: setting AK4613 clock rate to %d", __func__, sample_rate );
    switch( sample_rate ) {
        case 44100:
        case 88200:
        case 176400:
            cs2000_set(0);
            break;
        case 48000:
        case 96000:
        case 192000:
            cs2000_set(1);
            break;
    }
    return ak4613_rate_setting( sample_rate );
}

void rcar_mixer_register_dump()
{
    ak4613_register_dump();
    cs2000_dump();
}

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/hardware/deva/ctrl/rcar/nto/aarch64/dll.le.ak4613/rcar_mixer.c $ $Rev: 812929 $")
#endif
