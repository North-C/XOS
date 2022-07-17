#ifndef _ASM_PGTABLE_H
#define _ASM_PGTABLE_H

#include <asm-i386/pgtable-2level.h>

#define __pgd_offset(address) pgd_index(address)
#define pgd_index(address)  ((address >> PGDIR_SHIFT) & (PTRS_PER_PGD-1))
#define pages_to_mb(x) ((x) >> (20-PAGE_SHIFT))

#define PGDIR_SIZE  (1UL << PGDIR_SHIFT)

#define PMD_SIZE (1UL << PMD_SHIFT)

#define __flush_tlb_all() \
    do {								\
		if (cpu_has_pge)					\
			__flush_tlb_global();				\
		else							\
			__flush_tlb();					\
	} while (0)


#define __flush_tlb() \
    do{ \
        unsigned int tmpreg;        \
        __asm__ __volatile__(       \
            "movl %%cr3, %0; # flush TLB\n"  \
            "movl %0, %%cr3;            \n"     \
            : "=r" (tmpreg)         \
            ::   "memory");     \
    }while(0)

// 和一般的tlb 刷新有一些区别
#define __flush_tlb_global()						\
	do {								\
		unsigned int tmpreg;					\
									\
		__asm__ __volatile__(					\
			"movl %1, %%cr4;  # turn off PGE     \n"	\
			"movl %%cr3, %0;  # flush TLB        \n"	\
			"movl %0, %%cr3;                     \n"	\
			"movl %2, %%cr4;  # turn PGE back on \n"	\
			: "=&r" (tmpreg)				\
			: "r" (mmu_cr4_features & ~X86_CR4_PGE),	\
			  "r" (mmu_cr4_features)			\
			: "memory");					\
	} while (0)

// 物理页的地址
#define mk_pte_phys(physpage, pgprot) __mk_pte((phypage) >> PAGE_SHIFT, pgprot)

#endif