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


#define	CPUID_MASK			0xff00fff0u
#define	CPUID_IS_ARM(x)		((((x) >> 24) & 0xff) == 0x41)
#define	ARM_VARIANT(x)		(((x) >> 20) & 0xf)
#define	ARM_REVISION(x)		((x) & 0xf)

#define	AA64PFR0_SIMD(x)	((((x) >> 20) & 0xffUL) == 0)
#define	AA64PFR0_FP(x)		((((x) >> 16) & 0xffUL) == 0)

static paddr_t	el2_vbar;
int	trying_hvc;

static void
el2_setup(paddr_t vbar)
{
	//let init_one_cpuinfo() know that the CPU generated the proper HVC
	//exception type by clearing the trying_hvc variable
	trying_hvc = 0;
	aa64_sr_wr64(vbar_el2, vbar);
}


/*
 * Read cache size id register for this level and cache type
 */
static inline void
cache_sizes(unsigned csel, unsigned *nsets, unsigned *assoc, unsigned *lsize)
{
	aa64_sr_wr32(csselr_el1, csel);
	isb();
	unsigned const ccsidr = aa64_sr_rd32(ccsidr_el1);
	*nsets = ((ccsidr >> 13) & 0x7fff) + 1;
	*assoc = ((ccsidr >> 3) & 0x3ff) + 1;
	*lsize = 1 << ((ccsidr & 7) + 4);
}

static void
init_cache(struct cpuinfo_entry *cpu, unsigned cpunum)
{
	unsigned	clidr;
	unsigned	nsets;
	unsigned	lsize;
	unsigned	assoc;

	/*
	 * Cache maintenance instructions operate on all levels so we only
	 * add cacheattr information for the L1 cache(s).
	 */
	clidr = aa64_sr_rd32(clidr_el1);
	if (clidr & 4) {
		cache_sizes(0, &nsets, &assoc, &lsize);
		cpu->data_cache = add_cache(cpu->data_cache,
									CACHE_FLAG_UNIFIED,
									lsize, nsets * assoc,
									&cache_armv8_dcache);
	} else {
		if (clidr & 1) {
			cache_sizes(1, &nsets, &assoc, &lsize);
			cpu->ins_cache = add_cache(cpu->ins_cache,
										CACHE_FLAG_INSTR,
										lsize, nsets * assoc,
										&cache_armv8_icache);
		}
		if (clidr & 2) {
			cache_sizes(0, &nsets, &assoc, &lsize);
			cpu->data_cache = add_cache(cpu->data_cache,
										CACHE_FLAG_DATA,
										lsize, nsets * assoc,
										&cache_armv8_dcache);
		}
	}

	if (debug_flag) {
		int			i;
		unsigned	ctype;
		unsigned	ctr;
		static const char *l1ip[] = {
			"unknown", "AIVIVT", "VIPT", "PIPT"
		};

		ctr = aa64_sr_rd32(dczid_el0);
		if (ctr & (1u << 4)) {
			kprintf("cpu%d: DCZID_EL0=%w (%d bytes)\n",
					ctr, 1 << ((ctr & 0xf) + 2));
		}

		ctr = aa64_sr_rd32(ctr_el0);
		kprintf("cpu%d: CWG=%d ERG=%d Dminline=%d Iminline=%d %s\n",
			cpunum,
			(ctr >> 24) & 0xf,
			(ctr >> 20) & 0xf,
			(ctr >> 16) & 0xf,
			ctr & 0xf,
			l1ip[(ctr >> 14) & 3]);

		kprintf("cpu%d: CLIDR=%w LoUU=%d LoC=%d LoUIS=%d\n",
			cpunum,
			clidr,
			(clidr >> 27) & 7,
			(clidr >> 24) & 7,
			(clidr >> 21) & 7);

		for (i = 0, ctype = clidr; i < 7; i++, ctype >>= 3) {
			unsigned	t = (ctype & 7);

			if (t == 0 || t > 4) {
				/*
				 * Cache not present or has a bogus value
				 */
				break;
			}

			if (t & 4) {
				cache_sizes((i << 1), &nsets, &assoc, &lsize);
				kprintf("cpu%d: L%d Unified %dK linesz=%d set/way=%d/%d\n",
					cpunum, i+1,
					lsize * nsets * assoc / 1024,
					lsize, nsets, assoc);
			} else {
				/*
				 * Might have separate I/D caches at each level
				 */
				if (t & 1) {
					cache_sizes((i << 1) | 1, &nsets, &assoc, &lsize);
					kprintf("cpu%d: L%d Icache %dK linesz=%d set/way=%d/%d\n",
						cpunum, i+1,
						lsize * nsets * assoc / 1024,
						lsize, nsets, assoc);
				}
				if (t & 2) {
					cache_sizes((i << 1), &nsets, &assoc, &lsize);
					kprintf("cpu%d: L%d Dcache %dK linesz=%d set/way=%d/%d\n",
						cpunum, i+1,
						lsize * nsets * assoc / 1024,
						lsize, nsets, assoc);
				}
			}
		}
	}
}


