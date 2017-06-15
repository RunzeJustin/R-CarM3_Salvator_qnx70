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

static inline void rcar_i2c_reset(rcar_i2c_dev_t *dev)
{
    // disable interrupt, clear sr, stop
    out8(dev->regbase + RCAR_IIC_ICIC, in8(dev->regbase + RCAR_IIC_ICIC) & (3 << 6));
    out8(dev->regbase + RCAR_IIC_ICSR, 0);
    out8(dev->regbase + RCAR_IIC_ICCR, 0);
}

uint32_t rcar_i2c_poll_complete(rcar_i2c_dev_t *dev)
{
    uint64_t    ntime = 0x5000000;//1e9;
    int         intrerr = EOK;

    while (intrerr != ETIMEDOUT) {
        TimerTimeout(CLOCK_MONOTONIC, _NTO_TIMEOUT_INTR, NULL, &ntime, NULL);
        intrerr = InterruptWait_r(0, NULL);
        if (intrerr == ETIMEDOUT) {
            fprintf(stderr, "I2C timeout, status=%x, bytes %d, mode %x\n", in8(dev->regbase + RCAR_IIC_ICSR), dev->bytes, dev->mode);
            rcar_i2c_reset(dev);
        } else {
            return (dev->status);
        }
    }

    return I2C_BUSY;
}

/*
 * i2c interrupt handler
 */
