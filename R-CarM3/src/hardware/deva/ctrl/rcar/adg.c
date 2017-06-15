/*
 * $QNXLicenseC:
 * Copyright 2015, 2016 QNX Software Systems.
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

#include "adg.h"
#include <errno.h>
#include <stdint.h>
#include <arm/r-car-m3.h>
#include <audio_driver.h>

/* The audio clock generator (ADG) block supplies the necessary clocks for the
 * operation of the SSI, SCU and/or DTCP modules
 */

/* audio clock config for SSI 0 to 9: AUDIO_CLK_SEL0 register for SSI 0 to 3,
   AUDIO_CLK_SEL1 register for SSI 4 to 7,  AUDIO_CLK_SEL2 register for SSI 8 to 9 */
#define AUDIO_CLK_SEL_DIVSELSSI_MASK( ssi )          ( 0x19 << ( ( ( ssi & 0x3 ) << 3 ) + 3 ) )
#define AUDIO_CLK_SEL_DIVSELSSI_DIV1( ssi )          0
#define AUDIO_CLK_SEL_DIVSELSSI_DIV2( ssi )          ( 0x1 << ( ( ( ssi & 0x3 ) << 3 ) + 3 ) )
#define AUDIO_CLK_SEL_DIVSELSSI_DIV4( ssi )          ( 0x8 << ( ( ( ssi & 0x3 ) << 3 ) + 3 ) )
#define AUDIO_CLK_SEL_DIVSELSSI_DIV8( ssi )          ( 0x9 << ( ( ( ssi & 0x3 ) << 3 ) + 3 ) )
#define AUDIO_CLK_SEL_DIVSELSSI_DIV16( ssi )         ( 0x10 << ( ( ( ssi & 0x3 ) << 3 ) + 3 ) )
#define AUDIO_CLK_SEL_DIVSELSSI_DIV32( ssi )         ( 0x11 << ( ( ( ssi & 0x3 ) << 3 ) + 3 ) )
#define AUDIO_CLK_SEL_ACLKSELSSI_MASK( ssi )         ( 0x3 << ( ( ( ssi & 0x3 ) << 3 ) + 4 ) )
#define AUDIO_CLK_SEL_ACLKSELSSI_DIVCLK( ssi )       0
#define AUDIO_CLK_SEL_ACLKSELSSI_BRGA( ssi )         ( 0x1 << ( ( ( ssi & 0x3 ) << 3 ) + 4 ) )
#define AUDIO_CLK_SEL_ACLKSELSSI_BRGB( ssi )         ( 0x2 << ( ( ( ssi & 0x3 ) << 3 ) + 4 ) )
#define AUDIO_CLK_SEL_DIVCLKSELSSI_MASK( ssi )       ( 0x7 << ( ( ssi & 0x3 ) << 3 ) )
#define AUDIO_CLK_SEL_DIVCLKSELSSI_AUDIO_CLKA( ssi ) ( 0x1 << ( ( ssi & 0x3 ) << 3 ) )
#define AUDIO_CLK_SEL_DIVCLKSELSSI_AUDIO_CLKB( ssi ) ( 0x2 << ( ( ssi & 0x3 ) << 3 ) )
#define AUDIO_CLK_SEL_DIVCLKSELSSI_AUDIO_CLKC( ssi ) ( 0x3 << ( ( ssi & 0x3 ) << 3 ) )
#define AUDIO_CLK_SEL_DIVCLKSELSSI_MLPCLK( ssi )     ( 0x4 << ( ( ssi & 0x3 ) << 3 ) )


typedef struct {
    volatile uint32_t bbra;
    volatile uint32_t bbrb;
    volatile uint32_t ssickr;
    volatile uint32_t audio_clk_sel0;
    volatile uint32_t audio_clk_sel1;
    volatile uint32_t audio_clk_sel2;
    volatile uint32_t dummy[6];
    volatile uint32_t div_en;
    volatile uint32_t srcin_timsel0;
    volatile uint32_t srcin_timsel1;
    volatile uint32_t srcin_timsel2;
    volatile uint32_t srcin_timsel3;
    volatile uint32_t srcin_timsel4;
    volatile uint32_t srcout_timsel0;
    volatile uint32_t srcout_timsel1;
    volatile uint32_t srcout_timsel2;
    volatile uint32_t srcout_timsel3;
    volatile uint32_t srcout_timsel4;
    volatile uint32_t cmdout_timsel;
    volatile uint32_t dtcp_timsel;
} adg_hw_reg_t;

static adg_hw_reg_t *adg = MAP_FAILED;

int adg_init()
{
    adg = (adg_hw_reg_t *)ado_device_mmap (RCAR_ADG_BASE, sizeof(adg_hw_reg_t));
    if (adg == MAP_FAILED ) {
        ado_error ("adg_init: ADG register map failed");
        return ENOMEM;
    }

    return EOK;
}

void adg_deinit()
{
    if (adg != MAP_FAILED ) {
        ado_device_munmap(adg, sizeof(adg_hw_reg_t));
        adg = MAP_FAILED;
    }
}

