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

#include "proto.h"
#include <assert.h>
#include <atomic.h>
#include <string.h>
#include <time.h>

int
rcar_i2c_wait_complete(rcar_i2c_dev_t *dev)
{
    uint64_t    to = dev->len + 1;
    int         intrerr = EOK;

    to *= dev->scl_period * 100;

    while (intrerr != ETIMEDOUT) {
        TimerTimeout(CLOCK_MONOTONIC, _NTO_TIMEOUT_INTR, NULL, &to, NULL);
        intrerr = InterruptWait_r(0, NULL);

        if (intrerr == ETIMEDOUT) {
            rcar_i2c_reset(dev);
            dev->status = I2C_STATUS_ERROR;
            return 0;
        } else if (dev->status & I2C_STATUS_DONE) {
            if (dev->status & I2C_STATUS_ARBL) {
                rcar_i2c_reset(dev);
            }

            return dev->status;
        }

        rcar_i2c_slogf(dev, VERBOSE_QUIET, "I2C rcar_i2c_wait_compl: wrong interrupt (intrerr = %d)", intrerr);
    }

    return 0;
}

const struct sigevent* rcar_i2c_intr(void* arg)
{
    rcar_i2c_dev_t  *dev = (rcar_i2c_dev_t*) arg;
    uint32_t        msr;

    msr = in32(dev->regbase + RCAR_I2C_ICMSR);

    /* Master Address Transmitted Interrupt, Check MAT flasg first before check MDR or MDE flag */
    if (msr & RCAR_ICMSR_MAT) {
        /* clean MAT flasg */
        msr &= ~RCAR_ICMSR_MAT;
        out32(dev->regbase + RCAR_I2C_ICMSR, msr);

        if (dev->mode & CMODE_RECV) {
            if ((dev->len == 1) && (dev->mode & CMODE_DOSTOP)) {
                /* Enable STOP condition and Disable Start condition */
                out32(dev->regbase + RCAR_I2C_ICMCR, RCAR_ICMCR_MIE | RCAR_ICMCR_MDBS | RCAR_ICMCR_FSB);
            } else {
                /* Disable Start condition */
                out32(dev->regbase + RCAR_I2C_ICMCR, RCAR_ICMCR_MIE | RCAR_ICMCR_MDBS);
            }

            /* Note: need clean MDR flag for data transmitting, otherwise the data transmission suspended */
            msr &= ~RCAR_ICMSR_MDR;
            out32(dev->regbase + RCAR_I2C_ICMSR, msr);
        } else {
            if (dev->len == 0) {
                if (dev->mode & CMODE_DOSTOP) {
                    /* Enable STOP condition and Disable Start condition */
                    out32(dev->regbase + RCAR_I2C_ICMCR, RCAR_ICMCR_MIE | RCAR_ICMCR_MDBS | RCAR_ICMCR_FSB);

                    /* only Enable Master Stop Transmitted Interrupt, MNRE and MALE */
                    out32(dev->regbase + RCAR_I2C_ICMIER, RCAR_ICMIER_MSTE | RCAR_ICMIER_MNRE | RCAR_ICMIER_MALE);
                } else {
                    /* Disable Start condition */
                    out32(dev->regbase + RCAR_I2C_ICMCR, RCAR_ICMCR_MIE | RCAR_ICMCR_MDBS);

                    /* only Enable Master Data Transmitted Interrupt, MNRE and MALE */
                    out32(dev->regbase + RCAR_I2C_ICMIER, RCAR_ICMIER_MDTE | RCAR_ICMIER_MNRE | RCAR_ICMIER_MALE);
                }
            } else {
                /* Disable Start condition */
                out32(dev->regbase + RCAR_I2C_ICMCR, RCAR_ICMCR_MIE | RCAR_ICMCR_MDBS);
            }

            /* NOte: need Clean MDE flag for data transmitting, otherwise the data transmission suspended */
            msr &= ~RCAR_ICMSR_MDE;
            out32(dev->regbase + RCAR_I2C_ICMSR, msr);
        }

        return NULL;
    }

    /* Master Data Received Interrupt */
    if (msr & RCAR_ICMSR_MDR) {
        if (dev->len) {
            /* Save the recieved byte */
            *dev->buf = in32(dev->regbase + RCAR_I2C_ICRXD);
            dev->buf++; dev->len--;

            /* Clean MDR flasg */
            msr &= ~RCAR_ICMSR_MDR;
            out32(dev->regbase + RCAR_I2C_ICMSR, msr);

            /* check if need transmits a STOP condition */
            if ((dev->len == 1) && (dev->mode & CMODE_DOSTOP))
                out32(dev->regbase + RCAR_I2C_ICMCR, RCAR_ICMCR_MIE | RCAR_ICMCR_MDBS | RCAR_ICMCR_FSB);

            /* check if get all received data */
            if (dev->len == 0) {
                if ((dev->mode & CMODE_DOSTOP) == 0) {
                    /* No MST condition, Done */
                    out32(dev->regbase + RCAR_I2C_ICMIER, 0);
                    out32(dev->regbase + RCAR_I2C_ICMCR,  0);
                    out32(dev->regbase + RCAR_I2C_ICMSR,  0);

                    atomic_set(&dev->status, I2C_STATUS_DONE);
                    return &dev->intrEvent;
                } else {
                    /* Wait for MST interrupt */
                    return NULL;
                }
            }
        }
        /* Check MST interrupt if dev->len is 0 */
    }

    /* Master Stop Transmitted Interrupt */
    if (msr & RCAR_ICMSR_MST) {
        out32(dev->regbase + RCAR_I2C_ICMIER, 0);
        out32(dev->regbase + RCAR_I2C_ICMCR,  0);
        out32(dev->regbase + RCAR_I2C_ICMSR,  0);

        atomic_set(&dev->status, I2C_STATUS_DONE);
        return &dev->intrEvent;
    }

    /* Master Data Empty Interrupt */
    if (msr & RCAR_ICMSR_MDE) {
        if (dev->len) {
            /* Write next byte to TXD register */
            out32(dev->regbase + RCAR_I2C_ICTXD, *dev->buf);
            dev->buf++;dev->len--;
        } else {
            if (dev->mode & CMODE_DOSTOP) {
                /* Enable STOP condition */
                out32(dev->regbase + RCAR_I2C_ICMCR, RCAR_ICMCR_MIE | RCAR_ICMCR_MDBS | RCAR_ICMCR_FSB);

                /* only Enable Master Stop Transmitted Interrupt, MNRE and MALE */
                out32(dev->regbase + RCAR_I2C_ICMIER, RCAR_ICMIER_MSTE | RCAR_ICMIER_MNRE | RCAR_ICMIER_MALE);
            } else {
                /* Check if Master Data Transmitted Interrupt has been enabled
                 * If it has already been enabled, then I2C transmission done
                 * Note: This  interrupt has been enabled only for the last byte transmitting without
                 *       STOP condition to make sure the last byte been transmitted properly.
                 */
                if (in32(dev->regbase + RCAR_I2C_ICMIER) & RCAR_ICMIER_MDTE) {
                    /* Only disable interrupts, DO NOT clean MSR and MCR */
                    out32(dev->regbase + RCAR_I2C_ICMIER, 0);

                    atomic_set(&dev->status, I2C_STATUS_DONE);

                    return &dev->intrEvent;
                }

                /* only Enable Master Data Transmitted Interrupt, MNRE and MALE */
                out32(dev->regbase + RCAR_I2C_ICMIER, RCAR_ICMIER_MDTE | RCAR_ICMIER_MNRE | RCAR_ICMIER_MALE);
            }
        }

        /* Clean MDE bit for data transmitting, otherwise the data transmission suspended  */
        msr &= ~RCAR_ICMSR_MDE;
        out32(dev->regbase + RCAR_I2C_ICMSR, msr);

        return NULL;
    }

    /* Master NACK Received Interrupt */
    if (msr & RCAR_ICMSR_MNR) {
        msr &= ~RCAR_ICMSR_MNR;
        out32(dev->regbase + RCAR_I2C_ICMSR, msr);

        atomic_set(&dev->status, I2C_STATUS_NACK | I2C_STATUS_DONE);
        return &dev->intrEvent;
    }

    /* Master Arbitration Lost Interrupt */
    if (msr & RCAR_ICMSR_MAL) {
        out32(dev->regbase + RCAR_I2C_ICMIER, 0);
        out32(dev->regbase + RCAR_I2C_ICMCR,  0);
        out32(dev->regbase + RCAR_I2C_ICMSR,  0);

        atomic_set(&dev->status, I2C_STATUS_ARBL | I2C_STATUS_DONE);
        return &dev->intrEvent;
    }

    return NULL;
}

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/hardware/i2c/rcar-A/intr.c $ $Rev: 805407 $")
#endif
