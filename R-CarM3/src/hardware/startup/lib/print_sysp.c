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

//
// NOTE: This file is shared between pidin and the startup library.
// Ordinarily, we'd put it in a library that both code references, but
// that would complicate building startups in a manner that we don't
// want. Instead, a copy of this file is placed in both locations
// and a distinct print_sysp.h file in both directories provide the
// 'glue' to hook up this code with the particularities of the environment.
// If you make any changes to this file, you should also copy the new
// version to the other location (after first checking that no-one's
// modified the other copy and forgotten to update this location).
// Normally this won't be a problem since the file will only change in
// response to system page layout changes and the copy in startup will
// be modified first and then copied to the pidin tree.
//
// This file is in:
//	utils/p/pidin/print_sysp.c <=> hardware/startup/lib/print_sysp.c




#define NUM_ELTS(__array)	(sizeof(__array)/sizeof(__array[0]))

#include "print_sysp.h"

void
print_typed_strings(void) {
	struct typed_strings_entry	*string = _SYSPAGE_ENTRY(PSP_SYSPAGE, typed_strings);
	void		*type_ptr;
	unsigned	type;
	unsigned	i;

	i = 0;
	for( ;; ) {
		type_ptr = &string->data[i];
		type = PSP_NATIVE_ENDIAN32(*(uint32_t *)type_ptr);
		if(type == _CS_NONE) break;
		i += sizeof(uint32_t);
		kprintf("  off:%d type:%d string:'%s'\n", i-sizeof(uint32_t), type, &string->data[i]);
		i += strlen(&string->data[i]) + 1;
		i = (i + (sizeof(uint32_t)-1)) & ~(sizeof(uint32_t)-1);
	}
}


void
print_strings(void) {
	char		*p = _SYSPAGE_ENTRY(PSP_SYSPAGE,strings)->data;
	char		*start = p;
	unsigned	off;
	unsigned	len;
	char		buff[80];

	kprintf(" ");
	off = 1;
	while(*p != '\0') {
		PSP_SPRINTF(buff, " [%d]'%s'", (unsigned)(p - start), p);
		len = strlen(buff);
		if((off + len) >= 80) {
			kprintf("\n ");
			off = 1;
		}
		kprintf("%s", buff);
		off += len;
		p += strlen(p) + 1;
	}
	kprintf("\n");
}


void
print_system_private(void) {
	struct system_private_entry	*private = _SYSPAGE_ENTRY(PSP_SYSPAGE,system_private);

	kprintf("  syspage ptr user:%v kernel:%v\n", PSP_NATIVE_ENDIANPTR(private,user_syspageptr), PSP_NATIVE_ENDIANPTR(private,kern_syspageptr));
	kprintf("  cpupage ptr user:%v kernel:%v spacing:%d\n", PSP_NATIVE_ENDIANPTR(private,user_cpupageptr), PSP_NATIVE_ENDIANPTR(private,kern_cpupageptr), PSP_NATIVE_ENDIAN32(private->cpupage_spacing));
	kprintf("  kdebug info:%v callback:%v num_ready:%u\n", PSP_NATIVE_ENDIANPTR(private,kdebug_info), PSP_NATIVE_ENDIANPTR(private,kdebug_call), PSP_NATIVE_ENDIAN32(private->num_ready));
	kprintf("  pagesize:%x flags:%x, kdinfo:%x tracebuf:%v\n",
			PSP_NATIVE_ENDIAN32(private->pagesize),
			PSP_NATIVE_ENDIAN32(private->private_flags),
			PSP_NATIVE_ENDIAN32(private->kdump_info),
			PSP_NATIVE_ENDIANPTR(private,tracebuf));
}


void
print_meminfo(void) {
	struct meminfo_entry *ram = _SYSPAGE_ENTRY(PSP_SYSPAGE,meminfo);
	int i = 0;

	kprintf(" ");
	while(ram->type != PSP_NATIVE_ENDIAN32(MEMTYPE_NONE)) {
		if(++i%4 == 0)
			kprintf("\n ");
		kprintf(" t:%d a:%l s:%l",
			PSP_NATIVE_ENDIAN32(ram->type),
			PSP_NATIVE_ENDIAN32(ram->addr),
			PSP_NATIVE_ENDIAN32(ram->size));
		++ram;
	}
	kprintf("\n");
}


