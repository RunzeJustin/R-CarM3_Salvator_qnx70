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

#include "ravb.h"

#if (_NTO_VERSION < 660)
#include <drvr/mdi.h>
#else
#include <netdrvr/mdi.h>
#endif

/* MDIO bus is 2.5MHz so clock is 200ns high + 200ns low */
#define PHY_DELAY() nanospin_ns(200)

static inline void ravb_mdio_idle(ravb_dev_t *ravb)
{
    out32(ravb->base + PIR, 0);
    PHY_DELAY();
}

static inline void ravb_mdio_release_bus(ravb_dev_t *ravb)
{
    out32(ravb->base + PIR, 0);
    out32(ravb->base + PIR, PIR_MDC);
    PHY_DELAY();
    out32(ravb->base + PIR, 0);
    PHY_DELAY();
}

static inline void ravb_mdio_write_bit(ravb_dev_t *ravb, uint8_t data)
{
    out32(ravb->base + PIR, PIR_MMD | (data ? PIR_MDO : 0));
    out32(ravb->base + PIR, PIR_MMD | PIR_MDC | (data ? PIR_MDO : 0));
    PHY_DELAY();
    out32(ravb->base + PIR, PIR_MMD | (data ? PIR_MDO : 0));
    PHY_DELAY();
}

static uint8_t ravb_mdio_read_bit(ravb_dev_t *ravb)
{
    uint32_t val;

    out32(ravb->base + PIR, PIR_MDC);
    PHY_DELAY();
    out32(ravb->base + PIR, 0);
    PHY_DELAY();
    val = in32(ravb->base + PIR);

    return ((val & PIR_MDI) ? 1 : 0);
}

static uint16_t ravb_mii_read(void *handle, uint8_t phy_id, uint8_t location)
{
    ravb_dev_t      *ravb;
    int             i;
    uint16_t        val;
    int             retries;

    ravb = handle;

    for (retries = 0; retries < 3; retries++) {
        /* Preamble */
        for (i = 0; i < 32; i++) {
            ravb_mdio_write_bit(ravb, 1);
        }

        /* Start */
        ravb_mdio_write_bit(ravb, 0);
        ravb_mdio_write_bit(ravb, 1);

        /* Read */
        ravb_mdio_write_bit(ravb, 1);
        ravb_mdio_write_bit(ravb, 0);

        /* PHY Address */
        for (i = 4; i >=0; i--) {
            ravb_mdio_write_bit(ravb, (phy_id >> i) & 1);
        }

        /* Register */
        for (i = 4; i >=0; i--) {
            ravb_mdio_write_bit(ravb, (location >> i) & 1);
        }

        /* Turnaround */
        ravb_mdio_release_bus(ravb);

        /* Data */
        val = 0;
        for (i = 15; i >=0; i--) {
            val |= (ravb_mdio_read_bit(ravb) << i);
        }

        if (val != 0xffff) {
            break;
        }
    }

    if (ravb->cfg.verbose & VERBOSE_PHY) {
        slogf(_SLOGC_NETWORK, _SLOG_ERROR,
              "ravb: MDIO Read Phy %d Reg 0x%x Data 0x%x",
              phy_id, location, val);
    }

    return (val);
}

static void ravb_mii_write(void *handle, uint8_t phy_id, uint8_t location,
                            uint16_t data)
{
    ravb_dev_t      *ravb;
    int             i;

    ravb = handle;

    /* Preamble */
    for (i = 0; i < 32; i++) {
        ravb_mdio_write_bit(ravb, 1);
    }

    /* Start */
    ravb_mdio_write_bit(ravb, 0);
    ravb_mdio_write_bit(ravb, 1);

    /* Write */
    ravb_mdio_write_bit(ravb, 0);
    ravb_mdio_write_bit(ravb, 1);

    /* PHY Address */
    for (i = 4; i >=0; i--) {
        ravb_mdio_write_bit(ravb, (phy_id >> i) & 1);
    }

    /* Register */
    for (i = 4; i >=0; i--) {
        ravb_mdio_write_bit(ravb, (location >> i) & 1);
    }

    /* Turnaround */
    ravb_mdio_write_bit(ravb, 1);
    ravb_mdio_write_bit(ravb, 0);

    /* Data */
    for (i = 15; i >=0; i--) {
        ravb_mdio_write_bit(ravb, (data >> i) & 1);
    }

    ravb_mdio_idle(ravb);

    if (ravb->cfg.verbose & VERBOSE_PHY) {
    slogf(_SLOGC_NETWORK, _SLOG_ERROR,
          "ravb: MDIO Write Phy %d Reg 0x%x Data 0x%x",
          phy_id, location, data);
    }
}

