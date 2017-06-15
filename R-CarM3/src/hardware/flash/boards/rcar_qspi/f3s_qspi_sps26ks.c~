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

#include "f3s_spi.h"

/*
 * This is the write callout for SPI serial NOR flash.
 */
int32_t sps26ks_write(f3s_dbase_t *dbase,
                      f3s_access_t *access,
                      uint32_t flags,
                      uint32_t offset,
                      int32_t size,
                      uint8_t *buffer)
{
    int         rc;
    void        *buf;
    int         pagesz = dbase->buffer_size;
    int         left;
    int         cursz;
    void        *hdl = access->socket.socket_handle;

  uint8_t len = 32;
    uint8_t len2 = len-32;
    uint32_t addr=0xa00000;
    uint32_t addr2=addr+32;
    uint8_t     buf_read[len];
    uint8_t     buf_write[len + 10];
    uint8_t     tmp=0;


  norspi_read_memory(access->socket.socket_handle , addr, 3, buf_read , len );
     fprintf(stderr, "\n---------before eraser----------\n");
     for (int i = 0; i < len; i++) { 
         fprintf(stderr, "%#x\t",buf_read[i]); 
         buf_read[i]=0;
         if( (i+1)%8 == 0 ) fprintf(stderr, "\n" );
     }
     fprintf(stderr, "&&&\n" );

     norspi_sector_erase(access->socket.socket_handle, addr, 3 , 0 );

    
     norspi_read_memory(access->socket.socket_handle , addr, 3 , buf_read , len );
     fprintf(stderr, "\n---------after eraser----------\n");
     for (int i = 0; i < len; i++) { 
         fprintf(stderr, "%#x\t",buf_read[i]); 
         buf_read[i]=0;
         if((i+1)%8 == 0 && i != 0 ) fprintf(stderr, "\n" );
     }
     fprintf(stderr, "\n" );

    
    // //norspi_page_program(access->socket.socket_handle, addr, 3 , buf_write ,3);
     norspi_page_program(access->socket.socket_handle, addr , 3 , buf_write ,8);

     fprintf(stderr, "\n---------buffer write------------\n");
    for (int i = 0; i < len; i++) { 
         fprintf(stderr, "%#x\t",buf_write[i]); 
         buf_write[i]=0;
        if((i+1)%8 == 0 && i != 0 ) fprintf(stderr, "\n" );
     }
     fprintf(stderr, "\n" );

     norspi_read_memory(access->socket.socket_handle , addr, 3 , buf_read , len );
     fprintf(stderr, "\n---------after write----------\n");
     for (int i = 0; i < len; i++) { 
         fprintf(stderr, "%#x\t",buf_read[i]); 
         buf_read[i]=0;
         if((i+1)%8 == 0 && i != 0 ) fprintf(stderr, "\n" );
     }
     fprintf(stderr, "\n" );


    if (access->service->page(&access->socket, 0, offset, &size) == NULL)
        return -1;

    cursz = pagesz - (offset & (pagesz - 1));
    if (cursz > size)
        cursz = size;

    if ((rc = norspi_write_memory(hdl, offset, buffer, cursz)) < 0) {
        errno = EIO;
        return -1;
    }

    if (!(flags & F3S_VERIFY_WRITE))
        return rc;

    /* verify data was written correctly */
    buf  = alloca(rc);
    left = rc;
    while (left) {
        cursz = norspi_read_memory(hdl, offset, buf, left);

        if (cursz <= 0) {
            errno = EIO;
            return -1;
        }
        if (memcmp(buffer, buf, cursz)) {
            fprintf(stderr, "(devf  t%d::%s:%d) program verify error\n"
                        "between offset 0x%x and 0x%x, size = %d\n",
                        pthread_self(), __func__, __LINE__, offset, offset + cursz, cursz);
            errno = EIO;
            return -1;
        }
        /* adjust */
        left   -= cursz;
        offset += cursz;
        buffer += cursz;
    }

    return rc;
}

