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

unsigned long max_pfn;
unsigned long max_low_pfn;
unsigned long min_low_pfn;
extern pg_data_t contig_page_data;

// 静态函数声明
static unsigned long __init init_bootmem_core(pg_data_t *pgdat, unsigned long mapstart, unsigned long start, unsigned long end);
static void __init free_bootmem_core(bootmem_data_t *bdata, unsigned long addr, unsigned long size);
static void * __init __alloc_bootmem_core(bootmem_data_t *bdata, unsigned long size, unsigned long align, unsigned long goal);
static void __init reserve_bootmem_core(bootmem_data_t *bdata, unsigned long addr, unsigned long size);

unsigned long __init init_bootmem(unsigned long start, unsigned long pages)
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
 * @param start 物理内存的起始pfn
 * @param end 物理内存的顶点 max_low_pfn
 * @return unsigned long 返回大小
 */
static unsigned long __init init_bootmem_core(pg_data_t *pgdat, unsigned long mapstart, unsigned long start, unsigned long end)
{
    bootmem_data_t *bdata = pgdat->bdata;           
    unsigned long mapsize = ((end - start)+7)/8;        // 以字节为单位计算

    // 插入到内存节点链表的第一个
    pgdat->node_next = pgdat_list;     
    pgdat_list = pgdat;

    // 设置 引导内存数据结构
    mapsize = (mapsize + (sizeof(long) - 1UL)) & ~(sizeof(long) - 1UL);  // ?? 这是怎么计算的？
    bdata->node_bootmem_map = phys_to_virt(mapstart << PAGE_SHIFT);
    bdata->node_boot_start = (start << PAGE_SHIFT);     
    bdata->node_low_pfn = end;

    memset(bdata->node_bootmem_map, 0xff, mapsize);  // 初始化位图

    return mapsize;
}

void __init free_bootmem (unsigned long addr, unsigned long size)
{
    return(free_bootmem_core(contig_page_data.bdata, addr, size)); // ? 为啥要这么写
}

/**
 * @brief 释放内存
 * 
 * @param bdata 引导内存的数据结构
 * @param addr 内存地址
 * @param size 内存大小
 */
static void __init free_bootmem_core(bootmem_data_t *bdata, unsigned long addr, unsigned long size)
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

    for (i = sidx; i < eidx; i++){
        if(!test_and_clear_bit(i, bdata->node_bootmem_map)) // 已经释放，则提示错误
            BUG();
    }
}

// 开始时将位图中的所有位都设置为 1， 设置位全部不能用于分配，其后根据e820图来修改
void __init reserve_bootmem(unsigned long addr, unsigned long size)
{
    reserve_bootmem_core(contig_page_data.bdata, addr, size);
}

static void __init reserve_bootmem_core(bootmem_data_t *bdata, unsigned long addr, unsigned long size)
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

void * __init __alloc_bootmem(unsigned long size, unsigned long align, unsigned long goal)
{
    pg_data_t *pgdat;
    void *ptr;

    for_each_pgdat(pgdat)
        if((ptr = __alloc_bootmem_core(pgdat->bdata, size, align, goal)))
            return (ptr);
    /**
     * Whoops, 无法实现满足要求
     */
    printk(KERN_ALERT "bootmem alloc of %lu bytes failed!\n", size);
    panic("Out of memory");
    return NULL;
}

/**
 * @brief 分配内存
 * 
 * @param bdata 要分配的节点，在UMA结构中默认为 contig_page_data
 * @param size 要分配空间的大小
 * @param align 要求对齐的字节数
 * @param goal 最佳的分配的起始地址，一般从物理地址0开始
 * @return void* 
 */
static void * __init __alloc_bootmem_core(bootmem_data_t *bdata, unsigned long size, unsigned long align, unsigned long goal)
{
//     unsigned long i, start = 0;
//     void *ret;
//     unsigned long offset, remaining_size;
//     unsigned long areasize, preferred, incr;
//     unsigned long eidx = bdata->node_low_pfn - (bdata->node_boot_start >> PAGE_SHIFT);

//     if(!size) BUG();

//     if(align & (align-1))
//         BUG();
    
//     offset = 0;
//     if(align &&
//          (bdata->node_boot_start & (align - 1UL)) != 0)
//         offset = (align - (bdata->node_boot_start & (align - 1UL)));
//     offset >>= PAGE_SHIFT;

//     // 尝试在goal 的基础上分配内存
//     // 计算优先分配的页面 preferred
//     if (goal && (goal >= bdata->node_boot_start) && 
// 			((goal >> PAGE_SHIFT) < bdata->node_low_pfn)) {
// 		preferred = goal - bdata->node_boot_start;
// 	} else
// 		preferred = 0;
//     // 分配的内存的参数
//     preferred = ((preferred + align - 1) & ~(align - 1)) >> PAGE_SHIFT;
// 	preferred += offset;
// 	areasize = (size+PAGE_SIZE-1)/PAGE_SIZE;    // 分配内存的总的大小
// 	incr = align >> PAGE_SHIFT ? : 1;       // 每次增长多少

// restart_scan:
//     for(i = preferred; i < eidx; i += incr){
//         unsigned long j;
//         if(test_bit(i, bdata->node_bootmem_map))
//             continue;
//         for(j = i +1; j < i + areasize; j++){
//             if(j >= eidx)
//                 goto fail_block;
//             if(test_bit(j, bdata->node_bootmem_map))
//                 goto fail_block;
//         }
//         start = i;      // 页面号
//         goto found;
//     fail_block:;
//     }
//     if(preferred){
//         preferred = offset;
//         goto restart_scan;
//     }
//     return NULL;
// found:
//     if(start>=eidx)
//         BUG();

//     // 判断是否上次分配的内存 能否 和本次分配合并
//     // 合并的三个条件：1. align小于一个页面大小 PAGE_SIZE 
//     // 2. 上次分配的页与此次分配的页相邻 
//     // 3. 上一页有空闲空间，即last_offset不为0，表示没有全部使用
//     if (align <= PAGE_SIZE
// 	    && bdata->last_offset && bdata->last_pos+1 == start) {

// 		offset = (bdata->last_offset+align-1) & ~(align-1);
// 		if (offset > PAGE_SIZE)
// 			BUG();

// 		remaining_size = PAGE_SIZE-offset;
// 		if (size < remaining_size) {
// 			areasize = 0;
// 			// last_pos unchanged
// 			bdata->last_offset = offset+size;
// 			ret = phys_to_virt(bdata->last_pos*PAGE_SIZE + offset +
// 						bdata->node_boot_start);
// 		} else {
// 			remaining_size = size - remaining_size;
// 			areasize = (remaining_size+PAGE_SIZE-1)/PAGE_SIZE;

// 			ret = phys_to_virt(bdata->last_pos*PAGE_SIZE + offset +
// 						bdata->node_boot_start);

// 			bdata->last_pos = start+areasize-1;
// 			bdata->last_offset = remaining_size;
// 		}
// 		bdata->last_offset &= ~PAGE_MASK;

// 	} else {
// 		bdata->last_pos = start + areasize - 1;
// 		bdata->last_offset = size & ~PAGE_MASK;
// 		ret = phys_to_virt(start * PAGE_SIZE + bdata->node_boot_start);
// 	}

//     // 正式分配内存
//     for(i = start; i < start+areasize; i++)
//         if(test_and_set_bit(i, bdata->node_bootmem_map))
//             BUG();
//     memset(ret, 0, size);
//     return ret;
}
