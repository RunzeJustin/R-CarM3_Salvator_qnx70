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

/* Error interrupt handler */
static void ravb_error_interrupt(ravb_dev_t *ravb)
{
    uint32_t eis, ris2;

    eis = ravb->eis;
    ris2 = ravb->ris2;

    out32(ravb->base + EIS, ~EIS_QFS);
    if (eis & EIS_QFS) {
        out32(ravb->base + RIS2, ~(RIS2_QFF0 | RIS2_RFFF));
        /* Receive Descriptor Empty int */
        if (ris2 & RIS2_QFF0)
        {

        }
        /* Receive Descriptor Empty int */
        if (ris2 & RIS2_QFF1)
        {

        }
        /* Receive FIFO Overflow int */
        if (ris2 & RIS2_RFFF)
        {

        }
    }
}


static void ravb_ptp_interrupt(ravb_dev_t *ravb)
{
    uint32_t gis = in32(ravb->base + GIS);

    gis &= in32(ravb->base + GIC);
    if (gis & GIS_PTCF) {
    }
    if (gis & GIS_PTMF) {
    }

    if (gis) {
        out32(ravb->base + GIS, ~gis);
    }
}

int ravb_process_interrupt (void *arg, struct nw_work_thread *wtp)
{
    ravb_dev_t      *ravb;
    uint32_t        iss, tis, ris0;

    struct ifnet    *ifp;

    ravb = arg;
    ifp = &ravb->ecom.ec_if;

    iss = ravb->iss;
    tis = ravb->tis;
    ris0 = ravb->ris0;

    #define BIT01 (BIT(0) | BIT(1))
    if (iss & (ISS_FRS | ISS_FTS | ISS_TFUS))
    {
        /* Received interrupts */
        if (iss & ISS_FRS) /* Frame received Mirror */
        {
            if (ris0 & BIT(0))
            {
                out32(ravb->base + RIS0, ~BIT(0));
                ravb_receive(ravb, wtp);
            }
            if (ris0 & BIT(1))
            {
                out32(ravb->base + RIS0, ~BIT(1));
            }
        }
        /* Transmitted interrupts */
        if (iss & ISS_FTS) /* Frame transmitted Mirror */
        {
            if (tis  & BIT(0))
            {
                out32(ravb->base + TIS, ~BIT(0));
                NW_SIGLOCK_P(&ifp->if_snd_ex, ravb->iopkt, wtp);
                if (ifp->if_flags_tx & IFF_OACTIVE) {
                    ravb_start(ifp);
                } else {
                    NW_SIGUNLOCK_P(&ifp->if_snd_ex, ravb->iopkt, wtp);
                }
            }

            if (tis & BIT(1))
            {
                out32(ravb->base + TIS, ~BIT(1));
            }
        }

        /* TimeStamp Updated Interrupt */
        if (iss & ISS_TFUS) /* Time Stamp FIFO Update Interrupt */
        {
            /* Timestamp updated */
            if (tis & TIS_TFUF)
            {
                //ravb_get_tx_tstamp(ravb);
            }
        }
    }

    /* Error status summary */
    if (iss & ISS_ES) {
        ravb_error_interrupt(ravb);
    }

    if (iss & ISS_CGIS)
        ravb_ptp_interrupt(ravb);

    return 1;
}

int ravb_dmac_enable_interrupt (void * arg)
{
    ravb_dev_t      *ravb;

    ravb = arg;
    /* Clear interrupt status */
    /* Interrupt enable: */
    /* Frame receive */
    out32(ravb->base + RIC0, RIC0_FRE0 | RIC0_FRE1);
    /* Receive FIFO full error, descriptor empty */
    out32(ravb->base + RIC2, RIC2_QFE0 | RIC2_QFE1 | RIC2_RFFE);
    /* Frame transmitted */
    out32(ravb->base + TIC, TIC_FTE0 | TIC_FTE1);

    return 1;
}

const struct sigevent * ravb_dmac_isr (void *arg, int iid)
{
    ravb_dev_t      *ravb;

    ravb = arg;

    /* Store interrupt status */
    ravb->iss = in32(ravb->base + ISS);
    ravb->ris0 = in32(ravb->base + RIS0);
    ravb->ris2 = in32(ravb->base + RIS2);
    ravb->tis  = in32(ravb->base + TIS);
    ravb->eis = in32(ravb->base + EIS);

    /* Disable all interrupts */
    out32(ravb->base + RIC0 , 0);
    out32(ravb->base + RIC1 , 0);
    out32(ravb->base + RIC2 , 0);
    out32(ravb->base + TIC , 0);

    return interrupt_queue(ravb->iopkt, &ravb->ient);
}

const struct sigevent * ravb_emac_isr (void *arg, int iid)
{
    ravb_dev_t  *ravb;
    uint32_t    ecsr, iss;
    ravb = arg;

    /* Get interrupt status */
    iss = in32(ravb->base + ISS);
    ecsr = in32(ravb->base + ECSR);
    out32(ravb->base + ECSR, ecsr); /* clear interrupt */

    if ((iss & ISS_MS) && (ecsr & ECSR_LCHNG)) {
        /* Link changed */
    }
    return 0;
}

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/hardware/devnp/ravb/event.c $ $Rev: 810496 $")
#endif
