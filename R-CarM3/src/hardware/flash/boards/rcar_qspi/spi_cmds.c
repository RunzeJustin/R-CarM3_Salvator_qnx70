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

#include <f3s_spi.h>

int norspi_write_memory(void *hdl, uint32_t addr, uint8_t *dbuf, int dlen)
{
    spi_flash_t *spic = (spi_flash_t *)hdl;

    return spic->spi_write_flash_memory(hdl, addr, dbuf, dlen);
}

int norspi_write_command(void *hdl, uint32_t addr, uint32_t data)
{
  spi_flash_t *spic = (spi_flash_t *)hdl;

  return spic->spi_write_flash_cmd(hdl, addr, data);
}

int norspi_sector_erase(void *hdl, uint32_t addr, int sector_size)
{
    /* Write Entry Command */
    if (norspi_write_command(hdl, HYPER_UNLOCK1_ADDR, HYPER_UNLOCK1_DATA) < 0) {
        fprintf(stderr, "Unlock1 failed.\n");
        return (EIO);
    }

    if (norspi_write_command(hdl, HYPER_UNLOCK2_ADDR, HYPER_UNLOCK2_DATA) < 0) {
        fprintf(stderr, "Unlock2 failed.\n");
        return (EIO);
    }

    if (norspi_write_command(hdl, HYPER_ERASE_ADDR1, HYPER_ERASE_DATA1) < 0) {
        fprintf(stderr, "Third Erase Command failed.\n");
        return (EIO);
    }

    if (norspi_write_command(hdl, HYPER_ERASE_ADDR2, HYPER_ERASE_DATA2) < 0) {
        fprintf(stderr, "Fourth Erase Command failed.\n");
        return (EIO);
    }

    if (norspi_write_command(hdl, HYPER_ERASE_ADDR3, HYPER_ERASE_DATA3) < 0) {
        fprintf(stderr, "Fifth Erase Command failed.\n");
        return (EIO);
    }

    if (norspi_write_command(hdl, addr/2, HYPER_ERASE_DATA4) < 0) {
        fprintf(stderr, "Sixth Erase Command failed.\n");
        return (EIO);
    }

    if (norspi_busy_wait(hdl, HYPER_SECTOR_ERASE_TIME) == EOK)
        return (EOK);

    return (EIO);
}

int norspi_read_registers(void *hdl, uint32_t addr, uint8_t *buf, int len)
{
    spi_flash_t *spic = (spi_flash_t *)hdl;

    if (len == 0)
        return 0;

    return spic->spi_read_flash_registers(hdl, addr, buf, len);
}

int norspi_read_memory(void *hdl, uint32_t addr, uint8_t *buf, int len)
{
    spi_flash_t *spic = (spi_flash_t *)hdl;

    if (len == 0)
        return 0;

    return spic->spi_read_flash_memory(hdl, addr, buf, len);
}

int norspi_busy_wait(void *hdl, uint32_t tmo)
{
    uint8_t sts;

    while (tmo--) {
        if (norspi_write_command(hdl, HYPER_READ_STATUS_ADDR, HYPER_READ_STATUS_DATA) < 0) {
            fprintf(stderr, "Write command Read Status failed.\n");
            return (EIO);
        }

        if (norspi_read_registers(hdl, 0x0, &sts, 1) < 0) {
            fprintf(stderr, "Read status register failed.\n");
            return (EIO);
        }

        if (sts & HYPER_DEVICE_READY)
            return (EOK);

        nanospin_ns(1000);
    }

    return (EAGAIN);
}

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/hardware/flash/boards/rcar_qspi/spi_cmds.c $ $Rev: 811059 $")
#endif