static void ravb_mii_callback(void *handle, uint8_t phyaddr, uint8_t linkstate)
{
    ravb_dev_t      *ravb;
    struct ifnet    *ifp;
    nic_config_t    *cfg;
    char            *s, *f;
    int             i, mode;
    uint16_t        lpadvert;
    uint32_t        val, val1;

    ravb = handle;
    cfg = &ravb->cfg;
    ifp = &ravb->ecom.ec_if;

    switch (linkstate) {
        case MDI_LINK_UP:
            /* Link came up find out what speed duplex etc */
            i = MDI_GetActiveMedia(ravb->mdi, cfg->phy_addr, &mode);
            if (i != MDI_LINK_UP) {
                slogf(_SLOGC_NETWORK, _SLOG_ERROR,
                      "ravb:Link up but unknown media, ignoring");
                return;
            }

            switch (mode) {
                case MDI_100bT:
                    cfg->media_rate = 100000L;
                    cfg->duplex = 0;
                    s = "100baseTX";
                    break;

                case MDI_100bTFD:
                    cfg->media_rate = 100000L;
                    cfg->duplex = 1;
                    s = "100baseTX full-duplex";
                    break;

                case MDI_1000bT:
                    cfg->media_rate = 1000000L;
                    cfg->duplex = 0;
                    s = "1000baseT";
                    break;

                case MDI_1000bTFD:
                    cfg->media_rate = 1000000L;
                    cfg->duplex = 1;
                    s = "1000baseT full-duplex";
                    break;

                default:
                    slogf(_SLOGC_NETWORK, _SLOG_ERROR,
                          "ravb:Ignoring unknown media or RAVB does not supported");
                    return;
            }

            /* Sort out flow control */
            ravb->flow_status = 0;
            f = "";
            if (cfg->duplex == 1) {
                switch (ravb->set_flow) {
                    case -1:
                        /* Flow set to autoneg, check the partner */
                        lpadvert = ravb_mii_read(ravb, cfg->phy_addr,
                                                  MDI_ANLPAR);
                        if (lpadvert & MDI_FLOW) {
                            ravb->flow_status = IFM_FLOW;
                            f = "flowcontrol";
                        }
                        else if (lpadvert & MDI_FLOW_ASYM) {
                            ravb->flow_status = IFM_ETH_RXPAUSE;
                            f = "rxpause";
                        }
                        break;

                    case 1:
                        ravb->flow_status = IFM_FLOW;
                        f = "flowcontrol";
                        break;

                    case 2:
                        ravb->flow_status = IFM_ETH_RXPAUSE;
                        f = "rxpause";
                        break;
                    case 3:
                        ravb->flow_status = IFM_ETH_TXPAUSE;
                        f = "txpause";
                        break;
                }
            }

            if (cfg->verbose) {
                slogf(_SLOGC_NETWORK, _SLOG_ERROR, "ravb: %s Link up %s %s",
                      ifp->if_xname, s, f);
            }

            cfg->flags &= ~NIC_FLAG_LINK_DOWN;
            if_link_state_change(ifp, LINK_STATE_UP);

            /* Program speed/duplex/flowcontrol settings in to the MAC */
            val = in32(ravb->base + ECMR);
            val &= ~(ECMR_RE | ECMR_TE);
            out32(ravb->base + ECMR, val);

            val1 = in32(ravb->base + GECMR);

            if (cfg->media_rate == 1000000L) {
                val1 |= GECMR_SPEED_1000;
            }
            else {
                val1 &= ~GECMR_SPEED_1000;
            }
            out32(ravb->base + GECMR, val1);

            if (cfg->duplex) {
                val |= ECMR_DM;
            }
            else {
                val &= ~ECMR_DM;
            }

            val &= ~(ECMR_RXF | ECMR_TXF);
            if (ravb->flow_status == IFM_FLOW) {
                val |= (ECMR_RXF | ECMR_TXF);
            }
            else if (ravb->flow_status == IFM_ETH_RXPAUSE) {
                val |= ECMR_TXF;
            }
            else if (ravb->flow_status == IFM_ETH_TXPAUSE) {
                val |= ECMR_RXF;
            }

            val |= (ECMR_RE | ECMR_TE);
            out32(ravb->base + ECMR, val);
            break;

        case MDI_LINK_DOWN:
            if (cfg->verbose) {
                slogf(_SLOGC_NETWORK, _SLOG_ERROR, "ravb: %s Link down",
                      ifp->if_xname);
            }
            cfg->media_rate = cfg->duplex = -1;
            cfg->flags |= NIC_FLAG_LINK_DOWN;
            MDI_AutoNegotiate(ravb->mdi, cfg->phy_addr, NoWait);
            if_link_state_change(ifp, LINK_STATE_DOWN);
            break;

        default:
            slogf(_SLOGC_NETWORK, _SLOG_ERROR, "ravb: Unknown linkstate %d",
                  linkstate);
            break;
    }
}