/*
 * Summary
 *
 * MTD Version:    2 only
 * Locking Method: Persistent
 *
 * Description
 *
 * Use this for Spansion S26KS SPI flash capable of block locking.
 */
int sps26ks_unlockall(f3s_dbase_t *dbase, f3s_access_t *access,
                      uint32_t flags, uint32_t offset)
{
    void    *hdl = access->socket.socket_handle;

    /* check that this chip supports Persistent protection. */
    if (!(dbase->flags & F3S_PROTECT_PERSISTENT))
        return (ENOTSUP);

    /* Write Entry Command */
    if (norspi_write_command(hdl, HYPER_UNLOCK1_ADDR, HYPER_UNLOCK1_DATA) < 0) {
        fprintf(stderr, "Unlock1 failed.\n");
        return (EIO);
    }

    if (norspi_write_command(hdl, HYPER_UNLOCK2_ADDR, HYPER_UNLOCK2_DATA) < 0) {
        fprintf(stderr, "Unlock2 failed.\n");
        return (EIO);
    }

    if (norspi_write_command(hdl, HYPER_PPB_ENTRY_ADDR, HYPER_PPB_ENTRY_DATA) < 0) {
        fprintf(stderr, "Entry PPB failed.\n");
        return (EIO);
    }

    /* All PPB Erase command */
    if (norspi_write_command(hdl, 0, HYPER_PPB_ERASE_DATA1) < 0) {
        fprintf(stderr, "PPB Erase first command failed.\n");
        return (EIO);
    }

    if (norspi_write_command(hdl, 0, HYPER_PPB_ERASE_DATA2) < 0) {
        fprintf(stderr, "PPB Erase second command failed.\n");
        return (EIO);
    }

    /* Reset/ASO Exit Command */
    if (norspi_write_command(hdl, HYPER_RESET_ADDR, HYPER_RESET_DATA) < 0) {
        fprintf(stderr, "Reset/ASO exit failed.\n");
        return (EIO);
    }

    if (norspi_busy_wait(hdl, HYPER_RESET_TIME) == EOK)
        return (EOK);

    return (EIO);
}

/*
 * Summary
 *
 * MTD Version:    2 only
 * Locking Method: Non-persistent
 *
 * Description
 *
 * Use this for Spansion S26KS SPI flash capable of block locking.
 */
int sps26ks_unlock(f3s_dbase_t *dbase, f3s_access_t *access,
                   uint32_t flags, uint32_t offset)
{
    void *hdl = access->socket.socket_handle;

    /* check that this chip supports Dynamic protection. */
    if (!(dbase->flags & F3S_PROTECT_DYN))
        return (ENOTSUP);

    /* Write Entry Command */
    if (norspi_write_command(hdl, HYPER_UNLOCK1_ADDR, HYPER_UNLOCK1_DATA) < 0) {
        fprintf(stderr, "Unlock1 failed.\n");
        return (EIO);
    }

    if (norspi_write_command(hdl, HYPER_UNLOCK2_ADDR, HYPER_UNLOCK2_DATA) < 0) {
        fprintf(stderr, "Unlock2 failed.\n");
        return (EIO);
    }

    if (norspi_write_command(hdl, HYPER_DYB_ENTRY_ADDR, HYPER_DYB_ENTRY_DATA) < 0) {
        fprintf(stderr, "Entry failed.\n");
        return (EIO);
    }

    /* DYB Clear */
    if (norspi_write_command(hdl, 0, HYPER_DYB_PROGRAM_DATA) < 0) {
        fprintf(stderr, "Enter DYB Clear failed.\n");
        return (EIO);
    }

    /* Write value 1 to HyperFlash */
    if (norspi_write_command(hdl, offset, 0x1) < 0) {
        fprintf(stderr, "Unlock failed.\n");
        return (EIO);
    }

    /* Reset/ASO Exit Command */
    if (norspi_write_command(hdl, HYPER_RESET_ADDR, HYPER_RESET_DATA) < 0) {
        fprintf(stderr, "Reset/ASO exit failed.\n");
        return (EIO);
    }

    if (norspi_busy_wait(hdl, HYPER_RESET_TIME) == EOK)
        return (EOK);

    return (EIO);
}

