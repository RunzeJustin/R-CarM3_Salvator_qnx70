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

#ifndef WFD_COMMON_H_
#define WFD_COMMON_H_

#ifndef GCCATTR
# if __GNUC__ >= 4
#  define GCCATTR(x) __attribute__ ((x))
# else
#  define GCCATTR(x)
# endif
#endif

// SYM_PRIVATE indicates that the marked symbol won't be visible outside
// the compiled library.  It can be exported via a pointer, or used by
// other source files that are part of the same library.
#ifndef SYM_PRIVATE
# define SYM_PRIVATE GCCATTR(visibility ("hidden"))
#endif

// SYM_INTERNAL_ONLY indicates the symbol won't be used outside the library,
// even via a function pointer.
#ifndef SYM_INTERNAL_ONLY
# define SYM_INTERNAL_ONLY GCCATTR(visibility ("internal"))
#endif

SYM_INTERNAL_ONLY int create_temp_shm_object(void);

#endif /* WFD_COMMON_H_ */
