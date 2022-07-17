#ifndef _LINUX_BOOTMEM_H
#define _LINUX_BOOTMEM_H
#include <linux/init.h>

// 系统内存中的每一个节点都有一个对应的 bootmem_data
typedef struct bootmem_data{
    unsigned long node_boot_start;      // 系统引导后的第一个物理内存页面
    unsigned long node_low_pfn;         // 块的结束地址,或者该节点表示的ZONE_NORMAL的结束，即物理内存的顶点，不超过896MB
    void * node_bootmem_map;       // 指向一个保留页面位图，每一位代表着保留或不存在的，不能分配的物理页面
    unsigned long last_offset;        // 最后一次分配时的页面的偏移，为0，则表示全部使用
    unsigned long last_pos;             // 最后一次分配时的页面帧数
} bootmem_data_t;


extern unsigned long max_low_pfn;
extern unsigned long min_low_pfn;
extern unsigned long max_pfn;


extern unsigned long __init init_bootmem(unsigned long start, unsigned long pages);
extern void __init free_bootmem (unsigned long addr, unsigned long size);
extern void __init reserve_bootmem(unsigned long addr, unsigned long size);

#define alloc_bootmem_low_pages(x) \
    __alloc_bootmem((x), PAGE_SIZE, 0)
    
extern void * __init __alloc_bootmem(unsigned long size, unsigned long align, unsigned long goal);

#endif