/*
 * This is the sync callout for SPI serial NOR flash.
 */
int32_t sps26ks_sync (f3s_dbase_t *dbase,
                      f3s_access_t *access,
                      uint32_t flags,
                      uint32_t text_offset)
{
    if (access->service->page (&access->socket, 0, text_offset, NULL) == NULL)
        return (ERANGE);

    return norspi_busy_wait(access->socket.socket_handle, 1);
}

/*
 * This is the erase suspend callout for SPI serial NOR flash.
 */
int sps26ks_suspend(f3s_dbase_t *dbase,
                    f3s_access_t *access,
                    uint32_t flags,
                    uint32_t offset)
{
    int     loop;
    uint8_t sts;
    void    *hdl = access->socket.socket_handle;

    if (access->service->page(&access->socket, 0, offset, NULL) == NULL)
        return (ERANGE);

    if (norspi_write_command(access->socket.socket_handle, offset, HYPER_ERASE_SUSPEND_DATA) < 0)
        fprintf(stderr, "Erase suspend failed.\n");

    /* max 45us according to spec */
    for (loop = 64; loop > 0; loop--) {
        if (norspi_write_command(hdl, HYPER_READ_STATUS_ADDR, HYPER_READ_STATUS_DATA) < 0) {
            fprintf(stderr, "Write command Read Status failed.\n");
            return (EIO);
        }

        if (norspi_read_registers(hdl, 0x0, &sts, 1) < 0) {
            fprintf(stderr, "Read status register failed.\n");
            return (EIO);
        }

        if (sts & HYPER_DEVICE_READY) {       // device is ready
            if ((sts & HYPER_ERASE_SUSPEND) == 0)  // no WIP, no suspend, so erase completed
                return (ECANCELED);

            /* erase is suspended */

            return (EOK);
        }

        nanospin_ns(1000);
    }

    return (EIO);
}

/*
 * This is the erase resume callout for SPI serial NOR flash.
 */
int sps26ks_resume(f3s_dbase_t *dbase,
                   f3s_access_t *access,
                   uint32_t flags,
                   uint32_t offset)
{
    if (access->service->page(&access->socket, 0, offset, NULL) == NULL)
        return (ERANGE);

    if (norspi_write_command(access->socket.socket_handle, offset, HYPER_ERASE_RESUME_DATA) < 0)
        fprintf(stderr, "Erase Resume failed.\n");

    nanospin_ns(100 * 1000);

    return (EOK);
}

/*
 * This is the sync callout for SPI serial NOR flash.
 */
void sps26ks_reset (f3s_dbase_t *dbase,
                    f3s_access_t *access,
                    uint32_t flags,
                    uint32_t offset)
{
    if (norspi_write_command(access->socket.socket_handle, HYPER_RESET_ADDR, HYPER_RESET_DATA) < 0) {
        fprintf(stderr, "Software reset failed.\n");
        return ;
    }
}

/*
 * This is the read callout for SPI serial NOR flash.
 */
int32_t sps26ks_read (f3s_dbase_t *dbase,
                      f3s_access_t *access,
                      uint32_t flags,
                      uint32_t text_offset,
                      int32_t buffer_size,
                      uint8_t *buffer)
{
    int rc;

    rc = norspi_read_memory(access->socket.socket_handle,
            text_offset, buffer, buffer_size);
    if (rc < 0) {
        errno = EIO;
        return -1;
    }

    return  rc;     // return number of bytes read
}

/*
 * Summary
 *
 * MTD Version:    2 only
 * Locking Method: Persistent and Non-persistent
 *
 * Description
 *
 * Use this for Spansion S26KS SPI flash capable of block locking.
 */
