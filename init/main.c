#include <asm-i386/types.h>
#include <linux/stdio.h>
#include <asm-i386/system.h>

void start_kernel(void)
{
    printk("Start kernel\n");
    setup_arch();
    trap_init();
    init_IRQ();
    sched_init();
	softirq_init();
	time_init();

    kmem_cache_init();     // 初始化 slab 分配器
    printk("kmem_cache_init done\n");
    sti();
    mem_init();    // 清除之前使用的临时性的目录项，收集统计信息
    printk("mem_init done\n");
    // while(1);
    kmem_cache_sizes_init();   // 初始化指定大小的高速缓存
    printk("kmem_cache_sizes_init done\n");
    // page_cache_init();
    while(1);
}
