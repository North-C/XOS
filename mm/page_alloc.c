#include <linux/bootmem.h>
#include <linux/mm.h>
#include <linux/mmzone.h>
#include <linux/spinlock.h>
#include <linux/debug.h>
#include <linux/string.h>
#include <linux/kernel.h>
#include <linux/threads.h>
#include <linux/sched.h>
#include <asm-i386/current.h>
#include <asm-i386/page.h>
#include <asm-i386/bitops.h>
#include <asm-i386/pgtable.h>

// /*
//  * Temporary debugging check.
//  */
// #define BAD_RANGE(zone,x) (((zone) != (x)->zone) || (((x)-mem_map) < (zone)->offset) || (((x)-mem_map) >= (zone)->offset+(zone)->size))

/*
 * Temporary debugging check.
 */
#define BAD_RANGE(zone, page)										\
(																	\
	(((page) - mem_map) >= ((zone)->zone_start_mapnr+(zone)->size))	\
	|| (((page) - mem_map) < (zone)->zone_start_mapnr)				\
	|| ((zone) != page_zone(page))									\
)


// 全局的内存节点链表
pg_data_t *pgdat_list;      
extern pg_data_t contig_page_data;
extern mem_map_t* mem_map;

// 保存管理区的地址
zone_t *zone_table[MAX_NR_ZONES*MAX_NR_NODES];

// 方便打印名称
static char *zone_names[MAX_NR_ZONES] = {"DMA", "Normal", "HighMem" };
// 管理区的平衡因子
static int zone_balance_ratio[MAX_NR_ZONES] = {128, 128, 128, };
static int zone_balance_min[MAX_NR_ZONES] = {20, 20, 20, };
static int zone_balance_max[MAX_NR_ZONES] = {255, 255, 255, };



#define memlist_init(x) INIT_LIST_HEAD(x)
#define memlist_add_head list_add
#define memlist_add_tail list_add_tail
#define memlist_del list_del
#define memlist_entry list_entry
#define memlist_next(x) ((x)->next)
#define memlist_prev(x) ((x)->prev)


static inline void build_zonelists(pg_data_t *pgdat);

void free_area_init(unsigned long *zones_size)
{
 	free_area_init_core(0, &contig_page_data, &mem_map, zones_size, 0, 0, 0);
}

#define LONG_ALIGN(x) (((x)+(sizeof(long))-1)&~((sizeof(long))-1))

/**
 * @brief 初始化所有区域，建立管理区zone的数据结构，并在节点中分配它们的局部lmem_map，UMA中使用mem_map: 将所有的page都标记为reserved, 将内存等待队列标记为空，清空内存位图
 * 
 * @param nid 被初始化管理区中节点的标识符，nodeid
 * @param pgdat 节点数据结构
 * @param gmap 
 * @param zones_size 管理区大小
 * @param zone_start_paddr 第一个管理区的起始物理地址
 * @param zholes_size 管理区内存空洞的大小
 * @param lmem_map 
 */