static int sps26ks_dolock(void *hdl, uint32_t cmd, uint32_t offset)
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

    if (cmd == HYPER_PPB_ENTRY_DATA)
    {
        if (norspi_write_command(hdl, HYPER_PPB_ENTRY_ADDR, HYPER_PPB_ENTRY_DATA) < 0) {
            fprintf(stderr, "Entry failed.\n");
            return (EIO);
        }

        if (norspi_write_command(hdl, 0, HYPER_PPB_PROGRAM_DATA) < 0) {
            fprintf(stderr, "PPB Program failed.\n");
            return (EIO);
        }
    }
    else if (cmd == HYPER_DYB_ENTRY_DATA)
    {
        if (norspi_write_command(hdl, HYPER_DYB_ENTRY_ADDR, HYPER_DYB_ENTRY_DATA) < 0) {
            fprintf(stderr, "Entry failed.\n");
            return (EIO);
        }

        /* DYB Set */
        if (norspi_write_command(hdl, 0, HYPER_DYB_PROGRAM_DATA) < 0) {
            fprintf(stderr, "Enter DYB Set failed\n");
            return (EIO);
        }
    }

    /* Write value 0 to HyperFlash */
    if (norspi_write_command(hdl, offset, 0x0) < 0) {
        fprintf(stderr, "Lock failed.\n");
        return (EIO);
    }

    /* Reset/ASO Exit Command */
    if (norspi_write_command(hdl, HYPER_RESET_ADDR, HYPER_RESET_DATA) < 0) {
        fprintf(stderr, "Reset/ASO exit failed.\n");
        return (EIO);
    }

    if (norspi_busy_wait(hdl, HYPER_RESET_TIME) == EOK)
        return (EOK);

    return (EIO);
}

int sps26ks_lock(f3s_dbase_t *dbase, f3s_access_t *access,
                 uint32_t flags, uint32_t offset)
{
    void *hdl = access->socket.socket_handle;

    /* The DYB bits aren't locked, check the PPB bits */
    if (dbase->flags & F3S_PROTECT_PERSISTENT)
        return sps26ks_dolock(hdl, HYPER_PPB_ENTRY_DATA, offset);

    /* check that this chip supports Dynamic protection. */
    if (dbase->flags & F3S_PROTECT_DYN)
        return sps26ks_dolock(hdl, HYPER_DYB_ENTRY_DATA, offset);

    return (ENOTSUP);
}

/*
 * Summary
 *
 * MTD Version:    2 only
 * Locking Method: Persistent and Non-persistent
 *
 * Description
 *
 * Use this for Spansion S26KS SPI flash capable of block locking.
 */
