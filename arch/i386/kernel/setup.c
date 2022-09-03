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
#include <linux/ioport.h>
#include <asm-i386/io.h>


extern char _start, _end, _text, _etext, _data, _edata; 
struct cpuinfo_x86 boot_cpu_data = { 0, 0, 0, 0, -1, 1, 0, 0, -1 };
unsigned long mmu_cr4_features;

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

/* For PCI or other memory-mapped resources */
unsigned long pci_mem_start = 0x10000000;

// 用户定义的 high memory 的大小
static unsigned int highmem_pages = -1;

// 静态函数声明
static unsigned long find_max_low_pfn(void);
static void register_bootmem_low_pages(unsigned long max_low_pfn);
static void find_max_pfn(void);
void show_memory_map();

static void setup_memory_region(void);
static void init_biosmap();
static int sanitize_e820_map(struct e820entry * biosmap, int * pnr_map);
static int copy_e820_map(struct e820entry * biosmap, int nr_map);
static void add_memory_region(unsigned long long start, unsigned long long size, int type);
static void print_memory_map(char *who);
static unsigned long setup_memory(void);


struct resource standard_io_resources[] = {
	{ "dma1", 0x00, 0x1f, IORESOURCE_BUSY },
	{ "pic1", 0x20, 0x3f, IORESOURCE_BUSY },
	{ "timer", 0x40, 0x5f, IORESOURCE_BUSY },
	{ "keyboard", 0x60, 0x6f, IORESOURCE_BUSY },
	{ "dma page reg", 0x80, 0x8f, IORESOURCE_BUSY },
	{ "pic2", 0xa0, 0xbf, IORESOURCE_BUSY },
	{ "dma2", 0xc0, 0xdf, IORESOURCE_BUSY },
	{ "fpu", 0xf0, 0xff, IORESOURCE_BUSY }
};

#define STANDARD_IO_RESOURCES (sizeof(standard_io_resources)/sizeof(struct resource))

static struct resource code_resource = { "Kernel code", 0x100000, 0 };
static struct resource data_resource = { "Kernel data", 0, 0 };
static struct resource vram_resource = { "Video RAM area", 0xa0000, 0xbffff, IORESOURCE_BUSY };

// 系统的 ROM 资源
#define MAXROMS 6
static struct resource rom_resources[MAXROMS] = {
	{ "System ROM", 0xF0000, 0xFFFFF, IORESOURCE_BUSY },   // 系统 BIOS的 ROM 区域
	{ "Video ROM", 0xc0000, 0xc7fff, IORESOURCE_BUSY }
};

// ROM 区域的标志
#define romsignature(x) (*(unsigned short *)(x) == 0xaa55)

// 检测 rom 
static void probe_roms(void)
{
    int roms = 1;
    unsigned long base;
    unsigned char *romstart;

    request_resource(&iomem_resource, rom_resources+0);
    // video ROM 的标准区域是处于 C000:0000 - C7FF:0000，通过检测签名来判断
    for(base = 0xC0000; base < 0xE0000; base += 2048) {
        romstart = bus_to_virt(base);
        if(!romsignature(romstart))
            continue;
        request_resource(&iomem_resource, rom_resources + roms);
        roms++;
        break;
    }
    /* Extension roms at C800:0000 - DFFF:0000 */
	for (base = 0xC8000; base < 0xE0000; base += 2048) {
		unsigned long length;

		romstart = bus_to_virt(base);
		if (!romsignature(romstart))
			continue;
		length = romstart[2] * 512;
		if (length) {
			unsigned int i;
			unsigned char chksum;

			chksum = 0;
			for (i = 0; i < length; i++)
				chksum += romstart[i];

			/* Good checksum? */
			if (!chksum) {
				rom_resources[roms].start = base;
				rom_resources[roms].end = base + length - 1;
				rom_resources[roms].name = "Extension ROM";
				rom_resources[roms].flags = IORESOURCE_BUSY;

				request_resource(&iomem_resource, rom_resources + roms);
				roms++;
				if (roms >= MAXROMS)
					return;
			}
		}
	}

	/* Final check for motherboard extension rom at E000:0000 */
	base = 0xE0000;
	romstart = bus_to_virt(base);

	if (romsignature(romstart)) {
		rom_resources[roms].start = base;
		rom_resources[roms].end = base + 65535;
		rom_resources[roms].name = "Extension ROM";
		rom_resources[roms].flags = IORESOURCE_BUSY;

		request_resource(&iomem_resource, rom_resources + roms);
	}
}

