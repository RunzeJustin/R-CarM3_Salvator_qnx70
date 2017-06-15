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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/resmgr.h>
#include <sys/neutrino.h>
#include <hw/inout.h>
#include <sys/slog.h>
#include <sys/slogcodes.h>
#include <errno.h>
#include <arm/r-car-m3.h>
#include <sys/procmgr.h>
#include <drvr/hwinfo.h>

int main(int argc, char *argv[])
{
    int         opt;
    int         priority = 10;  /* default priority:default 10 */
    int         time = -1;      /* default time for watchdog timer kick */
    uintptr_t   base;
    paddr_t     physbase = RCAR_RWDT_BASE;
    int         verbose = 0;
	struct sched_param threadparam;
	int	sched_type;

    base    = 0;

    /* Process dash options.*/
    while ((opt = getopt(argc, argv, "v:p:t:")) != -1) {
        switch (opt) {
            case 'v':   // WDOG register physics based address
                verbose = 1;
                break;
            case 'p':   // priority
                priority = strtoul(optarg, NULL, 0);
                break;
            case 't':
                time = strtoul(optarg, NULL, 0);
                break;
        }
    }

    /* check if the params are valid */
    if (time == -1) {
        slogf(_SLOGC_HB_SERVER, _SLOG_INFO,
                "%s: Invalid default time for watchdog timer kick. Please check the command line or Hwinfo default setting", argv[0]);
        return EXIT_FAILURE;
    }

    if (verbose)
        fprintf(stdout, "  Watchdog timer kick frequency .: Every %d milliseconds\n", time);

    // Enable IO capability.
    if (ThreadCtl(_NTO_TCTL_IO, NULL) == -1) {
        slogf(_SLOGC_HB_SERVER, _SLOG_INFO, "%s: ThreadCtl NTO_TCTL_IO failed", argv[0]);
        return EXIT_FAILURE;
    }

    // Run in the background
    if (procmgr_daemon(EXIT_SUCCESS, PROCMGR_DAEMON_NOCLOSE | PROCMGR_DAEMON_NODEVNULL) == -1) {
        slogf(_SLOGC_HB_SERVER, _SLOG_INFO, "%s:  procmgr_daemon failed", argv[0]);
        return EXIT_FAILURE;
    }

    // If requested: Change priority.
    if (pthread_getschedparam(pthread_self(),&sched_type, &threadparam) != EOK){
    	slogf(_SLOGC_HB_SERVER, _SLOG_WARNING, "%s:  pthread_getschedparam failed", argv[0]);
    	return EXIT_FAILURE;
    }

    if (priority != threadparam.sched_priority){
    	threadparam.sched_priority = priority;
    	if (pthread_setschedparam(pthread_self(),sched_type, &threadparam) != EOK) {
    		slogf(_SLOGC_HB_SERVER, _SLOG_WARNING, "%s: Can't change priority", argv[0]);
    	}
    }

    base = mmap_device_io(12, physbase);
    if (base == MAP_DEVICE_FAILED) {
        slogf(_SLOGC_HB_SERVER, _SLOG_INFO, "%s: Failed to map WDOG registers", argv[0]);
        return EXIT_FAILURE;
    }

    if (!(in16(base + RCAR_WDT_CSRA) & (1 << 7))) {
        slogf(_SLOGC_HB_SERVER, _SLOG_INFO, "%s: Watchdog Timer is disabled now", argv[0]);
        slogf(_SLOGC_HB_SERVER, _SLOG_INFO, "%s: Terminate Watchdog Timer Module", argv[0]);
        goto done;
    }

    // Loop indefinately, kicking the timer every so often
    slogf(_SLOGC_HB_SERVER, _SLOG_INFO, "%s: Watchdog Timer kick every %d milliSeconds", argv[0], time);

    while (1) {
        out32(base + RCAR_WDT_CNT, 0x5A5AFF00);     // reset counter
        delay(time);
    }

done:
    munmap_device_io(base, 12);

    return EXIT_SUCCESS;
}

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/hardware/support/wdtkick-rcar/main.c $ $Rev: 808713 $")
#endif
