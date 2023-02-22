#ifndef _LINUX_MMZONE_H
#define _LINUX_MMZONE_H

#include <linux/bootmem.h>
#include <linux/list.h>
#include <linux/spinlock.h>

// 三种zone的类型
#define ZONE_DMA        0
#define ZONE_NORMAL     1
#define ZONE_HIGHMEM    2

#define MAX_NR_ZONES    3
#define MAX_ORDER        10

#define MAX_NR_NODES  1   // contig memory情况下

// GFP 表示 空闲页面
#define GFP_ZONEMASK  0x0f     // GFP标志位掩码，用于计算，具体定义在include/linux/mm.h 

typedef struct free_area_struct{
    struct list_head  free_list;  // 空闲页面块
    unsigned long    *map;   // 伙伴状态的位图
}free_area_t;

// 管理区的描述
typedef struct zone_struct {
    // TODO: 多核心同步相关的 spinlock
    spinlock_t      lock;
    unsigned long	offset;
    unsigned long   free_pages;       // 空闲页面的总数
    unsigned long	inactive_clean_pages;
	unsigned long	inactive_dirty_pages;
    unsigned long   pages_min;        // 管理区的极值
    unsigned long   pages_low;
    unsigned long   pages_high;
    int need_balance;              // 通知 页面换出kswapd进程 平衡该管理区

    struct list_head	inactive_clean_list;
    free_area_t  free_area[MAX_ORDER];      // 空闲区域
    
    // 等待队列的哈希表，其中是等待页面释放的进程组成
    // 为了减小内存的消耗，Linux将等待队列村昂在 zone当中而不是每个页面都有一个等待队列，使用哈希表来存储
   // wait_queue_head_t *wait_table;      
    unsigned long wait_table_size;    // 哈希数组的大小，2的幂
    unsigned long wait_table_shift;     // 等待表的大小

    struct pglist_data *zone_pgdat;         // 指向父 pg_data_t
    struct page *zone_mem_map;              // 设计的管理区在全局mem_map中的第一页
    unsigned long zone_start_paddr;         // 物理起始地址
    unsigned long zone_start_mapnr;         // 虚拟起始地址

    char *name;                 // 管理区的字符串名称，DMA Normal 或者 HighMem
    unsigned long size;         // 管理区的页面数
} zone_t;


// Zone管理区的链表，指向的第一个zone是在分配时优先级最高的
typedef struct zonelist_struct {
    zone_t * zones [MAX_NR_ZONES+1];        // NULL 为分隔符
} zonelist_t;


/** 
 * 表示内存的节点
*/
typedef struct pglist_data {
    zone_t node_zones[MAX_NR_ZONES];          // 节点所在的管理区
    zonelist_t node_zonelists[GFP_ZONEMASK+1];     // 按照分配时的管理区顺序排列
    int nr_zones;                               // 管理区的数目
    struct page *node_mem_map;                   // 节点中的页面，一般指向第一个页面
    unsigned long *valid_addr_bitmap;           // 内存节点中“空洞”的位图
    struct bootmem_data *bdata;             // 内存引导程序
    unsigned long node_start_paddr;         // 节点的起始物理地址
    unsigned long node_start_mapnr;         // 节点在全局mem_map中的页面偏移量
    unsigned long node_size;                // 页面的总数
    int node_id;                            // 节点的id号
    struct pglist_data *node_next;          // 指向下一个节点，NULL为结束
}pg_data_t;

// 连续均匀的节点
extern pg_data_t contig_page_data;    

/**
 * @brief 帮助链表进行遍历
 * pgdat = pgdat_list;
 * while(pgdat){
 * ...
 * pgdat = pgdat->node_next;
 * }
 */
extern pg_data_t* pgdat_list;
#define for_each_pgdat(pgdat) \
    for(pgdat = pgdat_list; pgdat; pgdat = pgdat->node_next)

// 构架 管理区zone的结构
extern void free_area_init_core(int nid, pg_data_t *pgdat, struct page **gmap,
	unsigned long *zones_size, unsigned long zone_start_paddr, 
	unsigned long *zholes_size, struct page *lmem_map);

#define MAP_ALIGN(x) ((((x) % sizeof(mem_map_t)) == 0) ? (x) : ((x) + \
		sizeof(mem_map_t) - ((x) % sizeof(mem_map_t))))
#endif