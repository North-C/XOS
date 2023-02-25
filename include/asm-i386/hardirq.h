#ifndef __ASM_I386_HARDIRQ_H
#define __ASM_I386_HARDIRQ_H

#include <linux/cache.h>
#include <linux/irq_cpustat.h>
typedef struct {
    // unsigned int __softirq_pending;   // 软中断请求
    unsigned int __softirq_active;
	unsigned int __softirq_mask;
    unsigned int __local_irq_count;     // 硬中断
    unsigned int __local_bh_count;
    unsigned int __syscall_count;
    struct task_struct * __ksoftirqd_task;
    unsigned int __nmi_count;
}____cacheline_aligned irq_cpustat_t;

extern irq_cpustat_t irq_stat[];


#define in_interrupt() ({ int __cpu = smp_processor_id(); \
    (local_irq_count(__cpu) + local_bh_count(__cpu) != 0); })

#define irq_enter(cpu, irq) (local_irq_count(cpu)++)
#define irq_exit(cpu, irq) (local_irq_count(cpu)--)

#define hardirq_trylock(cpu)	(local_irq_count(cpu) == 0)
#define hardirq_endlock(cpu)	do { } while (0)

#endif