const struct sigevent *rcar_i2c_intr(void *area, int id)
{
    rcar_i2c_dev_t  *dev=area;
    uint8_t         icsr = in8(dev->regbase + RCAR_IIC_ICSR);
    uint8_t         iccr = in8(dev->regbase + RCAR_IIC_ICCR);

    /* Arbitration lost or No ACK for transmission */
    if (dev->mode && ((icsr & RCAR_ICSR_AL) || ((icsr & RCAR_ICSR_TACK) && (iccr & RCAR_ICCR_MTM)))) {
        rcar_i2c_reset(dev);
        dev->status = icsr;
        dev->mode = 0;
        return (&dev->intrevent);
    } else {
        // Sending slave address
        if (dev->mode & CMODE_SADDR_L) {
            dev->max_ints = 0;
            // Send the seven bit address and R/W bit
            if (dev->mode & CMODE_RECV) {
                out8(dev->regbase + RCAR_IIC_ICDR, (dev->slave_addr & 0x7f) << 1 | 1);  // Slave address and receive mode(1)
                if ((dev->mode & CMODE_RECV_ONE) && !(dev->mode & CMODE_SADDR_H))
                    out8(dev->regbase + RCAR_IIC_ICIC, (in8(dev->regbase + RCAR_IIC_ICIC) & (3 << 6)) | RCAR_ICIC_AL | RCAR_ICIC_TACK | RCAR_ICIC_WAIT);
                else
                    out8(dev->regbase + RCAR_IIC_ICIC, (in8(dev->regbase + RCAR_IIC_ICIC) & (3 << 6)) | RCAR_ICIC_AL | RCAR_ICIC_TACK | RCAR_ICIC_DTE);
            } else {
                out8(dev->regbase + RCAR_IIC_ICDR, (dev->slave_addr & 0x7f) << 1);  // Slave address and send mode(0)
                out8(dev->regbase + RCAR_IIC_ICIC, (in8(dev->regbase + RCAR_IIC_ICIC) & (3 << 6)) | RCAR_ICIC_AL | RCAR_ICIC_TACK | RCAR_ICIC_DTE);
            }

            dev->mode &= ~CMODE_SADDR_L;
            out8(dev->regbase + RCAR_IIC_ICSR, in8(dev->regbase + RCAR_IIC_ICSR) & ~RCAR_ICSR_WAIT);
            if (!(dev->mode & CMODE_SADDR_H) && (dev->mode & CMODE_RECV) && !(dev->mode & CMODE_RECV_ONE)) {
                // Change to receive
                out8(dev->regbase + RCAR_IIC_ICCR, RCAR_ICCR_ICE | RCAR_ICCR_SCP);
            }

            return NULL;
        } else if (dev->mode & CMODE_SADDR_H) {
            out8(dev->regbase + RCAR_IIC_ICDR, (dev->slave_addr & 0x3ff) >> 7);
            dev->mode &= ~CMODE_SADDR_H;
            if (dev->mode & CMODE_RECV_ONE)
                out8(dev->regbase + RCAR_IIC_ICIC, (in8(dev->regbase + RCAR_IIC_ICIC) & (3 << 6)) | RCAR_ICIC_AL | RCAR_ICIC_TACK | RCAR_ICIC_WAIT);
            out8(dev->regbase + RCAR_IIC_ICSR, in8(dev->regbase + RCAR_IIC_ICSR) & ~RCAR_ICSR_WAIT);
            // Change to receive
            if ((dev->mode & CMODE_RECV) && !(dev->mode & CMODE_RECV_ONE))
                out8(dev->regbase + RCAR_IIC_ICCR, RCAR_ICCR_ICE | RCAR_ICCR_SCP);

            return NULL;
        }

        /*
         * For 1 byte receive, we need to use WAITE
         * to change the transfer direction to receive
         * and send the ACK bit
         */
        if ((icsr & RCAR_ICSR_WAIT) && (dev->mode & (CMODE_RECV_ONE | CMODE_RECV_TWO))) {
            if (icsr & RCAR_ICSR_TACK)
                out8(dev->regbase + RCAR_IIC_ICSR, in8(dev->regbase + RCAR_IIC_ICSR) & ~RCAR_ICSR_TACK);

            if (dev->mode & CMODE_RECV_ONE) {
                dev->mode &= ~CMODE_RECV_ONE;
                dev->mode |= CMODE_RECV_TWO;
                // Change to receive
                out8(dev->regbase + RCAR_IIC_ICCR, RCAR_ICCR_ICE | RCAR_ICCR_SCP);
                out8(dev->regbase + RCAR_IIC_ICSR, in8(dev->regbase + RCAR_IIC_ICSR) & ~RCAR_ICSR_WAIT);
            } else {
                int complete=0;
                if (in8(dev->regbase + RCAR_IIC_ICSR) & RCAR_ICSR_DTE) {
                    *(dev->buf) = in8(dev->regbase + RCAR_IIC_ICDR);
                    dev->buf++; dev->bytes++;
                    dev->len--;
                    complete = 1;
                } else {
                    out8(dev->regbase + RCAR_IIC_ICIC, (in8(dev->regbase + RCAR_IIC_ICIC) & (3 << 6)) | RCAR_ICIC_AL | RCAR_ICIC_TACK | RCAR_ICIC_WAIT | RCAR_ICIC_DTE);
                    out8(dev->regbase + RCAR_IIC_ICCR, RCAR_ICCR_ICE | RCAR_ICCR_RACK);
                }

                out8(dev->regbase + RCAR_IIC_ICSR, in8(dev->regbase + RCAR_IIC_ICSR) & ~RCAR_ICSR_WAIT);

                dev->mode &= ~CMODE_RECV_TWO;
                if (complete) {
                    dev->status = icsr;
                    out8(dev->regbase + RCAR_IIC_ICIC, in8(dev->regbase + RCAR_IIC_ICIC) & (3 << 6));
                    dev->mode = 0;
                    return (&dev->intrevent);
                }
            }

            return NULL;
        }

        if (dev->mode & CMODE_SEND ){   // Transmit
            if (dev->len)
                out8(dev->regbase + RCAR_IIC_ICDR, *(dev->buf));
            if (icsr & RCAR_ICSR_WAIT)
                out8(dev->regbase + RCAR_IIC_ICSR, in8(dev->regbase + RCAR_IIC_ICSR) & ~RCAR_ICSR_WAIT);
            ++dev->buf;
            dev->len--;
            dev->bytes++;
            if (dev->len == 0) {
                dev->status = icsr;
                if (dev->mode & CMODE_DOSTOP) {
                    // Generate a stop
                    out8(dev->regbase + RCAR_IIC_ICCR, RCAR_ICCR_ICE | RCAR_ICCR_MTM);
                    out8(dev->regbase + RCAR_IIC_ICIC, in8(dev->regbase + RCAR_IIC_ICIC) & (3 << 6));
                } else {
                    /* we have to disable DTEE and Enable WAIT to hold the SCL low */
                    out8(dev->regbase + RCAR_IIC_ICIC, (in8(dev->regbase + RCAR_IIC_ICIC) & (3 << 6)) | RCAR_ICSR_WAIT);
                }

                dev->mode = 0;

                return (&dev->intrevent);
            }
        } else if (dev->mode & CMODE_RECV) {    // Receive
            if (icsr & RCAR_ICSR_TACK) {
                out8(dev->regbase + RCAR_IIC_ICSR, in8(dev->regbase + RCAR_IIC_ICSR) & ~RCAR_ICSR_TACK);
            }

            // Read the data
            *(dev->buf) = in8(dev->regbase + RCAR_IIC_ICDR);
            dev->buf++; dev->bytes++; dev->len--;
            if (dev->len == 0) {
                dev->status = icsr;
                out8(dev->regbase + RCAR_IIC_ICIC, in8(dev->regbase + RCAR_IIC_ICIC) & (3 << 6));
                dev->mode = 0;

                return (&dev->intrevent);
            }

            if (dev->len == 1) {
                /*if we only have 1 more byte to receive, need to set the RACK */
                out8(dev->regbase + RCAR_IIC_ICCR, RCAR_ICCR_ICE | RCAR_ICCR_RACK);
            }
        } else if (icsr & RCAR_ICSR_WAIT) {
            // Waiting for the wait interrupt
            out8(dev->regbase + RCAR_IIC_ICSR, in8(dev->regbase + RCAR_IIC_ICSR) & ~RCAR_ICSR_WAIT);
            if (dev->mode) {
                dev->mode = 0;
                return (&dev->intrevent);
            }
        } else {
            /*
             * We shouldn't be getting here, clear it anyway
             * if there is a glitch, let's take to max
             * and then reset */
            out8(dev->regbase + RCAR_IIC_ICSR, 0);
            dev->max_ints++;
            if (dev->max_ints++ > 2000) {
                rcar_i2c_reset(dev);
                dev->status |= RCAR_ICSR_AL;    //assume it's arbitration lost
            }
        }

    }

    return (NULL);
}

uint32_t rcar_i2c_poll_busy(rcar_i2c_dev_t *dev, uint8_t mask, uint8_t bit)
{
    int     i = MAX_POOL;
    uint8_t icsr = in8(dev->regbase + RCAR_IIC_ICSR);

    while (((icsr & (mask)) != bit) && --i) {
        usleep(4000);
        icsr = in8(dev->regbase + RCAR_IIC_ICSR);
    }

    if (i <= 0) {
        fprintf(stderr, "I2C: timeout on pool ready !!!!!!! %d, sts = %x\n", i, icsr);
        return -1;
    }

    if (mask == RCAR_ICSR_BUSY)
        rcar_i2c_reset(dev);

    return 0;
}

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/hardware/i2c/rcar-B/intr.c $ $Rev: 805407 $")
#endif
