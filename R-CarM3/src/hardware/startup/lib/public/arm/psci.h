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
 * ARM Power State Co-ordination Interface.
 * Based on PSCI-1.0 (ARM DEN0022C).
 */

#ifndef __ARM_PSCI_H_INCLUDED
#define __ARM_PSCI_H_INCLUDED

#include <inttypes.h>

/*
 * Function IDs
 */
#define	PSCI_VERSION				0x84000000U
#define	PSCI_CPU_SUSPEND			0x84000001U
#define	PSCI_CPU_OFF				0x84000002U
#define	PSCI_CPU_ON					0x84000003U
#define	PSCI_AFFINITY_INFO			0x84000004U
#define	PSCI_MIGRATE				0x84000005U
#define	PSCI_MIGRATE_INFO_TYPE		0x84000006U
#define	PSCI_MIGRATE_INFO_UP_CPU	0x84000007U
#define	PSCI_SYSTEM_OFF				0x84000008U
#define	PSCI_SYSTEM_RESET			0x84000009U
#define	PSCI_FEATURES				0x8400000aU
#define	PSCI_CPU_FREEZE				0x8400000bU
#define	PSCI_CPU_DEFAULT_SUSPEND	0x8400000cU
#define	PSCI_NODE_HW_STATE			0x8400000dU
#define	PSCI_SYSTEM_SUSPEND			0x8400000eU
#define	PSCI_SET_SUSPEND_MODE		0x8400000fU
#define	PSCI_STAT_RESIDENCY			0x84000010U
#define	PSCI_STAT_COUNT				0x84000011U

/*
 * Error codes.
 */
#define	PSCI_SUCCESS			(0)
#define	PSCI_NOT_SUPPORTED		(-1)
#define	PSCI_INVALID_PARAMETERS	(-2)
#define	PSCI_DENIED				(-3)
#define	PSCI_ALREADY_ON			(-4)
#define	PSCI_ON_PENDING			(-5)
#define	PSCI_INTERNAL_FAILURE	(-6)
#define	PSCI_NOT_PRESENT		(-7)
#define	PSCI_DISABLED			(-8)
#define	PSCI_INVALID_ADDRESS	(-9)

/*
 * Invocation mechanism is determined at runtime: psci_hvc or psci_svc
 */
extern uint32_t	(*psci_call)(uint32_t __x0, uint32_t __x1, uint32_t __x2, uint32_t __x3);
extern uint32_t	psci_hvc(uint32_t __x0, uint32_t __x1, uint32_t __x2, uint32_t __x3);
extern uint32_t	psci_smc(uint32_t __x0, uint32_t __x1, uint32_t __x2, uint32_t __x3);

static inline uint32_t
psci_version(void)
{
	return (uint32_t)psci_call(PSCI_VERSION, 0, 0, 0);
}

static inline int32_t
psci_cpu_suspend(uint32_t power_state, uint32_t entry_point, uint32_t context_id)
{
	return (int32_t)psci_call(PSCI_CPU_SUSPEND, power_state, entry_point, context_id);
}

static inline int32_t
psci_cpu_off(void)
{
	return (int32_t)psci_call(PSCI_CPU_OFF, 0, 0, 0);
}

static inline int32_t
psci_cpu_on(uint64_t target_cpu, uint32_t entry_point, uint32_t context_id)
{
	return (int32_t)psci_call(PSCI_CPU_ON, target_cpu, entry_point, context_id);
}

static inline int32_t
psci_affinity_info(uint32_t target_affinity, uint32_t lowest_affinity)
{
	return (int32_t)psci_call(PSCI_AFFINITY_INFO, target_affinity, lowest_affinity, 0);
}

static inline int32_t
psci_migrate(uint32_t target_cpu)
{
	return (int32_t)psci_call(PSCI_MIGRATE, target_cpu, 0, 0);
}

static inline int32_t
psci_migrate_info_type(void)
{
	return (int32_t)psci_call(PSCI_MIGRATE_INFO_TYPE, 0, 0, 0);
}

static inline uint32_t
psci_migrate_info_up_cpu(void)
{
	return psci_call(PSCI_MIGRATE_INFO_UP_CPU, 0, 0, 0);
}

static inline void
psci_system_off(void)
{
	psci_call(PSCI_SYSTEM_OFF, 0, 0, 0);
}

static inline void
psci_system_reset(void)
{
	psci_call(PSCI_SYSTEM_RESET, 0, 0, 0);
}

static inline int32_t
psci_features(uint32_t func_id)
{
	return (int32_t)psci_call(PSCI_FEATURES, func_id, 0, 0);
}

static inline int32_t
psci_cpu_freeze(void)
{
	return (int32_t)psci_call(PSCI_CPU_FREEZE, 0, 0, 0);
}

static inline int32_t
psci_cpu_default_suspend(uint32_t target_cpu, uint32_t entry_point, uint32_t context_id)
{
	return (int32_t)psci_call(PSCI_CPU_DEFAULT_SUSPEND, target_cpu, entry_point, context_id);
}

static inline int32_t
psci_node_hw_state(uint32_t target_cpu, uint32_t power_level)
{
	return (int32_t)psci_call(PSCI_NODE_HW_STATE, target_cpu, power_level, 0);
}

static inline int32_t
psci_system_suspend(uint32_t entry_point, uint32_t context_id)
{
	return (int32_t)psci_call(PSCI_SYSTEM_SUSPEND, entry_point, context_id, 0);
}

static inline int32_t
psci_set_suspend_mode(uint32_t mode)
{
	return (int32_t)psci_call(PSCI_SET_SUSPEND_MODE, mode, 0, 0);
}

static inline uint64_t
psci_stat_residency(uint32_t target_cpu, uint32_t power_state)
{
	return psci_call(PSCI_STAT_RESIDENCY, target_cpu, power_state, 0);
}

static inline uint64_t
psci_stat_count(uint32_t target_cpu, uint32_t power_state)
{
	return psci_call(PSCI_STAT_COUNT, 0, 0, 0);
}

#endif	/* __AARCH64_PSCI_H_INCLUDED */

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/hardware/startup/lib/public/arm/psci.h $ $Rev: 805440 $")
#endif
