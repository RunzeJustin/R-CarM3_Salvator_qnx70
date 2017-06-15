/*
 * $QNXLicenseC:
 * Copyright 2014, QNX Software Systems. 
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

#include "startup.h"
#include "aarch64/mmu.h"

/*
 * Default MAIR_EL1 value.
 * Encodings make AttrIndex[2:0] identical to ARMv7 TEX/CB=00x/xx.
 *
 * Index  MAIR  ARMv8 meaning ARMv7 meaning
 * -----  ----  ------------- -------------
 *     0  0x00  device-nGnRnE strongly-ordered
 *     1  0x04  device-nGnRE  shared-device
 *     2  0xaa  normal-WTnWA  normal-WTnWA
 *     3  0xee  normal-WBnWA  normal-WBnWA
 *     4  0x44  normal-NC     normal-NC
 *     5  0x00  reserved      reserved
 *     6  0x00  impl defined  impl defined
 *     7  0xff  normal-WBWA   normal-WBWA
 */
unsigned long	mair_el1 = 0xff000044eeaa0400UL;

/*
 * Default value of SCTLR_E1 when enabling MMU:
 * - enable user mode access for cache maintenance (UCI/UCT)
 * - enable user mode wfe (nTWE) but not user mode wfi (nTWI = 0)
 * - enable data cache zero (DZE)
 * - enable stack alignment checks (SA0/SA)
 * - enable caches and MMU (I/C/M)
 * - disable SETEND instruction (SED)
 *
 * NOTE: assumes little-endian operation only (EE/E0E/SED = 0)
 */
unsigned	sctlr_el1 = AARCH64_SCTLR_EL1_RES1 |
						AARCH64_SCTLR_EL1_UCI |
						AARCH64_SCTLR_EL1_nTWE |
						AARCH64_SCTLR_EL1_nTWI |
						AARCH64_SCTLR_EL1_UCT |
						AARCH64_SCTLR_EL1_DZE |
						AARCH64_SCTLR_EL1_I |
						AARCH64_SCTLR_EL1_SED |
						AARCH64_SCTLR_EL1_SA0 |
						AARCH64_SCTLR_EL1_SA |
						AARCH64_SCTLR_EL1_C |
						AARCH64_SCTLR_EL1_M;

/*
 * Default value of TCR_EL1:
 * - 40 bit TTBR1 range
 * - 39 bit TTBR0 range
 * - 4K granule size for TTBR0/1
 * - inner/outer WBWA for page table lookups
 *
 * init_mmu() sets ASID and PA size based on the cpu configuration.
 */
unsigned long	tcr_el1 = AARCH64_TCR_EL1_T1SZ(40) |
						  AARCH64_TCR_EL1_T0SZ(39) |
						  AARCH64_TCR_EL1_TG1_4K |
						  AARCH64_TCR_EL1_TG0_4K |
						  AARCH64_TCR_EL1_ORGN1_WBWA |
						  AARCH64_TCR_EL1_ORGN0_WBWA |
						  AARCH64_TCR_EL1_IRGN1_WBWA |
						  AARCH64_TCR_EL1_IRGN0_WBWA;

unsigned long	ttbr0;
unsigned long	ttbr1[PROCESSORS_MAX];

#define	MMFR0_ASIDBITS(x)	(((x) >> 4) & 0b1111UL)
#define	MMFR0_PARANGE(x)	((x) & 0b111UL)

/*
 * Initialise MMU structures
 */
void
init_mmu(void)
{
	unsigned long	mmfr0;
	unsigned		ncpu = lsp.syspage.p->num_cpu;
	paddr_t			start;
	paddr_t			end;

	/*
	 * Set TCR_EL1 ASID/PA sizes and shareability
	 */
	__asm__ __volatile__("mrs	%0, id_aa64mmfr0_el1" : "=r"(mmfr0));
	if (MMFR0_ASIDBITS(mmfr0)) {
		tcr_el1 |= AARCH64_TCR_EL1_AS;
	}
	tcr_el1 |= AARCH64_TCR_EL1_IPS(MMFR0_PARANGE(mmfr0));

	if (ncpu > 1) {
		/*
		 * Mark page tables as inner-shareable
		 */
		tcr_el1 |= AARCH64_TCR_EL1_SH1_ISH | AARCH64_TCR_EL1_SH0_ISH;
	}
	if (debug_flag) {
		static int	par[] = { 32, 36, 40, 42, 44, 48 };

		kprintf("MMU: %d-bit ASID %d-bit PA TCR_EL1=%x\n",
			MMFR0_ASIDBITS(mmfr0) ? 16 : 8,
			par[MMFR0_PARANGE(mmfr0)],
			tcr_el1);
	}

	/*
	 * Set up initial page tables etc.
	 */
	aarch64_map_init();

	/*
	 * Create identity mapping for startup code/data
	 */
	start = TRUNCPG(shdr->image_paddr);
	end   = ROUNDPG(shdr->image_paddr + shdr->startup_size);
	aarch64_map(start, start, end-start, PROT_READ|PROT_WRITE|PROT_EXEC);
}



#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/hardware/startup/lib/aarch64/init_mmu.c $ $Rev: 778261 $")
#endif