void free_area_init_core(int nid, pg_data_t *pgdat, struct page **gmap,
	unsigned long *zones_size, unsigned long zone_start_paddr, 
	unsigned long *zholes_size, struct page *lmem_map)
{
	unsigned long i, j;
	unsigned long map_size;
	unsigned long totalpages, offset, realtotalpages;
    // 必须与伙伴系统的最大阶内存块对齐
	const unsigned long zone_required_alignment = 1UL << (MAX_ORDER-1);
    /* 计算每个zone的大小 */
	if(zone_start_paddr & ~PAGE_MASK){  // 必须页对齐
		BUG();
	}
	
	totalpages = 0;   // 初始化
	for (i = 0; i < MAX_NR_ZONES; i++) {  // 计算节点的总大小
		unsigned long size = zones_size[i];
		totalpages += size;
	}
	realtotalpages = totalpages;
	if (zholes_size)    // 计算实际内存页面数，减去空洞的页面数
		for (i = 0; i < MAX_NR_ZONES; i++)
			realtotalpages -= zholes_size[i];
			
	printk("On node %d totalpages: %lu\n", nid, realtotalpages);

	// 分配局部lmem_map，设置gmap位，在UMA当中，gmap就是mem_map
	map_size = (totalpages + 1) * sizeof(struct page);
	if (lmem_map == (struct page *)0) {
		lmem_map = (struct page *) alloc_bootmem_node(pgdat, map_size);
		lmem_map = (struct page *)(PAGE_OFFSET + 
			MAP_ALIGN((unsigned long)lmem_map - PAGE_OFFSET));
	}
	*gmap = pgdat->node_mem_map = lmem_map;
	pgdat->node_size = totalpages;
	pgdat->node_start_paddr = zone_start_paddr;
	pgdat->node_start_mapnr = (lmem_map - mem_map);
	pgdat->nr_zones = 0;

	offset = lmem_map - mem_map;	
    /* 遍历管理区，初始化节点中的每个 zone */
	for (j = 0; j < MAX_NR_ZONES; j++) {
		zone_t *zone = pgdat->node_zones + j;
		unsigned long mask;
		unsigned long size, realsize;
		// 设置表中的值， 节点id * 节点中最大zone数 + 管理区索引 j
		zone_table[nid * MAX_NR_ZONES + j] = zone;

		realsize = size = zones_size[j];
		if (zholes_size)
			realsize -= zholes_size[j];  // 减去空洞的大小
 
		printk("zone(%lu): %lu pages.\n", j, size);
		zone->size = size;
		zone->name = zone_names[j];
		// TODO: zone 当中的 spinlock
		zone->lock = SPIN_LOCK_UNLOCKED;
		zone->zone_pgdat = pgdat;
		zone->free_pages = 0;
		zone->need_balance = 0;
		if (!size)
			continue;

		/*
		 * 初始化管理区的等待队列。等待该管理区中页面的进程将会使用该哈希表来选择一个队列进行等待
		 TODO：等待后续进程加入
		 */
		// zone->wait_table_size = wait_table_size(size);
		// zone->wait_table_shift =
		// 	BITS_PER_LONG - wait_table_bits(zone->wait_table_size);
		// zone->wait_table = (wait_queue_head_t *)
		// 	alloc_bootmem_node(pgdat, zone->wait_table_size
		// 				* sizeof(wait_queue_head_t));

		// for(i = 0; i < zone->wait_table_size; ++i)
		// 	init_waitqueue_head(zone->wait_table + i);

		pgdat->nr_zones = j+1;  	// 更新节点中的管理区数目

		mask = (realsize / zone_balance_ratio[j]);
		if (mask < zone_balance_min[j])
			mask = zone_balance_min[j];
		else if (mask > zone_balance_max[j])
			mask = zone_balance_max[j];
		zone->pages_min = mask;
		zone->pages_low = mask*2;
		zone->pages_high = mask*3;

		zone->zone_mem_map = mem_map + offset;  // 第一个 page 的位置在mem_map当中的位置
		zone->zone_start_mapnr = offset;     // mem_map 中管理区起点的索引
		zone->zone_start_paddr = zone_start_paddr;   // zone 的起始物理区地址
		
		// 保证页面对齐，否则后续 slab 的位级操作就会失效
		if ((zone_start_paddr >> PAGE_SHIFT) & (zone_required_alignment-1)){
			printk("BUG: wrong zone alignment, it will crash\n");
			BUG();
		}

		/*
		将所有的页面都设置为 reserved，
		 * Initially all pages are reserved - free ones are freed
		 * up by free_all_bootmem() once the early boot process is
		 * done. Non-atomic initialization, single-pass.
		 */
		for (i = 0; i < size; i++) {
			struct page *page = mem_map + offset + i;
			// 设置页面，保存 zone_table 的索引
			set_page_zone(page, nid * MAX_NR_ZONES + j);
			// 设置页面的引用次数
			set_page_count(page, 0);
			// 将 page 设置为 reserved，页面无法被换出
			SetPageReserved(page);
			INIT_LIST_HEAD(&page->list);   // 初始化页表链表头结点
			if (j != ZONE_HIGHMEM)
				set_page_address(page, __va(zone_start_paddr));
			zone_start_paddr += PAGE_SIZE;
		}
		// 为 管理区的free_area 分配内存
		offset += size;
		for (i = 0; ; i++) {
			unsigned long bitmap_size;
			
			INIT_LIST_HEAD(&zone->free_area[i].free_list);
			if (i == MAX_ORDER-1) {
				zone->free_area[i].map = NULL;
				break;
			}

			bitmap_size = (size-1) >> (i+4);
			bitmap_size = LONG_ALIGN(bitmap_size+1);
			zone->free_area[i].map = 
			  (unsigned long *) alloc_bootmem_node(pgdat, bitmap_size);
		}
	}
	// 构造节点的管理区回退链表
	build_zonelists(pgdat);
}

