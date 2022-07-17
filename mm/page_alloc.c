#include <linux/mm.h>
#include <linux/bootmem.h>
#include <linux/mmzone.h>
#include <asm-i386/page.h>
#include <linux/debug.h>

// 全局的内存节点链表
pg_data_t *pgdat_list;      
extern pg_data_t contig_page_data;
extern mem_map_t mem_map;
// zone_t *zone_table[MAX_NR_ZONES*MAX_NR_NODES];

void __init free_area_init(unsigned long *zones_size)
{
// 	free_area_init_core(0, &contig_page_data, &mem_map, zones_size, 0, 0, 0);
}

/**
 * @brief 建立管理区zone的数据结构: 将所有的page都标记为reserved, 将内存等待队列标记为空，清空内存位图
 * 
 * @param nid 
 * @param pgdat 
 * @param gmap 
 * @param zones_size 
 * @param zone_start_paddr 
 * @param zholes_size 
 * @param lmem_map 
 */
// void __init free_area_init_core(int nid, pg_data_t *pgdat, struct page **gmap,
// 	unsigned long *zones_size, unsigned long zone_start_paddr, 
// 	unsigned long *zholes_size, struct page *lmem_map)
// {
// 	unsigned long i, j;
// 	unsigned long map_size;
// 	unsigned long totalpages, offset, realtotalpages;
// 	const unsigned long zone_required_alignment = 1UL << (MAX_ORDER-1);

// 	if(zone_start_paddr & ~PAGE_MASK){
// 		BUG();
// 	}
// 	// TODO: 注释下列代码
// 	totalpages = 0;
// 	for (i = 0; i < MAX_NR_ZONES; i++) {
// 		unsigned long size = zones_size[i];
// 		totalpages += size;
// 	}
// 	realtotalpages = totalpages;
// 	if (zholes_size)
// 		for (i = 0; i < MAX_NR_ZONES; i++)
// 			realtotalpages -= zholes_size[i];
			
// 	printk("On node %d totalpages: %lu\n", nid, realtotalpages);

// 	/*
// 	 * Some architectures (with lots of mem and discontinous memory
// 	 * maps) have to search for a good mem_map area:
// 	 * For discontigmem, the conceptual mem map array starts from 
// 	 * PAGE_OFFSET, we need to align the actual array onto a mem map 
// 	 * boundary, so that MAP_NR works.
// 	 */
// 	map_size = (totalpages + 1)*sizeof(struct page);
// 	if (lmem_map == (struct page *)0) {
// 		lmem_map = (struct page *) alloc_bootmem_node(pgdat, map_size);
// 		lmem_map = (struct page *)(PAGE_OFFSET + 
// 			MAP_ALIGN((unsigned long)lmem_map - PAGE_OFFSET));
// 	}
// 	*gmap = pgdat->node_mem_map = lmem_map;
// 	pgdat->node_size = totalpages;
// 	pgdat->node_start_paddr = zone_start_paddr;
// 	pgdat->node_start_mapnr = (lmem_map - mem_map);
// 	pgdat->nr_zones = 0;

// 	offset = lmem_map - mem_map;	
// 	for (j = 0; j < MAX_NR_ZONES; j++) {
// 		zone_t *zone = pgdat->node_zones + j;
// 		unsigned long mask;
// 		unsigned long size, realsize;

// 		zone_table[nid * MAX_NR_ZONES + j] = zone;
// 		realsize = size = zones_size[j];
// 		if (zholes_size)
// 			realsize -= zholes_size[j];

// 		printk("zone(%lu): %lu pages.\n", j, size);
// 		zone->size = size;
// 		zone->name = zone_names[j];
// 		zone->lock = SPIN_LOCK_UNLOCKED;
// 		zone->zone_pgdat = pgdat;
// 		zone->free_pages = 0;
// 		zone->need_balance = 0;
// 		if (!size)
// 			continue;

// 		/*
// 		 * The per-page waitqueue mechanism uses hashed waitqueues
// 		 * per zone.
// 		 */
// 		zone->wait_table_size = wait_table_size(size);
// 		zone->wait_table_shift =
// 			BITS_PER_LONG - wait_table_bits(zone->wait_table_size);
// 		zone->wait_table = (wait_queue_head_t *)
// 			alloc_bootmem_node(pgdat, zone->wait_table_size
// 						* sizeof(wait_queue_head_t));

