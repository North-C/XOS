#include <linux/bootmem.h>
#include <linux/init.h>
#include <linux/mmzone.h>
#include <asm-i386/io.h>
#include <linux/string.h>
#include <asm-i386/page.h>
#include <linux/stdio.h>
#include <asm-i386/bitops.h>
#include <linux/kernel.h>
#include <asm-i386/cpufeature.h>
#include <linux/debug.h>
#include <linux/mm.h>

unsigned long max_pfn;
unsigned long max_low_pfn;   // 从mem_map处高端内存开始的PFN
unsigned long min_low_pfn;
extern pg_data_t contig_page_data;

// 静态函数声明
static unsigned long init_bootmem_core(pg_data_t *pgdat, unsigned long mapstart, unsigned long start, unsigned long end);
static void free_bootmem_core(bootmem_data_t *bdata, unsigned long addr, unsigned long size);
static void * __alloc_bootmem_core(bootmem_data_t *bdata, unsigned long size, unsigned long align, unsigned long goal);
static void reserve_bootmem_core(bootmem_data_t *bdata, unsigned long addr, unsigned long size);

unsigned long init_bootmem(unsigned long start, unsigned long pages)
{
    max_low_pfn = pages;
    min_low_pfn = start;
    // 对contig_page_data进行初始化
    // UMA的内存是均匀的、连续的，只有一个pg_data_t类型的contig_page_data来表示一个节点
    // 而NUMA当中则有多个这样的数据结构，各个节点的pg_data_t通过链表链接在一起，第一个节点就是contig_page_data,如果还有其他节点，则在后续过程中添加到链表中即可。
    return (init_bootmem_core(&contig_page_data, start, 0, pages));
}

/**
 * @brief 初始化引导内存分配器
 * 
 * @param pgdat 
 * @param mapstart 内核映像以上第一个物理页面的起点
 * @param start 物理内存的起始pfn，值为0
 * @param end 物理内存的顶点 max_low_pfn
 * @return unsigned long 返回大小
 */
static unsigned long init_bootmem_core(pg_data_t *pgdat, unsigned long mapstart, unsigned long start, unsigned long end)
{
    bootmem_data_t *bdata = pgdat->bdata;           
    unsigned long mapsize = ((end - start)+7)/8;        // 以字节为单位计算
    printk("original mapsize = %lu\n", mapsize);

    // 将当前节点插入到内存节点链表的第一个
    pgdat->node_next = pgdat_list;     
    pgdat_list = pgdat;

    // 设置 引导内存数据结构
    mapsize = (mapsize + (sizeof(long) - 1UL)) & ~(sizeof(long) - 1UL);  
    printk("now mapsize = %lu\n", mapsize);
    bdata->node_bootmem_map = phys_to_virt(mapstart << PAGE_SHIFT);
    bdata->node_boot_start = (start << PAGE_SHIFT);   // 节点物理内存起始地址  
    bdata->node_low_pfn = end;

    memset(bdata->node_bootmem_map, 0xff, mapsize);  // 初始化位图

    return mapsize;
}

void free_bootmem (unsigned long addr, unsigned long size)
{
    return(free_bootmem_core(contig_page_data.bdata, addr, size));
}

/**
 * @brief 释放内存，设置bootmem_datat_t中对应的位图
 * 
 * @param bdata 引导内存的数据结构
 * @param addr 内存地址
 * @param size 内存大小
 */
static void free_bootmem_core(bootmem_data_t *bdata, unsigned long addr, unsigned long size)
{
    unsigned long i;
    unsigned long start;


    unsigned long sidx;
    unsigned long eidx = (addr + size - bdata->node_boot_start)/PAGE_SIZE;
    unsigned long end = (addr + size)/PAGE_SIZE;

    if(!size) BUG();        // 大小为0，则提示出错
    if(end > bdata->node_low_pfn)
        BUG();
    
    // 将位图当中对应的页面位清除
    start = (addr + PAGE_SIZE-1) / PAGE_SIZE;
    sidx = start - (bdata->node_boot_start/PAGE_SIZE);
    // 置位为1
    for (i = sidx; i < eidx; i++){
        if(!test_and_clear_bit(i, bdata->node_bootmem_map)) // 已经释放，则提示错误
            BUG();
    }
}

// 开始时将位图中的所有位都设置为 1， 设置位全部不能用于分配，其后根据e820图来修改
void reserve_bootmem(unsigned long addr, unsigned long size)
{
    reserve_bootmem_core(contig_page_data.bdata, addr, size);
}