void ravb_MDI_MonitorPhy(void *arg)
{
    ravb_dev_t      *ravb;
    nic_config_t    *cfg;

    ravb = arg;
    cfg = &ravb->cfg;

    if (((cfg->flags & NIC_FLAG_LINK_DOWN) != 0) || !ravb->pkts_received) {
        if (cfg->verbose & VERBOSE_PHY) {
            slogf(_SLOGC_NETWORK, _SLOG_ERROR, "ravb: Check link state");
        }
        MDI_MonitorPhy(ravb->mdi);
    }

    ravb->pkts_received = 0;

    /*
     * With 32 bit counters and 100Mb/s max pps the stats max out in 8 hours.
     * Without an interrupt to warn of approaching max, we need to poll
     * faster than that. We already have a 3 second poller here and the
     * poll of the stats is lightweight, 16 reads and writes, so we poll
     * here rather than setting up a new poller.
     */
    ravb_update_stats(ravb);

    callout_msec(&ravb->mii_callout, 3 * 1000, ravb_MDI_MonitorPhy, arg);
}

static void ravb_mediastatus(struct ifnet *ifp, struct ifmediareq *ifmr)
{
    ravb_dev_t      *ravb;

    ravb = ifp->if_softc;

    ravb->bsd_mii.mii_media_active = IFM_ETHER;
    ravb->bsd_mii.mii_media_status = IFM_AVALID;

    if ((ravb->cfg.flags & NIC_FLAG_LINK_DOWN) != 0) {
        ravb->bsd_mii.mii_media_active |= IFM_NONE;
    } else {
        ravb->bsd_mii.mii_media_status |= IFM_ACTIVE;

        switch (ravb->cfg.media_rate) {
            case 100000L:
                ravb->bsd_mii.mii_media_active |= IFM_100_TX;
                break;

            case 1000000L:
                ravb->bsd_mii.mii_media_active |= IFM_1000_T;
                break;

            default:
                slogf(_SLOGC_NETWORK, _SLOG_ERROR,
                      "ravb: Unknown media, forcing none");
            /* Fallthrough */
            case 0:
                ravb->bsd_mii.mii_media_active |= IFM_NONE;
                break;
        }

        if (ravb->cfg.duplex) {
            ravb->bsd_mii.mii_media_active |= IFM_FDX;
        }

        ravb->bsd_mii.mii_media_active |= ravb->flow_status;
    }

    /* Return the data */
    ifmr->ifm_status = ravb->bsd_mii.mii_media_status;
    ifmr->ifm_active = ravb->bsd_mii.mii_media_active;
}

