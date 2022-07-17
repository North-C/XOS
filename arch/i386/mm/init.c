/**
 * @file init.c
 * @brief 在启动后，进行页表的初始化
 * @date 2022-05-24
 * 
 */
#include <linux/init.h>
#include <asm-i386/page.h>
#include <linux/mmzone.h>
#include <asm-i386/pgtable.h>
#include <asm-i386/processor.h>
#include <linux/bootmem.h>
#include <asm-i386/io.h>
#include <linux/debug.h>

// 就是表示 Linux当中将其定义在 0x1000位置，这里就随意定义在这里??
pgd_t swapper_pg_dir[1024] __attribute__((__aligned__(PAGE_SIZE)))

/**
 * @brief 建立页表
 * 
 */
void __init paging_init()
{
    // 扩充在启动时已经建立好的8MB大小的页表映射
    pagetable_init();
    // 刷新cr3，重新加载页表的映射
    load_cr3(swapper_pg_dir);
    // 刷新TLB缓存
    __flush_tlb_all();
    // 管理区的初始化设置
    zone_sizes_init();
}

/**
 * @brief 扩充在初始化第一阶段中创建的PGD和PGT
 * 
 */
static void __init pagetable_init(void)
{
    unsigned long vaddr, end;
    pgd_t *pgd, *pgd_base;
    int i, j, k;
    pmd_t *pmd;
    pte_t *pte, *pte_base;

    end = (unsigned long) __va(max_low_pfn * PAGE_SIZE);

    pgd_base = swapper_pg_dir;  

    i = __pgd_offset(PAGE_OFFSET);  // pgd的位置
    pgd = pgd_base +1;

    // 页面映射为 1024个大小的指针数组，从768项开始，为系统空间准备
    for(; i < PTRS_PER_PGD; pgd++, i++){
        vaddr = i*PGDIR_SIZE;       

        if (end && (vaddr >= end))
            break;

        pmd = (pmd_t *)pgd;
        if(pmd != pmd_offset(pgd, 0))
            BUG();
        // PMD在2层页表中实质上是无效的
        for(j = 0; j < PTRS_PER_PMD; pmd++, j++){
            vaddr = i*PGDIR_SIZE + j*PMD_SIZE;
            if(end && (vaddr >= end)){
                break;
            }

            pte_base = pte = (pte_t *) alloc_bootmem_low_pages(PAGE_SIZE);
            // 遍历页表项
            for(k = 0; k<PTRS_PER_PTE; pte++, k++){
                vaddr = i*PGDIR_SIZE + j*PMD_SIZE + k*PAGE_SIZE;
                if(end && (vaddr >= end))
                    break;
                // 
                *pte = mk_pte_phys(__pa(vaddr), PAGE_KERNEL);
            }
            set_pmd(pmd, __pmd(_KERNPG_TABLE + __pa(pte_base)));
            if(pte_base != pte_offset(pmd, 0))
                BUG();
        }
    }
}

// 管理区的初始化
static void __init zone_sizes_init(void)
{
    unsigned long zones_size[MAX_NR_ZONES] = {0, 0, 0}; // 最多三种zone管理区
    unsigned int max_dma, high, low;

    max_dma = virt_to_phys((char *)MAX_DMA_ADDRESS) >> PAGE_SHIFT;
	low = max_low_pfn;
	high = highend_pfn;

	if (low < max_dma)
		zones_size[ZONE_DMA] = low;
	else {
		zones_size[ZONE_DMA] = max_dma;
		zones_size[ZONE_NORMAL] = low - max_dma;

    }
    // 释放
    free_area_init(zones_size);
}
