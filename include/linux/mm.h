#ifndef _LINUX_MM_H
#define _LINUX_MM_H
#include <linux/list.h>
#include <linux/mmzone.h>
#include <asm-i386/atomic.h>
#include <asm-i386/bitops.h>
#include <asm-i386/types.h>

/* GFP 掩码 */
// zone修饰符
#define __GFP_WAIT      0x01
#define __GFP_HIGH      0x02
#define __GFP_IO        0x04
#define __GFP_DMA       0x08
#define __GFP_HIGHMEM   0x10


#define GFP_BUFFER (__GFP_HIGH | __GFP_WAIT)
#define GFP_ATOMIC (__GFP_HIGH)
#define GFP_USER   (__GFP_WAIT | __GFP_IO | __GFP_HIGHMEM)
#define GFP_HIGHUSER	(             __GFP_WAIT | __GFP_IO | __GFP_HIGHMEM)
#define GFP_KERNEL	(__GFP_HIGH | __GFP_WAIT | __GFP_IO)   // 调用者可以做任意事情
#define GFP_NFS		(__GFP_HIGH | __GFP_WAIT | __GFP_IO)
#define GFP_KSWAPD	(                          __GFP_IO)

#define GFP_DMA   __GFP_DMA   // buffer用于DMA


extern unsigned long max_mapnr;
extern unsigned long num_physpages;

// 物理页面描述符
typedef struct page {
    struct list_head list;    // 表示列表的头
    // struct address_space *mapping;  // 内存映射的索引节点
    unsigned long index;        // 如页面属于文件映射，则为页面在文件中的偏移
    struct page *next_hash;     
    atomic_t count;             // 引用计数器
    unsigned long flags;       // 标志位，管理页框
    struct list_head lru;       // 页的最近最少使用双向链表的额指针
    unsigned long age;          
    struct page **pprev_hash;   /* Complement to *next_hash. */
    // struct buffer_head * buffers;  /* Buffer maps us to a disk block. */
    void *virtual;/* Kernel virtual address (NULL if not kmapped, ie. highmem) */
    struct zone_struct *zone;
} mem_map_t;

#define put_page_testzero(p)  atomic_dec_and_test(&(p)->count)
#define set_page_count(p,v) 	atomic_set(&(p)->count, v)

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

extern void __free_pages(struct page *page, unsigned long order);
extern void free_pages(unsigned long addr, unsigned long order);

#define SetPageReserved(page)		set_bit(PG_reserved, &(page)->flags)
#define ClearPageReserved(page)		clear_bit(PG_reserved, &(page)->flags)

#define __free_page(page)  __free_pages((page), 0)
#define free_page(addr) free_page((addr), 0)


#define PageSetSlab(page)	set_bit(PG_slab, &(page)->flags)
#define PageReserved(page)	test_bit(PG_reserved, &(page)->flags)

#define PageClearSlab(page)		clear_bit(PG_slab, &(page)->flags)

extern struct page * _alloc_pages(unsigned int gfp_mask, unsigned int order);

static inline struct page *alloc_pages(unsigned int gfp_mask, unsigned long order)
{
    if(order >= MAX_ORDER)
        return NULL;
    return _alloc_pages(gfp_mask, order);
}

#define alloc_page(gfp_mask) alloc_pages(gfp_mask, 0)

extern struct zone_struct *zone_table[];
static inline zone_t *page_zone(struct page *page)
{
    return zone_table[page->flags >> ZONE_SHIFT];
}

#endif