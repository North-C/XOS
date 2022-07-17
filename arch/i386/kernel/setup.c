#include <linux/init.h>
#include <linux/kernel.h>
#include <asm-i386/page.h>
#include <asm-i386/e820.h>
#include <linux/bootmem.h>
#include <linux/multiboot.h>
#include <linux/stdio.h>
#include <asm-i386/processor.h>
#include <asm-i386/pgtable.h>
#include <linux/string.h>

extern char _start, _end, _text, _etext, _data, _edata; 

// page frame 计算
#define PFN_UP(x)   (((x) + PAGE_SHIFT-1) >> PAGE_SHIFT)  // 上一个页面边界
#define PFN_DOWN(x) ((x) >> PAGE_SHIFT)             // 下一个页面边界
#define PFN_PHYS(x) ((x) << PAGE_SHIFT)             // 页框的物理地址

// 128MB 保留给vmalloc和initrd的区域
// vmalloc在原有的线性映射之外额外分配空间，同时建立虚拟地址和物理页面的映射，大小限于128MB
#define VMALLOC_RESERVE     (unsigned long)(128 << 20)
// 内核可以直接访问的，即最大的RAM空间容量， 1GB-128MB = 896MB，即MAXMEM
#define MAXMEM              (unsigned long) (-PAGE_OFFSET-VMALLOC_RESERVE)   // 128MB内存的保留,为啥这里是 - ??
#define MAXMEM_PFN          PFN_DOWN(MAXMEM)
#define MAX_NONPAE_PFN      (1 << 20)


// 启动时的setup例程设置
#define PARAM ((unsigned char *) empty_zero_page)
#define EXT_MEM_K (*(unsigned short *) (PARAM + 2))
#define ALT_MEM_K (*(unsigned long *) (PARAM+0x1e0))

// struct cpuinfo_x86 boot_cpu_data = { 0, 0, 0, 0, -1, 1, 0, 0, -1 };
// 全局内存映射图
struct e820map e820;
struct e820map bios_tmp;     // 暂时用于保存

// 用户定义的 high memory 的大小
static unsigned int highmem_pages __initdata = -1;

static unsigned long __init find_max_low_pfn(void);
static void __init register_bootmem_low_pages(unsigned long max_low_pfn);
static void __init find_max_pfn(void);
void show_memory_map();

static void __init setup_memory_region(void);
static void __init init_biosmap();
static int __init sanitize_e820_map(struct e820entry * biosmap, int * pnr_map);
static int __init copy_e820_map(struct e820entry * biosmap, int nr_map);
static void __init add_memory_region(unsigned long long start, unsigned long long size, int type);
static void __init print_memory_map(char *who);

void __init setup_arch()
{   
    // max_low_pfn 以低端内存区域表示的最大PFN
    // max_pfn 系统中可用的最大PFN
    // start_pfn 表示内存中内核映像以上第一个可以动态分配的页面
    // unsigned long start_pfn, max_pfn, max_low_pfn;
    
    // unsigned long bootmp_size;
    int i;

    // printk("kernel in memory start: 0x%08x\n", &_start);
    // printk("kernel in memory end: 0x%08x\n", &_end - 0xc0000000);
    // printk("kernel in memory used: %d KB\n\n", (&_end - 0xc0000000 - &_start + 1023)/1024);
    printk("kernel in memory start: 0x%08x\nkernel in memory end: 0x%08x\nkernel in memory used: %d KB\n\n ",&_start, &_end - 0xc0000000, (&_end - 0xc0000000 - &_start + 1023)/1024);

    show_memory_map();

    setup_memory_region();      // 将 e820图 的内容存放到安全的地址，构建内存映射图e820

    // // 建立内存页面所需的数据结构
    // max_low_pfn = setup_memory();  

    // // 建立内存页面管理机制
    // paging_init();
    
    // register_memory(max_low_pfn);
}

#define LOWMEMSIZE()   (0x9f000)

// 打印物理内存段
void  show_memory_map()
{
    // 保存内存信息结构的缓冲区地址
    multiboot_uint32_t mmap_addr = ((multiboot_info_t*)global_multiboot_info)->mmap_addr;
    multiboot_uint32_t mmap_length = ((multiboot_info_t*)global_multiboot_info)->mmap_length;

    printk("Memory map: address = 0x%x, length = 0x%x\n", mmap_addr, mmap_length);

    multiboot_memory_map_entry_t *mmap = (multiboot_memory_map_entry_t *)mmap_addr;

    while((uint32_t)mmap < (mmap_addr + mmap_length)){
        printk("size = 0x%x, base_addr = 0x%0x%08x, length = 0x%0x%08x, type = 0x%x\n", mmap->size, mmap->base_addr_high, mmap->base_addr_low, mmap->length_high, mmap->length_low, mmap->type);
        // 移动到下一个
        mmap = (multiboot_memory_map_entry_t *)((uint32_t)mmap + mmap->size + sizeof(mmap->size));
    }
}


