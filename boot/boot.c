#include <linux/init.h>
#include <asm-i386/gdt.h>
#include <linux/multiboot.h>
#include <asm-i386/types.h>
#include <asm-i386/page.h>

uint32_t kern_stack_top;
void kern_entry(void);
extern void *flush;
void* global_multiboot_info;


// 页目录表项，为啥放在.data.init段呢？为啥不放在.data段呢？
pgd_t pgd[1024] __attribute__((__aligned__(PAGE_SIZE))) __attribute__ ((__section__(".data.init")));
// 页表项
pte_t pte[1024] __attribute__((__section__(".data.init")));

// 页目录表的位置
#define PAGE_DIR_TABLE_POS 0x90000

unsigned long empty_zero_page[1024];    // 启动时的临时内存

/* 构建GDT */
__init static struct gdt_desc make_gdt_desc(uint32_t *desc_addr, uint32_t limit, uint8_t attr_high, uint8_t attr_low)
{
    uint32_t desc_base = (uint32_t)desc_addr;
    struct gdt_desc desc;
    desc.low_limit_word = (limit & 0x0000ffff);
    desc.low_base_word = (desc_base & 0x0000ffff);
    desc.mid_base_byte = ((desc_base & 0x00ff0000) >> 16);
    desc.low_attr_byte = (uint8_t)attr_low;
    desc.limit_high_attr_byte = (((limit & 0x000f0000) >> 16) + (uint8_t)attr_high);
    desc.high_base_byte = (desc_base >> 24);
    return desc;
}

__init static void gdt_create(void)
{
    // GDT 放在0x900
    // 空描述符
    *((struct gdt_desc *)0x900) = make_gdt_desc((uint32_t *)0, 0, 0, 0);

    // Linux将第2个保留不用，这里不做保留。
    // kernel code segment
    *((struct gdt_desc *)0x908) = make_gdt_desc((uint32_t *)0, 0xfffff, GDT_ATTR_HIGH, GDT_CODE_ATTR_LOW_DPL0);
    // kernel data segment
    *((struct gdt_desc *)0x910) = make_gdt_desc((uint32_t *)0, 0xfffff, GDT_ATTR_HIGH, GDT_DATA_ATTR_LOW_DPL0);
    // kernel video segment
    *((struct gdt_desc *)0x918) = make_gdt_desc((uint32_t *)0xb8000, 0x00007, GDT_ATTR_HIGH, GDT_DATA_ATTR_LOW_DPL0);
    // user code segment
    *((struct gdt_desc *)0x920) = make_gdt_desc((uint32_t *)0, 0xfffff, GDT_ATTR_HIGH, GDT_ATTR_LOW_CODE_DPL3);
    // user data segment
    *((struct gdt_desc *)0x928) = make_gdt_desc((uint32_t *)0, 0xfffff, GDT_ATTR_HIGH, GDT_ATTR_LOW_DATA_DPL3);

    // gdt的32位基地址 和 16位limit
    uint64_t gdt_operand = (((uint64_t)(uint32_t)0x900 << 16) | (8 * 6 - 1));

    asm volatile("lgdt %0"
                 :
                 : "m"(gdt_operand));
    asm volatile("call %0"
                 :
                 : "m"(flush)); // 刷新流水线
    // 设置 段寄存器
    asm volatile("mov %0, %%eax; mov %%eax, %%ds"
                 :
                 : "i"(SELECTOR_K_DATA));
    asm volatile("mov %0, %%eax; mov %%eax, %%es"
                 :
                 : "i"(SELECTOR_K_DATA));
    asm volatile("mov %0, %%eax; mov %%eax, %%fs"
                 :
                 : "i"(SELECTOR_K_DATA));
    asm volatile("mov %0, %%eax; mov %%eax, %%ss"
                 :
                 : "i"(SELECTOR_K_DATA));
}

/**
 * 进入分页模式，创建页表：
 *   共创建1024个页表
 *
 */
static void __init page_create()
{   
    int i = 0;
    // 清空页目录所占的空间
    for (i = 0; i < 1024; i++)
    {
        pgd[i].pgd = 0;
    }

    // 创建 PDE
    // 将第0项和第768项目录项都指向第一个页表
    pgd[0].pgd = (uint32_t)pte | PAGE_USER | PAGE_RW | PAGE_PRESENT;
    pgd[768].pgd = (uint32_t)pte | PAGE_USER | PAGE_RW | PAGE_PRESENT;
    // 最后一个页目录项指向自己
    // pgd[1023].pgd = (uint32_t)pgd | PAGE_USER | PAGE_RW | PAGE_PRESENT;

    /* 循环创建1024个PTE */
    // 初始化清空
    uint32_t phy_addr = 0;
    for (i = 0; i < 1024; i++)
    {
        pte[i].pte_low = 0;
    }
    for (i = 0; i < 1024; i++)
    {
        pte[i].pte_low = phy_addr | PAGE_USER | PAGE_RW | PAGE_PRESENT;
        phy_addr += PAGE_SIZE; // 一个页面的大小为4KB
    }

    global_multiboot_info = (multiboot_info_t *)((uint32_t)global_multiboot_info + PAGE_OFFSET);
    
    // 内核操作的显存段
    *((struct gdt_desc *)0x918) = make_gdt_desc((uint32_t *)0xc00b8000, 0x00007, GDT_ATTR_HIGH, GDT_DATA_ATTR_LOW_DPL0);

    // 设置 gdtr的内容, 存放在c0000900
    uint64_t gdt_operand = (((uint64_t)(uint32_t)0xc0000900 << 16) | (8 * 6 - 1));

    // 设置cr3
    asm volatile("mov %0, %%cr3"
                 :
                 : "r"(pgd));

    // 打开cr0的PG位，打开分页机制
    uint32_t cr0 = 0;
    asm volatile("mov %%cr0, %0"
                 : "=r"(cr0)
                 :);
    cr0 |= 0x80000000;
    asm volatile("mov %0, %%cr0"
                 :
                 : "r"(cr0));
    // 加载gdtr
    asm volatile("lgdt %0"
                 :
                 : "m"(gdt_operand)); // lgdt重新加载GDT

    // 刷新流水线
    asm volatile("call %0"
                 :
                 : "m"(flush));
    return;
}

/* 内核入口函数 */
__init void kern_entry(void)
{
    gdt_create();

    page_create();
    
    uint32_t kern_stack_top;
    // 切换内核栈
    asm volatile("movl %%esp, %0"
                 : "=r"(kern_stack_top));
    kern_stack_top += PAGE_OFFSET;

    asm volatile("movl %0, %%esp"
                 :
                 : "r"(kern_stack_top));
                 
    start_kernel();
}