static void reserve_bootmem_core(bootmem_data_t *bdata, unsigned long addr, unsigned long size)
{
    unsigned long i;

    // 在位图中设置
	unsigned long sidx = (addr - bdata->node_boot_start)/PAGE_SIZE;
	unsigned long eidx = (addr + size - bdata->node_boot_start + 
							PAGE_SIZE-1)/PAGE_SIZE;
	unsigned long end = (addr + size + PAGE_SIZE-1)/PAGE_SIZE;

	if (!size) BUG();

	if (sidx < 0)
		BUG();
	if (eidx < 0)
		BUG();
	if (sidx >= eidx)
		BUG();
	if ((addr >> PAGE_SHIFT) >= bdata->node_low_pfn)
		BUG();
	if (end > bdata->node_low_pfn)
		BUG();

    for(i = sidx; i < eidx; i++){   
        if(test_and_set_bit(i, bdata->node_bootmem_map))
            printk("hm, page %08lx reserved twice.\n", i*PAGE_SIZE);
    }
}

void * __alloc_bootmem(unsigned long size, unsigned long align, unsigned long goal)
{
    pg_data_t *pgdat;
    void *ptr;
    // 遍历节点，寻找合适的内存区域
    for_each_pgdat(pgdat)
        if((ptr = __alloc_bootmem_core(pgdat->bdata, size, align, goal)))
            return (ptr);
    /**
     * Whoops, 无法实现满足要求
     */
    printk("bootmem alloc of %lu bytes failed!\n", size);
    panic("Out of memory");
    return NULL;
}

void * __alloc_bootmem_node (pg_data_t *pgdat, unsigned long size, unsigned long align, unsigned long goal)
{
	void *ptr;

	ptr = __alloc_bootmem_core(pgdat->bdata, size, align, goal);
	if (ptr)
		return (ptr);

	/*
	 * Whoops, we cannot satisfy the allocation request.
	 */
	printk("ALERT: bootmem alloc of %lu bytes failed!\n", size);
	panic("Out of memory");
	return NULL;
}
// TODO: 内存分配时，在第 7 次分配时，返回错误的地址
/**
 * @brief 分配内存
 * 
 * @param bdata 节点中要分配结构体的启动内存，在UMA结构中默认为 contig_page_data
 * @param size 要分配空间的大小
 * @param align 要求对齐的字节数，2的幂级数
 * @param goal 最佳的分配的起始地址，一般从物理地址0开始
 * @return void* 
 */