int adg_set_clk(uint32_t ssi_channel, uint32_t clk) {
    uint32_t clk_sel = 0;

    switch( clk ) {
        case AUDIO_CLKA:
            clk_sel = AUDIO_CLK_SEL_DIVCLKSELSSI_AUDIO_CLKA(ssi_channel);
            break;
        case AUDIO_CLKB:
            clk_sel = AUDIO_CLK_SEL_DIVCLKSELSSI_AUDIO_CLKB(ssi_channel);
            break;
        case AUDIO_CLKC:
            clk_sel = AUDIO_CLK_SEL_DIVCLKSELSSI_AUDIO_CLKC(ssi_channel);
            break;
        default:
            ado_error("adg_sel_clk: Invalid clock %d", clk);
            return EINVAL;
    }

    ado_debug (DB_LVL_DRIVER, "Setting SSI%d Clock Select to %u", ssi_channel, clk);

    switch( ssi_channel ){
        case 0: case 1: case 2: case 3:
            adg->audio_clk_sel0 &= ~AUDIO_CLK_SEL_DIVCLKSELSSI_MASK( ssi_channel );
            adg->audio_clk_sel0 |= clk_sel;
            break;
        case 4: case 5: case 6: case 7:
            adg->audio_clk_sel1 &= ~AUDIO_CLK_SEL_DIVCLKSELSSI_MASK( ssi_channel );
            adg->audio_clk_sel1 |= clk_sel;
            break;
        case 9:
            adg->audio_clk_sel2 &= ~AUDIO_CLK_SEL_DIVCLKSELSSI_MASK( ssi_channel );
            adg->audio_clk_sel2 |= clk_sel;
            break;
        case 8: default:
            ado_error ("adg_sel_clk: Invalid ssi_channel %d", ssi_channel);
            return EINVAL;
    }

    return EOK;
}

int adg_set_divisor(uint32_t ssi_channel, uint32_t divisor)
{
    uint32_t divsel = 0;

    switch (divisor) {
        case 1:
            divsel = AUDIO_CLK_SEL_DIVSELSSI_DIV1( ssi_channel );
            break;

        case 2:
            divsel = AUDIO_CLK_SEL_DIVSELSSI_DIV2( ssi_channel );
            break;

        case 4:
            divsel = AUDIO_CLK_SEL_DIVSELSSI_DIV4( ssi_channel );
            break;

        case 8:
            divsel = AUDIO_CLK_SEL_DIVSELSSI_DIV8( ssi_channel );
            break;

        case 16:
            divsel = AUDIO_CLK_SEL_DIVSELSSI_DIV16( ssi_channel );
            break;

        case 32:
            divsel = AUDIO_CLK_SEL_DIVSELSSI_DIV32( ssi_channel );
            break;

        default:
            ado_error ("adg_set_divisor: Invalid divisor %d", divisor);
            return EINVAL;
    }

    ado_debug (DB_LVL_DRIVER, "Setting ADG divisor to %u, %d", divisor, divsel);

    switch( ssi_channel ) {
        case 0: case 1: case 2: case 3:
            adg->audio_clk_sel0 &= ~AUDIO_CLK_SEL_DIVSELSSI_MASK( ssi_channel );
            adg->audio_clk_sel0 |= divsel;
            break;
        case 4: case 5: case 6: case 7:
            adg->audio_clk_sel1 &= ~AUDIO_CLK_SEL_DIVSELSSI_MASK( ssi_channel );
            adg->audio_clk_sel1 |= divsel;
            break;
        case 9:
            adg->audio_clk_sel2 &= ~AUDIO_CLK_SEL_DIVSELSSI_MASK( ssi_channel );
            adg->audio_clk_sel2 |= divsel;
            break;
        case 8: default:
            ado_error ("adg_set_divisor: Invalid ssi_channel %d", ssi_channel);
            return EINVAL;
    }

    return EOK;
}

void adg_register_dump()
{
    if( adg == MAP_FAILED ) {
        ado_error ("adg_register_dump: memory not mapped");
        return;
    }

    ado_debug( DB_LVL_DRIVER, "ADG reg dump: BBRA=%x BBRB=%x SSICKR=%x DIV_EN=%x",
               adg->bbra, adg->bbrb, adg->ssickr, adg->div_en );
    ado_debug( DB_LVL_DRIVER, "ADG reg dump: AUDIO_CLK_SEL0=%x AUDIO_CLK_SEL1=%x AUDIO_CLK_SEL2=%x",
               adg->audio_clk_sel0, adg->audio_clk_sel1, adg->audio_clk_sel2 );
    ado_debug( DB_LVL_DRIVER, "ADG reg dump: SRCIN_TIMSEL=%x %x %x %x %x",
               adg->srcin_timsel0, adg->srcin_timsel1, adg->srcin_timsel2,
               adg->srcin_timsel3, adg->srcin_timsel4 );
    ado_debug( DB_LVL_DRIVER, "ADG reg dump: SRCOUT_TIMSEL=%x %x %x %x %x",
               adg->srcout_timsel0, adg->srcout_timsel1, adg->srcout_timsel2,
               adg->srcout_timsel3, adg->srcout_timsel4 );
    ado_debug( DB_LVL_DRIVER, "ADG reg dump: CMDOUT_TIMSEL=%x DTCP_TIMSEL=%x",
               adg->cmdout_timsel, adg->dtcp_timsel );
}

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/hardware/deva/ctrl/rcar/adg.c $ $Rev: 812827 $")
#endif
