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

#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <hw/inout.h>
#include <sys/mman.h>
#include <internal.h>
#include <sys/syspage.h>
#include <inttypes.h>

#ifdef SDIO_HC_RCAR_SDMMC
#include <rcar.h>
#include <arm/r-car-m3.h>

//#define RCAR_SDMMC_DMA_EVENT 1 /* DMA Error events */

static int rcar_sdmmc_reset(sdio_hc_t *hc);

static int rcar_sdmmc_dma_setup(sdio_hc_t *hc, sdio_cmd_t *cmd);
static void rcar_sdmmc_dma_start( sdio_hc_t *hc, sdio_cmd_t *cmd);
static void rcar_sdmmc_dma_cmplt( sdio_hc_t *hc);

static int rcar_sdmmc_intr_event(sdio_hc_t *hc)
{
    rcar_sdmmc_t    *sdmmc;
    sdio_cmd_t      *cmd;
    uint32_t        mask1, mask2;
    uint32_t        stat1, stat2;
    int             cs = CS_CMD_INPROG;

    sdmmc  = (rcar_sdmmc_t *)hc->cs_hdl;

    mask1 = ~sdmmc_read(sdmmc->vbase, MMC_SD_INFO1_MASK);
    mask2 = ~sdmmc_read(sdmmc->vbase, MMC_SD_INFO2_MASK);
    stat1 =  sdmmc_read(sdmmc->vbase, MMC_SD_INFO1) & mask1;
    stat2 =  sdmmc_read(sdmmc->vbase, MMC_SD_INFO2) & mask2;

    /* Clear interrupt status */
    sdmmc_write(sdmmc->vbase, MMC_SD_INFO1, ~stat1);
    sdmmc_write(sdmmc->vbase, MMC_SD_INFO2, ~stat2);

    /*
     * Card insertion and card removal events
     */
    if (stat1 & (SDH_INFO1_INST | SDH_INFO1_RMVL)) {
        sdio_hc_event(hc, HC_EV_CD);
    }

    /* no command ? */
    if ((cmd = hc->wspc.cmd) == NULL)
        return (EOK);

    // Start DMA ?
    if (stat1 & SDH_INFO1_RE) {
        if (cmd->flags & SCF_CTYPE_ADTC) {
            rcar_sdmmc_dma_start(hc, cmd);
        }
    }
    
    /* Check of errors */
    if (stat2 & (SDH_INFO2_ALL_ERR)) {
        uint16_t    ests1, ests2;

        // DMA error processing?
        if ((sdmmc->flags & OF_DMA_ACTIVE) && (cmd->flags & SCF_CTYPE_ADTC)) {
            uint32_t      dma_info1, dma_info2;
            
            dma_info1 = sdmmc_read(sdmmc->vbase, MMC_DM_CM_INFO1);
            dma_info2 = sdmmc_read(sdmmc->vbase, MMC_DM_CM_INFO2);
            
            if(dma_info2 & (DM_INFO2_DTRAN_ERR0 | DM_INFO2_DTRAN_ERR1)) {
                sdio_slogf( _SLOGC_SDIODI, _SLOG_ERROR, hc->cfg.verbosity, 1, "%s: CMD%d DMA error DMA info1: 0x%X DMA info2 0x%X",
                                    __func__, cmd->opcode, dma_info1, dma_info2 );
                // Reset DMA channels
                sdmmc_write(sdmmc->vbase, MMC_DM_CM_RST, sdmmc_read(sdmmc->vbase, MMC_DM_CM_RST)& ~(DM_RST_DTRANRST0 | DM_RST_DTRANRST1));
                delay(1);
                sdmmc_write(sdmmc->vbase, MMC_DM_CM_RST, sdmmc_read(sdmmc->vbase, MMC_DM_CM_RST) | (DM_RST_DTRANRST0 | DM_RST_DTRANRST1));
            }
        }
        
        ests1 = sdmmc_read(sdmmc->vbase, MMC_SD_ERR_STS1);
        ests2 = sdmmc_read(sdmmc->vbase, MMC_SD_ERR_STS2);

        sdio_slogf(_SLOGC_SDIODI, _SLOG_ERROR, hc->cfg.verbosity, 1,
                        "%s, ERROR in HC, CMD%d, %x : %x",
                        __func__, cmd->opcode, sdmmc_read(sdmmc->vbase, MMC_SD_ERR_STS1), sdmmc_read(sdmmc->vbase, MMC_SD_ERR_STS2));

        if (stat2 & SDH_INFO2_DTO)          cs = CS_DATA_TO_ERR;
        if (ests1 & (1 << 11))              cs = CS_DATA_CRC_ERR;
        if (ests1 & (1 << 10))              cs = CS_DATA_CRC_ERR;
        if (ests1 & ((1 << 8) | (1 << 9)))  cs = CS_CMD_CRC_ERR;
        if (ests1 & (1 << 5))               cs = CS_CMD_END_ERR;
        if (ests1 & (1 << 4))               cs = CS_DATA_END_ERR;
        if (ests1 & ((1 << 1) | (1 << 0)))  cs = CS_CMD_IDX_ERR;
        if (ests2 & (1 << 5))               cs = CS_DATA_CRC_ERR;
        if (ests2 & ((1 << 5) | (1 << 4)))  cs = CS_DATA_TO_ERR;
        if (ests2 & ((1 << 1) | (1 << 0)))  cs = CS_CMD_TO_ERR;
        if (!cs)                            cs = CS_CMD_CMP_ERR;
    } else {
        /* End of command */
        if (stat1 & SDH_INFO1_RE) {
            if (!(cmd->flags & SCF_CTYPE_ADTC)) {
                cs = CS_CMD_CMP;

                if ((cmd->flags & SCF_RSP_136)) {
                    uint32_t    *resp = &cmd->rsp[0];
                        resp[0] = sdmmc_read(sdmmc->vbase, MMC_SD_RSP76);
                        resp[1] = sdmmc_read(sdmmc->vbase, MMC_SD_RSP54);
                        resp[2] = sdmmc_read(sdmmc->vbase, MMC_SD_RSP32);
                        resp[3] = sdmmc_read(sdmmc->vbase, MMC_SD_RSP10);

                        resp[0] = (resp[0] << 8) | (resp[1] >> 24);
                        resp[1] = (resp[1] << 8) | (resp[2] >> 24);
                        resp[2] = (resp[2] << 8) | (resp[3] >> 24);
                        resp[3] = (resp[3] << 8);
                } else if ((cmd->flags & SCF_RSP_PRESENT))
                    cmd->rsp[0] = sdmmc_read(sdmmc->vbase, MMC_SD_RSP10);
            }
        } else if (stat1 & SDH_INFO1_AE) {
            /* End of data transfer */
            cs = CS_CMD_CMP;
            cmd->rsp[0] = sdmmc_read(sdmmc->vbase, MMC_SD_RSP10);
        }
    }

    if (stat1 & SDH_INFO1_RMVL)
        cs = CS_CARD_REMOVED;

    if (cs != CS_CMD_INPROG) {

        if (cmd->flags & SCF_CTYPE_ADTC)
            rcar_sdmmc_dma_cmplt(hc);     
		
        sdio_cmd_cmplt(hc, cmd, cs);
    }

    return (EOK);
}

