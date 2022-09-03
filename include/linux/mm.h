#ifndef _LINUX_MM_H
#define _LINUX_MM_H
#include <linux/list.h>
#include <asm-i386/atomic.h>
#include <asm-i386/bitops.h>
#include <asm-i386/types.h>

/* GFP 掩码 */
// zone修饰符
#define __GFP_DMA   0x01
#define __GFP_HIGHMEM   0x02

// 物理页面描述
typedef struct page {
    struct list_head list;    // 表示列表的头
    // struct address_space *mapping;  // 内存映射的索引节点
    unsigned long index;        // 如页面属于文件映射，则为页面在文件中的偏移
    struct page *next_hash;     
    atomic_t count;
    unsigned long flags;    
    struct list_head lru;
    unsigned long age;
    struct page **pprev_hash;
    // struct buffer_head * buffers;
    void *virtual;
    struct zone_struct *zone;
} mem_map_t;

extern mem_map_t* mem_map;

void free_area_init(unsigned long * zones_size);

// 页面 page 的标志位 flags
#define PG_locked		 0	/* Page is locked. Don't touch. */
#define PG_error		 1
#define PG_referenced		 2
#define PG_uptodate		 3
#define PG_dirty		 4
#define PG_unused		 5
#define PG_lru			 6
#define PG_active		 7
#define PG_slab			 8
#define PG_skip			10
#define PG_highmem		11
#define PG_checked		12	/* kill me in 2.5.<early>. */
#define PG_arch_1		13
#define PG_reserved		14  // 为特定页面设置，设置后将无法 swap out
#define PG_launder		15	/* written out by VM pressure.. */
#define PG_fs_1			16	/* Filesystem specific */


#define ZONE_SHIFT (BITS_PER_LONG - 8)

static inline void set_page_zone(struct page *page, unsigned long zone_num)
{
	page->flags &= ~(~0UL << ZONE_SHIFT);
	page->flags |= zone_num << ZONE_SHIFT;
}

// 设置页面的虚拟地址
#define set_page_address(page, address)			\
	do {						\
		(page)->virtual = (address);		\
	} while(0)

#define set_page_count(p,v) 	atomic_set(&(p)->count, v)

#define SetPageReserved(page)		set_bit(PG_reserved, &(page)->flags)
#define ClearPageReserved(page)		clear_bit(PG_reserved, &(page)->flags)

#endif