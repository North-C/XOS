#include <linux/mm.h>
#include <linux/bootmem.h>
#include <linux/mmzone.h>
#include <asm-i386/page.h>
#include <linux/debug.h>
#include <linux/string.h>
#include <linux/kernel.h>

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
	
	totalpages = 0;
	for (i = 0; i < MAX_NR_ZONES; i++) {  // 计算节点的总大小
		unsigned long size = zones_size[i];
		totalpages += size;
	}
	realtotalpages = totalpages;
	if (zholes_size)
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
		// zone->lock = SPIN_LOCK_UNLOCKED;
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

		zone->zone_mem_map = mem_map + offset;  // 第一个 page 的位置
		zone->zone_start_mapnr = offset;     // mem_map 中管理区起点的索引
		zone->zone_start_paddr = zone_start_paddr;   // zone 的起始物理区地址
		
		// 保证页面对齐，否则后续 slab 的位级操作就会失效
		if ((zone_start_paddr >> PAGE_SHIFT) & (zone_required_alignment-1))
			printk("BUG: wrong zone alignment, it will crash\n");

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
			INIT_LIST_HEAD(&page->list);   // 初始化头结点
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