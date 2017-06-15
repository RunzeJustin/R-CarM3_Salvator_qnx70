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

i2c_status_t
rcar_i2c_send(void *hdl, void *buf, unsigned int len, unsigned int stop)
{
    rcar_i2c_dev_t	*dev = hdl;
    uint32_t        status;
    uint8_t			icsr, icic;

    if (len <= 0)
        return I2C_STATUS_DONE;

    dev->buf = buf;
    dev->len = len;
    dev->bytes = 0;
    dev->mode = CMODE_SEND;
    dev->mode |= stop ? CMODE_DOSTOP : CMODE_WAIT;
    if (dev->slave_addr_fmt & I2C_ADDRFMT_7BIT)
        dev->mode |= CMODE_SADDR_L;
    if (dev->slave_addr_fmt & I2C_ADDRFMT_10BIT)
        dev->mode |= CMODE_SADDR_H;

    icsr = in8(dev->regbase + RCAR_IIC_ICSR);
    if (!(icsr & RCAR_ICSR_BUSY)) {     // restart condition?
        out8(dev->regbase + RCAR_IIC_ICCR, in8(dev->regbase + RCAR_IIC_ICCR) | RCAR_ICCR_ICE);
        icic = in8(dev->regbase + RCAR_IIC_ICIC);
        icic &= ~(3 << 6);
        icic |= (((dev->iccl & 0x100) >> 1) | ((dev->icch & 0x100) >> 2));
        out8(dev->regbase + RCAR_IIC_ICIC, icic);
        out8(dev->regbase + RCAR_IIC_ICCL, dev->iccl & 0xff);
        out8(dev->regbase + RCAR_IIC_ICCH, dev->icch & 0xff);
    }

    // Transmit start
    out8(dev->regbase + RCAR_IIC_ICIC,
                    in8(dev->regbase + RCAR_IIC_ICIC) | RCAR_ICIC_DTE | RCAR_ICIC_WAIT | RCAR_ICIC_TACK | RCAR_ICIC_WAIT);
    out8(dev->regbase + RCAR_IIC_ICCR, RCAR_ICCR_ICE | RCAR_ICCR_MTM | RCAR_ICCR_BBSY);

    if (icsr & RCAR_ICSR_WAIT)
        out8(dev->regbase + RCAR_IIC_ICSR, in8(dev->regbase + RCAR_IIC_ICSR) & ~RCAR_ICSR_WAIT);

    status = rcar_i2c_poll_complete(dev);
    if (!(status & (RCAR_ICSR_TACK | RCAR_ICSR_AL | I2C_BUSY))){
        if (stop) { //wait till it completes
            rcar_i2c_poll_busy(dev, RCAR_ICSR_BUSY, 0);
        } else {
            /*
             * This is a tricky part:
             * For not stop and retransmit operation,
             * we have to disable DTEE,
             * then we can not get interrupt for DTE
             * I have to wait for this final DTE bit show up and then return
             */
            rcar_i2c_poll_busy(dev, RCAR_ICSR_DTE, RCAR_ICSR_DTE);
        }

        return (I2C_STATUS_DONE);
    }

    if (status & RCAR_ICSR_TACK)
        return (I2C_STATUS_NACK | I2C_STATUS_DONE);
    else if (status & RCAR_ICSR_AL)
        return (I2C_STATUS_ARBL | I2C_STATUS_DONE);
    else
        return (I2C_STATUS_BUSY | I2C_STATUS_DONE);
}

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/hardware/i2c/rcar-B/send.c $ $Rev: 805407 $")
#endif
