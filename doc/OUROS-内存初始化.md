# 系统结构设置

`init/main/start_arch()`调用`/arch/i386/kernel/setup_arch()`方法进行系统结构相关的设置，进入到第一个部分是`setup_memory()`，它会确定可用物理页面的界限，后续就进行引导内存的初始化。

![image-20220523195810993](https://img-bed-l.oss-cn-beijing.aliyuncs.com/pic_bed/202205231958052.png)

```C
void __init setup_arch()
{   
    // max_low_pfn 以低端内存区域表示的最大PFN
    // max_pfn 系统中可用的最大PFN
    // start_pfn 表示内存中内核映像以上第一个可以动态分配的页面
    unsigned long start_pfn, max_pfn, max_low_pfn;

    unsigned long bootmp_size;

    setup_memory_region();      // 将 e820图 的内容存放到安全的地址

    // 建立内存页面所需的数据结构
    max_low_pfn = setup_memory();  

    // 建立内存页面管理机制
    paging_init();

    register_memory(max_low_pfn);
}
```

在启动过程的末期，`start_kernel()`函数知道此时可以安全地移除启动分配器和与之相关的所有数据结构。每种体系结构要求提高`mem_init()` 函数，该函数复杂消除启动内存分配器和与之相关的结构。

## 查看内存布局

在最初始的情况下，通过BIOS提供的接口在计算机上电启动时检测并计算内存的大小，在`arch/i386/boot/setup.S`中调用BIOS的`0x15`中断的子功能设置e820图，分为以下三个步骤

- 尝试 BIOS Int 0x15, 设置eax为 `0xE820`
- 无效则尝试 BIOS Int 0x15, ax = 0xE801 and BIOS Int 0x12
- If that didn't work, try BIOS Int 0x15, ah = 0x88 and BIOS Int 0x12

我们使用Grub进行启动和系统的加载，Grub通过multiboot协议，其中包含对于内存布局的描述，如下：

```C
// multiboot协议的头部
struct multiboot_info{
    ...
/* Memory Mapping buffer
   * 以下两项指出保存由BIOS提供的内存分布的缓冲区的地址和长度
   * mmap_addr是缓冲区的地址，mmap_length是缓冲区的总大小
   * 缓冲区由一个或者多个下面的大小/结构对 mmap_entry_t 组成
  */
  multiboot_uint32_t mmap_length;   // mmap_length是缓冲区的总大小
  multiboot_uint32_t mmap_addr;     // mmap_addr是缓冲区的地址
    ...
} __attribute__((packed));
typedef struct multiboot_info multiboot_info_t;

// 对于内存布局的描述
struct multiboot_mmap_entry
{
     multiboot_uint32_t size;        // 大小
       multiboot_uint32_t base_addr_low;
     multiboot_uint32_t base_addr_high;
      multiboot_uint32_t length_low;
      multiboot_uint32_t length_high;
    // 定义几种内存的状态或类型
#define MULTIBOOT_MEMORY_AVAILABLE              1
#define MULTIBOOT_MEMORY_RESERVED               2
#define MULTIBOOT_MEMORY_ACPI_RECLAIMABLE       3
#define MULTIBOOT_MEMORY_NVS                    4
#define MULTIBOOT_MEMORY_BADRAM                 5
     multiboot_uint32_t type;        // 内存类型
} __attribute__((packed));

typedef struct multiboot_mmap_entry multiboot_memory_map_t;

 // 进行遍历
int main(multiboot_info* mbt, unsigned int magic) {
     ...
    mmap_entry_t* entry = mbt->mmap_addr;
    while(entry < mbt->mmap_addr + mbt->mmap_length) {        // 保证没有越界
        // do something with the entry
        entry = (mmap_entry_t*) ((unsigned int) entry + entry->size + sizeof(entry->size));
    }
    ...
}
```

> 在GNU给出的示例当中将addr和len定义为`uint64_t`，但是最好不要将数据定义为64位，因为这样可能导致gcc在进行结构体的pack时出现错误。参考来源：https://forum.osdev.org/viewtopic.php?t=30318

内存结构的定义如下：

| 偏移量 | 字段             | 含义          |
| --- | -------------- | ----------- |
| 0   | size           | 结构的大小       |
| 4   | base_addr_low  | 启动地址的低32位   |
| 8   | base_addr_high | 启动地址的高32位   |
| 12  | length_low     | 内存区域大小的低32位 |
| 16  | length_high    | 内存区域大小的高32位 |
| 20  | type           | 地址区间的类型     |

* `size` 表示的是 以字节为单位的，相关内存区域标识结构的大小，它可以大于20字节。
  * 用`size+sizeof(mmap->size)`指向下一个内存映射的实例，因为`mmap->size`不会把自己考虑进去，`base_addr_low` 在该struct当中的偏移值为 `0`。
* `type` 字段中，设置值为`1`表示 `usable RAM`，其他值为`unusable`，代表保留区域。
* 本质上，Grub也是简单使用的 `INT=0x15`，`EAX=E820`来获取内存映射，同时不会验证映射的正确性，不会进行排序。（**没有经过验证，不确定新版Grub是怎样的**）

> Grub中获取内存布局信息：https://wiki.osdev.org/Detecting_Memory_(x86)#Memory_Map_Via_GRUB
> 
> Multiboot手册的3.3节 Boot information format ： https://www.gnu.org/software/grub/manual/multiboot/multiboot.html

Grub进行加载和内存探测的大致流程如下，不做细叙：

<img src="https://img-bed-l.oss-cn-beijing.aliyuncs.com/pic_bed/493248c03b158dfc612ddfc3ecd2f934.png" alt="img" style="zoom:50%;" />

> 来源：https://xie.infoq.cn/article/386bc5366bac88552085fd4ee

获取了内存区域的信息之后，将其打印出来，便于查看：

```C
void __init setup_arch()
{
    printk("kernel in memory start: 0x%08X\n", &_start);
    printk("kernel in memory end: 0x%08X\n", &_end - 0xc0000000);
    printk("kernel in memory used: %d KB\n\n", (&_end - 0xc0000000 - &_start + 1023)/1024);

    show_memory_map();
}

void show_memory_map()
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
```

运行后展示信息如下：

![image-20220717164109513](https://img-bed-l.oss-cn-beijing.aliyuncs.com/pic_bed/image-20220717164109513.png)

## setup_memory_region()

由于在Linux内核中不能作BIOS调用，所以由setup在引导阶段代为查询内存，并根据获得的信息生成一幅物理内存构成图，称为**e820图**，再通过参数块传给内核，使内核能知道系统中内存资源的配置。

>  之所以称为e820图，是因为在通过”int0x15”查询内存的构成时要把调用参数之一设置成`0xe820`。

全局的物理内存构成图定义在`arch/i386/kernel/setup.c`当中：

```C
// 全局内存映射图
struct e820map e820;
struct e820map bios_tmp;     // 暂时用于保存原始的内存映射信息
```

在`include/asm-i386/e820.h`中，e820map的定义为：

```C
#define E820MAP 0x2d0       // 存放的地址偏移量
#define E820MAX 32          // 在E820MAP当中有几个表项
#define E820NR  0x1e8       // E820MAP当中的entries


#define HIGH_MEMORY (1024 * 1024)   // 高于 1MB 的内存称之为 High Memory
struct e820map{
    int nr_map;         // 数量
    struct e820entry{       // 描述一个物理内存区间
        unsigned long long addr;        // 内存段的开始
        unsigned long long size;        // 内存段的大小
        unsigned long type;             // 内存段的类型
    }map[E820MAX];
};
```

首先调用`setup_memory_region()`获取e820map即**内存映射图**，并将其存储到全局变量e820当中：

```C
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
```

使用的是Grub进行操作系统的引导，所以直接将 `global_multiboot_info`中的内存信息传递到e820图中：

```C
// 用grub提供的内存描述信息初始化e820图
static void __init init_biosmap()
{
    // 保存内存信息结构的缓冲区地址
    multiboot_uint32_t mmap_addr = ((multiboot_info_t*)global_multiboot_info)->mmap_addr;
    multiboot_uint32_t mmap_length = ((multiboot_info_t*)global_multiboot_info)->mmap_length;
    int i = 0;
    // 保存到暂时的e820图--- biosmap 当中
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
```

`sanitize_e820_map()`负责消除内存中可能出现的重叠现象，注释当中展示了一种简单的内存overlap的情况：

```
/*
        Visually we're performing the following (1,2,3,4 = memory types)...

        Sample memory map (w/overlaps):
           ____22__________________
           ______________________4_
           ____1111________________
           _44_____________________
           11111111________________
           ____________________33__
           ___________44___________
           __________33333_________
           ______________22________
           ___________________2222_
           _________111111111______
           _____________________11_
           _________________4______

        Sanitized equivalent (no overlap):
           1_______________________
           _44_____________________
           ___1____________________
           ____22__________________
           ______11________________
           _________1______________
           __________3_____________
           ___________44___________
           _____________33_________
           _______________2________
           ________________1_______
           _________________4______
           ___________________2____
           ____________________33__
           ______________________4_
    */
```

删除旧有重叠部分，按照内存顺序生成一个正确的内存映射图：

```C
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
```

`copy_e820_map()`负责将启动时的内存图复制到全局变量`e820`当中，同时处理 640k-1M中可能出现BIOS内存问题：

```C
static int __init copy_e820_map(struct e820entry * biosmap, int nr_map)
{
    if(nr_map < 2){     // 仅一个内存区域
        return -1;
    }

    do{
        unsigned long long start = biosmap->addr;
        unsigned long long size = biosmap->size;
        unsigned long long end = start + size;

        // 溢出
        if(start > end){   
            return -1;
        }

        // 一些BIOS位于 640K - 1M 的区域，修复它
        if(type == E820_RAM){
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
        // 帮助函数，在e820当中创建一个新的条目
        add_memory_region(start, size, type);
    }while(biosmap++, --nr_map);
    return 0;
}
```

辅助函数`add_memory_region()`负责在e820图中添加一个内存区域：

```C
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
```

最后，调用`print_memory_map()`来打印出处理后的内存区域情况：

```C
static void __init print_memory_map(char *who)
{
    int i;
    // 遍历e820中的图，展示类型和对应的地址
    for(i = 0; i < e820.nr_map; i++){
        printk(" %s: %016Lx - %016Lx ", who, e820.map[i].addr, e820.map[i].addr + e820.map[i].size);
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
```

内存结果如截图所示：

![image-20220717163710236](https://img-bed-l.oss-cn-beijing.aliyuncs.com/pic_bed/image-20220717163710236.png)

## setup_memory()

`setup_memory`为引导内存分配器初始化自身 获取 所需的信息，主要的功能如下：

* 找到**低端内存**的PFN的起点和终点(`min_ low_ pfn` , `max_low_pfn`)，找到**高端内存**的
  PFN的起点和终点(`highstart_pfn`, `highend_pfn`), 以及找到系统中最后一页的PFN。（省略了高端内存的处理）
* 初始化`bootmem_data`结构以及声明可能被引导内存分配器用到的页面。
* 标记所有系统可用的页面为空闲，然后为那些表示页面的位图保留页面。
* 在SMP配置或initrd镜像存在时，为它们保留页面。（暂时不支持SMP）

代码如下：

```C
static unsigned long __init setup_memory(void)
{   
    unsigned long bootmap_size, start_pfn, max_low_pfn;
    // 获取可用的第一个物理页面帧的偏移量
    start_pfn = PFN_UP(__pa(&_end)); // _end是载入内核镜像的最终地址，在链接脚本中提供

    find_max_pfn();         // 遍历e820图，查找系统中最大的可用PFN

    max_low_pfn = find_max_low_pfn();       // 在ZONE_NORMAL中找到可寻址的最高页面帧

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
```

在操作内存地址时，定义一些基本的宏方法如下所示，具体的基本宏参数定义在`include/asm-i386/page.h`：

```C
#define PFN_UP(x)   (((x) + PAGE_SIZE-1) >> PAGE_SHIFT)  // 下一个页面的页框
#define PFN_DOWN(x) ((x) >> PAGE_SHIFT)             // 该页面边界
#define PFN_PHYS(x) ((x) << PAGE_SHIFT)             // 页框的物理地址

// 获取物理地址
#define __pa(x) ((unsigned long)(x) - PAGE_OFFSET)
// 获取虚拟地址
#define __va(x) ((void *)((unsigned long)(x) + PAGE_OFFSET))
```

`find_max_pfn()` 遍历e820图，查找系统中最大的可用PFN（物理内存页面序列号），代码如下：

```C
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
```

`find_max_low_pfn`寻找低端内存的最大pfn, 划分低端内存和高端内存区域：

```C
/**
 * @brief 寻找低端内存的最大pfn, 划分低端内存和高端内存区域
 * @return unsigned long 低端内存的最大pfn
 */
static unsigned long __init find_max_low_pfn(void)
{
    unsigned long max_low_pfn;

    max_low_pfn = max_pfn;
    if(max_low_pfn > MAXMEM_PFN){       // MAXMEM_PFN 最大的低端内存页框号
        if(highmem_pages == -1)         // 还没有设置，进行初始化
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
```

其中`MAXMEM_PFN`是怎样获得的？

```C
// 地址空间的保留区域 --- vmalloc 和 iomap
#define MAXMEM_PFN          PFN_DOWN(MAXMEM)
#define MAX_NONPAE_PFN      (1 << 20)

// 定义在page.h当中
// 128MB 保留给vmalloc和initrd的区域
// vmalloc在原有的线性映射之外额外分配空间，同时建立虚拟地址和物理页面的映射，大小限于128MB
#define VMALLOC_RESERVE     (unsigned long)(128 << 20)
// 内核可以直接访问的，即最大的RAM空间容量，直接取反: 1GB-128MB = 896MB (0x37FFFFFF)
#define MAXMEM              (unsigned long) (-PAGE_OFFSET-VMALLOC_RESERVE)
```

从内核角度为主来看虚拟地址空间如图所示，其中`vmalloc()`和`kmap()`以及固定映射区域所需的区域大小限制了`ZONE_NORMAL` 的大小，因此内核需要这些部分，所以在地址空间的顶部，必须保留    `VMALLOC_RESERVE`大小的区域，**在x86中其大小为 128MB**，因此`ZONE_NORMAL`通常只有 **896MB**，即 `MAXMEM`值。

![image-20220517111625477](https://img-bed-l.oss-cn-beijing.aliyuncs.com/pic_bed/%E5%86%85%E6%A0%B8%E5%9C%B0%E5%9D%80%E7%A9%BA%E9%97%B4.png)

---

在确定了可用物理页面的界限之后，调用`init_bootmem`准备建立物理内存页面管理机制，建立一个页面位图。

```C
// setup_memory里的调用：
bootmap_size = init_bootmem(start_pfn, max_low_pfn);

// pages是该节点可寻址内存的PFN末端，而不是页面数
unsigned long __init init_bootmem(unsigned long start, unsigned long pages)
{
    max_low_pfn = pages;
    min_low_pfn = start;
    // 对contig_page_data进行初始化
    // UMA的内存是均匀的、连续的，只有一个pg_data_t类型的contig_page_data来表示一个节点
    // 而NUMA当中则有多个这样的数据结构，各个节点的pg_data_t通过链表链接在一起，第一个节点就是contig_page_data,如果还有其他节点，则在后续过程中添加到链表中即可。
    return (init_bootmem_core(&contig_page_data, start, 0, pages));
}
```

在这里，对于UMA和NUMA存在一些不同，UMA调用`init_bootmem()`直接初始化 `contig_page_data`，而NUMA则调用`init_bootmem_node()`直接初始化一个具体的节点，但是都是调用`init_bootmem_core()`来完成核心工作。

> **均匀访存模型**（英语：Uniform Memory Access）通常简称**UMA**，亦称作**统一寻址技术**或**统一内存存取架构**，指所有的物理存储器被均匀共享，即处理器访问它们的时间是一样的。
> 
> **非统一内存访问架构**（英语：**Non-uniform memory access**，简称**NUMA**）是一种为[多处理器](https://zh.wikipedia.org/wiki/多處理器)的电脑设计的内存架构，内存访问时间取决于内存相对于处理器的位置。在NUMA下，处理器访问它自己的本地内存的速度比非本地内存（内存位于另一个处理器，或者是处理器之间共享的内存）快一些。

在UMA当中，内存时均匀连续的，因此将整个节点设置为`contig_page_data`，内核将内存分为节点`pg_data_t`（以链表形式进行管理）–> 管理区`zone` –> 页面`page`等多个级别的层次管理。

<img src="https://img-bed-l.oss-cn-beijing.aliyuncs.com/pic_bed/%E8%8A%82%E7%82%B9%E3%80%81%E7%AE%A1%E7%90%86%E5%8C%BA%E5%92%8C%E9%A1%B5%E9%9D%A2%E7%9A%84%E5%85%B3%E7%B3%BB.png" alt="image-20220505143648530" style="zoom:50%;" />

```C
/** 
 * 表示内存的节点
*/
typedef struct pglist_data {
    zone_t node_zones[MAX_NR_ZONES];          // 节点所在的管理区
    zonelist_t node_zonelists[GFP_ZONEMASK+1];     // 按照分配时的管理区顺序排列
    int nr_zones;                               // 节点中管理区的数目
    struct page *node_mem_map;                   // 节点中的页面，一般指向第一个页面
    unsigned long *valid_addr_bitmap;           // 内存节点中“空洞”的位图
    struct bootmem_data *bdata;             // 内存引导程序
    unsigned long bode_start_paddr;         // 节点的起始物理地址
    unsigned long node_start_mapnr;         // 节点在全局mem_map中的页面偏移量
    unsigned long node_size;                // 页面的总数
    int node_id;                            // 节点的id号
    struct pglist_data *node_next;          // 指向下一个节点，NULL为结束
} pg_data_t;

// 连续均匀的节点
extern pg_data_t contig_page_data;
```

`init_bootmem_core()`函数具体的内容：

* 位图大小以字节为单位来计算：`mapsize = ((end - start)+7)/8` ，设置节点的引导内存
* 引导内存的描述结构：`bootmem_data_t`，系统内存中的每一个节点都有一个对应的`bootmem_data`
* 将位图全部填充为`1`，标记所有页面都已分配。后续修改为空闲。

```c
// 系统内存中的每一个节点都有一个对应的 bootmem_data
typedef struct bootmem_data{
    unsigned long node_boot_start;      // 系统引导后的第一个物理内存页面
    unsigned long node_low_pfn;         // 块的结束地址,或者该节点表示的ZONE_NORMAL的结束，即物理内存的顶点，不超过896MB
    void * node_bootmem_map;       // 指向一个保留页面位图，每一位代表着保留或不存在的，不能分配的物理页面
    unsigned long last_offset;        // 最后一次分配时的页面的偏移，为0，则表示全部使用
    unsigned long last_pos;             // 最后一次分配时的页面帧数
} bootmem_data_t;


/**
 * @brief 初始化引导内存分配器
 * 
 * @param pgdat 要初始化的节点描述符
 * @param mapstart 内核映像以上第一个物理页面的起点，即可用内存的节点
 * @param start 物理内存节点的起始pfn
 * @param end 物理内存节点的末尾pfn
 * @return unsigned long 返回大小
 */
static unsigned long __init init_bootmem_core(pg_data_t *pgdat, unsigned long mapstart, unsigned long start, unsigned long end)
{
    bootmem_data_t *bdata = pgdat->bdata;           
    unsigned long mapsize = ((end - start)+7)/8;   // 向上取整，每一位都需要设置，以字节为单位计算位图的大小

    // 将当前节点插入到内存节点链表的开头，方便调用
    pgdat->node_next = pgdat_list;
    pgdat_list = pgdat;

    // 设置 引导内存数据结构
    mapsize = (mapsize + (sizeof(long) - 1UL)) & ~(sizeof(long) - 1UL);  // ?? 这是怎么计算的？
    bdata->node_bootmem_map = phys_to_virt(mapstart << PAGE_SHIFT); 
    bdata->node_boot_start = (start << PAGE_SHIFT);     
    bdata->node_low_pfn = end;
    // 用 1 填充整个图，标记所有的页面都已经分配
    memset(bdata->node_bootmem_map, 0xff, mapsize);  // 初始化位图，1 为空闲

    return mapsize;
}
```

`register_bootmem_low_pages()`检测e820内存映射图，并在每一个可用页面上调用`free_bootmem()`函数，将页面标记为 `0` 空闲，然后再调用`reserve_bootmem()`为保存实际位图所需的页面预留空间。

```c
/**
 * @brief 从e820图中获取到可用的RAM，将其对应的位图设置为1
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
        // 可用RAM的末端PFN
        last_pfn = PFN_DOWN(e820.map[i].addr + e820.map[i].size);

        if(last_pfn > max_low_pfn){     // 超出了低端内存区域的界限
            last_pfn = max_low_pfn;
        }

        if(last_pfn <= curr_pfn){
            continue;
        }

        size = last_pfn - curr_pfn; 
        // 操作contig_page_data结构中的位图，设置为 1
        free_bootmem(PFN_PHYS(curr_pfn), PFN_PHYS(size));
    }
}
```

其中调用`free_bootmem`清理位图中的位，对一个`contig_ page_ data` 结构中的位图进行操作，将其中的某些位清`0`，表示相应的物理内存页面可以投入分配。

```c
void __init free_bootmem (unsigned long addr, unsigned long size)
{
    return(free_bootmem_core(contig_page_data.bdata, addr, size));
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


    unsigned long sidx;        // 开始
    unsigned long eidx = (addr + size - bdata->node_boot_start)/PAGE_SIZE; // 末端
    unsigned long end = (addr + size)/PAGE_SIZE;

    if(!size) BUG();        // 大小为0，则提示出错
    if(end > bdata->node_low_pfn)
        BUG();

    // 将位图当中对应的页面位清除
    start = (addr + PAGE_SIZE-1) / PAGE_SIZE;
    sidx = start - (bdata->node_boot_start/PAGE_SIZE);
    // 释放全部的满页面，清理启动位图中的位，如果已经为0，则表示重复释放或没有使用过
    for (i = sidx; i < eidx; i++){
        // 原始值为1，修改为0，如果原始值为0，则返回的是0，报错
        if(!test_and_clear_bit(i, bdata->node_bootmem_map)) // 已经释放，则提示错误
            BUG();
    }
}
```

对位图的操作使用的是原子化操作：

```C
/**
 * @brief 清除一个bit位，并返回其旧值，原子化操作
 * 
 * @param nr 要清除的bit
 * @param addr 开始计算的地址  
 * @return int
 */
static __inline__ int test_and_clear_bit(int nr, volatile void * addr)
{
    int oldbit;

    __asm__ __volatile__(LOCK_PRTEFIX 
        "btrl %2, %1\n\tsbbl %0,%0"
        :"=r" (oldbit), "=m" (ADDR)
        :"Ir" (nr) : "memory");

    return oldbit;
}
```

关键指令：

**`BTR - Bit Test and Reset`**

![image-20220524195803551](https://img-bed-l.oss-cn-beijing.aliyuncs.com/pic_bed/BTR.png)

将选中的bit清除为 0，可以操作 register或者memory。

```textile
CF<-Bit(BitBase, BitOffset)
Bit(BitBase, BitOffset) <- 0
```

**`SBB - Integer Subtraction with Borrow`**

借位减法，从 目的操作数中减去 源操作数 和 CF标志位之和。

`DEST <- (DEST - (SRC + CF))`

**该指令不能区分 有符号或者无符号数。**

等价于：

```textile
DEST ← (DEST – (SRC + CF));
```

根据结果，OF SF(表示有符号结果的符号) ZF AF PF 和 CF都会被置位。

---

`reserve_bootmem()`负责为位图分配内存，而与`free_bootmem()`正好相反，一开始时把位图中的所有位都设置成`1`，**假定全部都不能用于动态分配**，然后根据e820数据结构中的内容以及一些特殊的区间和页面加以找补。其中特别加以保留的有:

* 从`HIGH_ MEMORY`，值为`1024*1024`，即1MB边界开始，直到`(start pfn + bootmap_ size)`所在的页面为止。这些是**内核映象和“保留页面位图”本身**所在的页面。
* **页面0**，即起始地址为0的页面。BIOS 通常用这个页面保存一些与引导以及 BIOS本身有关的信息，所以也要加以保留。
* 对于SMP结构的系统，页面1，即起始地址为PAGE SIZE的页面也要保留，次CPU转入运
  行时需要用这个页面作为“跳板"。
* 此外，还有一些特殊用途的页面，例如用作RAMDISK的页面。不过这些页面不是在这儿保留的。

对`reserver_bootmem()`的调用分为两个部分，表示上述两个需要保留的：

```C
 // 分配位图本身的内存
  reserve_bootmem(HIGH_MEMORY, (PFN_PHYS(start_pfn) + bootmap_size + PAGE_SIZE-1) - (HIGH_MEMORY));
  // 分配 物理页 0,用于很多BIOS功能的页面
  reserve_bootmem(0, PAGE_SIZE);
```

具体的实现代码如下：

```c
// 开始时将位图中的所有位都设置为 1， 设置位全部不能用于分配，其后根据e820图来修改
void __init reserve_bootmem(unsigned long addr, unsigned long size)
{
    reserve_bootmem_core(contig_page_data.bdata, addr, size);
}
// 
static void __init reserve_bootmem_core(bootmem_data_t *bdata, unsigned long addr, unsigned long size)
{
    unsigned long i;

    // 在位图中设置
    unsigned long sidx = (addr - bdata->node_boot_start)/PAGE_SIZE; // 页面起始索引
    unsigned long eidx = (addr + size - bdata->node_boot_start + 
                            PAGE_SIZE-1)/PAGE_SIZE;         // 页面结尾索引，向上取整
    unsigned long end = (addr + size + PAGE_SIZE-1)/PAGE_SIZE;  // 受本次保留影响的最后的PFN

    if (!size) BUG();

    if (sidx < 0)       // 起始索引不在节点起点之前
        BUG();
    if (eidx < 0)        // 末尾索引不在节点末端之后
        BUG();
    if (sidx >= eidx)
        BUG();
    if ((addr >> PAGE_SHIFT) >= bdata->node_low_pfn)  // 检测起始地址没有超出
        BUG();
    if (end > bdata->node_low_pfn)
        BUG();
    // 测试和设置启动内存分配图中表示页面依据分配的位，已设置则打印提醒消息
    for(i = sidx; i < eidx; i++){   
        if(test_and_set_bit(i, bdata->node_bootmem_map))
            printk("hm, page %08lx reserved twice.\n", i*PAGE_SIZE);
    }
}
```

`HIGH_MEMORY`的定义为：

```C
#define HIGH_MEMORY (1024 * 1024)   // 高于 1MB 的内存称之为 High Memory
```



输出结果：全部内存设置为 1GB，检测得到 896MB 的 `LOWMEM` 内存。

![image-20220829191742018](https://img-bed-l.oss-cn-beijing.aliyuncs.com/pic_bed/image-20220829191742018.png)



## paging_init

Linux 将计算机分为**独立层和依赖层**，采用与体系结构无关代码的三层页表机制来管理内存。

在不支持PAE的x86机器上，两层页表就足以满足使用，中间页目录PMD被定义为1，在初始化时直接映射为全局页目录PGD。三层页表机制如图所示：

<img src="https://img-bed-l.oss-cn-beijing.aliyuncs.com/pic_bed/202205282219276.png" alt="image-20220528221903216" style="zoom:50%;" />

为了方便地将线性地址进行划分，系统提供三类宏来实现该功能：`SHIFT` `SIZE` `MASK`。

* `SHIFT` 指定页面每层映射的长度

<img src="https://img-bed-l.oss-cn-beijing.aliyuncs.com/pic_bed/202205282219147.png" alt="image-20220528221930107" style="zoom:50%;" />

* `MASK` 用于从线性地址当中找到属于各个页表层次的位
* `SIZE` 表示某一个页表层次的每一项所代表的空间大小

<img src="https://img-bed-l.oss-cn-beijing.aliyuncs.com/pic_bed/202205282219600.png" alt="image-20220528221945572" style="zoom:50%;" />

`setup_memory()`建立起内存管理所需的数据结构之后，`paging_init()`进一步完善内存页面映射机制。

```C
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
```

### pagetable_init

`pagetable_init()`进行页表的建立，初始化页表，扩充第一阶段中创建的**页面映射目录PGD**和**页面映射表PTE**。

```c
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
    // 当系统总内存为512MB时，只需要124个pgd，对应496MB可用NORMAL内存
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
                // 构造页表项
                *pte = mk_pte_phys(__pa(vaddr), PAGE_KERNEL);
            }
            set_pmd(pmd, __pmd(_KERNPG_TABLE + __pa(pte_base)));
            if(pte_base != pte_offset(pmd, 0))
                BUG();
        }
    }
}
```

#### alloc_bootmem_low_pages

其中，调用`alloc_bootmem_low_pages()`进行内存的分配，而最终调用的是`__alloc_bootmem_core()`方法，该方法是从带有bootmem allocator的节点中进行内存分配的核心函数。

```c
/**
 * @brief 分配内存
 * 
 * @param bdata 节点中要分配结构体的启动内存，在UMA结构中默认为 contig_page_data
 * @param size 要分配空间的大小
 * @param align 要求对齐的字节数，2的幂级数
 * @param goal 最佳的分配的起始地址，一般从物理地址0开始
 * @return void* 
 */
static void * __init __alloc_bootmem_core(bootmem_data_t *bdata, unsigned long size, unsigned long align, unsigned long goal)
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

    } else {
        bdata->last_pos = start + areasize - 1;
        bdata->last_offset = size & ~PAGE_MASK;
        ret = phys_to_virt(start * PAGE_SIZE + bdata->node_boot_start);
    }

    // 正式分配内存，将位图中分配页设置为1
    for(i = start; i < start+areasize; i++)
        if(test_and_set_bit(i, bdata->node_bootmem_map)) // 避免重复分配
            BUG();
    memset(ret, 0, size);  // 内存清零
    return ret;
}
```

---

在设置了ZONE_NORMAL部分的内存后，利用`__fix_to_virt()`获取固定虚拟地址空间起始位置的PMD，其次调用`fixrange_init()`来为固定虚拟地址空间创建有效的PGD和PMD。

```c
 vaddr = __fix_to_virt(__end_of_fixed_addresses - 1) & PMD_MASK;
 fixrange_init(vaddr, 0, pgd_base);      // 返回
```

可以看到，虚拟地址空间图如下所示：

![image-20220517111625477](https://img-bed-l.oss-cn-beijing.aliyuncs.com/pic_bed/%E5%86%85%E6%A0%B8%E5%9C%B0%E5%9D%80%E7%A9%BA%E9%97%B4.png)

用户地址空间和内核地址空间以 `PAGE_OFFSET` 为分界点，而固定虚拟地址空间从 `FIXADDR_TOP` 开始，x86下定义为`0xffffe000UL`，即虚拟地址结束的前一个页面。在编译时通过减去`__FIXADDR_SIZE` 变量得到FIXADDR_START起始位置。

> 该区域给 编译时需要直到虚拟地址的子系统使用，例如：高级可编程的中断控制器APIC

在`fixmap.h`当中定义了vmalloc和fixaddr部分的位置，`__end_of_fixed_addresses`即表示末尾。

```c
// 在 vmalloc 和fixedmap起始 之间留下一个 空 的 page
#define FIXADDR_TOP (0xffffe000UL)
#define __fix_to_virt(x) (FIXADDR_TOP - ((x) << PAGE_SHIFT))

// 固定区域的位置
enum fixed_addresses{
    __end_of_permanent_fixed_addresses,
#define NR_FIX_BITMAPS  16
    FIX_BITMAP_END = __end_of_permanent_fixed_addresses;
    FIX_BITMAP_BEGIN = FIX_BITMAP_END + NR_FIX_BITMAPS - 1;
    __end_of_fixed_addresses
};
```

`fixrange_init()`负责构建虚拟地址空间的PGD和PMD，而PTE则留待后续。

```c
static void __init fixrange_init(unsigned long start, unsigned long end, pgd_t *pgd_base)
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
                if (pte != pte_offset(pmd, 0)){
                    BUG();
                }
            }
            vaddr += PMD_SIZE;
        }
        j = 0;
    }

}
```

### load_cr3

将页表起始地址加载cr3控制寄存器当中。

```c
#define load_cr3(pgdir) asm volatile("movl %0, %%cr3": :"r"(__pa(pgdir)));
```

控制寄存器的结构如下：
<img src="https://img-bed-l.oss-cn-beijing.aliyuncs.com/pic_bed/%E6%8E%A7%E5%88%B6%E5%AF%84%E5%AD%98%E5%99%A8.png" alt="image-20220528184629291" style="zoom:80%;" />

具体的CR3和页表项：

![image-20220528202506926](https://img-bed-l.oss-cn-beijing.aliyuncs.com/pic_bed/202205282025964.png)

### __flush_tlb_all

刷新 tlb 缓存。

```c
#define __flush_tlb() \
    do{ \
        unsigned int tmpreg;        \
        __asm__ __volatile__(       \
            "movl %%cr3, %0; # flush TLB\n"  \
            "movl %0, %%cr3;            \n"     \
            : "=r" (tmpreg)         \
            ::   "memory");     \
    }while(0)
```

### zone_sizes_init

计算三个管理区的大小，利用zones_size数组存储三种不同的zone管理区。

三个管理区分别是是：

- ZONE_DMA：从0～16MB分配给这个区
- ZONE_NORMAL：从16MB～896MB分配给这个区
- ZONE_HIGH：未使用该管理区

每个管理区使用zone_struct(也定义为 zone_t )进行描述:
```c

typedef struct zone_struct {

    unsigned long free_pages;       // 空闲页面的总数
    unsigned long pages_min;        // 管理区的极值
    unsigned long pages_low;
    unsigned long pages_high;
    int need_balance;              // 通知 页面换出kswapd进程 平衡该管理区

    free_area_t  free_area[MAX_ORDER];      // 空闲区域
    
    // 等待队列的哈希表，其中是等待页面释放的进程组成
    // 为了减小内存的消耗，Linux将等待队列村昂在 zone当中而不是每个页面都有一个等待队列，使用哈希表来存储
    wait_queue_head_t *wait_table;      
    unsigned long wait_table_size;    // 哈希数组的大小，2的幂
    unsigned long wait_table_shift;     // 等待表的大小

    struct pglist_data *zone_pgdat;         // 指向父 pg_data_t
    struct page *zone_mem_map;              // 设计的管理区在全局mem_map中的第一页
    unsigned long zone_start_paddr;         
    unsigned long zone_start_mapnr;

    char *name;                 // 管理区的字符串名称，DMA Normal 或者 HighMem
    unsigned long size;         // 管理区的页面数
} zone_t;

```


```c
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
```

#### free_area_init_core

进行空闲空间的初始化与分配，关键在于 `free_area_init()`函数，其中调用的关系为下图所示，核心在于`free_area_init_core()`函数。

![image-20220505161514491](https://img-bed-l.oss-cn-beijing.aliyuncs.com/pic_bed/free_area_init.png)

具体实现如下图所示：

```c
/**
 * @brief 建立管理区zone的数据结构: 将所有的page都标记为reserved, 将内存等待队列标记为空，清空内存位图
 * 
 * @param nid 被初始化管理区中节点的标识符，nodeid
 * @param pgdat 节点数据结构
 * @param gmap 
 * @param zones_size 管理区大小
 * @param zone_start_paddr 第一个管理区的起始物理地址
 * @param zholes_size 管理区内存空洞的大小
 * @param lmem_map 
 */
void __init free_area_init_core(int nid, pg_data_t *pgdat, struct page **gmap,
    unsigned long *zones_size, unsigned long zone_start_paddr, 
    unsigned long *zholes_size, struct page *lmem_map)
{
    unsigned long i, j;
    unsigned long map_size;
    unsigned long totalpages, offset, realtotalpages;
    const unsigned long zone_required_alignment = 1UL << (MAX_ORDER-1);

    if(zone_start_paddr & ~PAGE_MASK){
        BUG();
    }

    totalpages = 0;
    for (i = 0; i < MAX_NR_ZONES; i++) {
        unsigned long size = zones_size[i];
        totalpages += size;
    }
    realtotalpages = totalpages;
    if (zholes_size)
        for (i = 0; i < MAX_NR_ZONES; i++)
            realtotalpages -= zholes_size[i];

    printk("On node %d totalpages: %lu\n", nid, realtotalpages);

    // 
    map_size = (totalpages + 1)*sizeof(struct page);
    if (lmem_map == (struct page *)0) {
        lmem_map = (struct page *) alloc_bootmem_node(pgdat, map_size);
        lmem_map = (struct page *)(PAGE_OFFSET + 
            MAP_ALIGN((unsigned long)lmem_map - PAGE_OFFSET));
    }
    *gmap = pgdat->node_mem_map = lmem_map;
    pgdat->node_size = totalpages;
    pgdat->node_start_paddr = zone_start_paddr;
    pgdat->node_start_mapnr = (lmem_map - mem_map);
    pgdat->nr_zones = 0;

    offset = lmem_map - mem_map;    
    for (j = 0; j < MAX_NR_ZONES; j++) {
        zone_t *zone = pgdat->node_zones + j;
        unsigned long mask;
        unsigned long size, realsize;

        zone_table[nid * MAX_NR_ZONES + j] = zone;
        realsize = size = zones_size[j];
        if (zholes_size)
            realsize -= zholes_size[j];

        printk("zone(%lu): %lu pages.\n", j, size);
        zone->size = size;
        zone->name = zone_names[j];
        zone->lock = SPIN_LOCK_UNLOCKED;
        zone->zone_pgdat = pgdat;
        zone->free_pages = 0;
        zone->need_balance = 0;
        if (!size)
            continue;

        /*
         * The per-page waitqueue mechanism uses hashed waitqueues
         * per zone.
         */
        zone->wait_table_size = wait_table_size(size);
        zone->wait_table_shift =
            BITS_PER_LONG - wait_table_bits(zone->wait_table_size);
        zone->wait_table = (wait_queue_head_t *)
            alloc_bootmem_node(pgdat, zone->wait_table_size
                        * sizeof(wait_queue_head_t));

        for(i = 0; i < zone->wait_table_size; ++i)
            init_waitqueue_head(zone->wait_table + i);

        pgdat->nr_zones = j+1;

        mask = (realsize / zone_balance_ratio[j]);
        if (mask < zone_balance_min[j])
            mask = zone_balance_min[j];
        else if (mask > zone_balance_max[j])
            mask = zone_balance_max[j];
        zone->pages_min = mask;
        zone->pages_low = mask*2;
        zone->pages_high = mask*3;

        zone->zone_mem_map = mem_map + offset;
        zone->zone_start_mapnr = offset;
        zone->zone_start_paddr = zone_start_paddr;

        if ((zone_start_paddr >> PAGE_SHIFT) & (zone_required_alignment-1))
            printk("BUG: wrong zone alignment, it will crash\n");

        /*
         * Initially all pages are reserved - free ones are freed
         * up by free_all_bootmem() once the early boot process is
         * done. Non-atomic initialization, single-pass.
         */
        for (i = 0; i < size; i++) {
            struct page *page = mem_map + offset + i;
            set_page_zone(page, nid * MAX_NR_ZONES + j);
            set_page_count(page, 0);
            SetPageReserved(page);
            INIT_LIST_HEAD(&page->list);
            if (j != ZONE_HIGHMEM)
                set_page_address(page, __va(zone_start_paddr));
            zone_start_paddr += PAGE_SIZE;
        }

        offset += size;
        for (i = 0; ; i++) {
            unsigned long bitmap_size;

            INIT_LIST_HEAD(&zone->free_area[i].free_list);
            if (i == MAX_ORDER-1) {
                zone->free_area[i].map = NULL;
                break;
            }

            bitmap_size = (size-1) >> (i+4);
            bitmap_size = LONG_ALIGN(bitmap_size+1);
            zone->free_area[i].map = 
              (unsigned long *) alloc_bootmem_node(pgdat, bitmap_size);
        }
    }
    build_zonelists(pgdat);
}
```



#### build_zonelists

`build_zonelists()`建立节点的管理区回退链表，用于不能满足内存分配时考察下一个管理区：

```c
static inline void build_zonelists(pg_data_t *pgdat)
{
    int i, j, k;

    for(i = 0; i <= GFP_ZONEMASK; i++){
        zonelist_t *zonelist;
        zone_t *zone;

        zonelist = pgdat->node_zonelists + i;
        memset(zonelist, 0, sizeof(*zonelist));

        j = 0;
        k = ZONE_NORMAL;
        if(i & __GFP_HIGHMEM)
            k = ZONE_HIGHMEM;
        if(i & __GFP_DMA)
            k = ZONE_DMA;

        switch (k){
            default:
                BUG();
                        /*
             * fallthrough:
             */
            case ZONE_HIGHMEM:
                zone = pgdat->node_zones + ZONE_HIGHMEM;
                if (zone->size) {
#ifndef CONFIG_HIGHMEM
                    BUG();
#endif
                    zonelist->zones[j++] = zone;
                }
            case ZONE_NORMAL:
                zone = pgdat->node_zones + ZONE_NORMAL;
                if (zone->size)
                    zonelist->zones[j++] = zone;
            case ZONE_DMA:
                zone = pgdat->node_zones + ZONE_DMA;
                if (zone->size)
                    zonelist->zones[j++] = zone;
        }
        zonelist->zones[j++] = NULL;
    }
}
```

其中包含了几个标志宏，`__GFP_DMA` 和 `__GFP_HIGHMEM` ，属于GFP标志位。

   GFP --- Get Free Page标志，是 贯穿整个虚拟内存管理的一个概念。该标志位决定了分配器和kswapd进程如何进行内存页的分配和释放。总共有三组GFP标志位，都定义在<linux/mm.h>当中。

* 第一组，zone 修饰符。影响zone的内存分配，分别有：

* ```c
  #define __GFP_DMA   0x01      // 表示尽可能从 ZONE_DMA 当中分配内存
  #define __GFP_HIGHMEM   0x02 // 表示尽可能从 ZONE_HIGHMEM 当中分配内存
  ```

* 第二组，action 修饰符。后续展开。



### 细节

**原子化**



**类型检查的 `min` 和 `max` 宏**

在 `free_area_init` 在调用 `wait_table_size(unsigned long *pages)` 计算等待队列时，用到 `min` 宏方法：

```c
#define min(x, y)({ \
    const typeof(x) _x = (x);  \
    const typeof(y) _y = (y);  \
    (void) (&_x == &_y);       \
    _x < _y ? _x : _y; })

#define max(x, y)({
    const typeof(x) _x = (x);  \
    const typeof(y) _y = (y);  \
    (void) (&_x == &_y);       \
    _x > _y ? _x : _y; })
```

其中利用typeof 进行了类型检查，GCC 有关于 `typeof` 的介绍：[typeof](https://gcc.gnu.org/onlinedocs/gcc/Typeof.html)。其语法形式类似 `sizeof`，但实际行为则与 `typedef` 定义的一个类型相似。





## 错误

在 `paging_init` 的 `pagetable_init` 当中 出现页对齐的 BUG()：

```c
if(pte_base != pte_offset(pmd, 0)){ // 保证页表项有效
	BUG();    
```

打印时发现，计算 pte_offset(0xc0116c18, 0) 的结果出现错误，没有添加上 0xC0000000。

原因在于分配页面时，出现错误。

```c
  pte_base = pte = (pte_t *) alloc_bootmem_low_pages(PAGE_SIZE);
```



![image-20220824150219540](https://img-bed-l.oss-cn-beijing.aliyuncs.com/pic_bed/image-20220824150219540.png)





倒数第二次：`end` 在出错的第二次时，变为 `0xf8000001`，随后修改为 `0xbde06301`。其地址在 `0xc0007f77`。

![image-20220830221201079](https://img-bed-l.oss-cn-beijing.aliyuncs.com/pic_bed/image-20220830221201079.png)



```
(gdb) p/x &pte_base
$5 = 0xc0007f6b
(gdb) p/x &vaddr
$6 = 0xc0007f6f
(gdb) p/x &pgd_base
$9 = 0xc0007f73
(gdb) p/x &end
$2 = 0xc0007f77
(gdb) p/x &pte
$4 = 0xc0007f7b
(gdb) p/x &pmd
$7 = 0xc0007f7f
(gdb) p/x &k
$13 = 0xc0007f83
(gdb) p/x &j
$12 = 0xc0007f87
(gdb) p/x &i
$11 = 0xc0007f8b
(gdb) p/x &pgd
$8 = 0xc0007f8f
```



![image-20220831171056403](https://img-bed-l.oss-cn-beijing.aliyuncs.com/pic_bed/image-20220831171056403.png)

```

(gdb) p/x swapper_pg_dir
$19 = {{pgd = 0x0} <repeats 768 times>, {pgd = 0x1063}, {pgd = 0x2063}, {pgd = 0x3063}, {
    pgd = 0x4063}, {pgd = 0x5063}, {pgd = 0x6063}, {pgd = 0x7063}, {pgd = 0x8063}, {
    pgd = 0x9063}, {pgd = 0xa063}, {pgd = 0xb063}, {pgd = 0xc063}, {pgd = 0xd063}, {
    pgd = 0xbdb06366}, {pgd = 0x0} <repeats 242 times>}
```

在 swapper_pg_dir 当可以看到，当初始化到 第 `0xe` 个pgd 时，就出现了问题。为什么？

查看具体的页表项，发现直到位置 `0xc000ee78` 时，映射出现问题：

```
(gdb) p/x *0xc000ee74
$30 = 0x379d063
(gdb) p/x *0xc000ee78
$31 = 0x8079e063
```

---

正确进入后，寄存器情况如下：

![image-20220831222449682](https://img-bed-l.oss-cn-beijing.aliyuncs.com/pic_bed/image-20220831222449682.png)

GDT 信息如下所示：

```
(gdb) x/20x 0xc0000900
0xc0000900:     0x00000000      0x00000000      0x0000ffff      0x00cf9800
0xc0000910:     0x0000ffff      0x00cf9300      0x80000007      0xc0c0920b
0xc0000920:     0x0000ffff      0x00cff800      0x0000ffff      0x00cff200
```

512MB的内存总大小的情况下，内存映射分布如下：

![image-20220902115343612](https://img-bed-l.oss-cn-beijing.aliyuncs.com/pic_bed/image-20220902115343612.png)

通过调整 swapper_pg_dir 的位置到 `0xc011d000`，依然在初始化至 `pte_base = 0xc000e000` 的 第 961 项(`0xc000ef07`)时出现问题，说明这和 swapper_pg_dir 具体的内存位置无关。

当将整体的内存调整到 512MB而非1024MB时，内存的设置就不会出现问题。

## register_memory

`resource` 的捕捉方案



![image-20220903193818681](https://img-bed-l.oss-cn-beijing.aliyuncs.com/pic_bed/image-20220903193818681.png)

> 《操作系统真象还原》实模式下的内存布局

### probe_roms





# 修改

将所有的 `__init`  和 `__init_data` 去除，这两个段没有进行虚拟地址的转换，所有使用通常的文本段。