int ravb_mediachange(struct ifnet *ifp)
{
    ravb_dev_t      *ravb;
    struct ifmedia  *ifm;
    nic_config_t    *cfg;
    int             an_media;

    if (!(ifp->if_flags & IFF_UP)) {
        return 0;
    }

    ravb = ifp->if_softc;
    ifm = &ravb->bsd_mii.mii_media;
    cfg = &ravb->cfg;

    /* Media is changing so link will be down until autoneg completes */
    cfg->flags |= NIC_FLAG_LINK_DOWN;
    if_link_state_change(ifp, LINK_STATE_DOWN);

    switch (ifm->ifm_media & IFM_TMASK) {
        case IFM_NONE:
            ravb->set_speed = 0;
            ravb->set_duplex = 0;
            ravb->set_flow = 0;

            /* Special case, shut down the PHY and bail out */
            callout_stop(&ravb->mii_callout);
            MDI_DisableMonitor(ravb->mdi);
            MDI_PowerdownPhy(ravb->mdi, cfg->phy_addr);
            cfg->flags |= NIC_FLAG_LINK_DOWN;
            if_link_state_change(ifp, LINK_STATE_DOWN);
            return 0;

        case IFM_AUTO:
            ravb->set_speed = -1;
            ravb->set_duplex = -1;
            ravb->set_flow = -1;

            MDI_GetMediaCapable(ravb->mdi, cfg->phy_addr, &an_media);
            /* Enable Pause in autoneg */
            if ((ravb->mdi->PhyData[ cfg->phy_addr]->VendorOUI == KENDIN) &&
            (ravb->mdi->PhyData[ cfg->phy_addr]->Model == KSZ9031)) {
                /* Bug in KSZ9031 PHY */
                an_media |= MDI_FLOW;
            } else {
                an_media |= MDI_FLOW | MDI_FLOW_ASYM;
            }
            break;

        case IFM_100_TX:
            ravb->set_speed = 100000L;
            ravb->set_duplex = 0;
            ravb->set_flow = 0;

            if ((ifm->ifm_media & IFM_FDX) == 0) {
                an_media = MDI_100bT;
            }
            else {
                ravb->set_duplex = 1;
                an_media = MDI_100bTFD;
                if (ifm->ifm_media & IFM_FLOW) {
                    ravb->set_flow = 1;
                    an_media |= MDI_FLOW;
                }
                else if (ifm->ifm_media & IFM_ETH_RXPAUSE) {
                    ravb->set_flow = 2;
                    an_media |= MDI_FLOW | MDI_FLOW_ASYM;
                }
                else if (ifm->ifm_media & IFM_ETH_TXPAUSE) {
                    ravb->set_flow = 3;
                    an_media |= MDI_FLOW_ASYM;
                }
            }
            break;

        case IFM_1000_T:
            ravb->set_speed = 1000000L;
            ravb->set_duplex = 0;
            ravb->set_flow = 0;

            if ((ifm->ifm_media & IFM_FDX) == 0) {
                an_media = MDI_1000bT;
            }
            else {
                ravb->set_duplex = 1;
                an_media = MDI_1000bTFD;
                if (ifm->ifm_media & IFM_FLOW) {
                    ravb->set_flow = 1;
                    an_media |= MDI_FLOW;
                }
                else if (ifm->ifm_media & IFM_ETH_RXPAUSE) {
                    ravb->set_flow = 2;
                    an_media |= MDI_FLOW | MDI_FLOW_ASYM;
                }
                else if (ifm->ifm_media & IFM_ETH_TXPAUSE) {
                    ravb->set_flow = 3;
                    an_media |= MDI_FLOW_ASYM;
                }
            }
            break;

        default:
            slogf(_SLOGC_NETWORK, _SLOG_ERROR, "ravb: Unknown media type");
            return 1;
    }

    MDI_PowerupPhy(ravb->mdi, cfg->phy_addr);
    MDI_EnableMonitor(ravb->mdi, 0);
    MDI_SetAdvert(ravb->mdi, cfg->phy_addr, an_media);
    MDI_AutoNegotiate(ravb->mdi, cfg->phy_addr, NoWait);
    callout_msec(&ravb->mii_callout, 3 * 1000, ravb_MDI_MonitorPhy, ravb);

    return 0;
}