// 获取到内存布局，将其保存到安全的地方
static void __init setup_memory_region(void)
{
    char *who = "BIOS-e820";

    // 用global_multiboot_info中的内存信息直接初始化e820图
    init_biosmap();
    printk("e820.map addr = 0x%x, e820.nr_map = %d\n", bios_tmp.map, bios_tmp.nr_map);
    // 消除e820当中的区域重叠
    sanitize_e820_map(bios_tmp.map, &bios_tmp.nr_map);
    // 将 E820_MAP 复制到一个安全的地方
    copy_e820_map(bios_tmp.map, bios_tmp.nr_map);       // 省略了一些对于错误情况的处理
    打印内存区域的分布情况
    printk("BIOS-provided pysical RAM map:\n");
    print_memory_map(who);
    printk("setup_memory_region done!\n");
}

// 确定可用物理页面的界限，构建引导内存分配器
static unsigned long __init setup_memory(void)
{   
    // bootmap_size 
    unsigned long bootmap_size, start_pfn, max_low_pfn;

    start_pfn = PFN_UP(__pa(&_end)); // _end是内核镜像的最终地址，在链接脚本中提供

    find_max_pfn();         // 查找系统中最大的可用PFN

    max_low_pfn = find_max_low_pfn();       // 最大的low memory的页面号

    printk("%ldMB LOWMEM available.\n", pages_to_mb(max_low_pfn));

   // 准备建立物理内存页面管理机制，建立一个页面位图
    bootmap_size = init_bootmem(start_pfn, max_low_pfn);
    
    // 初始化可用的RAM内存，将其清空
    register_bootmem_low_pages(max_low_pfn);

    // 分配位图本身的内存
    reserve_bootmem(HIGH_MEMORY, (PFN_PHYS(start_pfn) + bootmap_size + PAGE_SIZE-1) - (HIGH_MEMORY));
    
    // 分配 物理页 0,用于很多BIOS功能的页面
    reserve_bootmem(0, PAGE_SIZE);

    return max_low_pfn;
}

/* 查找系统中最大的可用PFN */
static void __init find_max_pfn(void)
{
    int i;

    max_pfn = 0;        // 获取系统可用的最大页面号
    for (i = 0; i < e820.nr_map; i++){
        unsigned long start, end;

        if(e820.map[i].type != E820_RAM)
            continue;
        start = PFN_UP(e820.map[i].addr);
        end = PFN_DOWN(e820.map[i].addr + e820.map[i].size);
        if(start >= end)
            continue;
        if(end > max_pfn)
            max_pfn = end;
    }
}

/**
 * @brief 寻找低端内存的最大pfn, 划分低端内存和高端内存区域
 * @return unsigned long 低端内存的最大pfn
 */
static unsigned long __init find_max_low_pfn(void)
{
    unsigned long max_low_pfn;

    max_low_pfn = max_pfn;
    if(max_low_pfn > MAXMEM_PFN){       // 什么是 MAXMEM_PFN？ 
        if(highmem_pages == -1)         // 还没有设置，进行初始化就好
            highmem_pages = max_pfn - MAXMEM_PFN;

        if(highmem_pages + MAXMEM_PFN < max_pfn)        // 调整最大可用的内存
            max_pfn = MAXMEM_PFN + highmem_pages;

        if(highmem_pages + MAXMEM_PFN > max_pfn){   // 忽略了一些内存没有使用
            printk("only %luMB highmem pages available, ignore highmem size of %uMB.\n", pages_to_mb(max_pfn - MAXMEM_PFN), pages_to_mb(highmem_pages));

            highmem_pages = 0;
        }
        max_low_pfn = MAXMEM_PFN;       
    }else{
        if(highmem_pages == -1)
            highmem_pages = 0;
    }

    return max_low_pfn;
}

/**
 * @brief 注册可用的 低端RAM 页面上
 * 
 * @param max_low_pfn 
 */