// 给所有的标准 RAM 和 ROM 资源请求地址空间，包含那些e820图中标记为 reserved 的区域
static void register_memory(unsigned long max_low_pfn)
{
	unsigned long low_mem_size;
	int i;
	probe_roms();     // 检测 ROM 资源
	for (i = 0; i < e820.nr_map; i++) {
		struct resource *res;
		if (e820.map[i].addr + e820.map[i].size > 0x100000000ULL)
			continue;
		res = alloc_bootmem_low(sizeof(struct resource));   // 分配内存
		switch (e820.map[i].type) {
		case E820_RAM:	res->name = "System RAM"; break;
		case E820_ACPI:	res->name = "ACPI Tables"; break;
		case E820_NVS:	res->name = "ACPI Non-volatile Storage"; break;
		default:	res->name = "reserved";
		}
		res->start = e820.map[i].addr;
		res->end = res->start + e820.map[i].size - 1;
		res->flags = IORESOURCE_MEM | IORESOURCE_BUSY;
		request_resource(&iomem_resource, res);          // io 内存映射区域
		if (e820.map[i].type == E820_RAM) {
			/*
			 *  We dont't know which RAM region contains kernel data,
			 *  so we try it repeatedly and let the resource manager
			 *  test it.
			 */
			request_resource(res, &code_resource);
			request_resource(res, &data_resource);
		}
	}
	request_resource(&iomem_resource, &vram_resource);

	/* request I/O space for devices used on all i[345]86 PCs */
	for (i = 0; i < STANDARD_IO_RESOURCES; i++)
		request_resource(&ioport_resource, standard_io_resources+i);

	/* 告知 PCI 层不要把内存分配的太靠近 RAM 区域  */
	low_mem_size = ((max_low_pfn << PAGE_SHIFT) + 0xfffff) & ~0xfffff;
	if (low_mem_size > pci_mem_start)
		pci_mem_start = low_mem_size;
}


void setup_arch()
{   
    // max_low_pfn 以低端内存区域表示的最大PFN
    // max_pfn 系统中可用的最大PFN
    // start_pfn 表示内存中内核映像以上第一个可以动态分配的页面
    // unsigned long start_pfn, max_pfn, max_low_pfn;
    unsigned long max_low_pfn;
    // unsigned long bootmp_size;
    int i;

    // printk("kernel in memory start: 0x%08x\n", &_start);
    // printk("kernel in memory end: 0x%08x\n", &_end - 0xc0000000);
    // printk("kernel in memory used: %d KB\n\n", (&_end - 0xc0000000 - &_start + 1023)/1024);
    printk("kernel in memory start: 0x%08x\nkernel in memory end: 0x%08x\nkernel in memory used: %d KB\n\n ",&_start, &_end - 0xc0000000, (&_end - 0xc0000000 - &_start + 1023)/1024);

    show_memory_map();

    setup_memory_region();      // 将 e820图 的内容存放到安全的地址，构建内存映射图e820

    // 建立内存页面所需的数据结构
    max_low_pfn = setup_memory(); 
    printk("max_low_pfn: %x\n", max_low_pfn);
    
    // 建立内存页面管理机制
    paging_init();
    
    register_memory(max_low_pfn);
    printk("setup_arch done\n");
    while(1);
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
static void setup_memory_region(void)
{
    char *who = "BIOS-e820";

    // 用global_multiboot_info中的内存信息直接初始化e820图
    init_biosmap();
    printk("e820.map addr = 0x%x, e820.nr_map = %d\n", bios_tmp.map, bios_tmp.nr_map);
    // 消除e820当中的区域重叠
    sanitize_e820_map(bios_tmp.map, &bios_tmp.nr_map);
    // 将 E820_MAP 复制到一个安全的地方
    if(copy_e820_map(bios_tmp.map, bios_tmp.nr_map)<0){       // 省略了一些对于错误情况的处理打印内存区域的分布情况
        e820.nr_map = 0;
        // add_memory_region(0, LOWMEMSIZE(), E820_RAM);
        // add_memory_region(HIGH_MEMORY, mem_size << 10, E820_RAM);
    }

    printk("BIOS-provided pysical RAM map:\n");
    print_memory_map(who);
    printk("setup_memory_region done!\n");
}

// 确定可用物理页面的界限，构建引导内存分配器
static unsigned long setup_memory(void)
{   
    // bootmap_size 
    unsigned long bootmap_size, start_pfn;

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
static void find_max_pfn(void)
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
static unsigned long find_max_low_pfn(void)
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
static void register_bootmem_low_pages(unsigned long max_low_pfn)
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
static void init_biosmap()
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
static int sanitize_e820_map(struct e820entry * biosmap, int * pnr_map){
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

static int copy_e820_map(struct e820entry * biosmap, int nr_map)
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
static void add_memory_region(unsigned long long start, unsigned long long size, int type)
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

static void print_memory_map(char *who)
{
    int i;

    for(i = 0; i < e820.nr_map; i++){
        printk("i = %d\n", i);
        printk(" %s: %08x - %08x ", who, (uint32_t)e820.map[i].addr, (uint32_t)e820.map[i].addr + (uint32_t)e820.map[i].size);
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


