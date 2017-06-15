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

#include <net/ifdrvcom.h>
#include <sys/sockio.h>

void ravb_update_stats (ravb_dev_t *ravb)
{
    nic_ethernet_stats_t        *estats;

    estats = &ravb->stats.un.estats;
    /*
     * HW counters do not clear on read and do not rollover.
     * We read the hardware and accumulate in to the software stat,
     * and then write back a 0 to reset the hardware stat. This has the
     * risk of missing counts that happen between the read and write back,
     * but it is the best we can do with this hardware design.
     */

    /* CEFCR */
    estats->fcs_errors += in32(ravb->base + CEFCR);
    out32(ravb->base + CEFCR, 0);
    /* FRECR */
    estats->symbol_errors += in32(ravb->base + FRECR);
    out32(ravb->base + FRECR, 0);
    /* TSFRCR */
    estats->short_packets += in32(ravb->base + TSFRCR);
    out32(ravb->base + TSFRCR, 0);
    /* TLFRCR */
    estats->oversized_packets += in32(ravb->base + TLFRCR);
    out32(ravb->base + TLFRCR, 0);

    /* MAFCR is multicast rx which is already counted in ravb_receive() */
}

void ravb_clear_stats (ravb_dev_t *ravb)
{
    /* Clear the counters in hw as part of reading them */
    ravb_update_stats(ravb);

    /* Now clear counters in our data structure */
    memset(&ravb->stats, 0, sizeof(ravb->stats));

    /* Reset stats info for devctl */
    ravb->stats.revision = NIC_STATS_REVISION;

    ravb->stats.valid_stats =
    NIC_STAT_TXED_MULTICAST | NIC_STAT_RXED_MULTICAST |
    NIC_STAT_TXED_BROADCAST | NIC_STAT_RXED_BROADCAST |
    NIC_STAT_TX_FAILED_ALLOCS | NIC_STAT_RX_FAILED_ALLOCS;

    ravb->stats.un.estats.valid_stats =
    NIC_ETHER_STAT_FCS_ERRORS |
    NIC_ETHER_STAT_SYMBOL_ERRORS |
    NIC_ETHER_STAT_OVERSIZED_PACKETS |
    NIC_ETHER_STAT_SHORT_PACKETS;
}

int ravb_ioctl (struct ifnet *ifp, unsigned long cmd, caddr_t data)
{
    ravb_dev_t              *ravb;
    int                     error;
    struct ifdrv_com        *ifdc;
    struct drvcom_config    *dcfgp;
    struct drvcom_stats     *dstp;
    struct ifreq            *ifr;

    ravb = ifp->if_softc;
    error = EOK;

    switch (cmd) {
    case SIOCGDRVCOM:
    ifdc = (struct ifdrv_com *)data;
    switch (ifdc->ifdc_cmd) {
    case DRVCOM_CONFIG:
        dcfgp = (struct drvcom_config *)ifdc;

        if (ifdc->ifdc_len != sizeof(nic_config_t)) {
        error = EINVAL;
        break;
        }
        memcpy(&dcfgp->dcom_config, &ravb->cfg, sizeof(ravb->cfg));
        break;

    case DRVCOM_STATS:
        dstp = (struct drvcom_stats *)ifdc;

        if (ifdc->ifdc_len != sizeof(nic_stats_t)) {
        error = EINVAL;
        break;
        }

        ravb_update_stats(ravb);
        memcpy(&dstp->dcom_stats, &ravb->stats,
           sizeof(dstp->dcom_stats));
        break;

    default:
        error = EOPNOTSUPP;
        break;
    }
    break;

    case SIOCSIFMEDIA:
    case SIOCGIFMEDIA:
    ifr = (struct ifreq *)data;
    error = ifmedia_ioctl(ifp, ifr, &ravb->bsd_mii.mii_media, cmd);
    break;

    case SIOCSIFMTU:
    ifr = (struct ifreq *)data;
    /* Hardware only supports 1500 MTU */
    if (ifr->ifr_mtu == ETH_MAX_DATA_LEN) {
        error = EOK;
    } else {
        error = EINVAL;
    }
    break;

    default:
    error = ether_ioctl(ifp, cmd, data);
    if (error == ENETRESET) {
        /* No multicast filtering in hardware to update */
        error = EOK;
    }
    break;
    }
    return error;
}

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/hardware/devnp/ravb/devctl.c $ $Rev: 810496 $")
#endif
