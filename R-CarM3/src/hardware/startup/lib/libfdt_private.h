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
#ifndef _LIBFDT_PRIVATE_H
#define _LIBFDT_PRIVATE_H

#include <libfdt.h>

/**********************************************************************/
/* Util functions                                                    */
/**********************************************************************/

typedef int		fdtintr_t[FDT_MAX_NCELLS];

int fdt_get_u32(const void *fdt, int node, int offset, const char *name, uint32_t *pp);
int fdt_get_u64(const void *fdt, int node, int offset, const char *name, uint64_t *pp);
int fdt_get_num64(const void *fdt, int node, const char *name, uint64_t *pp);
int fdt_get_str(const void *fdt, int node, const char *name, const char **pp);
int fdt_find_node(const void *fdt, int node, const char *type, const char *compatible);
int fdt_get_reg64_cells(const void *fdt, int node, int index, int num, uint64_t *addr, uint64_t *size, int addr_cells, int size_cells);
int fdt_get_reg64(const void *fdt, int node, int index, uint64_t *addr, uint64_t *size);
int fdt_get_reg32(const void *fdt, int node, int index, uint32_t *addr, uint32_t *size);
int fdt_get_reg_int(const void *fdt, int node, int *pp);
int fdt_get_reg_addr(const void *fdt, int node, unsigned *addr);
int fdt_get_int(const void *fdt, int node, int offset, const char *name, int *pp);
int fdt_get_intr_cells(const void *fdt, int node, int *parent);
int fdt_get_intr(const void *fdt, int node, int index, fdtintr_t intr[], int num, int *controller);

#endif /* _LIBFDT_PRIVATE_H */
#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/hardware/startup/lib/libfdt_private.h $ $Rev: 811485 $")
#endif