void
init_one_cpuinfo(unsigned cpunum)
{
	struct cpuinfo_entry	*cpu;
	unsigned				cpuid;
	unsigned long			mpidr;
	const char				*name;
	unsigned long			pfr;
	struct aarch64_cpuid	**idp;
	struct aarch64_cpuid	*id;

	if(have_el2) {
		trying_hvc = 1;
		register void (*func) asm ("x1") = el2_setup;
		register uintptr_t parm asm ("x0") = el2_vbar;
		asm volatile ("hvc #0" :: "r" (parm), "r" (func) : "x2", "memory");
		if(trying_hvc) {
			// If trying_hvc is still set after the instruction, the HVC
			// opcode is not enabled/supported (e.g. EL3 has it turned off).
			have_el2 = 0;
			trying_hvc = 0;
		}
	}

	cpu = &lsp.cpuinfo.p[cpunum];

	/*
	 * Get the CPUID and processor name
	 */
	cpuid = aa64_sr_rd32(midr_el1);
	cpu->cpu = cpuid;

	for (idp = aarch64_cpuid; (id = *idp) != 0; idp++) {
		if (id->midr == (cpuid & CPUID_MASK)) {
			break;
		}
	}
	name = id ? id->name : "Unknown";
	cpu->name = add_string(name);

	/*
	 * FIXME_AARCH64: smp_hwcoreid is 32-bit so we lose MPIDR.Aff3
	 *                This is the least of our problems if we encounter a
	 *                system with 4 levels of cpu clusters...
	 */
	mpidr = aa64_sr_rd64(mpidr_el1);
	cpu->smp_hwcoreid = (unsigned)mpidr;

	if (cpu_freq != 0) {
		cpu->speed = cpu_freq / 1000000;
	} else {
		cpu->speed = aarch64_cpuspeed();
	}

	if (debug_flag) {
		kprintf("cpu%d: MPIDR=%x\n", cpunum, mpidr);
		if (CPUID_IS_ARM(cpuid)) {
			kprintf("cpu%d: MIDR=%w %s r%dp%d\n",
					cpunum, cpuid, name,
					ARM_VARIANT(cpuid), ARM_REVISION(cpuid));
		} else {
			kprintf("cpu%d: MIDR=%w %s\n", cpunum, cpuid, name);
		}
	}

	cpu->flags = 0;
	if (shdr->flags1 & STARTUP_HDR_FLAGS1_VIRTUAL) {
		cpu->flags |= CPU_FLAG_MMU;
	}
	if (lsp.syspage.p->num_cpu > 1) {
		cpu->flags |= AARCH64_CPU_FLAG_SMP;
	}

	/*
	 * Detect if we have FP/SIMD.
	 * Both share the same register file so set CPU_FLAG_FPU.
	 */
	pfr = aa64_sr_rd64(id_aa64pfr0_el1);
	if (AA64PFR0_SIMD(pfr) || AA64PFR0_FP(pfr)) {
		cpu->flags |= CPU_FLAG_FPU;
		if (AA64PFR0_SIMD(pfr)) {
			cpu->flags |= AARCH64_CPU_FLAG_SIMD;
		}
	}

	/*
	 * Enable EL0 access to CNTVCT_EL0 for ClockCycles()
	 */
	aa64_sr_wr32(cntkctl_el1, 1 << 1);

	/*
	 * Set legacy ARMv7 flags for Aarch32 binary compatibility
	 */
	if (cpu->flags & AARCH64_CPU_FLAG_SMP) {
		cpu->flags |= AARCH32_CPU_FLAG_V7_MP;
	}
	if (cpu->flags & CPU_FLAG_FPU) {
		cpu->flags |= AARCH32_CPU_FLAG_VFP_D32;
	}
	cpu->flags |= AARCH32_CPU_FLAG_V6|AARCH32_CPU_FLAG_V7|AARCH32_CPU_FLAG_IDIV;

	/*
	 * Set up cache callouts
	 */
	init_cache(cpu, cpunum);

	/*
	 * Set up GIC CPU interface for this cpu
	 */
	if (gic_cpu_init) {
		gic_cpu_init(cpunum);
	}

	/*
	 * Perform any cpu-specific setup (eg. errata workarounds)
	 */
	if (id && id->cpuinfo) {
		id->cpuinfo(cpunum, cpu);
	}
}

void
init_cpuinfo()
{
	struct cpuinfo_entry	*cpu;
	unsigned				num;
	unsigned				i;

	if(have_el2) {
		// allocate an area for the EL2 exception vector table
		el2_vbar = alloc_ram(NULL_PADDR, 0x800, 0x800);
		if(el2_vbar == NULL_PADDR) {
			crash("No memory for EL2 exception table.\n");
		}
	}
	num = lsp.syspage.p->num_cpu;

	cpu = set_syspage_section(&lsp.cpuinfo, sizeof(*lsp.cpuinfo.p) * num);
	for (i = 0; i < num; i++) {
		cpu[i].ins_cache  = system_icache_idx;
		cpu[i].data_cache = system_dcache_idx;
	}
	init_one_cpuinfo(0);
	if(have_el2) {
		as_add(el2_vbar, el2_vbar+0x7ff, AS_ATTR_RAM, "hypervisor_vector",
				as_find(AS_NULL_OFF, "memory", NULL));
	}
}



#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/hardware/startup/lib/aarch64/init_cpuinfo.c $ $Rev: 812970 $")
#endif
