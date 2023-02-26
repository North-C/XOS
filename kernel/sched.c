#include <asm-i386/smp.h>
#include <linux/interrupt.h>
#include <linux/kernel_stat.h>
#include <asm-i386/softirq.h>
#include <linux/sched.h>

extern void timer_bh(void);
struct kernel_stat kstat;

// TODO: schedule
void sched_init(void)
{
    init_bh(TIMER_BH, timer_bh);
}