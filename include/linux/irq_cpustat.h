#ifndef __ASM_I386_IRQ_CPUSTAT_H
#define __ASM_I386_IRQ_CPUSTAT_H


extern irq_cpustat_t irq_stat[];

// 单处理器
#define __IRQ_STAT(cpu, member) (irq_stat[cpu].member)

#define softirq_pending(cpu)  __IRQ_STAT((cpu), __softirq_pending)
#define local_irq_count(cpu)  __IRQ_STAT((cpu), __local_irq_count)


#endif