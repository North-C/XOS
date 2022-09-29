#include <asm-i386/smp.h>
#include <linux/interrupt.h>
#include <linux/kernel_stat.h>

struct kernel_stat kstat;
// TODO: schedule
// void sched_init(void)
// {
//     // init_bh(TIMER_BH, timer_bh);
// }