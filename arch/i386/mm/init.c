/**
 * @file init.c
 * @brief 在启动后，进行页表的初始化
 * @date 2022-05-24
 * 
 */
#include <linux/init.h>
#include <asm-i386/page.h>
#include <linux/bootmem.h>
#include <linux/mmzone.h>
#include <asm-i386/pgtable.h>
#include <asm-i386/processor.h>
#include <asm-i386/io.h>
#include <linux/debug.h>
#include <asm-i386/pgtable-2level.h>
#include <asm-i386/fixmap.h>
#include <asm-i386/e820.h>
#include <linux/mm.h>
#include <linux/stdio.h>
#include <linux/string.h>

unsigned long highstart_pfn, highend_pfn;
static unsigned long totalram_pages;
static unsigned long totalhigh_pages;

// 将页面映射表定义在此处
pgd_t swapper_pg_dir[1024] __attribute__((__aligned__(PAGE_SIZE)));

extern struct e820map e820;
extern void* high_memory;

static void pagetable_init(void);
static void fixrange_init(unsigned long start, unsigned long end, pgd_t *pgd_base);
static void zone_sizes_init(void);

extern unsigned long free_all_bootmem (void);
extern unsigned int nr_free_pages (void);

unsigned long highend_pfn;

/**
 * @brief 建立页表
 * 
 */
void paging_init(void)
{
    printk("starting paging_init...\n");
    printk("starting pagetable_init...\n");
    // 扩充在启动时已经建立好的8MB大小的页表映射
    pagetable_init();

    printk("starting load_cr3 and flush_tlb...\n");

    // 刷新cr3，重新加载页表的映射
    load_cr3(swapper_pg_dir);
    // 刷新TLB缓存
    __flush_tlb_all();          // BUG: 刷新TLB之后则崩溃
    printk("starting zone_sizes_init...\n");
    // 管理区的初始化设置
    zone_sizes_init();
    printk("paging_init done\n");
}

/**
 * @brief 扩充在初始化第一阶段中创建的PGD和PGT
 * 
 */
static void pagetable_init(void)
{
    unsigned long vaddr, end;
    pgd_t *pgd, *pgd_base;
    int i, j, k;
    pmd_t *pmd;
    pte_t *pte, *pte_base;
    // ZONE_NORMAL 中物理内存的末端
    end = (unsigned long) __va(max_low_pfn * PAGE_SIZE);

    pgd_base = swapper_pg_dir;          // 指向位于 swapper_pg_dir 的页表的pgd

    i = __pgd_offset(PAGE_OFFSET);  // PAGE_OFFSET 在 pgd 中对应的偏移量 0 
    pgd = pgd_base + i;            // 内核部分的起点

    printk("swapper_pg_dir: %x\n", pgd_base);
    // 页面映射为 1024个大小的指针数组，从768项开始，为系统空间准备，只会计算线性地址空间的内核部分
    for(; i < PTRS_PER_PGD; pgd++, i++){
        vaddr = i * PGDIR_SIZE;       

        if (end && (vaddr >= end))
            break;

        pmd = (pmd_t *)pgd;    // 没有pmd，直接映射到 pgd
        if(pmd != pmd_offset(pgd, 0))   // 保证pmd有效
            BUG();
        // PMD在2层页表机制中实质上是无效的，初始化为 1 
        for(j = 0; j < PTRS_PER_PMD; pmd++, j++){
            vaddr = i*PGDIR_SIZE + j*PMD_SIZE;
            if(end && (vaddr >= end)){
                break;
            }
            // 分配一个页面
            pte_base = pte = (pte_t *) alloc_bootmem_low_pages(PAGE_SIZE);  

            // 遍历页表项
            for(k = 0; k < PTRS_PER_PTE; pte++, k++){
                vaddr = i*PGDIR_SIZE + j*PMD_SIZE + k*PAGE_SIZE;
                // BUG: 在 vaddr = 0xc1be3000, pte = 0xc0007f05，即设置第961次页表项时，end 突然从 f800000 被修改，突然变为乱码，导致从此处跳出，使得后续无法对齐
                if(end && (vaddr >= end))   // 超出界限,直接跳出了范围
                    break;
                // 构造完整的页表项，物理地址 + 标志位
                *pte = mk_pte_phys(__pa(vaddr), PAGE_KERNEL);
            }
            // 设置 pmd 
            set_pmd(pmd, __pmd(_KERNPG_TABLE + __pa(pte_base)));
            
            // printk("pte_offset = %x\n", pte_offset(pmd, 0));
            // BUG: 在pmd: 0xc0116c18, pte_base: 0xc000e000 时出现BUG，没有对齐
            // 当整体内存设置为 512MB 时，不会出现错误，而在 896MB 时会出现如上错误
            if(pte_base != pte_offset(pmd, 0)){ // 保证页表项有效
                BUG();    
            }      
        }
    }
    // TODO: 固定虚拟地址空间的位置从FIXADDR_TOP开始，在地址空间前面结束
    // __end_of_fixed_addresses 是上一个由固定虚拟地址空间用到的下标
    // 返回固定虚拟地址空间起始位置的PMD虚拟地址
    // vaddr = __fix_to_virt(__end_of_fixed_addresses - 1) & PMD_MASK;
    // fixrange_init(vaddr, 0, pgd_base);
    printk("pagetable_init done.\n");
}