static char *
get_string(unsigned off) {
	return &_SYSPAGE_ENTRY(PSP_SYSPAGE,strings)->data[off];
}


#if !defined(PSP_STARTUP)		
static void
asinfo_string_name(struct asinfo_entry *base, struct asinfo_entry *curr) {
	struct list {
		struct list	*next;
		struct asinfo_entry	*as;
	}		*chain, *new;

	chain = NULL;
	for( ;; ) {
		uint16_t   own;
		new = alloca(sizeof(*chain));
		new->next = chain;
		new->as = curr;
		chain = new;
		own = PSP_NATIVE_ENDIAN16(curr->owner);
		if(own == AS_NULL_OFF) break;
		curr = (struct asinfo_entry *)((uint8_t *)base + own);
	}
	while(chain != NULL) {
		kprintf("/%s", get_string(PSP_NATIVE_ENDIAN16(chain->as->name)));
		chain = chain->next;
	}
}
#endif

static void
print_an_asinfo(const syspage_entry_info *const info, unsigned const elsize) {
	struct asinfo_entry 	*as = (void *)((uintptr_t)PSP_SYSPAGE + info->entry_off);
	int						num;
	int						i;

	num = info->entry_size / elsize;
	for(i = 0; i < num; ++i) {
		kprintf("  %w) %L-%L o:%w a:%w p:%d ",
				i*sizeof(*as),
				PSP_NATIVE_ENDIAN64(as->start),
				PSP_NATIVE_ENDIAN64(as->end),
				PSP_NATIVE_ENDIAN16(as->owner),
				PSP_NATIVE_ENDIAN16(as->attr),
				PSP_NATIVE_ENDIAN16(as->priority));
		if(as->alloc_checker64==0) {
			kprintf("c:0 n:");
		} else {
			kprintf("c:%v n:", PSP_NATIVE_ENDIANPTR(as,alloc_checker));
		}
#if defined(PSP_STARTUP)		
		kprintf("%d\n", as->name);
#else
		asinfo_string_name((void *)((uintptr_t)PSP_SYSPAGE + info->entry_off), as);
		kprintf("\n");
#endif		
		as = SYSPAGE_ARRAY_ADJ_OFFSET(asinfo, as, elsize);
	}
}

void
print_asinfo(void) {
	print_an_asinfo((syspage_entry_info *)&PSP_SYSPAGE->asinfo, _SYSPAGE_ELEMENT_SIZE(PSP_SYSPAGE, asinfo));
}

#if !defined(PSP_STARTUP)		
static void
print_old_asinfo(void) {
	print_an_asinfo(&PSP_SYSPAGE->old_asinfo, sizeof(struct asinfo_entry));
}
#endif


void
print_hwinfo(void) {
	hwi_tag				*tag = (hwi_tag *)_SYSPAGE_ENTRY(PSP_SYSPAGE,hwinfo);
	void				*base;
	void				*next;
	char				*name;

	while(tag->prefix.size != 0) {
		next = (hwi_tag *)((uint32_t *)tag +
			PSP_NATIVE_ENDIAN16(tag->prefix.size));
		base = (void *)(&tag->prefix + 1);
		name = get_string(PSP_NATIVE_ENDIAN16(tag->prefix.name));
		kprintf("  %d) size:%d tag:%d(%s)", 
				hwi_tag2off(tag),
				PSP_NATIVE_ENDIAN16(tag->prefix.size),
				PSP_NATIVE_ENDIAN16(tag->prefix.name), name);
		if(*name >= 'A' && *name <= 'Z') {
			base = (void *) (&tag->item + 1);
			kprintf(" isize:%d, iname:%d(%s), owner:%d, kids:%d",
					PSP_NATIVE_ENDIAN16(tag->item.itemsize),
					PSP_NATIVE_ENDIAN16(tag->item.itemname), 
					get_string(PSP_NATIVE_ENDIAN16(tag->item.itemname)),
					PSP_NATIVE_ENDIAN16(tag->item.owner), PSP_NATIVE_ENDIAN16(tag->item.kids));
		}
		if(base != next) {
			kprintf("\n    ");
			while(base < next) {
				uint8_t		*p = base;
	
				kprintf(" %b", *p);
				base = p + 1;
			}
		}
		kprintf("\n");
		tag = next;
	}
}