static int rcar_sdmmc_dma_setup(sdio_hc_t *hc, sdio_cmd_t *cmd)
{
    rcar_sdmmc_t    *sdmmc;
    sdio_sge_t      *sgp;
    int              sgc;

    sdmmc = (rcar_sdmmc_t *)hc->cs_hdl;

    sgc = cmd->sgc;
    sgp = cmd->sgl;

    if (!(cmd->flags & SCF_DATA_PHYS)) {
        sdio_vtop_sg(sgp, sdmmc->sgl, sgc, cmd->mhdl);
        sgp = sdmmc->sgl;
    }

    /* Enable read/write by DMA */
    sdmmc_write(sdmmc->vbase, MMC_CC_EXT_MODE, (1 << 8) | (1 << 4) | BUF_ACC_DMAWEN);

    // Set the address mode
    sdmmc_write(sdmmc->vbase, MMC_DM_CM_DTRAN_MODE, ((cmd->flags & SCF_DIR_IN) ? CH_NUM_UPSTREAM : CH_NUM_DOWNSTREAM)
                        | BUS_WID_64BIT | INCREMENT_ADDRESS);
    
    // Set the SDMA address
    if( (sgp->sg_address & 0x7) ) {
        // The DMA has an 8 byte alignment requirement
        sdmmc_write(sdmmc->vbase, MMC_CC_EXT_MODE, (1 << 8) | (1 << 4));
        
        return( EINVAL );
    }
    
    sdmmc_write(sdmmc->vbase, MMC_DM_DTRAN_ADDR, sgp->sg_address);

    return (EOK);
}

static void rcar_sdmmc_dma_start( sdio_hc_t *hc, sdio_cmd_t *cmd )
{
    rcar_sdmmc_t *sdmmc;
#ifdef RCAR_SDMMC_DMA_EVENT
    uint32_t dir;
#endif

    sdmmc = (rcar_sdmmc_t *)hc->cs_hdl;

    // Clear status
    sdmmc_write(sdmmc->vbase, MMC_DM_CM_INFO1, 0x00000000);
    sdmmc_write(sdmmc->vbase, MMC_DM_CM_INFO2, 0x00000000);

#ifdef RCAR_SDMMC_DMA_EVENT	    
    // Unmask DMA interrupt for channel (TX or RX)    
    dir = ( cmd->flags & SCF_DIR_IN ) ? DM_INFO1_DTRAN_END1 : DM_INFO1_DTRAN_END1;
    sdmmc_write(sdmmc->vbase, MMC_DM_CM_INFO2_MASK, sdmmc_read(sdmmc->vbase, MMC_DM_CM_INFO2_MASK)&~dir);
    sdmmc_write(sdmmc->vbase, MMC_DM_CM_INFO1_MASK, sdmmc_read(sdmmc->vbase, MMC_DM_CM_INFO2_MASK)&~dir);
#endif
 
    sdmmc->flags |= OF_DMA_ACTIVE;
    
    // Start DMA
    sdmmc_write(sdmmc->vbase, MMC_DM_CM_DTRAN_CTRL, DM_START);
}
	