static void __init register_bootmem_low_pages(unsigned long max_low_pfn)
{
    int i;

    for(i = 0; i < e820.nr_map; i++){
        unsigned long curr_pfn, last_pfn, size;

        if(e820.map[i].type != E820_RAM){   // 不是可用的RAM
            continue;
        }

        curr_pfn = PFN_UP(e820.map[i].addr);    
        if(curr_pfn >= max_low_pfn){    // 超出了低端内存区域的界限
            continue;
        }

        last_pfn = PFN_DOWN(e820.map[i].addr + e820.map[i].size);

        if(last_pfn > max_low_pfn){     // 超出了低端内存区域的界限
            last_pfn = max_low_pfn;
        }

        if(last_pfn <= curr_pfn){
            continue;
        }

        size = last_pfn - curr_pfn; 
        // 操作contig_page_data结构中的位图，释放位对应的物理页面
        free_bootmem(PFN_PHYS(curr_pfn), PFN_PHYS(size));
    }
}

// 用grub提供的内存描述信息初始化e820图
static void __init init_biosmap()
{
    // 保存内存信息结构的缓冲区地址
    multiboot_uint32_t mmap_addr = ((multiboot_info_t*)global_multiboot_info)->mmap_addr;
    multiboot_uint32_t mmap_length = ((multiboot_info_t*)global_multiboot_info)->mmap_length;
    int i = 0;

    multiboot_memory_map_entry_t *mmap = (multiboot_memory_map_entry_t *)mmap_addr;
    while((multiboot_uint32_t)mmap < (mmap_addr + mmap_length)){
        bios_tmp.map[i].addr = mmap->base_addr_low;
        bios_tmp.map[i].size = mmap->length_low;
        bios_tmp.map[i].type = mmap->type;
        i++;
        // 移动到下一个
        mmap = (multiboot_memory_map_entry_t *)((uint32_t)mmap + mmap->size + sizeof(mmap->size));
    }
    bios_tmp.nr_map = i;
}

/**
 * @brief 消除 e820map当中的内存区域重叠,并进行排序
 * 
 * @param biosmap 已有的e820图
 * @param pnr_map 物理内存区域的数量
 * @return int 
 */
static int __init sanitize_e820_map(struct e820entry * biosmap, int * pnr_map){
    struct change_member{
        struct e820entry *pbios;        // 指向 原有的bios 内存条目
        unsigned long long addr;        // 对应的修改点的地址
    }; 

    struct change_member change_point_list[2*E820MAX];  // 修改点列表, E820MAX 为 32
    struct change_member *change_point[2*E820MAX];   // 修改点, 记录起始和结束
    struct e820entry *overlap_list[E820MAX];   // 重叠区域的列表
    struct e820entry new_bios[E820MAX];     
    struct change_member *change_tmp;        
    unsigned long current_type, last_type;    // 
    unsigned long long last_addr;       // 
    int chgidx, still_changing;
    int overlap_entries;
    int new_bios_entry;
    int old_nr, new_nr, chg_nr;
    int i;

    if(*pnr_map < 2){       // 只有一个内存区域
        return -1;
    }

    old_nr = *pnr_map;

    /* 内存区域的地址不合理 */
    for(i = 0; i < old_nr; i++){
        if(biosmap[i].addr + biosmap[i].size < biosmap[i].addr){
            return -1;
        }
    }

    /* 创建修改点，后续进行排序 */
    for(i = 0; i < 2*old_nr; i++){
        change_point[i] = &change_point_list[i];
    }

    /* 记录所有的已知的的修改点，忽略大小为0的内存区域 */
    chgidx = 0;
    for(i = 0; i < old_nr; i++){
        if(biosmap[i].size != 0){
            change_point[chgidx]->addr = biosmap[i].addr;  // 起始地址
            change_point[chgidx++]->pbios = &biosmap[i];
            change_point[chgidx]->addr = biosmap[i].addr + biosmap[i].size; // 结束地址
            change_point[chgidx++]->pbios = &biosmap[i];
        }
    }
    chg_nr = chgidx;        // 修改点的数目

    /* 从低到高排序修改点的列表 */
    still_changing = 1;
    while(still_changing){
        still_changing = 0;
        for(i = 1; i < chg_nr; i++){
            // 两种情形：<当前地址> 小于 <前一个地址> 或者 当前的地址
            if((change_point[i]->addr < change_point[i-1]->addr) ||
                ((change_point[i]->addr == change_point[i-1]->addr) && (change_point[i]->addr == change_point[i]->pbios->addr) && (change_point[i-1]->addr != change_point[i-1]->pbios->addr))){
                    change_tmp = change_point[i];
                    change_point[i] = change_point[i-1];
                    change_point[i-1] = change_tmp;
                    still_changing = 1;
                }
        }
    }
    // 创建一个新的bios 内存映射，消除重叠
    overlap_entries = 0;   // 重叠列表中的条目数量
    new_bios_entry = 0;   // 新的bios内存映射的条目index
    last_type = 0;      // 未定义的内存类型
    last_addr = 0;      // 从0开始，最新的地址方向

    // 遍历所有的change-points
    for(chgidx = 0; chgidx < chg_nr; chgidx++){
        // 记录所有重叠的bios内存映射条目
        if(change_point[chgidx]->addr == change_point[chgidx]->pbios->addr){
            // 记录到overlap_list当中
            overlap_list[overlap_entries++] = change_point[chgidx]->pbios;
        }
        else{
            for(i = 0; i < overlap_entries; i++){
                if(overlap_list[i] == change_point[chgidx]->pbios)
                    overlap_list[i] = overlap_list[overlap_entries-1];
            }
            overlap_entries--;
        }

        // 对于重叠的条目，决定使用哪个类型。`1`为可用RAM，其余为不可用类型
        current_type = 0;
        for(i = 0; i < overlap_entries; i++){
            if(overlap_list[i]->type > current_type)
                current_type = overlap_list[i]->type;
        }
        // 继续构建新的bios内存映射
        if(current_type != last_type){
            if(last_type != 0){
                new_bios[new_bios_entry].size = change_point[chgidx]->addr - last_addr;
                if(new_bios[new_bios_entry].size != 0){
                    if(++new_bios_entry >= E820MAX)
                        break;
                }
            }
            if(current_type != 0){
                new_bios[new_bios_entry].addr = change_point[chgidx]->addr;
                new_bios[new_bios_entry].type = current_type;
                last_addr = change_point[chgidx]->addr;
            }
            last_type = current_type;
        }
    }
    new_nr = new_bios_entry;        // 新的bios条目的数量

    // 复制新的bios内存映射
    memcpy(biosmap, new_bios, new_nr*sizeof(struct e820entry));
    *pnr_map = new_nr;

    return 0;
}

