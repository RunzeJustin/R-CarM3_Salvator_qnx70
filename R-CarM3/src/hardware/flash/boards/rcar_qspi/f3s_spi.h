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

#ifndef __F3S_SPI_H_INCLUDED
#define __F3S_SPI_H_INCLUDED

/*
** Includes
*/
#include <sys/f3s_mtd.h>
#include <pthread.h>

/*
** Function Prototypes
*/
int32_t f3s_qspi_open(f3s_socket_t *socket, uint32_t flags);
uint8_t *f3s_qspi_page(f3s_socket_t *socket, uint32_t page, uint32_t offset, int32_t *size);
int32_t f3s_qspi_status(f3s_socket_t *socket, uint32_t flags);
void f3s_qspi_close(f3s_socket_t *socket, uint32_t flags);

/* Spansion HyperFlash S26KS512 */
#define HYPER_BUS_WIDTH                 (4)
#define HYPER_CHIP_INTERLEAVE           (2)
#define HYPER_ENTRY_ADDR                (0x555)
#define HYPER_UNLOCK1_ADDR              (0x555)
#define HYPER_UNLOCK1_DATA              (0xAA << 24)
#define HYPER_UNLOCK2_ADDR              (0x2AA)
#define HYPER_UNLOCK2_DATA              (0x55 << 24)
#define HYPER_ID_ENTRY_ADDR             (HYPER_ENTRY_ADDR)
#define HYPER_ID_ENTRY_DATA             (0x90 << 24)
#define HYPER_ERASE_ADDR1               (0x555)
#define HYPER_ERASE_DATA1               (0x80 << 24)
#define HYPER_ERASE_ADDR2               (0x555)
#define HYPER_ERASE_DATA2               (0xAA << 24)
#define HYPER_ERASE_ADDR3               (0x2AA)
#define HYPER_ERASE_DATA3               (0x55 << 24)
#define HYPER_ERASE_DATA4               (0x30 << 24)
#define HYPER_READ_STATUS_ADDR          (0x555)
#define HYPER_READ_STATUS_DATA          (0x70 << 24)
#define HYPER_RESET_ADDR                (0x00)
#define HYPER_RESET_DATA                (0xF0 << 24)
#define HYPER_WORD_PROGRAM_DATA         (0xA0 << 24)
#define HYPER_BUFFER_PROGRAM_DATA       (0x25 << 24)
#define HYPER_BUFFER_PROGRAM_CONFIRM    (0x29 << 24)
#define HYPER_SECTOR_ERASE_DATA         (0x30 << 24)
#define HYPER_CHIP_ERASE_DATA           (0x10 << 24)
#define HYPER_ERASE_RESUME_DATA         (0x30 << 24)
#define HYPER_ERASE_SUSPEND_DATA        (0xB0 << 24)

/* HyperFlash PPB address and data command */
#define HYPER_PPB_ENTRY_ADDR            (HYPER_ENTRY_ADDR)
#define HYPER_PPB_ENTRY_DATA            (0xC0 << 24)
#define HYPER_PPB_LOCK_ENTRY_DATA       (0x50 << 24)
#define HYPER_PPB_PROGRAM_DATA          (0xA0 << 24)
#define HYPER_PPB_ERASE_DATA1           (0x80 << 24)
#define HYPER_PPB_ERASE_DATA2           (0x30 << 24)

/* HyperFlash DYB address and data command */
#define HYPER_DYB_ENTRY_ADDR            (HYPER_ENTRY_ADDR)
#define HYPER_DYB_ENTRY_DATA            (0xE0 << 24)
#define HYPER_DYB_PROGRAM_DATA          (0xA0 << 24)

/* HyperFlash status register */
#define HYPER_DEVICE_READY              (1 << 7)    // Device ready bit
#define HYPER_ERASE_SUSPEND             (1 << 6)    // erase suspended

#define HYPER_SECTOR_ERASE_TIME         (930000)
#define HYPER_RESET_TIME                (4096)

int32_t sps26ks_ident(f3s_dbase_t * dbase, f3s_access_t * access,
                      _uint32 flags, _uint32 offset);

void sps26ks_reset(f3s_dbase_t *dbase,
                    f3s_access_t *access,
                    uint32_t flags,
                    uint32_t offset);

int32_t sps26ks_read(f3s_dbase_t *dbase,
                    f3s_access_t *access,
                    uint32_t flags,
                    uint32_t offset,
                    int32_t buffer_size,
                    uint8_t *buffer);

int32_t sps26ks_write(f3s_dbase_t * dbase, f3s_access_t * access,
                      _uint32 flags, _uint32 offset,
                      _int32 size, _uint8 * buffer);

int sps26ks_suspend(f3s_dbase_t *dbase, f3s_access_t *access,
                    uint32_t flags, uint32_t offset);

int sps26ks_resume(f3s_dbase_t *dbase, f3s_access_t *access,
                   uint32_t flags, uint32_t offset);

int sps26ks_erase(f3s_dbase_t * dbase, f3s_access_t * access,
                  _uint32 flags, _uint32 offset);

int32_t sps26ks_sync(f3s_dbase_t *dbase, f3s_access_t *access,
                     uint32_t flags, uint32_t offset);

int sps26ks_islock(f3s_dbase_t *dbase, f3s_access_t *access,
                   uint32_t flags, uint32_t offset);

int sps26ks_lock(f3s_dbase_t *dbase, f3s_access_t *access,
                 uint32_t flags, uint32_t offset);

int sps26ks_unlock(f3s_dbase_t *dbase, f3s_access_t *access,
                   uint32_t flags, uint32_t offset);

int sps26ks_unlockall(f3s_dbase_t *dbase, f3s_access_t *access,
                      uint32_t flags, uint32_t offset);

/* common SPI flash calls */
int norspi_read_registers(void *hdl, uint32_t addr, uint8_t *buf, int len);
int norspi_read_memory(void *hdl, uint32_t addr, uint8_t *buf, int len);
int norspi_sector_erase(void *hdl, uint32_t addr, int sector_size);
int norspi_write_command(void *hdl, uint32_t addr, uint32_t data);
int norspi_write_memory(void *hdl, uint32_t addr, uint8_t *dbuf, int dlen);
int norspi_busy_wait(void *hdl, uint32_t tmo);

/* host/board specifc SPI flash calls */
typedef struct _spi_flash_t {
    void        *hdl;
    uint32_t    hcap;       // SPI controller capability
    uint32_t    mode;       // current mode
    int         (*dinit)(void *hdl);
    int         (*spi_cfg_bus)(void *hdl, void *cfg);
    int         (*spi_read_flash_registers)(void *hdl, uint32_t addr, uint8_t *dbuf, int dlen);
    int         (*spi_read_flash_memory)(void *hdl, uint32_t addr, uint8_t *dbuf, int dlen);
    int         (*spi_write_flash_cmd)(void *hdl, uint32_t addr, uint32_t data);
    int         (*spi_write_flash_memory)(void *hdl, uint32_t addr, uint8_t *dbuf, int dlen);
} spi_flash_t;

#endif /* __F3S_SPI_H_INCLUDED */

/*
** End
*/

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/hardware/flash/boards/rcar_qspi/f3s_spi.h $ $Rev: 811059 $")
#endif
