#ifndef _I386_PAGE_H
#define _I386_PAGE_H

// 决定页面的大小
#define PAGE_SHIFT 12
#define PAGE_SIZE (1UL << PAGE_SHIFT)
#define PAGE_MASK  (~(PAGE_SIZE-1))
#define __PAGE_OFFSET   (0xC0000000)
#define PAGE_OFFSET     ((unsigned long)__PAGE_OFFSET)

// 没有设置 PAE 位的情况
typedef struct { unsigned long pte_low; } pte_t;
typedef struct { unsigned long pmd;  } pmd_t;
typedef struct { unsigned long pgd; } pgd_t;

#define PAGE_PRESENT    0x001
#define PAGE_RW         0x002       // Read 为0，Write 为1
#define PAGE_USER       0x004
#define PAGE_PWT        0x008
#define PAGE_PCD        0x010
#define PAGE_ACCESSED   0x020
#define PAGE_DIRTY      0x040



#endif