void
print_qtime(void) {
	struct qtime_entry *qtime = _SYSPAGE_ENTRY(PSP_SYSPAGE,qtime);

	kprintf("  boot:%x CPS:%L rate/scale:%d/-%d intr:%d\n",
		PSP_NATIVE_ENDIAN32(qtime->boot_time),
		PSP_NATIVE_ENDIAN64(qtime->cycles_per_sec),
		PSP_NATIVE_ENDIAN32(qtime->timer_rate),
		-(int)PSP_NATIVE_ENDIAN32(qtime->timer_scale),
		(int)PSP_NATIVE_ENDIAN32(qtime->intr)
		);
	kprintf("  flags:%x load:%d epoch:%d rr_mul:%d adj count/inc:%d/%d\n",
		PSP_NATIVE_ENDIAN32(qtime->flags),
		PSP_NATIVE_ENDIAN32(qtime->timer_load),
		PSP_NATIVE_ENDIAN32(qtime->epoch),
		PSP_NATIVE_ENDIAN32(qtime->rr_interval_mul),
		PSP_NATIVE_ENDIAN32(qtime->adjust.tick_count),
		PSP_NATIVE_ENDIAN32(qtime->adjust.tick_nsec_inc));

#if !defined(PSP_STARTUP)	
	kprintf("  nsec:%L stable:%L inc:%x\n", 
			PSP_NATIVE_ENDIAN64(qtime->nsec),
			PSP_NATIVE_ENDIAN64(qtime->nsec_stable),
			PSP_NATIVE_ENDIAN32(qtime->nsec_inc));
	kprintf("  nsec_tod_adj:%L\n", PSP_NATIVE_ENDIAN64(qtime->nsec_tod_adjust));
#endif	
}


static void
print_an_cpuinfo(const syspage_entry_info *const info, unsigned const elsize) {
	struct cpuinfo_entry *cpu = (void *)((uintptr_t)PSP_SYSPAGE + info->entry_off);
	unsigned i;

	for(i = 0; i < (unsigned)PSP_SYSPAGE->num_cpu; ++i) {
		kprintf("  %d) cpu:%x flg:%x spd:%d hwid:%x cache i/d:%d/%d name:%d\n",
			i,
			PSP_NATIVE_ENDIAN32(cpu->cpu),
			PSP_NATIVE_ENDIAN32(cpu->flags),
			PSP_NATIVE_ENDIAN32(cpu->speed),
			PSP_NATIVE_ENDIAN32(cpu->smp_hwcoreid),
			cpu->ins_cache,
			cpu->data_cache,
			PSP_NATIVE_ENDIAN16(cpu->name));
#if !defined(PSP_STARTUP)
		kprintf("     history:%L\n", PSP_NATIVE_ENDIAN64(cpu->idle_history));
#endif		
		cpu = SYSPAGE_ARRAY_ADJ_OFFSET(cpuinfo, cpu, elsize);
	}
}

void
print_cpuinfo(void) {
	print_an_cpuinfo((syspage_entry_info *)&PSP_SYSPAGE->cpuinfo, _SYSPAGE_ELEMENT_SIZE(PSP_SYSPAGE, cpuinfo));
}

#if !defined(PSP_STARTUP)		
static void
print_old_cpuinfo(void) {
	print_an_cpuinfo(&PSP_SYSPAGE->old_cpuinfo, sizeof(struct cpuinfo_entry));
}
#endif

static void
print_an_cacheattr(const syspage_entry_info *const info, unsigned const elsize) {
	struct cacheattr_entry *cache = (void *)((uintptr_t)PSP_SYSPAGE + info->entry_off);
	int						num;
	int						i;

	num = _SYSPAGE_ENTRY_SIZE(PSP_SYSPAGE,cacheattr) / elsize;
	for(i = 0; i < num; ++i ) {
		kprintf("  %d) flags:%b size:%w #lines:%w ways:%w control:%v next:%d\n",
			i,
			PSP_NATIVE_ENDIAN32(cache->flags),
			PSP_NATIVE_ENDIAN32(cache->line_size),
			PSP_NATIVE_ENDIAN32(cache->num_lines),
			PSP_NATIVE_ENDIAN16(cache->ways),
			PSP_NATIVE_ENDIANPTR(cache,control),
			PSP_NATIVE_ENDIAN32(cache->next));
		cache = SYSPAGE_ARRAY_ADJ_OFFSET(cacheattr, cache, elsize);
	}
}