static void * __alloc_bootmem_core(bootmem_data_t *bdata, unsigned long size, unsigned long align, unsigned long goal)
{
    unsigned long i, start = 0;
    void *ret;
    unsigned long offset, remaining_size;
    unsigned long areasize, preferred, incr;
    unsigned long eidx = bdata->node_low_pfn - (bdata->node_boot_start >> PAGE_SHIFT);   // 末尾位索引
    
    /* 保证参数的正确性 */
    if(!size) BUG();

    if(align & (align-1))
        BUG();
    
    offset = 0;  // 对齐的默认偏移是0
    // 如果指定了align对齐，且节点的对齐方式和请求的对齐方式
    if(align &&
         (bdata->node_boot_start & (align - 1UL)) != 0)
        offset = (align - (bdata->node_boot_start & (align - 1UL)));
    offset >>= PAGE_SHIFT;

    /* 尝试在 goal 的基础上分配内存 */
    // 指定了 goal 参数，goal在该节点的开始地址之后，goal不超出该节点可寻址的pfn
    if (goal && (goal >= bdata->node_boot_start) && 
			((goal >> PAGE_SHIFT) < bdata->node_low_pfn)) {
		preferred = goal - bdata->node_boot_start;  // 此时优先从 goal 位置开始分配
	} else
		preferred = 0;                 
    // 调整偏移量，使得能够正确对齐
    preferred = ((preferred + align - 1) & ~(align - 1)) >> PAGE_SHIFT;
	preferred += offset;
	areasize = (size+PAGE_SIZE-1)/PAGE_SIZE;    // 分配内存的总的大小
	incr = align >> PAGE_SHIFT ? : 1;       // 每次增长的页面数，大于1，则满足对齐请求

/* 从 preferred 开始扫描内存 */
restart_scan:
    for(i = preferred; i < eidx; i += incr){
        unsigned long j;
        if(test_bit(i, bdata->node_bootmem_map))  // 1表示已经分配
            continue;
        for(j = i + 1; j < i + areasize; j++){  // 检测是否有满足大小的内存区域
            if(j >= eidx)
                goto fail_block;
            if(test_bit(j, bdata->node_bootmem_map))
                goto fail_block;
        }
        start = i;      // 起始页面号
        goto found;   // 成功找到，跳转执行
    fail_block:;
    }
    if(preferred){  // 失败后，重新寻找一次
        preferred = offset;
        goto restart_scan;
    }
    return NULL;  // 失败两次后，退出

/* 判断是否上次分配的内存 能否 和本次分配合并 */
found:
    if(start>=eidx)
        BUG();

    // 合并的三个条件：1. align小于一个页面大小 PAGE_SIZE 
    // 2. 上次分配的页与此次分配的页相邻 
    // 3. 上一页有空闲空间，即last_offset不为0，表示没有全部使用
    if (align <= PAGE_SIZE
	    && bdata->last_offset && bdata->last_pos+1 == start) {

		offset = (bdata->last_offset+align-1) & ~(align-1);
		if (offset > PAGE_SIZE)
			BUG();

		remaining_size = PAGE_SIZE-offset;
		if (size < remaining_size) {
			areasize = 0;
			// last_pos unchanged
			bdata->last_offset = offset+size;
			ret = phys_to_virt(bdata->last_pos*PAGE_SIZE + offset +
						bdata->node_boot_start);
		} else {
			remaining_size = size - remaining_size;
			areasize = (remaining_size+PAGE_SIZE-1)/PAGE_SIZE;

			ret = phys_to_virt(bdata->last_pos*PAGE_SIZE + offset +
						bdata->node_boot_start);

			bdata->last_pos = start+areasize-1;
			bdata->last_offset = remaining_size;
		}
		bdata->last_offset &= ~PAGE_MASK;
        // debug
        // printk("merge\n");
	} else {
		bdata->last_pos = start + areasize - 1;
		bdata->last_offset = size & ~PAGE_MASK;
		ret = phys_to_virt(start * PAGE_SIZE + bdata->node_boot_start);
        // debug
        // printk("node_boot_start: %x, unmerge\n", bdata->node_boot_start);
        // printk("start: %x\n", start);
        // printk("areasize: %lu\n", areasize);
        // printk("ret phys: %x\n", start * PAGE_SIZE + bdata->node_boot_start);
        // printk("ret virt: %x\n", ret);
	}

    // 正式分配内存，将位图中分配页设置为1
    for(i = start; i < start+areasize; i++)
        if(test_and_set_bit(i, bdata->node_bootmem_map)) // 避免重复分配
            BUG();
    memset(ret, 0, size);  // 内存清零
    // printk("after memset ret virt: %x\n", ret);
    return ret;
}

/* 销毁引导内存分配器 */
static unsigned long free_all_bootmem_core(pg_data_t *pgdat)
{
	struct page *page = pgdat->node_mem_map;
	bootmem_data_t *bdata = pgdat->bdata;
	unsigned long i, count, total = 0;
	unsigned long idx;

	if (!bdata->node_bootmem_map) BUG();

	count = 0;
    // 节点最大的可寻址索引
	idx = bdata->node_low_pfn - (bdata->node_boot_start >> PAGE_SHIFT) - 1;
    // TODO: 循环到 i =9 时，直接从test_bit跳出循环，程序执行结束
	for (i = 0; i < idx; i++, page++) {  
        printk("release %d page\n", i);
		// 位图当中比特位为1时，表示这个页已经分配，为0时，表示当前指示的页是空闲的
        // 该node_bootmem_map只分配到第13个字节末尾
        if (!test_bit(i, bdata->node_bootmem_map)) {  // 对于未分配页面
			count++;
			ClearPageReserved(page);    // 清楚页面中的 PG_reserved 标志
			set_page_count(page, 1);    // 设置计数为1
			__free_page(page);         // 伙伴分配器将其加入到空闲链表中
		}
	}
	total += count;    // 该函数释放的页面总数，或者加入到空闲链表的页面总数

	/*
	 * Now free the allocator bitmap itself, it's not
	 * needed anymore:
	 */
	page = virt_to_page(bdata->node_bootmem_map);
	count = 0;
	for (i = 0; i < ((bdata->node_low_pfn-(bdata->node_boot_start >> PAGE_SHIFT))/8 + PAGE_SIZE-1)/PAGE_SIZE; i++,page++) {
		count++;
		ClearPageReserved(page);
		set_page_count(page, 1);
		__free_page(page);
	}
	total += count;
	bdata->node_bootmem_map = NULL;

	return total;
}

unsigned long free_all_bootmem (void)
{
	return(free_all_bootmem_core(&contig_page_data));
}