#ifndef _ASM_FIXMAP_H
#define _ASM_FIXMAP_H
#include <asm-i386/page.h>

// 在 vmalloc 和fixedmap起始 之间留下一个 空 的 page
#define FIXADDR_TOP (0xffffe000UL)
#define __fix_to_virt(x) (FIXADDR_TOP - ((x) << PAGE_SHIFT))


// 固定区域
enum fixed_addresses{
    __end_of_permanent_fixed_addresses,
#define NR_FIX_BITMAPS  16
    FIX_BITMAP_END = __end_of_permanent_fixed_addresses,
    FIX_BITMAP_BEGIN = FIX_BITMAP_END + NR_FIX_BITMAPS - 1,
    __end_of_fixed_addresses
};


#endif