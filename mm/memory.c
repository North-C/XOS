#include <linux/mm.h>

unsigned long max_mapnr;   // 系统中最大的页面数
unsigned long num_physpages;    //  系统中的最大页面数
unsigned long num_mappedpages;
mem_map_t * mem_map;   // 全局的物理内存页面
void* high_memory;   // 高端内存开始处的虚拟地址