#ifndef _I386_PGTABLE_2LEVEL_H
#define _I386_PGTABLE_2LEVEL_H

// 传统的两层的页表映射机制
#define PGDIR_SHIFT 22
#define PTRS_PER_PGD  1024

// 逻辑上有PMD，而物理上是不存在
#define PMD_SHIFT 22
#define PTRS_PER_PMD   1

#define PTRS_PER_PTE    1024

static inline int pgd_none(pgd_t pgd) { return 0; }

static inline pmd_t * pmd_offset(pgd_t * dir, unsigned long address)
{
    return (pmd_t *)dir;
}

#define __mk_pte(page_nr, pgprot) __pte(((page_nr) << PAGE_SHIFT) | pgprot_val(pgprot))

#endif