static void rcar_sdmmc_dma_cmplt( sdio_hc_t *hc)
{
    rcar_sdmmc_t  *sdmmc = (rcar_sdmmc_t *)hc->cs_hdl;
    
    /* Clear DMA info */
    sdmmc_write(sdmmc->vbase, MMC_DM_CM_INFO1, 0x00000000);
    sdmmc_write(sdmmc->vbase, MMC_DM_CM_INFO2, 0x00000000);
    
    /* The SD_BUF read/write DMA transfer is disabled */
    sdmmc_write(sdmmc->vbase, MMC_CC_EXT_MODE, (1 << 8) | (1 << 4));

#ifdef RCAR_SDMMC_DMA_EVENT	
    // Mask DMA interrupt for channel (TX or RX)    
    sdmmc_write(sdmmc->vbase, MMC_DM_CM_INFO2_MASK, DM_INFO2_DTRAN_ERR1 | DM_INFO2_DTRAN_ERR0);
    sdmmc_write(sdmmc->vbase, MMC_DM_CM_INFO1_MASK, DM_INFO1_DTRAN_END0 | DM_INFO1_DTRAN_END0);
#endif	

    sdmmc->flags &= ~OF_DMA_ACTIVE;
}

static int rcar_sdmmc_dma_event( sdio_hc_t *hc )
{
    rcar_sdmmc_t  *sdmmc;
    sdio_cmd_t    *cmd;
    uint32_t      dm_info1;
    uint32_t      dm_info2;
    int i, status;

    sdmmc = (rcar_sdmmc_t *)hc->cs_hdl;

    for (i = 0; i < SDHI_TMOUT; i++) {
        status =  sdmmc_read(sdmmc->vbase, MMC_SD_INFO2);
        if (!(status & SDH_INFO2_CBSY) && (status & SDH_INFO2_SCLKDIVEN))
            break;
        nanospin_ns(100000);
    }

    if (i >= SDHI_TMOUT) {
        sdio_slogf(_SLOGC_SDIODI, _SLOG_INFO, hc->cfg.verbosity, 1, "%s: Busy state! Cannot enable DMA!", __func__);
        return (EAGAIN);
    }

    dm_info1 = sdmmc_read(sdmmc->vbase, MMC_DM_CM_INFO1);
    dm_info2 = sdmmc_read(sdmmc->vbase, MMC_DM_CM_INFO2);

    sdmmc_write(sdmmc->vbase, MMC_DM_CM_INFO1, 0x00000000);
    sdmmc_write(sdmmc->vbase, MMC_DM_CM_INFO2, 0x00000000);

    sdmmc_write(sdmmc->vbase, MMC_DM_CM_INFO2_MASK, DM_INFO2_DTRAN_ERR0 | DM_INFO2_DTRAN_ERR1);
    sdmmc_write(sdmmc->vbase, MMC_DM_CM_INFO1_MASK, DM_INFO1_DTRAN_END0 | DM_INFO1_DTRAN_END1);

    if( ( cmd = hc->wspc.cmd ) == NULL || !( sdmmc->flags & OF_DMA_ACTIVE ) ) {
        return( EOK );
    }

    sdmmc->flags &= ~OF_DMA_ACTIVE;
    if(dm_info1 & (DM_INFO1_DTRAN_END0 | DM_INFO1_DTRAN_END1)){  // DMA complete or error
        // INFO2: error occured
        if(dm_info2 & (DM_INFO2_DTRAN_ERR0 | DM_INFO2_DTRAN_ERR0)){  // DMA error
            sdio_cmd_cmplt( hc, cmd, CS_CMD_CMP_ERR );  // error end
        }
        else if( sdmmc->cmd->sgc ) {
            rcar_sdmmc_dma_setup( hc, cmd );
            sdmmc->cmd->sgc--;
			sdmmc->cmd->sgl++;
        }
        else {
            if( sdmmc->cs ) {
                sdio_cmd_cmplt( hc, cmd, sdmmc->cs );
            }
        }
    }

    return( EOK );
}

static int rcar_sdmmc_event(sdio_hc_t *hc, sdio_event_t *ev)
{
    rcar_sdmmc_t    *sdmmc = (rcar_sdmmc_t *)hc->cs_hdl;
    int             status = CS_CMD_INPROG;

    switch (ev->code) {
        case HC_EV_INTR:
            status = rcar_sdmmc_intr_event(hc);
            InterruptUnmask(sdmmc->irq, hc->hc_iid);
            break;
        case HC_EV_DMA:
            status = rcar_sdmmc_dma_event(hc);
            InterruptUnmask(sdmmc->dma_irq, sdmmc->dma_iid);
            break;
        default:
            break;
    }

    return (status);
}

static int rcar_sdmmc_xfer_setup(sdio_hc_t *hc, sdio_cmd_t *cmd)
{
    rcar_sdmmc_t    *sdmmc;
    int             status = EOK;
    int             i;

    sdmmc = (rcar_sdmmc_t *)hc->cs_hdl;

    for (i = 0; i < SDHI_TMOUT; i++) {
        status =  sdmmc_read(sdmmc->vbase, MMC_SD_INFO2);
        if (!(status & SDH_INFO2_CBSY) && (status & SDH_INFO2_SCLKDIVEN))
            break;
        nanospin_ns(1000);
    }

    if (i >= SDHI_TMOUT) {
        sdio_slogf(_SLOGC_SDIODI, _SLOG_ERROR, hc->cfg.verbosity, 1,
            "%s: SD: cannot set block size and block count", __func__);
        return (EAGAIN);
    }

    sdio_sg_start(hc, cmd->sgl, cmd->sgc);

    /* block size */
    sdmmc_write(sdmmc->vbase, MMC_SD_SIZE, cmd->blksz);

    /* only valid for multi-block transfer */
    if (cmd->blks > 1)
        sdmmc_write(sdmmc->vbase, MMC_SD_SECCNT, cmd->blks);

    sdmmc->cmd = cmd;

    if (cmd->sgc && (hc->caps & HC_CAP_DMA)) {
        if ((status = rcar_sdmmc_dma_setup(hc, cmd)) == EOK) {
        }
    }

    return (status);
}

