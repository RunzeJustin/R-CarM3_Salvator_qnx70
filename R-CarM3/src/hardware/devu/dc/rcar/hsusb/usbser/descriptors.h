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


/*
    Descriptor information for USB CDC Serial device (ACM)
*/

#ifndef _USB_CDC_SERIAL_RCAR_DESCRIPTORS_H_INCLUDED
#define _USB_CDC_SERIAL_RCAR_DESCRIPTORS_H_INCLUDED

/* Override Vendor/Device ID            */

/* Override String Descriptor Table     */

/* Include Base Descriptor Header       */
#include <hw/usbdc_desc_serial.h>

/* Set DMA on endpoints( endpoint 0 doesn't support DMA ) */
#define USBDC_DMA_ENABLE_MASK_IN    0x0
#define USBDC_DMA_ENABLE_MASK_OUT   0x0

#endif

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/hardware/devu/dc/rcar/hsusb/usbser/descriptors.h $ $Rev: 810496 $")
#endif