void
print_cacheattr(void) {
	print_an_cacheattr((syspage_entry_info *)&PSP_SYSPAGE->cacheattr, _SYSPAGE_ELEMENT_SIZE(PSP_SYSPAGE, cacheattr));
}

#if !defined(PSP_STARTUP)		
static void
print_old_cacheattr(void) {
	print_an_cacheattr(&PSP_SYSPAGE->old_cacheattr, sizeof(struct cacheattr_entry));
}
#endif


void
print_callout(void) {
	struct callout_entry	*call = _SYSPAGE_ENTRY(PSP_SYSPAGE,callout);
	unsigned				i;

	kprintf("  reboot:%v power:%v watchdog:%v\n",
		PSP_NATIVE_ENDIANPTR(call,reboot),
		PSP_NATIVE_ENDIANPTR(call,power),
		PSP_NATIVE_ENDIANPTR(call,debug_watchdog));
	kprintf("  timer_load:%v reload:%v value:%v\n",
			PSP_NATIVE_ENDIANPTR(call,timer_load),
			PSP_NATIVE_ENDIANPTR(call,timer_reload),
			PSP_NATIVE_ENDIANPTR(call,timer_value));
	for(i = 0; i < 2; ++i) {
#if defined(PSP_STARTUP) 
		struct debug_callout	*dbg = &call->debug[i];

		kprintf("  %d) display:%v poll:%v break:%v\n", i,
			PSP_NATIVE_ENDIANPTR(dbg,display_char),
			PSP_NATIVE_ENDIANPTR(dbg,poll_key),
			PSP_NATIVE_ENDIANPTR(dbg,break_detect));
#else
		if(PSP_SYSPAGE->type & SYSPAGE_64BIT) {
			struct debug_callout64	*dbg = &call->debug64[i];

			kprintf("  %d) display:%L poll:%L break:%L\n", i,
				PSP_NATIVE_ENDIAN64(dbg->display_char64),
				PSP_NATIVE_ENDIAN64(dbg->poll_key64),
				PSP_NATIVE_ENDIAN64(dbg->break_detect64));
		} else {
			struct debug_callout32	*dbg = &call->debug32[i];

			kprintf("  %d) display:%x poll:%x break:%x\n", i,
				PSP_NATIVE_ENDIAN32(dbg->display_char32),
				PSP_NATIVE_ENDIAN32(dbg->poll_key32),
				PSP_NATIVE_ENDIAN32(dbg->break_detect32));
		}
#endif	
	}
}

static void
print_intrgen(char *name, struct __intrgen_data64 *gen) {
	kprintf("     %s => flags:%w, size:%w, rtn:%v\n",
		name,
		PSP_NATIVE_ENDIAN16(gen->genflags),
		PSP_NATIVE_ENDIAN16(gen->size),
		PSP_NATIVE_ENDIANPTR(gen,rtn));
}

void
print_intrinfo(void) {
 	struct intrinfo_entry *intr = _SYSPAGE_ENTRY(PSP_SYSPAGE,intrinfo);
	int						num;
	int						i;

	const unsigned elsize = _SYSPAGE_ELEMENT_SIZE(PSP_SYSPAGE, intrinfo);
	num = _SYSPAGE_ENTRY_SIZE(PSP_SYSPAGE, intrinfo) / elsize;
	for( i = 0; i < num; ++i ) {
		kprintf("  %d) vector_base:%x, #vectors:%d, cascade_vector:%x\n",
				i,
				PSP_NATIVE_ENDIAN32(intr->vector_base),
				PSP_NATIVE_ENDIAN32(intr->num_vectors),
				PSP_NATIVE_ENDIAN32(intr->cascade_vector));
		kprintf("     cpu_intr_base:%x, cpu_intr_stride:%d, flags:%w, local_stride:%d\n",
				PSP_NATIVE_ENDIAN32(intr->cpu_intr_base),
				PSP_NATIVE_ENDIAN16(intr->cpu_intr_stride),
				PSP_NATIVE_ENDIAN16(intr->flags),
				PSP_NATIVE_ENDIAN32(intr->local_stride));
		print_intrgen(" id", &intr->id);
		print_intrgen("eoi", &intr->eoi);
		
		kprintf("     mask:%v, unmask:%v, config:%v\n",
			PSP_NATIVE_ENDIANPTR(intr,mask),
			PSP_NATIVE_ENDIANPTR(intr,unmask),
			PSP_NATIVE_ENDIANPTR(intr,config));
		intr = SYSPAGE_ARRAY_ADJ_OFFSET(new_intrinfo, intr, elsize);
	}
}

