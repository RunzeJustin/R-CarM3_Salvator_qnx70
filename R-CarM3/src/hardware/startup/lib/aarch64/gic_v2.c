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

#include <startup.h>
#include <aarch64/gic_v2.h>

/*
 * GICv2 support code
 */

static struct startup_intrinfo gic_v2_intr = {
    .vector_base      = _NTO_INTR_CLASS_EXTERNAL,
    .num_vectors      = 0,
    .cascade_vector   = _NTO_INTR_SPARE,
    .cpu_intr_base    = 0,
    .cpu_intr_stride  = 0,
    .flags            = 0,
    .id  = { INTR_GENFLAG_LOAD_SYSPAGE, 0, &interrupt_id_gic_v2},
    .eoi = { INTR_GENFLAG_LOAD_SYSPAGE | INTR_GENFLAG_LOAD_INTRMASK, 0, &interrupt_eoi_gic_v2},
    .mask             = &interrupt_mask_gic_v2,
    .unmask           = &interrupt_unmask_gic_v2,
    .config           = &interrupt_config_gic_v2,
};

static unsigned
gic_v2_cpunum(void)
{
	unsigned	mask;
	unsigned	i;

	/*
	 * Figure out our GIC cpu interface number for sending IPIs.
	 * GICv2 supports only 8 processors and ITARGETSR0 byte 0 contains the
	 * target mask for this cpu's ID0 SGI interrupt.
	 */
	mask = in32(gic_gicd + ARM_GICD_ITARGETSRn) & 0xff;
	if ((mask == 0) && (lsp.syspage.p->num_cpu == 1)) {
		// uniprocessor
		return 0;
	}
	for (i = 0; i < 8; i++) {
		if (mask == (1 << i)) {
			return i;
		}
	}
	crash("gic_v2_cpunum: no target cpu detected %x\n", in32(gic_gicd + ARM_GICD_ITARGETSRn));
	return 0;
}

static void
gic_v2_gicc_init(unsigned cpu)
{
	struct aarch64_gic_map_entry	*gic_map = lsp.cpu.aarch64_gic_map.p;

	if (gic_gicd == 0 || gic_gicc == 0) {
		crash("gic not initialised");
	}
	gic_map->gic_cpu[cpu] = gic_v2_cpunum();
	if (debug_flag) {
		kprintf("cpu%d: GICv2 cpu interface %d\n", cpu, gic_map->gic_cpu[cpu]);
	}

	/*
	 * Disable all banked PPI interrupts
	 * Enable all SGI interrupts
	 */
	out32(gic_gicd + ARM_GICD_ICENABLERn, 0xffff0000);
	out32(gic_gicd + ARM_GICD_ISENABLERn, 0x0000ffff);

	/*
	 * Set priority mask to allow all interrupts and enable cpu interface
	 */
	out32(gic_gicc + ARM_GICC_PMR, 0xF0);
	out32(gic_gicc + ARM_GICC_CTLR, ARM_GICC_CTLR_EN);
}

void
gic_v2_init(paddr_t gicd, paddr_t gicc)
{
	struct aarch64_gic_map_entry	*gic_map = lsp.cpu.aarch64_gic_map.p;
	unsigned	gic_cpu;
	unsigned	itn;
	unsigned	i;

	gic_gicd = gicd;
	gic_gicc = gicc;
	gic_cpu_init = gic_v2_gicc_init;
	gic_sendipi = &sendipi_gic_v2;

	/*
	 * Initialise the GIC cpu map with invalid values.
	 * gic_v2_gicc_init() will set the mapping for each cpu as it comes up.
	 */
	for (i = 0; i < lsp.syspage.p->num_cpu; i++) {
		gic_map->gic_cpu[i] = ~0u;
	}
	gic_cpu = gic_v2_cpunum();

	/*
	 * Disable distributor
	 */
	out32(gicd + ARM_GICD_CTLR, 0);

	/*
	 * Calculate number of interrupt lines.
	 */
	itn = ((in32(gicd + ARM_GICD_TYPER) & ARM_GICD_TYPER_ITLN) + 1) * 32;
	gic_v2_intr.num_vectors = itn;

	if (debug_flag) {
		kprintf("GICv2: %d interrupts\n", itn);
	}

	/*
	 * Disable all interrupts and clear pending state
	 */
	for (i = 0; i < itn; i += 32) {
		out32(gicd + ARM_GICD_ICENABLERn + (i * 4 / 32), 0xffffffff);
		out32(gicd + ARM_GICD_ICPENDRn + (i * 4 / 32), 0xffffffff);
	}

	/*
	 * Set default priority of all interrupts to 0xa0
	 */
	for (i = 0; i < itn; i += 4) {
		out32(gicd + ARM_GICD_IPRIORITYn + i, 0xa0a0a0a0);
	}

	/*
	 * Route all SPI interrupts to cpu0
	 */
	if (debug_flag) {
		kprintf("GICv2: routing SPIs to gic cpu %d\n", gic_cpu);
	}
	for (i = 32; i < itn; i += 4) {
		out32(gicd + ARM_GICD_ITARGETSRn + i, (0x01010101u << gic_cpu));
	}

	/*
	 * Default all PPI/SPI interrupts as level triggered
	 */
	for (i = 16; i < itn; i += 16) {
		out32(gicd + ARM_GICD_ICFGRn + (i * 4 / 16), 0);
	}

	/*
	 * Enable distributor - cpu interface is initialised via init_cpuinfo().
	 */
	out32(gicd + ARM_GICD_CTLR, ARM_GICD_CTLR_EN);

	/*
	 * Add the interrupt callouts
	 */
	add_interrupt(&gic_v2_intr);
}



#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/hardware/startup/lib/aarch64/gic_v2.c $ $Rev: 811926 $")
#endif