static int rcar_sdmmc_cmd(sdio_hc_t *hc, sdio_cmd_t *cmd)
{
    rcar_sdmmc_t    *sdmmc;
    int             status, i;
    uint32_t     command;

    sdmmc = (rcar_sdmmc_t *)hc->cs_hdl;

    for (i = 0; i < SDHI_TMOUT; i++) {
        status =  sdmmc_read(sdmmc->vbase, MMC_SD_INFO2);
        if (!(status & SDH_INFO2_CBSY) && (status & SDH_INFO2_SCLKDIVEN))
            break;
        nanospin_ns(1000);
    }

    if (i >= SDHI_TMOUT) {
        sdio_slogf(_SLOGC_SDIODI, _SLOG_ERROR, hc->cfg.verbosity, 1,
            "%s: SD: CMD%d cannot execute because BUS busy", __func__, cmd->opcode);
        return (EAGAIN);
    }

    /* Clear Status */
    sdmmc_write(sdmmc->vbase, MMC_SD_INFO1, sdmmc_read(sdmmc->vbase, MMC_SD_INFO1) & ~(SDH_INFO1_AE | SDH_INFO1_RE));
    sdmmc_write(sdmmc->vbase, MMC_SD_INFO2, sdmmc_read(sdmmc->vbase, MMC_SD_INFO2) & ~SDH_INFO2_ALL_ERR);

    command = cmd->opcode;
    switch (cmd->flags & (0x1F << 4)) {
        case SCF_RSP_NONE:
            command |= SDH_CMD_NORSP;
            break;
        case SCF_RSP_R1:
            command |= SDH_CMD_RSPR1;
            break;
        case SCF_RSP_R1B:
            command |= SDH_CMD_RSPR1B;
            break;
        case SCF_RSP_R2:
            command |= SDH_CMD_RSPR2;
            break;
        case SCF_RSP_R3:
            command |= SDH_CMD_RSPR3;
            break;
    }

    if (cmd->flags & SCF_CTYPE_ADTC) {
        command |= SDH_CMD_ADTC;
        if (cmd->flags & SCF_DIR_IN)
            command |= SDH_CMD_DAT_READ;
         if (cmd->flags & SCF_MULTIBLK) {
              sdmmc_write(sdmmc->vbase, MMC_SD_STOP, SDH_STOP_SEC);
            command |= SDH_CMD_DAT_MULTI;
            if (!(hc->caps & HC_CAP_ACMD12))
                command |= SDH_CMD_NOAC12;
         } else
            sdmmc_write(sdmmc->vbase, MMC_SD_STOP, 0);

        if ((status = rcar_sdmmc_xfer_setup(hc, cmd)) != EOK)
            return (status);

        /* card insertion/removal are always enabled */
        sdmmc_write(sdmmc->vbase, MMC_SD_INFO1_MASK, ~(SDH_INFO1_AE | SDH_INFO1_RE | SDH_INFO1_RMVL | SDH_INFO1_INST));
    } else
        sdmmc_write(sdmmc->vbase, MMC_SD_INFO1_MASK, ~(SDH_INFO1_RE | SDH_INFO1_RMVL | SDH_INFO1_INST));

    sdmmc_write(sdmmc->vbase, MMC_SD_INFO2_MASK, ~(SDH_INFO2_ALL_ERR));

    sdmmc_write(sdmmc->vbase, MMC_SD_ARG, cmd->arg);

    sdmmc_write(sdmmc->vbase, MMC_SD_CMD, command);

    return (EOK);
}

static int rcar_sdmmc_abort(sdio_hc_t *hc, sdio_cmd_t *cmd)
{
    return (EOK);
}

static int rcar_sdmmc_pwr(sdio_hc_t *hc, int vdd)
{
    hc->vdd = vdd;

    return (EOK);
}

static int rcar_sdmmc_bus_mode(sdio_hc_t *hc, int bus_mode)
{
    hc->bus_mode = bus_mode;

    return (EOK);
}

static int rcar_sdmmc_bus_width(sdio_hc_t *hc, int width)
{
    rcar_sdmmc_t *sdmmc;
    uint32_t    hctl;
    int i, status;

    sdmmc = (rcar_sdmmc_t *)hc->cs_hdl;

    for (i = 0; i < SDHI_TMOUT; i++) {
        status =  sdmmc_read(sdmmc->vbase, MMC_SD_INFO2);
        if (!(status & SDH_INFO2_CBSY) && (status & SDH_INFO2_SCLKDIVEN))
            break;
        nanospin_ns(10000);
    }

    if (i >= SDHI_TMOUT) {
        sdio_slogf(_SLOGC_SDIODI, _SLOG_INFO, hc->cfg.verbosity, 1, "%s: Busy state! Cannot change the bus width!", __func__);
        return (EAGAIN);
    }

    hctl = sdmmc_read(sdmmc->vbase, MMC_SD_OPTION);

    if (width == 8) {
        hctl &= ~(SDH_OPTION_WIDTH_1);
        hctl |=  (SDH_OPTION_WIDTH_8);
    }
    else if (width == 4) {
        hctl &= ~(SDH_OPTION_WIDTH_1 | SDH_OPTION_WIDTH_8);
    }
    else {
        hctl |=  (SDH_OPTION_WIDTH_1);
        hctl &= ~(SDH_OPTION_WIDTH_8);
    }

    sdmmc_write(sdmmc->vbase, MMC_SD_OPTION, hctl);

    hc->bus_width = width;

    return (EOK);
}