// 		for(i = 0; i < zone->wait_table_size; ++i)
// 			init_waitqueue_head(zone->wait_table + i);

// 		pgdat->nr_zones = j+1;

// 		mask = (realsize / zone_balance_ratio[j]);
// 		if (mask < zone_balance_min[j])
// 			mask = zone_balance_min[j];
// 		else if (mask > zone_balance_max[j])
// 			mask = zone_balance_max[j];
// 		zone->pages_min = mask;
// 		zone->pages_low = mask*2;
// 		zone->pages_high = mask*3;

// 		zone->zone_mem_map = mem_map + offset;
// 		zone->zone_start_mapnr = offset;
// 		zone->zone_start_paddr = zone_start_paddr;

// 		if ((zone_start_paddr >> PAGE_SHIFT) & (zone_required_alignment-1))
// 			printk("BUG: wrong zone alignment, it will crash\n");

// 		/*
// 		 * Initially all pages are reserved - free ones are freed
// 		 * up by free_all_bootmem() once the early boot process is
// 		 * done. Non-atomic initialization, single-pass.
// 		 */
// 		for (i = 0; i < size; i++) {
// 			struct page *page = mem_map + offset + i;
// 			set_page_zone(page, nid * MAX_NR_ZONES + j);
// 			set_page_count(page, 0);
// 			SetPageReserved(page);
// 			INIT_LIST_HEAD(&page->list);
// 			if (j != ZONE_HIGHMEM)
// 				set_page_address(page, __va(zone_start_paddr));
// 			zone_start_paddr += PAGE_SIZE;
// 		}

// 		offset += size;
// 		for (i = 0; ; i++) {
// 			unsigned long bitmap_size;

// 			INIT_LIST_HEAD(&zone->free_area[i].free_list);
// 			if (i == MAX_ORDER-1) {
// 				zone->free_area[i].map = NULL;
// 				break;
// 			}

// 			/*
// 			 * Page buddy system uses "index >> (i+1)",
// 			 * where "index" is at most "size-1".
// 			 *
// 			 * The extra "+3" is to round down to byte
// 			 * size (8 bits per byte assumption). Thus
// 			 * we get "(size-1) >> (i+4)" as the last byte
// 			 * we can access.
// 			 *
// 			 * The "+1" is because we want to round the
// 			 * byte allocation up rather than down. So
// 			 * we should have had a "+7" before we shifted
// 			 * down by three. Also, we have to add one as
// 			 * we actually _use_ the last bit (it's [0,n]
// 			 * inclusive, not [0,n[).
// 			 *
// 			 * So we actually had +7+1 before we shift
// 			 * down by 3. But (n+8) >> 3 == (n >> 3) + 1
// 			 * (modulo overflows, which we do not have).
// 			 *
// 			 * Finally, we LONG_ALIGN because all bitmap
// 			 * operations are on longs.
// 			 */
// 			bitmap_size = (size-1) >> (i+4);
// 			bitmap_size = LONG_ALIGN(bitmap_size+1);
// 			zone->free_area[i].map = 
// 			  (unsigned long *) alloc_bootmem_node(pgdat, bitmap_size);
// 		}
// 	}
// 	build_zonelists(pgdat);
// }

/**
 * @brief 建立节点的管理区回退链表
 * 
 * @param pgdat 
 */
// static inline void build_zonelists(pg_data_t *pgdat)
// {
// 	int i, j, k;

// 	for(i = 0; i <= GFP_ZONEMASK; i++){
// 		zonelist_t *zonelist;
// 		zone_t *zone;

// 		zonelist = pgdat->node_zonelists + i;
// 		memset(zonelist, 0, sizeof(*zonelist));

// 		j = 0;
// 		k = ZONE_NORMAL;
// 		if(i & __GFP_HIGHMEM)
// 			k = ZONE_HIGHMEM;
// 		if(i & __GFP_DMA)
// 			k = ZONE_DMA;

// 		switch (k){
// 			default:
// 				BUG();
// 			case ZONE_HIGHMEM:
// 		}
// 	}
// }