/*
 * Common helper functions.
 */
unsigned long __get_free_pages(int gfp_mask, unsigned long order)
{
	struct page * page;

	page = alloc_pages(gfp_mask, order);
	if (!page)
		return 0;
	return (unsigned long) page_address(page);
}

/**
 * @brief 建立节点的管理区回退链表，用于不能满足内存分配时考察下一个管理区
 * 
 * @param pgdat 
 */
static inline void build_zonelists(pg_data_t *pgdat)
{
	int i, j, k;

	for(i = 0; i <= GFP_ZONEMASK; i++){
		zonelist_t *zonelist;
		zone_t *zone;

		zonelist = pgdat->node_zonelists + i;
		memset(zonelist, 0, sizeof(*zonelist));

		j = 0;
		k = ZONE_NORMAL;
		if(i & __GFP_HIGHMEM)
			k = ZONE_HIGHMEM;
		if(i & __GFP_DMA)
			k = ZONE_DMA;

		switch (k){
			default:
				BUG();
						/*
			 * fallthrough:
			 */
			case ZONE_HIGHMEM:
				zone = pgdat->node_zones + ZONE_HIGHMEM;
				if (zone->size) {
#ifndef CONFIG_HIGHMEM
					BUG();
#endif
					zonelist->zones[j++] = zone;
				}
			case ZONE_NORMAL:
				zone = pgdat->node_zones + ZONE_NORMAL;
				if (zone->size)
					zonelist->zones[j++] = zone;
			case ZONE_DMA:
				zone = pgdat->node_zones + ZONE_DMA;
				if (zone->size)
					zonelist->zones[j++] = zone;
		}
		zonelist->zones[j++] = NULL;
	}
}

#define PAGES_PER_WAITQUEUE  256   // 制定 page 和 waitqueue 的比例
/**
 * @brief 计算出合适的等待队列的大小，使得其足够大以至于减少冲突的发生
 * 
 * @param pages 
 * @return unsigned long 
 */
static inline unsigned long wait_table_size(unsigned long pages)
{
	unsigned long size = 1;

	pages /= PAGES_PER_WAITQUEUE;

	while(size < pages)
		size << 1;
	
	size = min(size, 4096UL);
	
	return size;
}

/**
 * @brief 
 * 
 * @param size 
 * @return unsigned long 
 */
static inline unsigned long wait_table_bits(unsigned long size)
{
	return ffz(~size);
}

