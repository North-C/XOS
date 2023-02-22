#ifndef _ASM_PGTABLE_H
#define _ASM_PGTABLE_H

#include <asm-i386/pgtable-2level.h>
#include <asm-i386/cpufeature.h>
#include <asm-i386/page.h>

extern void paging_init(void);
extern unsigned long mmu_cr4_features;

// 页表指示位的位置
#define _PAGE_BIT_PRESENT   0
#define _PAGE_BIT_RW		1
#define _PAGE_BIT_USER		2
#define _PAGE_BIT_PWT		3
#define _PAGE_BIT_PCD		4
#define _PAGE_BIT_ACCESSED  5
#define _PAGE_BIT_DIRTY		6
#define _PAGE_BIT_PAT       7
#define _PAGE_BIT_GLOBAL	8

#define _PAGE_PRESENT   0x001
#define _PAGE_RW		0x002
#define _PAGE_USER		0x004
#define _PAGE_PWT		0x008
#define _PAGE_PCD		0x010
#define _PAGE_ACCESSED  0x020
#define _PAGE_DIRTY		0x040
#define _PAGE_PAT       0x080
#define _PAGE_GLOBAL	0x100


#define pgd_index(address)  ((address >> PGDIR_SHIFT) & (PTRS_PER_PGD-1))
#define __pgd_offset(address) pgd_index(address)

#define __pmd_offset(address) (((address) >> PMD_SHIFT) & (PTRS_PER_PMD - 1))

/*
 * Permanent address of a page. Obviously must never be
 * called on a highmem page.
 */
#define page_address(page) ((page)->virtual)
#define pages_to_mb(x) ((x) >> (20-PAGE_SHIFT))

#define PGDIR_SIZE  (1UL << PGDIR_SHIFT)

#define pmd_page(pmd) ((unsigned long) __va(pmd_val(pmd) & PAGE_MASK))

#define __pte_offset(address) ((address >> PAGE_SHIFT) & (PTRS_PER_PTE - 1))
  
#define pte_offset(dir, address) ((pte_t *) pmd_page(*(dir)) + __pte_offset(address))

#define pmd_none(x)	(!pmd_val(x))
#define pmd_present(x)	(pmd_val(x) & _PAGE_PRESENT)
#define pmd_clear(xp)	do { set_pmd(xp, __pmd(0)); } while (0)


#define PMD_SIZE (1UL << PMD_SHIFT)
#define PMD_MASK (~(PMD_SIZE-1))

#define __flush_tlb() \
    do{ \
        unsigned int tmpreg;        \
        __asm__ __volatile__(       \
            "movl %%cr3, %0; # flush TLB\n"  \
            "movl %0, %%cr3;            \n"     \
            : "=r" (tmpreg)         \
            ::   "memory");     \
    }while(0)

// 物理页的地址
#define mk_pte_phys(physpage, pgprot) __mk_pte((physpage) >> PAGE_SHIFT, pgprot)


#define _PAGE_TABLE (_PAGE_PRESENT | _PAGE_RW | _PAGE_USER | _PAGE_ACCESSED | _PAGE_DIRTY)
// 内核页表属性位
#define _KERNPG_TABLE (_PAGE_PRESENT | _PAGE_RW | _PAGE_ACCESSED | _PAGE_DIRTY)

// 内核页表
#define __PAGE_KERNEL (_PAGE_PRESENT | _PAGE_RW | _PAGE_DIRTY | _PAGE_ACCESSED)

#define MAKE_GLOBAL(x) 			\
	({							\
		pgprot_t __ret;			\
		__ret = __pgprot(x);	\
		__ret;					\
})

#define PAGE_KERNEL MAKE_GLOBAL(__PAGE_KERNEL)

#endif