int sps26ks_islock(f3s_dbase_t *dbase,
                   f3s_access_t *access,
                   uint32_t flags,
                   uint32_t offset)
{
    uint8_t data;

    /* check that this chip supports Dynamic protection. */
    if (dbase->flags & F3S_PROTECT_DYN) {
        /* Write Entry Command */
        if (norspi_write_command(access->socket.socket_handle, HYPER_UNLOCK1_ADDR, HYPER_UNLOCK1_DATA) < 0) {
            fprintf(stderr, "Unlock1 failed.\n");
            return (EIO);
        }

        if (norspi_write_command(access->socket.socket_handle, HYPER_UNLOCK2_ADDR, HYPER_UNLOCK2_DATA) < 0) {
            fprintf(stderr, "Unlock2 failed.\n");
            return (EIO);
        }

        if (norspi_write_command(access->socket.socket_handle, HYPER_DYB_ENTRY_ADDR, HYPER_DYB_ENTRY_DATA) < 0) {
            fprintf(stderr, "Entry failed.\n");
            return (EIO);
        }

        /* Read data */
        if (norspi_read_registers(access->socket.socket_handle, offset, &data, 1) < 0) {
            fprintf(stderr, " failed.\n");
            return (EIO);
        }

        if (!(data & 0x01))
            return (EROFS);

        /* Reset/ASO Exit Command */
        if (norspi_write_command(access->socket.socket_handle, HYPER_RESET_ADDR, HYPER_RESET_DATA) < 0) {
            fprintf(stderr, "Reset/ASO exit failed.\n");
            return (EIO);
        }
    }

    /* The DYB bits aren't locked, check the PPB bits */
    if (dbase->flags & F3S_PROTECT_PERSISTENT) {
        /* Write Entry Command */
        if (norspi_write_command(access->socket.socket_handle, HYPER_UNLOCK1_ADDR, HYPER_UNLOCK1_DATA) < 0) {
            fprintf(stderr, "Unlock1 failed.\n");
            return (EIO);
        }

        if (norspi_write_command(access->socket.socket_handle, HYPER_UNLOCK2_ADDR, HYPER_UNLOCK2_DATA) < 0) {
            fprintf(stderr, "Unlock2 failed.\n");
            return (EIO);
        }

        if (norspi_write_command(access->socket.socket_handle, HYPER_PPB_ENTRY_ADDR, HYPER_PPB_ENTRY_DATA) < 0) {
            fprintf(stderr, "Entry failed.\n");
            return (EIO);
        }

        /* Read data */
        if (norspi_read_registers(access->socket.socket_handle, offset, &data, 1) < 0) {
            fprintf(stderr, " failed.\n");
            return (EIO);
        }

        if (!(data & 0x01))
            return (EROFS);

        /* Reset/ASO Exit Command */
        if (norspi_write_command(access->socket.socket_handle, HYPER_RESET_ADDR, HYPER_RESET_DATA) < 0) {
            fprintf(stderr, "Reset/ASO exit failed.\n");
            return (EIO);
        }
    }

    return (EOK);
}

/*
 * This is the ident callout for Spansion S26KS SPI serial NOR flash.
 */