#if !defined(PSP_STARTUP)

static void
print_old_intrgen(char *name, struct __intrgen_data32 *gen) {
	kprintf("     %s => flags:%w, size:%w, rtn:%x\n",
		name,
		PSP_NATIVE_ENDIAN16(gen->genflags),
		PSP_NATIVE_ENDIAN16(gen->size),
		PSP_NATIVE_ENDIAN32(gen->rtn32));
}

static void
print_old_intrinfo(void) {
 	struct old_intrinfo_entry *intr = _SYSPAGE_ENTRY(PSP_SYSPAGE,old_intrinfo);
	int						num;
	int						i;

	num = _SYSPAGE_ENTRY_SIZE(PSP_SYSPAGE, old_intrinfo) / sizeof(*intr);
	for( i = 0; i < num; ++i ) {
		kprintf("  %d) vector_base:%x, #vectors:%d, cascade_vector:%x\n",
				i,
				PSP_NATIVE_ENDIAN32(intr->vector_base),
				PSP_NATIVE_ENDIAN32(intr->num_vectors),
				PSP_NATIVE_ENDIAN32(intr->cascade_vector));
		kprintf("     cpu_intr_base:%x, cpu_intr_stride:%d, flags:%w\n",
				PSP_NATIVE_ENDIAN32(intr->cpu_intr_base),
				PSP_NATIVE_ENDIAN16(intr->cpu_intr_stride),
				PSP_NATIVE_ENDIAN16(intr->flags));
		print_old_intrgen(" id", &intr->id);
		print_old_intrgen("eoi", &intr->eoi);
		
		kprintf("     mask:%x, unmask:%x, config:%x\n",
			PSP_NATIVE_ENDIAN32(intr->mask32),
			PSP_NATIVE_ENDIAN32(intr->unmask32),
			PSP_NATIVE_ENDIAN32(intr->config32));
		intr = SYSPAGE_ARRAY_ADJ_OFFSET(old_intrinfo, intr, sizeof(*intr));
	}
}
#endif

void
print_smp(void) {
	struct smp_entry *smp = _SYSPAGE_ENTRY(PSP_SYSPAGE,smp);

	kprintf("  send_ipi:%v cpu:%x cpu2:%L\n",
		PSP_NATIVE_ENDIANPTR(smp,send_ipi),
		PSP_NATIVE_ENDIAN32(smp->cpu),
		PSP_NATIVE_ENDIAN64(smp->cpu2));
}

void
print_pminfo(void) {
	struct pminfo_entry *pm = _SYSPAGE_ENTRY(PSP_SYSPAGE,pminfo);

	kprintf("  wakeup_pending:%x wakeup_condition:%x\n",
		PSP_NATIVE_ENDIAN32(pm->wakeup_pending),
		PSP_NATIVE_ENDIAN32(pm->wakeup_condition));
}

void
print_mdriver(void) {
	struct new_mdriver_entry *md = _SYSPAGE_ENTRY(PSP_SYSPAGE,new_mdriver);
	int						num;
	int						i;

	unsigned const elsize = _SYSPAGE_ELEMENT_SIZE(PSP_SYSPAGE,new_mdriver);
	num = _SYSPAGE_ENTRY_SIZE(PSP_SYSPAGE,new_mdriver) / elsize;
	for(i = 0; i < num; ++i) {
		kprintf("  %d) name=%d, intr=%x, rtn=%v, paddr=%L, size=%d\n", i, 
				PSP_NATIVE_ENDIAN32(md->name),
				PSP_NATIVE_ENDIAN32(md->intr),
				PSP_NATIVE_ENDIANPTR(md,handler),
				PSP_NATIVE_ENDIAN64(md->data_paddr),
				PSP_NATIVE_ENDIAN32(md->data_size));
		md = SYSPAGE_ARRAY_ADJ_OFFSET(new_mdriver, md, elsize);
	}
}

