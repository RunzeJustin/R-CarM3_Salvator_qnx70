/*
 * $QNXLicenseC:
 * Copyright 2013, QNX Software Systems. 
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
 
#include <stdlib.h>
#include <stdio.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <errno.h>
#include <stdbool.h>
#include <string.h>
#include "wfd_common.h"
#include "debug.h"
#define MAX_EEXIST_RETRIES 5

/* create a shared memory object which will allow us to control cache options */
SYM_INTERNAL_ONLY int
create_temp_shm_object()
{
    char name[32];
    int fd, retries = MAX_EEXIST_RETRIES;
    static bool isRandSeeded = false;
    TRACE_WFD;
    if(!isRandSeeded) {
        srandom((unsigned int) time(NULL));
        isRandSeeded = true;
    }
    do {
        /* create a random name to prevent namespace squatting attacks */
        snprintf(name, sizeof(name), "/wfd:%ld", random());
        fd = shm_open(name, O_RDWR | O_CREAT | O_EXCL, 0600);
        if (fd != -1) {
            shm_unlink(name);
            return fd;
        }
        DC_ERROR("Failed to create %s - %s", name, strerror(errno));
    } while (errno == EEXIST && retries--);
    return -1;
}