int32_t sps26ks_ident(f3s_dbase_t *dbase,
                      f3s_access_t *access,
                      uint32_t flags,
                      uint32_t offset)
{
    int32_t     unit_size;
    int32_t     geo_index;
    int32_t     geo_pos;
    uint8_t     buf[88];
    int32_t     chip_size = 0;

    /* Write Entry Command */
    if (norspi_write_command(access->socket.socket_handle, HYPER_UNLOCK1_ADDR, HYPER_UNLOCK1_DATA) < 0) {
        fprintf(stderr, "Unlock1 failed.\n");
        return (EIO);
    }

    if (norspi_write_command(access->socket.socket_handle, HYPER_UNLOCK2_ADDR, HYPER_UNLOCK2_DATA) < 0) {
        fprintf(stderr, "Unlock2 failed.\n");
        return (EIO);
    }

    if (norspi_write_command(access->socket.socket_handle, HYPER_ID_ENTRY_ADDR, HYPER_ID_ENTRY_DATA) < 0) {
        fprintf(stderr, "Unlock2 failed.\n");
        return (EIO);
    }

    /* Read ID */
    if (norspi_read_registers(access->socket.socket_handle, 0, buf, 80) < 0) {
        fprintf(stderr, "Read JEDEC-ID failed\n");
        return (ENOENT);
    }

    /* Reset/ASO Exit Command */
    if (norspi_write_command(access->socket.socket_handle, HYPER_RESET_ADDR, HYPER_RESET_DATA) < 0) {
        fprintf(stderr, "Reset/ASO exit failed.\n");
        return (EIO);
    }

    if (buf[0x10] != 'Q' || buf[0x11] != 'R' || buf[0x12] != 'Y' ||
            buf[0x40] != 'P' || buf[0x41] != 'R' || buf[0x42] != 'I') {
        fprintf(stderr, "\n(devf  t%d::%s:%d) Identifed not supported \n", pthread_self(), __func__, __LINE__);
        fprintf(stderr, "%02x %02x %02x\n", buf[0x10], buf[0x11], buf[0x12]);
        return (ENOTSUP);
    }

    /* Fill dbase entry */
    dbase->struct_size = sizeof(*dbase);
    dbase->jedec_hi    = buf[0];
    dbase->jedec_lo    = ((uint16_t)buf[1] << 8) | buf[2];
    dbase->name        = "Spansion MirrotBit SIO";

    /* Read buffer size information */
    dbase->buffer_size = buf[0x2b];
    dbase->buffer_size <<= 8;
    dbase->buffer_size += buf[0x2a];

    /* Value is 2^N bytes per chip */
    dbase->buffer_size = 1 << dbase->buffer_size;

    /* Read number of geometries */
    dbase->geo_num = buf[0x2c];
    if (verbose > 3)
        fprintf(stderr, "(devf  t%d::%s:%d) dbase->geo_num = %d\n", pthread_self(), __func__, __LINE__, dbase->geo_num);

    /* Read geometry information */
    for (geo_index = 0, geo_pos = 0x2d; geo_index < dbase->geo_num; geo_index++, geo_pos += 4) {
        /* Read number of units */
        dbase->geo_vect[geo_index].unit_num   = buf[geo_pos + 1];
        dbase->geo_vect[geo_index].unit_num <<= 8;
        dbase->geo_vect[geo_index].unit_num  += buf[geo_pos + 0];
        dbase->geo_vect[geo_index].unit_num  += 1;

        /* Read size of unit */
        unit_size   = buf[geo_pos + 3];
        unit_size <<= 8;
        unit_size  += buf[geo_pos + 2];

        /* Interpret according to the CFI specs */
        if (unit_size == 0) unit_size  = 128;
        else                unit_size *= 256;

        chip_size += unit_size * dbase->geo_vect[geo_index].unit_num;

        /* Convert size to power of 2 */
        dbase->geo_vect[geo_index].unit_pow2 = 0;
        while (unit_size > 1) {
            unit_size >>= 1;
            dbase->geo_vect[geo_index].unit_pow2++;
        }

        if (verbose > 3) {
            fprintf(stderr, "(devf  t%d::%s:%d) dbase->geo_vect[%d].unit_pow2 = %d\n",
                pthread_self(), __func__, __LINE__, geo_index,dbase->geo_vect[geo_index].unit_pow2);
            fprintf(stderr, "(devf  t%d::%s:%d) dbase->geo_vect[%d].unit_num  = %d\n",
                pthread_self(), __func__, __LINE__, geo_index,dbase->geo_vect[geo_index].unit_num);
        }
    }

    access->socket.window_size = chip_size;

    /* Detect read / write suspend */
    if      (buf[0x46] == 1) dbase->flags = F3S_ERASE_FOR_READ;
    else if (buf[0x46] == 2) dbase->flags = F3S_ERASE_FOR_READ | F3S_ERASE_FOR_WRITE;
    else                     dbase->flags = 0;

    return (EOK);
}

/*
 * This is the erase callout for SPI serial NOR flash.
 */
int sps26ks_erase(f3s_dbase_t *dbase,
                  f3s_access_t *access,
                  uint32_t flags,
                  uint32_t offset)
{
    int         geo_index;
    int         size = 0;

    if (access->service->page(&access->socket, 0, offset, NULL) == NULL)
        return (ERANGE);

    for (geo_index = 0; geo_index < dbase->geo_num; geo_index++) {
        size += (1 << dbase->geo_vect[geo_index].unit_pow2) * dbase->geo_vect[geo_index].unit_num;

        if (size > offset)
            break;
    }

    /* 4 bytes address cycle, variable sector size */
    return norspi_sector_erase(access->socket.socket_handle,
        offset, 1 << dbase->geo_vect[geo_index].unit_pow2);
}

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/hardware/flash/boards/rcar_qspi/f3s_qspi_sps26ks.c $ $Rev: 811059 $")
#endif