#if !defined(PSP_STARTUP)
void
print_old_mdriver(void) {
	struct old_mdriver_entry *md = _SYSPAGE_ENTRY(PSP_SYSPAGE,old_mdriver);
	int						num;
	int						i;

	num = _SYSPAGE_ENTRY_SIZE(PSP_SYSPAGE,old_mdriver) / sizeof(*md);
	for(i = 0; i < num; ++i) {
		kprintf("  %d) name=%d, intr=%x, rtn=%x, paddr=%x, size=%d\n", i, 
				PSP_NATIVE_ENDIAN32(md->name),
				PSP_NATIVE_ENDIAN32(md->intr),
				PSP_NATIVE_ENDIAN32(md->handler32),
				PSP_NATIVE_ENDIAN32(md->data_paddr),
				PSP_NATIVE_ENDIAN32(md->data_size));
		md = SYSPAGE_ARRAY_ADJ_OFFSET(old_mdriver, md, sizeof(*md));
	}
}
#endif

#define SYSPAGE_TYPE_MASK	0x01ffu
#define BC_SECTION			0x0200u
#define ARRAY_SECTION		0x0400u
#define INFO_SECTION		0x0800u
#define IMPLICIT_DISABLE	0x1000u
#define EXPLICIT_DISABLE	0x2000u
#define EXPLICIT_ENABLE		0x4000u

struct debug_syspage_section {
	const char 		*name;
	unsigned short	loc;
	unsigned short	flags;
	void			(*print)(void);
};