static void fixrange_init(unsigned long start, unsigned long end, pgd_t *pgd_base)
{
    pgd_t *pgd;
    pmd_t *pmd;
    pte_t *pte;
    int i, j;
    unsigned long vaddr;

    vaddr = start;
    i = __pgd_offset(vaddr);
    j = __pmd_offset(vaddr);
    pgd = pgd_base + i;

    for( ; i < PTRS_PER_PGD && (vaddr != end); pgd++, i++){
        pmd = (pmd_t *)pgd;

        for(; (j < PTRS_PER_PMD) && (vaddr != end); pmd++, j++){
            if(pmd_none(*pmd)){
                // 分配一个页面
                pte = (pte_t *) alloc_bootmem_low_pages(PAGE_SIZE);
                set_pmd(pmd, __pmd(_KERNPG_TABLE + __pa(pte)));
                // BUG
                // if(pte != pte_offset(pmd, 0)){
                //     BUG();
                // }
            }
            vaddr += PMD_SIZE;
        }
        j = 0;
    }
}

// 管理区的初始化
static void zone_sizes_init(void)
{
    unsigned long zones_size[MAX_NR_ZONES] = {0, 0, 0}; // 最多三种zone管理区
    unsigned int max_dma, high, low;
    // 16MB 用于DMA的内存, max_dma=0x1000
    max_dma = virt_to_phys((char *)MAX_DMA_ADDRESS) >> PAGE_SHIFT;
	low = max_low_pfn;

	if (low < max_dma)
		zones_size[ZONE_DMA] = low;
	else {
		zones_size[ZONE_DMA] = max_dma;
		zones_size[ZONE_NORMAL] = low - max_dma;

    }
    // 初始化空闲区域
    free_area_init(zones_size);
    printk("zone_sizes_init done.\n");
}

static inline int page_is_ram (unsigned long pagenr)
{
	int i;

	for (i = 0; i < e820.nr_map; i++) {
		unsigned long addr, end;

		if (e820.map[i].type != E820_RAM)	/* not usable memory */
			continue;
		/*
		 *	!!!FIXME!!! Some BIOSen report areas as RAM that
		 *	are not. Notably the 640->1Mb area. We need a sanity
		 *	check here.
		 */
		addr = (e820.map[i].addr+PAGE_SIZE-1) >> PAGE_SHIFT;
		end = (e820.map[i].addr+e820.map[i].size) >> PAGE_SHIFT;
		if  ((pagenr >= addr) && (pagenr < end))
			return 1;
	}
	return 0;
}

extern char _text, _etext, _edata, _end, _start;
extern unsigned long empty_zero_page[1024];
extern unsigned long num_mappedpages;

void mem_init(void)
{
    int codesize, reservedpages, datasize, initsize;
	int tmp;

	if (!mem_map)
		BUG();
    
    max_mapnr = num_mappedpages = num_physpages = max_low_pfn;
    high_memory = (void *) __va(max_low_pfn * PAGE_SIZE);

    /* clear the zero-page */
	memset(empty_zero_page, 0, PAGE_SIZE);

    /* this will put all low memory onto the freelists */
	totalram_pages += free_all_bootmem();
    printk("%s: %d: totalram_pages = %d\n", __func__, __LINE__, totalram_pages);
    
	reservedpages = 0;
	for (tmp = 0; tmp < max_low_pfn; tmp++)
		/*
		 * Only count reserved RAM pages
		 */
		if (page_is_ram(tmp) && PageReserved(mem_map+tmp))
			reservedpages++;

    //codesize =  (unsigned long) &_etext - (unsigned long) &_text;
	// datasize =  (unsigned long) &_edata - (unsigned long) &_etext;
	// initsize =  (unsigned long) &__init_end - (unsigned long) &__init_begin;

	// printk("Memory: %luk/%luk available\n",
	// 	(unsigned long) nr_free_pages() << (PAGE_SHIFT-10),
	// 	max_mapnr << (PAGE_SHIFT-10));
    
    printk("kernel in memory start: 0x%08X\n", &_start);
    printk("kernel in memory end:   0x%08X\n", &_end - 0xc0000000);
    printk("kernel in memory used:   %d KB\n\n", (&_end - 0xc0000000 - &_start + 1023) / 1024);
}