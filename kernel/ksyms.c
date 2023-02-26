#include <linux/interrupt.h>


// software interrupts
EXPORT_SYMBOL(tasklet_hi_vec);
EXPORT_SYMBOL(tasklet_vec);
EXPORT_SYMBOL(bh_task_vec);
EXPORT_SYMBOL(init_bh);
EXPORT_SYMBOL(remove_bh);
EXPORT_SYMBOL(tasklet_init);
EXPORT_SYMBOL(tasklet_kill);
EXPORT_SYMBOL(__run_task_queue);
EXPORT_SYMBOL(do_softirq);
EXPORT_SYMBOL(raise_softirq);
EXPORT_SYMBOL(cpu_raise_softirq);
EXPORT_SYMBOL(__tasklet_schedule);
EXPORT_SYMBOL(__tasklet_hi_schedule);