#define PRT_SYSPAGE_RTN(name, array)	\
	{ #name, (unsigned short)offsetof(struct syspage_entry, name), INFO_SECTION|(array), print_##name }

#define CPU_PRT_SYSPAGE_RTN(upper_cpu, lower_cpu, flags, name)	\
	{ #name, (unsigned short)offsetof(struct syspage_entry, un.lower_cpu.name), \
		(flags) + (SYSPAGE_##upper_cpu+1), \
		lower_cpu##_print_##name }

#if defined(PSP_STARTUP)
	#define PRT_SYSPAGE_RTN_BC(name)
#else
	#define PRT_SYSPAGE_RTN_BC(name) PRT_SYSPAGE_RTN(name,BC_SECTION|IMPLICIT_DISABLE),
#endif

static struct debug_syspage_section sp_section[] = {
	PRT_SYSPAGE_RTN(system_private,0),
	PRT_SYSPAGE_RTN(qtime,0),
	PRT_SYSPAGE_RTN(callout,0),
//	PRT_SYSPAGE_RTN(callin),
	PRT_SYSPAGE_RTN(cpuinfo,ARRAY_SECTION),
	PRT_SYSPAGE_RTN_BC(old_cpuinfo)
	PRT_SYSPAGE_RTN(cacheattr,ARRAY_SECTION),
	PRT_SYSPAGE_RTN_BC(old_cacheattr)
	PRT_SYSPAGE_RTN(asinfo,ARRAY_SECTION),
	PRT_SYSPAGE_RTN_BC(old_asinfo)
	PRT_SYSPAGE_RTN(hwinfo,0),
	PRT_SYSPAGE_RTN(typed_strings,0),
	PRT_SYSPAGE_RTN(strings,0),
	PRT_SYSPAGE_RTN(intrinfo,ARRAY_SECTION),
	PRT_SYSPAGE_RTN_BC(old_intrinfo)
	PRT_SYSPAGE_RTN(smp,0),
	PRT_SYSPAGE_RTN(pminfo,0),
	PRT_SYSPAGE_RTN(mdriver,ARRAY_SECTION),
	PRT_SYSPAGE_RTN_BC(old_mdriver)
// This second include of print_sysp.h will cause the CPU_PRT_SYSPAGE_RTN
// definitions for the various routines to be added.
#include "print_sysp.h"	
};

void
print_syspage_enable(const char *name) {
	unsigned	i;
	unsigned	on_bit;
	unsigned	off_mask;
	
	if(*name == '~') {
		++name;
		on_bit = EXPLICIT_DISABLE;
		off_mask = ~EXPLICIT_ENABLE;
	} else {
		on_bit = EXPLICIT_ENABLE;
		off_mask = ~EXPLICIT_DISABLE;
	}
	for(i = 0; i < NUM_ELTS(sp_section); ++i) {
		if(strcmp(sp_section[i].name, name) == 0) {
			sp_section[i].flags &= off_mask;
			sp_section[i].flags |= on_bit;
		}
		if(on_bit & EXPLICIT_ENABLE) {
			// If we have an explict enable, we mark all the sections
			// as implicitly disabled so that we don't print them out
			// unless we end up with an explict enablement of the entry
			sp_section[i].flags |= IMPLICIT_DISABLE;
		}
	}
}

void
print_syspage_sections(void) {
	unsigned	i;
	unsigned	flags;
	unsigned	type;

	kprintf("Header size=0x%x, Total Size=0x%x, #Cpu=%d, Type=%d\n",
		PSP_SYSPAGE->size,
		PSP_SYSPAGE->total_size,
		PSP_SYSPAGE->num_cpu,
		PSP_SYSPAGE->type);
#if !defined(PSP_STARTUP)
		for(i = 0; i < NUM_ELTS(sp_section); ++i) {
			flags = sp_section[i].flags;
			if(flags & EXPLICIT_ENABLE) {
				flags &= ~IMPLICIT_DISABLE;
			}
			type = flags & SYSPAGE_TYPE_MASK;
			if((type != 0) && (type != (PSP_SYSPAGE->type + 1))) {
				// Not a section on this system page
				flags |= EXPLICIT_DISABLE;
			}
			if(!(flags & (EXPLICIT_DISABLE|IMPLICIT_DISABLE))) {
				syspage_array_info	  *info = (void *)((uint8_t *)PSP_SYSPAGE + sp_section[i].loc);
				if(sp_section[i].flags & ARRAY_SECTION) {
					if(sp_section[i].loc > PSP_SYSPAGE->size) {
						// We don't have the sized array, show the
						// backwards compact section
						sp_section[i+1].flags &= ~IMPLICIT_DISABLE;
					} else if(info->entry_size == 0) {
						syspage_array_info	  *bc_info = (void *)((uint8_t *)PSP_SYSPAGE + sp_section[i+1].loc);
						if(bc_info->entry_size != 0) {
							// We don't have any information in
							// the array section, but there's something
							// in the backwards compat one, so show that
							sp_section[i+1].flags &= ~IMPLICIT_DISABLE;
						}
					}
				}
				if(syspage_cross_endian) {
					/* need to swap the various section's 
					 * entry_off/entry_size fields ahead of time
					 * since some sections depend on other sections 
					 * (for example asinfo and hwinfo are printing data
					 * from the strings section)
					 */
					if(sp_section[i].loc < PSP_SYSPAGE->size) {
						if(sp_section[i].flags & INFO_SECTION) {
							ENDIAN_SWAP16(&info->entry_off);
							ENDIAN_SWAP16(&info->entry_size);
							if(sp_section[i].flags & ARRAY_SECTION) {
								ENDIAN_SWAP16(&info->element_size);
							}
						}
					}
				}
			}
		}
#endif

	for(i = 0; i < NUM_ELTS(sp_section); ++i) {
		flags = sp_section[i].flags;
		if(flags & EXPLICIT_ENABLE) {
			flags &= ~IMPLICIT_DISABLE;
		}
		type = flags & SYSPAGE_TYPE_MASK;
		if((type != 0) && (type != (PSP_SYSPAGE->type + 1))) {
			// Not a section on this system page
			flags |= EXPLICIT_DISABLE;
		}
		if(sp_section[i].loc > PSP_SYSPAGE->size) {
			// Section isn't present on this system page
			flags |= EXPLICIT_DISABLE;
		}
		if(!(flags & (EXPLICIT_DISABLE|IMPLICIT_DISABLE))) {
			kprintf("Section:%s ", sp_section[i].name);
			if(sp_section[i].flags & INFO_SECTION) {
				syspage_array_info	*info;

				info = (void *)((uint8_t *)PSP_SYSPAGE + sp_section[i].loc);
				kprintf("offset:0x%x size:0x%x", info->entry_off, info->entry_size);
				if(flags & ARRAY_SECTION) {
					kprintf(" elsize:0x%x", info->element_size);
				}
				kprintf("\n");
				if(info->entry_size > 0 && PSP_VERBOSE(2)) {
					sp_section[i].print();
				}
			} else {
				kprintf("offset:0x%x\n", sp_section[i].loc);
				if(PSP_VERBOSE(2)) {
					sp_section[i].print();
				}
			}
		}
	}
}

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/hardware/startup/lib/print_sysp.c $ $Rev: 780356 $")
#endif
