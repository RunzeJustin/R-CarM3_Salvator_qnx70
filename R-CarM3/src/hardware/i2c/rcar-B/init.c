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
#include <sys/mman.h>

void *rcar_i2c_init(int argc, char *argv[])
{
    rcar_i2c_dev_t  *dev;
    unsigned        speed;

    if (-1 == ThreadCtl(_NTO_TCTL_IO, 0)) {
        perror("ThreadCtl");
        return NULL;
    }

    if ((dev = calloc(1, sizeof(rcar_i2c_dev_t))) == NULL)
        return NULL;

    if (-1 == rcar_i2c_options(dev, argc, argv))
        goto fail;

    dev->regbase = (uintptr_t)mmap_device_memory(NULL, RCAR_IIC_SIZE,
                                PROT_READ | PROT_WRITE | PROT_NOCACHE , 0, dev->physbase);

    if (dev->regbase == (uintptr_t)MAP_FAILED) {
        perror("mmap_device_memory with regbase");
        goto fail;
    }

    /* clear bus if necessary */
    out8(dev->regbase + RCAR_IIC_ICCR, 0);
    delay(1);

    // clear status
    out8(dev->regbase + RCAR_IIC_ICSR, 0);

    /* Force initialization of I2C bus clock registers */
    speed = dev->speed;
    dev->speed = -1;
    rcar_i2c_set_bus_speed(dev, speed, NULL);

    /* Initialize interrupt handler */
    SIGEV_INTR_INIT(&dev->intrevent);
    dev->iid = InterruptAttach(dev->irq, rcar_i2c_intr, dev, 0, 0);
    if (dev->iid == -1) {
        perror("InterruptAttach");
        goto fail;
    }

    return dev;

fail:
    free(dev);

    return NULL;
}

void
rcar_i2c_fini(void *hdl)
{
    free(hdl);
}

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/hardware/i2c/rcar-B/init.c $ $Rev: 805407 $")
#endif