static uint8_t clock_div(int hclk, int *clock)
{
    uint32_t clk;
    int      new_clock;

    for (new_clock = hclk/512, clk = 0x80000080; *clock >= (new_clock * 2); clk >>= 1)
        new_clock <<= 1;

    *clock = new_clock;

    if ((clk >> 22) & 1)
        clk |= 0xff;

    return (uint8_t)clk;
}

static int rcar_sdmmc_clk(sdio_hc_t *hc, int clk)
{
    rcar_sdmmc_t  *sdmmc;
    uint8_t       clkctl;
    int           clock;
    uint32_t      info2;
    int           cnt;

    sdmmc = (rcar_sdmmc_t *)hc->cs_hdl;

    for (cnt = SDHI_TMOUT; cnt >= 0; --cnt) {
        info2 = sdmmc_read(sdmmc->vbase, MMC_SD_INFO2);
        if (!(info2 & SDH_INFO2_CBSY) && (info2 & SDH_INFO2_SCLKDIVEN))
            break;

        nanospin_ns(1000);
    }
    if (cnt <= 0) {
        sdio_slogf(_SLOGC_SDIODI, _SLOG_INFO, hc->cfg.verbosity, 1, "%s: Busy state! Cannot change the clock!", __func__);
        return (EAGAIN);
    }

    /* stop clock */
    sdmmc_write(sdmmc->vbase, MMC_SD_CLK_CTRL, sdmmc_read(sdmmc->vbase, MMC_SD_CLK_CTRL) & ~SDH_CLKCTRL_SCLKEN);

    if (clk > hc->clk_max)
        clk = hc->clk_max;

    clock = clk;

    clkctl = clock_div(sdmmc->pclk, &clock);

    sdmmc_write(sdmmc->vbase, MMC_SD_CLK_CTRL, clkctl);

    while ((sdmmc_read(sdmmc->vbase, MMC_SD_INFO2) & SDH_INFO2_SCLKDIVEN) == 0)
        ;

    sdmmc_write(sdmmc->vbase, MMC_SD_CLK_CTRL, sdmmc_read(sdmmc->vbase, MMC_SD_CLK_CTRL) | SDH_CLKCTRL_SCLKEN);

    hc->clk = sdmmc->busclk = clock;

    return (EOK);
}

static int rcar_sdmmc_signal_voltage(sdio_hc_t *hc, int signal_voltage)
{
    return (EOK);
}

static int rcar_sdmmc_timing(sdio_hc_t *hc, int timing)
{
    rcar_sdmmc_t    *sdmmc;

    sdmmc = (rcar_sdmmc_t *)hc->cs_hdl;

    if (timing == TIMING_HS400 )
        sdmmc_write(sdmmc->vbase, MMC_SDIF_MODE, 1);

    hc->timing = timing;

    return (EOK);
}

static int rcar_sdmmc_init_tuning(sdio_hc_t *hc)
{
    rcar_sdmmc_t    *sdmmc;
    int             scc_tapnum;
    int             taps_num;

    sdmmc       = (rcar_sdmmc_t *)hc->cs_hdl;
    scc_tapnum  = 8;

    /* set sampling clock selection range */
    if (scc_tapnum) {
        sdmmc_write(sdmmc->vbase, MMC_SCC_DTCNTL, scc_tapnum << 16);
    }

    /* Initialize SCC */
    sdmmc_write(sdmmc->vbase, MMC_SD_INFO1, 0x0000);
    sdmmc_write(sdmmc->vbase, MMC_SD_INFO2, 0x0000);

    sdmmc_write(sdmmc->vbase, MMC_SCC_DTCNTL, RCAR_SDHI_SCC_DTCNTL_TAPEN | sdmmc_read(sdmmc->vbase, MMC_SCC_DTCNTL));

    sdmmc_write(sdmmc->vbase, MMC_SD_CLK_CTRL, sdmmc_read(sdmmc->vbase, MMC_SD_CLK_CTRL) & ~(1 << 8));

    sdmmc_write(sdmmc->vbase, MMC_SCC_CKSEL, RCAR_SDHI_SCC_CKSEL_DTSEL | sdmmc_read(sdmmc->vbase, MMC_SCC_CKSEL));

    sdmmc_write(sdmmc->vbase, MMC_SD_CLK_CTRL, sdmmc_read(sdmmc->vbase, MMC_SD_CLK_CTRL) | (1 << 8));

    sdmmc_write(sdmmc->vbase, MMC_SCC_RVSCNTL, (1 << 1) | (~RCAR_SDHI_SCC_RVSCNTL_RVSEN & sdmmc_read(sdmmc->vbase, MMC_SCC_RVSCNTL)));

    sdmmc_write(sdmmc->vbase, MMC_SCC_DT2FF, 0x00000300);

    /* Read TAPNUM */
    taps_num = (sdmmc_read(sdmmc->vbase, MMC_SCC_DTCNTL) >> 16) & 0xf;

    return (taps_num);
}

