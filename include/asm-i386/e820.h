#ifndef __E820_HEADER
#define __E820_HEADER

#include <asm-i386/e820.h>

#define E820MAP 0x2d0       // 存放的地址偏移量
#define E820MAX 32          // 在E820MAP当中有几个表项
#define E820NR  0x1e8       // E820MAP当中的entries


#define HIGH_MEMORY (1024 * 1024)   // 高于 1MB 的内存称之为 High Memory
struct e820map{
    int nr_map;         // 数量
    struct e820entry{       // 描述一个物理内存区间
        unsigned long long addr;        // 内存段的开始
        unsigned long long size;        // 内存段的大小
        unsigned long type;             // 内存段的类型
    }map[E820MAX];
};

extern struct e820map e820;     

// 四种不同的物理内存类型
#define E820_RAM       1
#define E820_RESERVED  2
#define E820_ACPI      3
#define E820_NVS       4   // Non-Volatile Storage 不挥发存储器，ROM等

#endif