static void __free_pages_ok(struct page *page, unsigned int order)
{
	unsigned long index, page_idx, mask, flags;
	free_area_t *area;
	struct page *base;
	zone_t *zone;

	// if (PageLRU(page)) {
	// 	if (unlikely(in_interrupt()))
	// 		BUG();
	// 	lru_cache_del(page);
	// }

	// if (page->buffers)
	// 	BUG();
	// if (page->mapping)
	// 	BUG();
	if (!VALID_PAGE(page))
		BUG();
	// if (PageLocked(page))
	// 	BUG();
	// if (PageActive(page))
	// 	BUG();
	page->flags &= ~((1<<PG_referenced) | (1<<PG_dirty));

	// if (current->flags & PF_FREE_PAGES)
	// 	goto local_freelist;
back_local_freelist:

	zone = page_zone(page);
	
	mask = (~0UL) << order;   // 获取一个后order个位为0的长整型数字
	base = zone->zone_mem_map;   // 获取内存管理区管理的开始内存页
	page_idx = page - base;   // 当前页面在内存管理区的索引
	if (page_idx & ~mask)
		BUG();
	index = page_idx >> (1 + order);   // 伙伴标记位索引

	area = zone->free_area + order;   // 内存块所在的空闲链表

	spin_lock_irqsave(&zone->lock, flags);

	zone->free_pages -= mask;   // 添加释放的内存块所占用的内存页数

	while (mask + (1 << (MAX_ORDER-1))) {   // 遍历(MAX_ORDER-order-1, MAX_ORDER等于10)次, 也就是说最多循环9次
		struct page *buddy1, *buddy2;

		if (area >= zone->free_area + MAX_ORDER)
			BUG();
		if (!test_and_change_bit(index, area->map))
			/*
			 * the buddy page is still allocated.
			 */
			break;
		/*
		 * Move the buddy up one level.
		 * This code is taking advantage of the identity:
		 * 	-mask = 1+~mask
		 */
		buddy1 = base + (page_idx ^ -mask);
		buddy2 = base + page_idx;
		if (BAD_RANGE(zone,buddy1))
			BUG();
		if (BAD_RANGE(zone,buddy2))
			BUG();

		list_del(&buddy1->list);
		mask <<= 1;
		area++;
		index >>= 1;
		page_idx &= mask;
	}
	list_add(&(base + page_idx)->list, &area->free_list);

	spin_unlock_irqrestore(&zone->lock, flags);
	return;

// local_freelist:
// 	if (current->nr_local_pages)
// 		goto back_local_freelist;
// 	if (in_interrupt())
// 		goto back_local_freelist;		

// 	list_add(&page->list, &current->local_pages);
// 	page->index = order;
// 	current->nr_local_pages++;
	return;
}

void __free_pages(struct page *page, unsigned long order)
{
	if(!PageReserved(page) && put_page_testzero(page)){
		// printk("start free pages\n");
		__free_pages_ok(page, order);
		// printk("free pages ok\n");
	}
}

void free_pages(unsigned long addr, unsigned long order)
{
	struct page *fpage;

	fpage = virt_to_page(addr);
	if (VALID_PAGE(fpage))
		__free_pages(fpage, order);
}

/*
 * Total amount of free (allocatable) RAM:
 */
unsigned int nr_free_pages (void)
{
	unsigned int sum;
	zone_t *zone;
	pg_data_t *pgdat = pgdat_list;

	sum = 0;
	while (pgdat) {
		for (zone = pgdat->node_zones; zone < pgdat->node_zones + MAX_NR_ZONES; zone++)
			sum += zone->free_pages;
		pgdat = pgdat->node_next;
	}
	return sum;
}

#define MARK_USED(index, order, area) \
	change_bit((index) >> (1+(order)), (area)->map)
/*
 zone 分配的管理区
 page 待分割块的页面
 index 页面在mem_map当中的索引
 low 需要分配的页面阶
 high 分配时要分割的页面阶
 area 代表高阶页面块的free_area_t
 */
static inline struct page * expand (zone_t *zone, struct page *page,
	 unsigned long index, int low, int high, free_area_t * area)
{
	unsigned long size = 1 << high;

	while (high > low) {
		if (BAD_RANGE(zone,page))
			BUG();
		area--;
		high--;
		size >>= 1;
		memlist_add_head(&(page)->list, &(area)->free_list);  // 空闲的另一半加入下一个阶次的空闲列表
		MARK_USED(index, high, area);  // 已占用
		index += size;
		page += size;
	}
	if (BAD_RANGE(zone,page))
		BUG();
	return page;
}

// static FASTCALL(struct page * rmqueue(zone_t *zone, unsigned long order));
/* 负责找到一块足够大的用于分配的内存块 */
static struct page * rmqueue(zone_t *zone, unsigned int order)
{
    free_area_t * area = zone->free_area + order;  // 空闲区域
    unsigned int curr_order = order;
    struct list_head *head, *curr;
    unsigned long flags;
    struct page *page;