static int rcar_sdmmc_prepare_tuning(sdio_hc_t *hc, uint32_t tap)
{
    rcar_sdmmc_t *sdmmc;

    sdmmc = (rcar_sdmmc_t *)hc->cs_hdl;

    /* Set sampling clock position */
    sdmmc_write(sdmmc->vbase, MMC_SCC_TAPSET, tap);

    return (EOK);
}

static int rcar_sdmmc_select_tuning(sdio_hc_t *hc, int *tap)
{
    rcar_sdmmc_t *sdmmc;
    uint32_t      tap_num;    /* total number of taps */
    uint32_t      tap_cnt;    /* counter of tuning success */
    uint32_t      tap_set;    /* tap position */
    uint32_t      tap_start;  /* start position of tuning success */
    uint32_t      tap_end;    /* end position of tuning success */
    uint32_t      ntap;       /* temporary counter of tuning success */
    int           i;

    sdmmc = (rcar_sdmmc_t *)hc->cs_hdl;

    /* Clear SCC_RVSREQ */
    sdmmc_write(sdmmc->vbase, MMC_SCC_RVSREQ, 0x00000000);

    /* Select SCC */
    tap_num = (sdmmc_read(sdmmc->vbase, MMC_SCC_DTCNTL) >> 16) & 0xf;

    tap_cnt   = 0;
    ntap      = 0;
    tap_start = 0;
    tap_end   = 0;

    for (i = 0; i < tap_num * 2; i++) {
        if (tap[i] == 0)
            ntap++;
        else {
            if (ntap > tap_cnt) {
                tap_start = i - ntap;
                tap_end = i - 1;
                tap_cnt = ntap;
            }
            ntap = 0;
        }
    }

    if (ntap > tap_cnt) {
        tap_start = i - ntap;
        tap_end = i - 1;
        tap_cnt = ntap;
    }

    if (tap_cnt >= RCAR_SDHI_MAX_TAP)
        tap_set = (tap_start + tap_end) / 2 % tap_num;
    else
        return (EIO);

    /* Set SCC */
    sdmmc_write(sdmmc->vbase, MMC_SCC_TAPSET, tap_set);

    /* Enable auto re-tuning */
    sdmmc_write(sdmmc->vbase, MMC_SCC_RVSCNTL, (1 << 1) | RCAR_SDHI_SCC_RVSCNTL_RVSEN | sdmmc_read(sdmmc->vbase, MMC_SCC_RVSCNTL));

    return (EOK);
}

static void rcar_sdmmc_scc_reset(sdio_hc_t *hc)
{
    rcar_sdmmc_t *sdmmc;

    sdmmc = (rcar_sdmmc_t *)hc->cs_hdl;

    /* Reset SCC */
    sdmmc_write(sdmmc->vbase, MMC_SD_CLK_CTRL, sdmmc_read(sdmmc->vbase, MMC_SD_CLK_CTRL) & ~0x0100);

    sdmmc_write(sdmmc->vbase, MMC_SCC_CKSEL, ~RCAR_SDHI_SCC_CKSEL_DTSEL & sdmmc_read(sdmmc->vbase, MMC_SCC_CKSEL));

    sdmmc_write(sdmmc->vbase, MMC_SD_CLK_CTRL, sdmmc_read(sdmmc->vbase, MMC_SD_CLK_CTRL) | 0x0100);

    sdmmc_write(sdmmc->vbase, MMC_SCC_DTCNTL, ~RCAR_SDHI_SCC_DTCNTL_TAPEN & sdmmc_read(sdmmc->vbase, MMC_SCC_DTCNTL));

    sdmmc_write(sdmmc->vbase, MMC_SCC_RVSCNTL, ~RCAR_SDHI_SCC_RVSCNTL_RVSEN & sdmmc_read(sdmmc->vbase, MMC_SCC_RVSCNTL));
}

static int rcar_sdmmc_tune(sdio_hc_t *hc, int op)
{
    struct sdio_cmd *cmd;
    sdio_sge_t      sge;
    uint32_t        *td;
    int             tap[RCAR_SDHI_TUNING_RETRIES];
    int             tlc;
    int             tlen;
    int             status;
    int             val;
    int             tap_num;
    int             timeout;

    /* return if not HS200 or SDR104, and not SDR50 that requires tuning */
    if ((hc->timing != TIMING_SDR104) && (hc->timing != TIMING_HS200) && ((hc->timing == TIMING_SDR50)))
        return (EOK);

    val     = 0;
    tap_num = rcar_sdmmc_init_tuning(hc);
    if (hc->bus_width == 8)
        tlen = 128;
    else
        tlen = 64;

    if ((cmd = sdio_alloc_cmd()) == NULL)
        return (ENOMEM);

    /* on RCAR the driver has to read the tuning data and compare it with the reference data
     * therefore we need a buffer here for the tuning data */
    if ((td = (uint32_t *)sdio_alloc(tlen)) == NULL) {
        sdio_free_cmd(cmd);
        return (ENOMEM);
    }

    tlc     = RCAR_SDHI_TUNING_RETRIES;
    timeout = RCAR_SDHI_TUNING_TIMEOUT;

    do {
        /* clear tuning data buffer to avoid comparing old data after unsuccessful transfer */
        memset(td, 0, tlen);

        rcar_sdmmc_prepare_tuning(hc, val % tap_num);

        if (!tlc && !timeout)
            break;

        /* setup tuning command */
        sdio_setup_cmd(cmd, SCF_CTYPE_ADTC | SCF_RSP_R1, op, 0);
        sge.sg_count = tlen; sge.sg_address = (paddr_t)td;
        sdio_setup_cmd_io(cmd, SCF_DIR_IN, 1, tlen, &sge, 1, NULL);

        if ((status = sdio_issue_cmd(&hc->device, cmd, RCAR_SDHI_TUNING_TIMEOUT)))
            break;

        /* determine largest timing window where data transfer is working */
        if ((cmd->status != CS_CMD_CMP))
            tap[val] = -1;
        else
            tap[val] = 0;

        val++;
        timeout--;
        tlc--;
        delay(1);
    } while ((val < (tap_num * 2)) && (tlc || timeout));

    if (tlc || timeout) {
        status = rcar_sdmmc_select_tuning(hc, tap);
    } else {
        sdio_slogf(_SLOGC_SDIODI, _SLOG_ERROR, hc->cfg.verbosity, 1, "%s: Tuning procedure failed", __func__);
        status = EIO;
    }

    sdio_free(td, tlen);
    sdio_free_cmd(cmd);

    if (status != EOK)
        rcar_sdmmc_scc_reset(hc);

    return (status);
}

