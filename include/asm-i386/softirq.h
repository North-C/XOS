#ifndef __ASM_I386_SOFTIRQ_H
#define __ASM_I386_SOFTIRQ_H

#include <asm-i386/hardirq.h>
#include <asm-i386/atomic.h>
#include <linux/kernel.h>

#define __cpu_bh_enable(cpu) \
		do { barrier(); local_bh_count(cpu)--; } while (0)
#define cpu_bh_disable(cpu) \
		do { local_bh_count(cpu)++; barrier(); } while (0)

#define local_bh_disable()	cpu_bh_disable(smp_processor_id())
#define __local_bh_enable()	__cpu_bh_enable(smp_processor_id())

void __tasklet_schedule(struct tasklet_struct *t);
void __tasklet_hi_schedule(struct tasklet_struct *t);

void init_bh(int nr, void (*routine)(void));

#endif