    spin_lock_irqsave(&zone->lock, flags);
    do {
        head = &area->free_list;
        curr = head->next;

        if (curr != head) {
            unsigned int index;
			// 内存块
            page = list_entry(curr, struct page, list);
            if (BAD_RANGE(zone,page))
                BUG();
            list_del(curr);
            index = page - zone->zone_mem_map;  // page在管理区中的索引
            if (curr_order != MAX_ORDER-1)
                MARK_USED(index, curr_order, area);
            zone->free_pages -= 1UL << order;  // 除去内存块所占用的内存页数

			// 将页面块分割成更高阶，大页面分裂为小页面
            page = expand(zone, page, index, order, curr_order, area);
            spin_unlock_irqrestore(&zone->lock, flags);

            set_page_count(page, 1);
            if (BAD_RANGE(zone,page))
                BUG();
            // if (PageLRU(page))
            // 	BUG();
            // if (PageActive(page))
            //     BUG();
            return page;
        }
        curr_order++;
        area++;
    } while (curr_order < MAX_ORDER);
    spin_unlock_irqrestore(&zone->lock, flags);

    return NULL;
}

/* 伙伴系统分配器的核心 */
struct page * __alloc_pages(unsigned int gfp_mask, unsigned int order, zonelist_t *zonelist)
{
	// printk("%s: %d: zonelist = 0x%p\n", __func__, __LINE__, zonelist);
	unsigned long min;
	zone_t **zone, *classzone;
	struct page *page;

	zone = zonelist->zones;
	classzone = *zone;    // 将合适的管理区标记为 classzone
	if(classzone == NULL)
		return NULL;
	
	// 遍历所有管理区，看是否有满足不超过极值的分配器
	min = 1UL << order;   // 最小的内存
	for(;;){
		zone_t *z = *(zone++);
		
		if(!z)   // 最后一个管理区，退出
			break;
		// printk("z = %p\n", z);
		min += z->pages_low;   // 将极值分配器的页面数加一，使得减少只使用一个回退管理区的概率
		
		// printk("min = %d\n", min);
		// printk("z->free_pages = %d\n", z->free_pages);
		if(z->free_pages > min){   // 不超过 pages_min 极值
			page = rmqueue(z, order);    // 从管理区当中重新移动该页面块
			if(page)  // 分配完成
				return page;
		}
	}

	/* 页面换入换出的平衡 */
// 	classzone->need_balance = 1;   // 将管理区标记为需要平衡，后续由 kswapd 进行使用
// 	mb();             // 内存屏障，保证所有CPU都能看到
// 	if (waitqueue_active(&kswapd_wait))   // kswapd 处于睡眠状态，则唤醒它
// 		wake_up_interruptible(&kswapd_wait);

// 	zone = zonelist->zones;      
// 	min = 1UL << order;
// 	for (;;) {
// 		unsigned long local_min;
// 		zone_t *z = *(zone++);
// 		if (!z)
// 			break;

// 		local_min = z->pages_min;
// 		if (!(gfp_mask & __GFP_WAIT))
// 			local_min >>= 2;
// 		min += local_min;
// 		if (z->free_pages > min) {
// 			page = rmqueue(z, order);
// 			if (page)
// 				return page;
// 		}
// 	}

// 	/* here we're in the low on memory slow path */

// rebalance:
// 	if (current->flags & (PF_MEMALLOC | PF_MEMDIE)) {
// 		zone = zonelist->zones;
// 		for (;;) {
// 			zone_t *z = *(zone++);
// 			if (!z)
// 				break;

// 			page = rmqueue(z, order);
// 			if (page)
// 				return page;
// 		}
// 		return NULL;
// 	}

// 	/* Atomic allocations - we can't balance anything */
// 	if (!(gfp_mask & __GFP_WAIT))
// 		return NULL;

// 	page = balance_classzone(classzone, gfp_mask, order, &freed);
// 	if (page)
// 		return page;

// 	zone = zonelist->zones;
// 	min = 1UL << order;
// 	for (;;) {
// 		zone_t *z = *(zone++);
// 		if (!z)
// 			break;

// 		min += z->pages_min;
// 		if (z->free_pages > min) {
// 			page = rmqueue(z, order);
// 			if (page)
// 				return page;
// 		}
// 	}

	/* Don't let big-order allocations loop */
	// if (order > 3)
	// 	return NULL;

	// /* Yield for kswapd, and try again */
	// // yield();
	// goto rebalance;
}


struct page *_alloc_pages(unsigned int gfp_mask, unsigned int order)
{
	return __alloc_pages(gfp_mask, order,
                         contig_page_data.node_zonelists+(gfp_mask & GFP_ZONEMASK));
}