static int rcar_sdmmc_cd(sdio_hc_t *hc)
{
    rcar_sdmmc_t *sdmmc;
    int         cstate, pstate;

    sdmmc  = (rcar_sdmmc_t *)hc->cs_hdl;
    cstate = CD_RMV;
    pstate = sdmmc_read(sdmmc->vbase, MMC_SD_INFO1);

    hc->caps |= HC_CAP_CD_INTR;

    if ((pstate & SDH_INFO1_CD)) {
        cstate  |= CD_INS;
        if (!(pstate & SDH_INFO1_WP))
            cstate |= CD_WP;
    }

    return (cstate);
}

static int rcar_sdmmc_dma_init(sdio_hc_t *hc)
{
#ifdef RCAR_SDMMC_DMA_EVENT
    rcar_sdmmc_t  *sdmmc;
#endif
    sdio_hc_cfg_t *cfg;

    cfg   = &hc->cfg;

#ifdef RCAR_SDMMC_DMA_EVENT
    sdmmc = (rcar_sdmmc_t *)hc->cs_hdl;
    /* set defaults */
    sdmmc->dma_iid = -1;
    sdmmc->dma_irq = RCAR_INTCSYS_DMASDHI0 + cfg->idx;

    SIGEV_PULSE_INIT( &sdmmc->dma_ev, hc->hc_coid, SDIO_PRIORITY, HC_EV_DMA, NULL );
    if ((sdmmc->dma_iid = InterruptAttachEvent(sdmmc->dma_irq, &sdmmc->dma_ev, _NTO_INTR_FLAGS_TRK_MSK)) != -1) {
        return( EOK );
    }
    else {
        sdio_slogf(_SLOGC_SDIODI, _SLOG_ERROR, hc->cfg.verbosity, 1, "%s:  InterruptAttach DMA:  (%s)", __FUNCTION__, strerror( errno ) );
    }

    return( errno );
#else
    cfg->sg_max = 1;

    return (EOK);
#endif
}

int rcar_sdmmc_dma_dinit(sdio_hc_t *hc)
{
#ifdef RCAR_SDMMC_DMA_EVENT
    rcar_sdmmc_t  *sdmmc;

    sdmmc = (rcar_sdmmc_t *)hc->cs_hdl;

    if( sdmmc->dma_iid != -1 ) {
        InterruptDetach( sdmmc->dma_iid );
    }
#endif
    return( EOK );
}

int rcar_sdmmc_dinit(sdio_hc_t *hc)
{
    rcar_sdmmc_t *sdmmc;

    if (!hc || !hc->cs_hdl)
        return (EOK);

    sdmmc = (rcar_sdmmc_t *)hc->cs_hdl;

    if (sdmmc->vbase) {
        if (hc->hc_iid != -1)
            InterruptDetach(hc->hc_iid);

        munmap_device_io(sdmmc->vbase, RCAR_MMCIF_SIZE);
    }
#ifdef RCAR_SDMMC_DMA_EVENT
    rcar_sdmmc_dma_dinit(hc);
#endif
    free(sdmmc);
    hc->cs_hdl = NULL;

    return (EOK);
}

static int rcar_sdmmc_reset(sdio_hc_t *hc)
{
    rcar_sdmmc_t    *sdmmc = (rcar_sdmmc_t *)hc->cs_hdl;

    sdmmc_write(sdmmc->vbase, MMC_SOFT_RST, SOFT_RST_ON);
    delay(1);
    sdmmc_write(sdmmc->vbase, MMC_SOFT_RST, SOFT_RST_OFF);
    delay(1);

    sdmmc_write(sdmmc->vbase, MMC_SD_INFO1_MASK, 0x0001031D);  // all mask (0x0000FFFE in HWM)
    sdmmc_write(sdmmc->vbase, MMC_SD_INFO2_MASK, 0x00008B7F);  // all mask (0x00007F80 in HWM)
    sdmmc_write(sdmmc->vbase, MMC_HOST_MODE, 0x00000000);    // SD_BUF access width = 64-bit
    sdmmc_write(sdmmc->vbase, MMC_SD_OPTION, 0x0000C0EE);    // Bus width = 1bit, timeout=MAX
    sdmmc_write(sdmmc->vbase, MMC_SD_CLK_CTRL, 0x00000080);  // Automatic Control=Disable, Clock Output=Disable
    sdmmc_write(sdmmc->vbase, MMC_SDIF_MODE, 0x00000000);
    sdmmc_write(sdmmc->vbase, 0xE4 << 2, 0x00000000);

    sdmmc_write(sdmmc->vbase, MMC_SD_INFO1_MASK,  sdmmc_read(sdmmc->vbase, MMC_SD_INFO1_MASK) & ~(SDH_INFO1_INST | SDH_INFO1_RMVL));

    sdmmc->busclk = 400 * 1000;      // 400KHz clock for ident

    // configure clock
    rcar_sdmmc_clk(hc, sdmmc->busclk);

    // wait 20ms
    delay(20);

    return (EOK);
}

