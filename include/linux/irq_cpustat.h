#ifndef __ASM_I386_IRQ_CPUSTAT_H
#define __ASM_I386_IRQ_CPUSTAT_H
#include <asm-i386/hardirq.h>

// 单处理器
#define __IRQ_STAT(cpu, member) (irq_stat[cpu].member)
// 获取标志位
// #define softirq_pending(cpu)  __IRQ_STAT((cpu), __softirq_pending) 后续版本的改进
#define softirq_active(cpu) __IRQ_STAT((cpu), __softirq_active)
#define softirq_mask(cpu)	__IRQ_STAT((cpu), __softirq_mask)
#define local_irq_count(cpu)  __IRQ_STAT((cpu), __local_irq_count)
#define local_bh_count(cpu)	__IRQ_STAT((cpu), __local_bh_count)

#endif