static int __init copy_e820_map(struct e820entry * biosmap, int nr_map)
{
    if(nr_map < 2){     // 仅一个内存区域
        return -1;
    }

    do{
        unsigned long long start = biosmap->addr;
        unsigned long long size = biosmap->size;
        unsigned long long end = start + size;
        unsigned long type = biosmap->type;

        // 溢出
        if(start > end){   
            return -1;
        }

        // 一些BIOS位于 640K - 1M 的区域，修复它
        if(type == E820_RAM){           // 可用的RAM类型
            if(start < 0x100000ULL && end > 0xA0000ULL){
                // start在640K以下，则在start 至 640K 额外添加一个内存区域
                if(start < 0xA0000ULL)
                    add_memory_region(start, 0xA0000ULL-start, type);
                if(end <= 0x100000ULL)      // 处于640K-1MB区域，等待下一次的end位置调整
                    continue;
                start = 0x100000ULL;
                size = end - start;
            }
        }
        add_memory_region(start, size, type);
    }while(biosmap++, --nr_map);
    return 0;
}

// 在全局变量e820增加一个内存区域条目，这是最后可用的结果
static void __init add_memory_region(unsigned long long start, unsigned long long size, int type)
{
    int x = e820.nr_map;

    if(x == E820MAX){
        printk(KERN_ERR "Ooops! Too many entris in the memory map!\n");
        return ;
    }

    e820.map[x].addr = start;
    e820.map[x].size = size;
    e820.map[x].type = type;
    e820.nr_map++;
}

static void __init print_memory_map(char *who)
{
    int i;

    for(i = 0; i < e820.nr_map; i++){
        printk("i = %d\n", i);
        printk(" %s: %0x - %0x ", who, e820.map[i].addr, e820.map[i].addr + e820.map[i].size);
        switch (e820.map[i].type){
        case E820_RAM:
            printk("(usable)\n");
            break;
        case E820_RESERVED:
            printk("(reserved)\n");
            break;
        case E820_ACPI:
            printk("(ACPI data)\n");
            break;
        case E820_NVS:
            printk("(ACPI NVS)\n");
            break;
        default:
            printk("(type %lu)\n", e820.map[i].type);
            break;
        }
    }
}


