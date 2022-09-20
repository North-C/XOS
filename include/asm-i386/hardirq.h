#ifndef __ASM_I386_HARDIRQ_H
#define __ASM_I386_HARDIRQ_H

#include <linux/cache.h>

typedef struct {
    unsigned int __softirq_pending;
    unsigned int __local_irq_count;
    unsigned int __local_bh_count;
    unsigned int __syscall_count;
    struct task_struct * __ksoftirqd_task;
    unsigned int __nmi_count;
}____cacheline_aligned irq_cpustat_t;

#include <linux/irq_cpustat.h>

#define irq_enter(cpu, irq) (local_irq_count(cpu)++)
#define irq_exit(cpu, irq) (local_irq_count(cpu)--)

#endif