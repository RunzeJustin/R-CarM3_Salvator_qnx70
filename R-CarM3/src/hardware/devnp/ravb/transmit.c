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

#include <bpfilter.h>
#if NBPFILTER > 0
#include <net/bpf.h>
#include <net/bpfdesc.h>
#endif

void ravb_reap_tx (ravb_dev_t *ravb)
{
    uint32_t idx;

    idx = ravb->tx_cidx;
    while ((idx != ravb->tx_pidx) &&
       (ravb->tx_bd[idx].die_dt == DT_FEMPTY))
    {
        if (ravb->cfg.verbose & VERBOSE_TX)
        {
            slogf(_SLOGC_NETWORK, _SLOG_INFO,
              "ravb: Tx reap index %d", idx);
        }

        m_freem(ravb->tx_pkts[idx]);
        ravb->tx_pkts[idx] = NULL;
        idx = (idx + 1) % NUM_TX_DESC;
    }

    ravb->tx_cidx = idx;
}

void ravb_start (struct ifnet *ifp)
{
    ravb_dev_t              *ravb;
    struct nw_work_thread   *wtp;
    struct mbuf             *m, *m2;
    uint32_t                idx, next_idx;

    ravb = ifp->if_softc;
    wtp = WTP;

    ifp->if_flags_tx |= IFF_OACTIVE;

    while (1)
    {
        idx = ravb->tx_pidx;
        next_idx = (idx + 1) % NUM_TX_DESC;

        if (next_idx == ravb->tx_cidx) {
            /* Ran out of Tx descriptors, see if we can free some up */
            ravb_reap_tx(ravb);
            if (next_idx == ravb->tx_cidx) {
                /* Out of Tx descriptors, leave IFF_OACTIVE set */
                NW_SIGUNLOCK_P(&ifp->if_snd_ex, ravb->iopkt, wtp);
                return;
            }
        }

        IFQ_DEQUEUE(&ifp->if_snd, m);
        if (m == NULL) {
            /* Done */
            ifp->if_flags_tx &= ~IFF_OACTIVE;
            NW_SIGUNLOCK_P(&ifp->if_snd_ex, ravb->iopkt, wtp);
            return;
        }

        ifp->if_opackets++;

        if (((ifp->if_flags & IFF_RUNNING) == 0) ||
            ((ravb->cfg.flags & NIC_FLAG_LINK_DOWN) != 0))
        {
            m_freem(m);
            ifp->if_oerrors++;
            ravb->stats.un.estats.no_carrier++;
            continue;
        }

#if NBPFILTER > 0
        /* Pass the packet to any BPF listeners */
        if (ifp->if_bpf) {
            bpf_mtap(ifp->if_bpf, m);
        }
#endif

        /* Hardware cannot do gather DMA */
        if (m->m_next != NULL)
        {
            m2 = m_getcl(M_NOWAIT, MT_DATA, M_PKTHDR);
            if (m2 == NULL)
            {
                m_freem(m);
                ravb->stats.tx_failed_allocs++;
                ifp->if_oerrors++;
                continue;
            }

            m_copydata(m, 0, m->m_pkthdr.len, mtod(m2, caddr_t));
            m2->m_pkthdr.len = m2->m_len = m->m_pkthdr.len;
            m_freem(m);
            m = m2;
        }
        ravb->tx_pkts[idx] = m;
        ravb->tx_bd[idx].ds_tagl = m->m_len;
        ravb->tx_bd[idx].dptr = mbuf_phys(m);
        ravb->tx_bd[idx].die_dt = DT_FSINGLE;
        CACHE_FLUSH(&ravb->cachectl, m->m_data, ravb->tx_bd[idx].dptr,
                        m->m_len);

        if (ravb->cfg.verbose & VERBOSE_TX)
        {
            slogf(_SLOGC_NETWORK, _SLOG_INFO,
              "ravb: Transmit packet length %d at index %d",
              m->m_len, idx);
        }

        ravb->stats.txed_ok++;
        ravb->stats.octets_txed_ok += m->m_len;
        if (m->m_flags & M_MCAST) {
            ifp->if_omcasts++;
            ravb->stats.txed_multicast++;
        }
        if (m->m_flags & M_BCAST) {
            ifp->if_omcasts++;
            ravb->stats.txed_broadcast++;
        }

        ravb->tx_pidx = next_idx;

        /* Restart the transmitter if disabled */
        if (!(in32(ravb->base + TCCR) & TCCR_TSRQ0))
            out32(ravb->base + TCCR, in32(ravb->base + TCCR) | (TCCR_TSRQ0));

        nanospin_ns(500);   // (unknown) if don't have it, transmit will be drop or very slowly.
    }
}

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/hardware/devnp/ravb/transmit.c $ $Rev: 810496 $")
#endif