static void ravb_mediainit(ravb_dev_t *ravb)
{
    nic_config_t    *cfg;
    struct ifmedia  *ifm;

    cfg = &ravb->cfg;
    ifm = &ravb->bsd_mii.mii_media;

    ravb->bsd_mii.mii_ifp = &ravb->ecom.ec_if;

    ifmedia_init(ifm, IFM_IMASK, ravb_mediachange, ravb_mediastatus);

    ifmedia_add(ifm, IFM_ETHER | IFM_NONE, 0, NULL);
    ifmedia_add(ifm, IFM_ETHER | IFM_AUTO, 0, NULL);

    ifmedia_add(ifm, IFM_ETHER | IFM_100_TX | IFM_FDX, 0, NULL);
    ifmedia_add(ifm, IFM_ETHER | IFM_100_TX | IFM_FDX | IFM_ETH_TXPAUSE, 0, NULL);
    ifmedia_add(ifm, IFM_ETHER | IFM_100_TX | IFM_FDX | IFM_ETH_RXPAUSE, 0, NULL);
    ifmedia_add(ifm, IFM_ETHER | IFM_100_TX | IFM_FDX | IFM_FLOW, 0, NULL);

    ifmedia_add(ifm, IFM_ETHER | IFM_1000_T | IFM_FDX, 0, NULL);
    ifmedia_add(ifm, IFM_ETHER | IFM_1000_T | IFM_FDX | IFM_FLOW, 0, NULL);
    ifmedia_add(ifm, IFM_ETHER | IFM_1000_T | IFM_FDX | IFM_ETH_TXPAUSE, 0, NULL);
    ifmedia_add(ifm, IFM_ETHER | IFM_1000_T | IFM_FDX | IFM_ETH_RXPAUSE, 0, NULL);

    /*
     * nic_parse_options() sets speed / duplex in cfg but those are for
     * reporting state. Copy them across to the right place.
     */
    ravb->set_speed = cfg->media_rate;
    ravb->set_duplex = cfg->duplex;
    cfg->media_rate = 0;
    cfg->duplex = 0;

    switch (ravb->set_speed) {
        case -1:
            ifm->ifm_media = IFM_ETHER | IFM_AUTO;
            break;

        case 100 *1000L:
            ifm->ifm_media = IFM_ETHER | IFM_100_TX;
            if (ravb->set_duplex != 0) {
                ifm->ifm_media |= IFM_FDX;
                switch (ravb->set_flow) {
                    case 1:
                        ifm->ifm_media |= IFM_FLOW;
                        break;
                    case 2:
                        ifm->ifm_media |= IFM_ETH_RXPAUSE;
                        break;
                    case 3:
                        ifm->ifm_media |= IFM_ETH_TXPAUSE;
                        break;
                    default:
                        break;
                }
            }
            break;

        case 1000 * 1000L:
            ifm->ifm_media = IFM_ETHER | IFM_1000_T;
            if (ravb->set_duplex != 0) {
                ifm->ifm_media |= IFM_FDX;
                switch (ravb->set_flow) {
                    case 1:
                        ifm->ifm_media |= IFM_FLOW;
                        break;
                    case 2:
                        ifm->ifm_media |= IFM_ETH_RXPAUSE;
                        break;
                    case 3:
                        ifm->ifm_media |= IFM_ETH_TXPAUSE;
                        break;
                    default:
                        break;
                }
            }
            break;

        default:
            slogf(_SLOGC_NETWORK, _SLOG_ERROR,
                  "ravb: Unknown initial media, forcing none");
        /* Fallthrough */

        case 0:
            ifm->ifm_media = IFM_ETHER | IFM_NONE;
            break;
    }

    ifmedia_set(ifm, ifm->ifm_media);
}

int ravb_phy_init(ravb_dev_t *ravb)
{
    nic_config_t    *cfg;
    struct ifnet    *ifp;
    int             rc;

    cfg = &ravb->cfg;
    ifp = &ravb->ecom.ec_if;

    rc = MDI_Register_Extended(ravb, ravb_mii_write,
                ravb_mii_read, ravb_mii_callback, &ravb->mdi, NULL, 0, 0);
    if (rc != MDI_SUCCESS) {
        slogf(_SLOGC_NETWORK, _SLOG_ERROR,
              "ravb: Failed to register with MDI");
        return (ENODEV);
    }

    for (cfg->phy_addr = 0; cfg->phy_addr < 32; cfg->phy_addr++) {
        if (MDI_FindPhy (ravb->mdi, cfg->phy_addr) == MDI_SUCCESS) {
            if (cfg->verbose) {
                slogf(_SLOGC_NETWORK, _SLOG_ERROR, "ravb: PHY at address %d",
                      cfg->phy_addr);
            }
            break;
        }
    }
    if (cfg->phy_addr == 32) {
        slogf(_SLOGC_NETWORK, _SLOG_ERROR, "Failed to find PHY");
        return ENODEV;
    }

    rc = MDI_InitPhy(ravb->mdi, cfg->phy_addr);
    if (rc != MDI_SUCCESS) {
        slogf(_SLOGC_NETWORK, _SLOG_ERROR, "ravb: Failed to init the PHY");
        return ENODEV;
    }
    MDI_ResetPhy(ravb->mdi, cfg->phy_addr, WaitBusy);
    MDI_PowerdownPhy(ravb->mdi, cfg->phy_addr);
    cfg->flags |= NIC_FLAG_LINK_DOWN;
    if_link_state_change(ifp, LINK_STATE_DOWN);

    ravb_mediainit(ravb);

    return EOK;
}

void ravb_phy_fini(ravb_dev_t *ravb)
{
    ifmedia_delete_instance(&ravb->bsd_mii.mii_media, IFM_INST_ANY);

    MDI_DeRegister(&ravb->mdi);
    ravb->mdi = NULL;
}

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/hardware/devnp/ravb/phy.c $ $Rev: 813325 $")
#endif
