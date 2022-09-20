#include <linux/interrupt.h>
#include <linux/types.h>
#include <asm-i386/smp.h>
#include <asm-i386/hardirq.h>
#include <linux/cache.h>
#include <linux/threads.h>

irq_cpustat_t irq_stat[NR_CPUS] ____cacheline_aligned;

// asmlinkage void do_softirq()
// {
//     int cpu = smp_processor_id();
// 	__u32 pending;
// 	unsigned long flags;
// 	__u32 mask;

//     if(in_interrupt())
//         return;

//     local_irq_save(flags);

//     pending = softirq_pending(cpu);

//     if(pending){
//         struct softirq_action *h;

//         mask = ~pending;
//         local_bh_disablle();

// restart:
//         softirq_pending(cpu) = 0;

//         local_irq_enable();

//         h = softirq_vec;

//         do{
//             if(pending & 1)
//                 h->action(h);
//             h++;
//             pending >>= 1;
//         }while(pending);

//         local_irq_disable();

//         pending = softirq_pending(cpu);
// 		if (pending & mask) {
// 			mask &= ~pending;
// 			goto restart;
// 		}
// 		__local_bh_enable();

// 		if (pending)
// 			wakeup_softirqd(cpu);
// 	}

// 	local_irq_restore(flags);
// }

// void softirq_init()
// {
//     int i;


// }