static sdio_hc_entry_t rcar_sdmmc_entry = {
    16,
    rcar_sdmmc_dinit, NULL,
    rcar_sdmmc_cmd, rcar_sdmmc_abort,
    rcar_sdmmc_event, rcar_sdmmc_cd, rcar_sdmmc_pwr,
    rcar_sdmmc_clk, rcar_sdmmc_bus_mode,
    rcar_sdmmc_bus_width, rcar_sdmmc_timing,
    rcar_sdmmc_signal_voltage, NULL, NULL, rcar_sdmmc_tune, NULL
};


int rcar_sdmmc_init(sdio_hc_t *hc)
{
    sdio_hc_cfg_t   *cfg;
    rcar_sdmmc_t    *sdmmc;
    struct sigevent event;

    hc->hc_iid  = -1;
    cfg         = &hc->cfg;

    memcpy(&hc->entry, &rcar_sdmmc_entry, sizeof(sdio_hc_entry_t));

    if ((sdmmc = hc->cs_hdl = calloc(1, sizeof(rcar_sdmmc_t))) == NULL)
        return (ENOMEM);

    if (cfg->base_addrs > 0 && cfg->dma_chnls > 0 && cfg->irqs > 0) {
        sdmmc->pbase = cfg->base_addr[0];
        sdmmc->irq   = cfg->irq[0];
    } else if (cfg->idx == 0) {
        sdmmc->pbase = RCAR_SDHI0_BASE;
        sdmmc->irq   = RCAR_INTCSYS_SDHI0;
    } else if (cfg->idx == 1) {
        sdmmc->pbase = RCAR_SDHI1_BASE;
        sdmmc->irq   = RCAR_INTCSYS_SDHI1;
    } else if (cfg->idx == 2) {
        sdmmc->pbase = RCAR_SDHI2_BASE;
        sdmmc->irq   = RCAR_INTCSYS_SDHI2;
    } else if (cfg->idx == 3) {
        sdmmc->pbase = RCAR_SDHI3_BASE;
        sdmmc->irq   = RCAR_INTCSYS_SDHI3;
    } else {
        rcar_sdmmc_dinit(hc);
        return (ENODEV);
    }

    if ((sdmmc->vbase = (uintptr_t)mmap_device_io(RCAR_MMCIF_SIZE, sdmmc->pbase)) == (uintptr_t)MAP_FAILED) {
        sdio_slogf(_SLOGC_SDIODI, _SLOG_ERROR, hc->cfg.verbosity, 1,
            "%s: MMCIF base mmap_device_io (0x%lx) %s", __FUNCTION__, sdmmc->pbase, strerror(errno));
        rcar_sdmmc_dinit(hc);
        return (errno);
    }

    if (cfg->clk)
        sdmmc->clock = cfg->clk;
    else
        sdmmc->clock = 200 * 1000 * 1000;;

    hc->clk_max = sdmmc->clock;

    sdmmc->pclk = cfg->clk > 0 ? cfg->clk : sdmmc->clock;

    hc->caps |= HC_CAP_BSY | HC_CAP_BW4 | HC_CAP_BW8;
    hc->caps |= HC_CAP_ACMD12;
    hc->caps |= HC_CAP_DMA;
    hc->caps |= HC_CAP_HS | HC_CAP_HS200 | HC_CAP_SDR50 | HC_CAP_SDR104;

    hc->caps &= cfg->caps;      /* reconcile command line options */

    if (rcar_sdmmc_dma_init(hc) != EOK) {
        rcar_sdmmc_dinit(hc);
        return (ENODEV);
    }

    rcar_sdmmc_reset(hc);

    /* we don't want this interrupt at the driver startup */
    while (sdmmc_read(sdmmc->vbase, MMC_SD_INFO1) & SDH_INFO1_INST)
        sdmmc_write(sdmmc->vbase, MMC_SD_INFO1, ~(SDH_INFO1_INST | SDH_INFO1_RMVL));

    sdmmc_write(sdmmc->vbase, MMC_SD_INFO1_MASK,  sdmmc_read(sdmmc->vbase, MMC_SD_INFO1_MASK) & ~(SDH_INFO1_INST | SDH_INFO1_RMVL));

    SIGEV_PULSE_INIT(&event, hc->hc_coid, SDIO_PRIORITY, HC_EV_INTR, NULL);
    if ((hc->hc_iid = InterruptAttachEvent(sdmmc->irq, &event, _NTO_INTR_FLAGS_TRK_MSK)) == -1) {
        rcar_sdmmc_dinit(hc);
        return (errno);
    }

    return (EOK);
}
#endif

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/hardware/devb/sdmmc/sdiodi/hc/rcar.c $ $Rev: 810496 $")
#endif
