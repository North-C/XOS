#include <linux/slab.h>
#include <linux/list.h>
#include <linux/cache.h>
#include <linux/string.h>
#include <linux/spinlock.h>
#include <linux/mm.h>
#include <linux/string.h>
#include <asm-i386/cache.h>
#include <asm-i386/page.h>
#include <asm-i386/semaphore.h>
#include <asm-i386/system.h>

static void kmem_cache_estimate(unsigned long gfporder, size_t size, int flags, size_t* left_over, unsigned int* num);

extern unsigned int nr_free_pages (void);
extern unsigned long __get_free_pages(int gfp_mask, unsigned long order);

#define BYTES_PER_WORD  sizeof(void *)

// 对象描述符 kmem_bufctl_t
#define BUFCTL_END  0xffffFFFF
#define SLAB_LIMIT  0xffffFFFE
typedef unsigned int kmem_bufctl_t;

// cache 当中最大objs-per-slab 
static unsigned long offslab_limit;

// kmem_cache_create 的创建掩码
#define CREATE_MASK  (SLAB_HWCACHE_ALIGN | SLAB_NO_REAP | SLAB_CACHE_DMA)

/* slab_t 
   管理 slab 当中的对象,存放在被分配内存的开头, 内存一般是从 slab 或者一个普通缓存中分配
   slab 通过一个有序链表的链接起来，分为三类: fully used, partial, fully free
*/
typedef struct slab_s {
    struct list_head list;      // 三类双向链表中的一个
    unsigned long colouroff;   // slab 当中第一个对象的偏移
    void *s_mem;     // slab 中第一个对象（被分配或者空闲）的地址
    unsigned int inuse;   // slab 当中非空闲的对象数量
    kmem_bufctl_t  free;   // slab 中下一个空闲对象的下标
}slab_t;

#define slab_bufctl(slabp) \
    ((kmem_bufctl_t *)(((slab_t*)slabp)+1))

#define CACHE_NAMELEN   20   // slab cache 当中的最大名称长度

struct kmem_cache_s {
    struct list_head slabs;
    struct list_head *firstnotfull;
    unsigned int objsize;   // 对象大小
    unsigned int flags;     // 对高速缓存的决定
    unsigned int num;       // 对象个数
    spinlock_t    spinlock;   // 并发控制锁

    /* slab 附属 */
    unsigned int gfporder;   // 占用 2^gfporder 个连续页面，由伙伴系统分配

    unsigned int gfpflags;

    size_t colour;      // cache 着色的范围
    unsigned int		colour_off;	/* colour offset */
	unsigned int		colour_next;	/* cache colouring */
	kmem_cache_t		*slabp_cache;   // 对应的cache
	unsigned int		growing;
	unsigned int		dflags;		/* dynamic flags */

	/* constructor func */
	void (*ctor)(void *, kmem_cache_t *, unsigned long);  // 对象构造

	/* de-constructor func */
	void (*dtor)(void *, kmem_cache_t *, unsigned long);  // 对象析构

	unsigned long		failures;
    
    /* cache 创建和删除 */
    char			name[CACHE_NAMELEN];    // cache 的名称
	struct list_head	next;        // 指向下一个高速缓存
    
    /* slab的状态信息 */
    unsigned long		num_active;
	unsigned long		num_allocations;
	unsigned long		high_mark;
	unsigned long		grown;
	unsigned long		reaped;
	unsigned long 		errors;
};

// 内部 cache 描述符对象变量
static kmem_cache_t cache_cache = {
    slabs:		LIST_HEAD_INIT(cache_cache.slabs),
	firstnotfull:	&cache_cache.slabs,
	objsize:	sizeof(kmem_cache_t),
	flags:		SLAB_NO_REAP,            // 创建时的标志位
	// spinlock:	SPIN_LOCK_UNLOCKED,      // 并发控制锁
	colour_off:	L1_CACHE_BYTES,          // slab 中的相对于第一个对象基地址的着色偏移
	name:		"kmem_cache", 
};

/* 内部的 SLAB 管理标志位 */
#define CFLGS_OFF_SLAB  0x010000UL   // slab 管理
#define CFLGS_OPTIMIZE  0x020000UL   // 优化 slab 查询

// 获取访问该成员的并发锁
#define DFLGS_GROWN  0x000001UL

#define	OFF_SLAB(x)	((x)->flags & CFLGS_OFF_SLAB)
#define	OPTIMIZE(x)	((x)->flags & CFLGS_OPTIMIZE)

// gfp order 的绝对值
#define MAX_GFP_ORDER   5

// 保护 cache_chain
static struct semaphore cache_chain_sem;

#define cache_chain (cache_cache.next)

// 一个对象的最大page数
#define MAX_OBJ_ORDER  5

// 不要超出限制
#define	BREAK_GFP_ORDER_HI	2
#define	BREAK_GFP_ORDER_LO	1
static int slab_break_gfp_order = BREAK_GFP_ORDER_LO;

// 存储或检索 cachep 或者 slab,用于查找对象所属的 slab
#define	SET_PAGE_CACHE(pg,x)  ((pg)->list.next = (struct list_head *)(x))
#define	GET_PAGE_CACHE(pg)    ((kmem_cache_t *)(pg)->list.next)
#define	SET_PAGE_SLAB(pg,x)   ((pg)->list.prev = (struct list_head *)(x))
#define	GET_PAGE_SLAB(pg)     ((slab_t *)(pg)->list.prev)

/* 普通高速缓存的大小描述符 */
typedef struct cache_sizes {
    size_t cs_size;
    kmem_cache_t *cs_cachep;
    kmem_cache_t *cs_dmacachep;
} cache_sizes_t;

static cache_sizes_t cache_sizes[] ={
    {    64,	NULL, NULL},
	{   128,	NULL, NULL},
	{   256,	NULL, NULL},
	{   512,	NULL, NULL},
	{  1024,	NULL, NULL},
	{  2048,	NULL, NULL},
	{  4096,	NULL, NULL},
	{  8192,	NULL, NULL},
	{ 16384,	NULL, NULL},
	{ 32768,	NULL, NULL},
	{ 65536,	NULL, NULL},
	{131072,	NULL, NULL},
	{     0,	NULL, NULL}
};

void kmem_cache_init(void)
{
    size_t left_over;

    init_MUTEX(&cache_chain_sem);   // 信号量
    INIT_LIST_HEAD(&cache_chain);   // 高速缓存链表
    
    // 计算对象的数量和消耗的字节数
    kmem_cache_estimate(0, cache_cache.objsize, 0, &left_over, &cache_cache.num);

    if(!cache_cache.num)   // 没有kmem_cache_t 存放在页面中
        BUG();
    
    cache_cache.colour = left_over/cache_cache.colour_off;
    cache_cache.colour_next = 0;
}

/* 初始化指定大小的高速缓存 */
void kmem_cache_sizes_init(void)
{
    cache_sizes_t* sizes = cache_sizes;
    char name[20];

    if(num_physpages > (32 >> 20) >> PAGE_SHIFT)
        slab_break_gfp_order = BREAK_GFP_ORDER_HI;
    
    do{
        sprintf(name, "size-%d", sizes->cs_size);
        if(!(sizes->cs_cachep = kmem_cache_create(name, sizes->cs_size, 0, SLAB_HWCACHE_ALIGN, NULL, NULL)))
            BUG();
        
        if(!(OFF_SLAB(sizes->cs_cachep))){
            offslab_limit = sizes->cs_size - sizeof(slab_t);
            offslab_limit /= 2;
        }
        // sprintf(name, "size-%Zd(DMA)", sizes->cs_size);
        // sizes->cs_dmacachep = kmem_cache_create(name, sizes->cs_size, 0,       SLAB_CACHE_DMA | SLAB_HWCACHE_ALIGN, NULL, NULL);

        // if(!sizes->cs_dmacachep)
        //     BUG();
        sizes++;
    } while(sizes->cs_size);
}

/* 计算 slab 当中可以存放的对象数量，以及消耗的空间
    gfporder - slab 被分配的页面数 2^gfporder
    size - 对象的大小
    flags - 高速缓存标志位
    left_over - 是slab中剩余的字节数，由调用者返回
    num - 填入slab的对象数
 */
static void kmem_cache_estimate(unsigned long gfporder, size_t size, int flags, size_t* left_over, unsigned int* num){
    int i;
    size_t wastage = PAGE_SIZE<<gfporder;  // 递减数，从可能的最大消耗量开始
    size_t extra = 0;     // 存储 kmem_bufctl_t 需要的字节数
    size_t base = 0;     // slab 开始位置的可用内存地址

    if(!(flags & CFLGS_OFF_SLAB)){      // 保存在高速缓存当中，则及地址开始于 slab_t 末端
        base = sizeof(slab_t);
        extra = sizeof(kmem_bufctl_t);
    }
    i = 0;
    while(i*size + L1_CACHE_ALIGN(base+i*extra) <= wastage)
        i++;
    if(i > 0)
        i--;
    
    if(i > SLAB_LIMIT)
        i = SLAB_LIMIT;

    *num = i;
    wastage -= i * size;
    wastage -= L1_CACHE_ALIGN(base + i*extra);
    *left_over = wastage;
}

// 需按照大小合适的普通高速缓存
kmem_cache_t* kmem_find_general_cachep(size_t size, int gfpflags)
{
    cache_sizes_t *csizep = cache_sizes;

    for(; csizep->cs_size; csizep++) {
        if(size > csizep->cs_size)
            continue;
        break;
    }
    return (gfpflags & GFP_DMA) ? csizep->cs_dmacachep : csizep->cs_cachep;
}

// 系统页面分配接口
static inline void *kmem_getpages(kmem_cache_t *cachep, unsigned long flags)
{
    void *addr;

    flags |= cachep->gfpflags;
    addr = (void*) __get_free_pages(flags, cachep->gfporder);

    return addr;

}

static inline void kmem_cache_alloc_head(kmem_cache_t *cachep, int flags)
{
#if DEBUG
	if (flags & SLAB_DMA) {
		if (!(cachep->gfpflags & GFP_DMA))
			BUG();
	} else {
		if (cachep->gfpflags & GFP_DMA)
			BUG();
	}
#endif
}

/* Get the memory for a slab management obj. */
static inline slab_t * kmem_cache_slabmgmt (kmem_cache_t *cachep,
			void *objp, int colour_off, int local_flags)
{
	slab_t *slabp;
	
	if (OFF_SLAB(cachep)) {
		/* Slab management obj is off-slab. */
		slabp = kmem_cache_alloc(cachep->slabp_cache, local_flags);
		if (!slabp)
			return NULL;
	} else {
		/* FIXME: change to
			slabp = objp
		 * if you enable OPTIMIZE
		 */
		slabp = objp+colour_off;
		colour_off += L1_CACHE_ALIGN(cachep->num *
				sizeof(kmem_bufctl_t) + sizeof(slab_t));
	}
	slabp->inuse = 0;
	slabp->colouroff = colour_off;
	slabp->s_mem = objp+colour_off;

	return slabp;
}

static inline void kmem_cache_init_objs(kmem_cache_t *cachep, slab_t * slabp, unsigned long ctor_flags)
{
    int i;

    for(i = 0; i < cachep->num; i++){
        void *objp = slabp->s_mem + cachep->objsize++;
        if(cachep->ctor)
            cachep->ctor(objp, cachep, ctor_flags);
        slab_bufctl(slabp)[i] = i + 1;
    }

    slab_bufctl(slabp)[i-1] = BUFCTL_END;
    slabp->free = 0;
}

static inline void kmem_freepages(kmem_cache_t *cachep, void *addr)
{
    unsigned long i = (1<<cachep->gfporder);
    struct page *page = virt_to_page(addr);

    while(i--){
        PageClearSlab(page);
        page++;
    }
    free_pages((unsigned long) addr, cachep->gfporder);
}

// 从 slab 当中分配一个对象
static inline void *kmem_cache_alloc_one_tail(kmem_cache_t *cachep, slab_t *slabp)
{
    void *objp;
    // 设置三个统计计数
    // STATS_INC_ALLOCED(cachep);   // 已经分配的对象
    // STATS_INC_ACTIVE(cachep);    // 活跃对象的总数
    // STATS_SET_HIGH(cachep);      // 某个时间上活跃对象的最大数

    // 获取对象指针
    slabp->inuse++;       // slab上的活动对象数
    // s_mem 是slab上第一个对象的指针，free记录空闲对象的索引
    objp = slabp->s_mem + slabp->free * cachep->objsize;
    slabp->free = slab_bufctl(slabp)[slabp->free];   // 更新空闲指针

    if(slabp->free == BUFCTL_END)
    // slab 已经满，进入下一个slab分配
        cachep->firstnotfull = slabp->list.next;

    return objp;
}


#define kmem_cache_alloc_one(cachep)    \
({              \
    slab_t *slabp;              \
    \
    {           \
        struct list_head* p = cachep->firstnotfull;     \
        if(p == &cachep->slabs)         \
            goto alloc_new_slab;       \
        slabp = list_entry(p, slab_t, list);        \
    }           \
    kmem_cache_alloc_one_tail(cachep, slabp);       \
})

// 在 cache 当中增长 slab 的数量
// 当 cache 当中没有可用 obj 时, kmem_cache_grow 会调用该函数
static int kmem_cache_grow(kmem_cache_t *cachep, int flags)
{
    slab_t	*slabp;
	struct page	*page;
	void		*objp;
	size_t		 offset;
	unsigned int	 i, local_flags;
	unsigned long	 ctor_flags;
	unsigned long	 save_flags;

    /* 有效性检查 */ 
    if (flags & ~(SLAB_DMA|SLAB_LEVEL_MASK|SLAB_NO_GROW))
		BUG();
	if (flags & SLAB_NO_GROW)
		return 0;

	/*
	 * The test for missing atomic flag is performed here, rather than
	 * the more obvious place, simply to reduce the critical path length
	 * in kmem_cache_alloc(). If a caller is seriously mis-behaving they
	 * will eventually be caught here (where it matters).
	 */
	// if (in_interrupt() && (flags & SLAB_LEVEL_MASK) != SLAB_ATOMIC)
	//	BUG();

	ctor_flags = SLAB_CTOR_CONSTRUCTOR;
	local_flags = (flags & SLAB_LEVEL_MASK);
	if (local_flags == SLAB_ATOMIC)
		/*
		 * Not allowed to sleep.  Need to tell a constructor about
		 * this - it might need to know...
		 */
		ctor_flags |= SLAB_CTOR_ATOMIC;
    
    /* 计算slab中对象的颜色偏移 */
	/* About to mess with non-constant members - lock. */
	spin_lock_irqsave(&cachep->spinlock, save_flags);   // 中断安全锁

	/* Get colour for the slab, and cal the next value. */
	offset = cachep->colour_next;    // slab 中的对象偏移
	cachep->colour_next++;         // 获取下一个颜色偏移
	if (cachep->colour_next >= cachep->colour)   // 没有更多的偏移
		cachep->colour_next = 0;
	offset *= cachep->colour_off;  // offset * 每个偏移的大小，即对象偏移的字节数
	cachep->dflags |= DFLGS_GROWN;

	cachep->growing++;       // 调用者计数
	spin_unlock_irqrestore(&cachep->spinlock, save_flags);

	/* A series of memory allocations for a new slab.
	 * Neither the cache-chain semaphore, or cache-lock, are
	 * held, but the incrementing c_growing prevents this
	 * cache from being reaped or shrunk.
	 * Note: The cache could be selected in for reaping in
	 * kmem_cache_reap(), but when the final test is made the
	 * growing value will be seen.
	 */

	/* Get mem for the objs. */
	if (!(objp = kmem_getpages(cachep, flags)))
		goto failed;

	/* Get slab management. */
	if (!(slabp = kmem_cache_slabmgmt(cachep, objp, offset, local_flags)))
		goto opps1;

	/* Nasty!!!!!! I hope this is OK. */
	i = 1 << cachep->gfporder;
	page = virt_to_page(objp);
	do {
		SET_PAGE_CACHE(page, cachep);
		SET_PAGE_SLAB(page, slabp);
		PageSetSlab(page);
		page++;
	} while (--i);
    
    // 初始化 slab 对象
	kmem_cache_init_objs(cachep, slabp, ctor_flags);

	spin_lock_irqsave(&cachep->spinlock, save_flags);
	cachep->growing--;

	/* Make slab active. */
	list_add_tail(&slabp->list,&cachep->slabs);
	if (cachep->firstnotfull == &cachep->slabs)
		cachep->firstnotfull = &slabp->list;
	// STATS_INC_GROWN(cachep);
	cachep->failures = 0;

	spin_unlock_irqrestore(&cachep->spinlock, save_flags);
	return 1;
opps1:
	kmem_freepages(cachep, objp);
failed:
	spin_lock_irqsave(&cachep->spinlock, save_flags);
	cachep->growing--;
	spin_unlock_irqrestore(&cachep->spinlock, save_flags);
	return 0;
}

static inline void * __kmem_cache_alloc(kmem_cache_t *cachep, int flags)
{
    unsigned long save_flags;
    void* objp;

    kmem_cache_alloc_head(cachep, flags);

try_again:
    local_irq_save(save_flags);
    objp = kmem_cache_alloc_one(cachep);
    local_irq_restore(save_flags);

    return objp;
alloc_new_slab:
    local_irq_restore(save_flags);
	if (kmem_cache_grow(cachep, flags))
		/* Someone may have stolen our objs.  Doesn't matter, we'll
		 * just come back here again.
		 */
		goto try_again;
	return NULL;
}


// 分配 slab中的对象
void *kmem_cache_alloc(kmem_cache_t *cachep, int flags)
{
    unsigned long save_flags;
    void *objp;

    kmem_cache_alloc_head(cachep, flags);
try_again:
    local_irq_save(save_flags);
    objp = kmem_cache_alloc_one(cachep);

    local_irq_restore(save_flags);
    return objp;

alloc_new_slab:
    local_irq_restore(save_flags);
    if(kmem_cache_grow(cachep, flags))
        goto try_again;
    return NULL;
}

/* 创建一个cache */
kmem_cache_t* kmem_cache_create(const char *name, size_t size, size_t offset, unsigned long flags, void (*ctor)(void*, kmem_cache_t *, unsigned long), void(*dtor)(void*, kmem_cache_t *, unsigned long))
{
    const char *func_nm = "kmem_create: ";
    size_t left_over, align, slab_size;
    kmem_cache_t *cachep = NULL;

    // in_interrupt() 暂时去掉
    if((!name) || ((strlen(name) >= CACHE_NAMELEN - 1)) || (size < BYTES_PER_WORD) || (size > (1<<MAX_OBJ_ORDER)*PAGE_SIZE) || 
        (dtor && !ctor) ||
        (offset < 0 || offset > size))
            BUG();

    // TODO: 在进行内存分配时出现问题
    // 分配缓存的描述符对象
    cachep = (kmem_cache_t *)kmem_cache_alloc(&cache_cache, SLAB_KERNEL);
    if(!cachep)
        goto opps;
    memset(cachep, 0, sizeof(kmem_cache_t));

    // 将对象大小对齐于某个字大小边界
    if(size & (BYTES_PER_WORD-1)){
        size += (BYTES_PER_WORD-1);    // 增加一个字的大小
        size &= ~(BYTES_PER_WORD-1);   // 屏蔽低位
        printk("%sForcing size word alignment - %s\n", func_nm, name);
    }

    align = BYTES_PER_WORD;
    if(flags & SLAB_HWCACHE_ALIGN)  // 设置对齐到 L1 CPU 高速缓存
        align = L1_CACHE_BYTES;
    
    // 对象比较大，则采用offslab的排布，更加紧密的排列
    if(size >= (PAGE_SIZE>>3))
        flags |= CFLGS_OFF_SLAB;
    
    if(flags & SLAB_HWCACHE_ALIGN){  // 对齐硬件高速缓存
        // 调整大小,使得对象能够对齐
        while(size < align/2)
            align /= 2;
        size = (size+align-1)&(~(align-1));
    }
// 计算 slab 中的页面数，和每个slab当中的对象数
    do{
        unsigned int break_flag = 0;
cal_wastage:
        kmem_cache_estimate(cachep->gfporder, size, flags, &left_over, &cachep->num);

        // 在使用 offslab 类描述符后，填入 slab 的对象数超过了 slab 存放的对象数，则设置了 break_flag
        if(break_flag)
            break;
        if(cachep->gfporder >= MAX_GFP_ORDER)
            break;
        if(!cachep->num)
            goto next;
        // 高速缓存中没有 slab 描述符，但是对象书超过了 bufctl 的 off-slab 所确定的数目
        if(flags & CFLGS_OFF_SLAB && cachep->num > offslab_limit){
            cachep->gfporder--;
            break_flag++;
            goto cal_wastage;
        }

        if(cachep->gfporder >= slab_break_gfp_order)
            break;
        // 检查外部碎片，消耗1/8的高速缓存则可以接受
        if((left_over*8) <= (PAGE_SIZE<<cachep->gfporder))
            break;
next:
        cachep->gfporder++;
    }while(1);

    if(!cachep->num){   // 释放高速缓存描述符
        printk("kmem_cache_create: couldn't create cache %s.\n", name);
        kmem_cache_free(&cache_cache, cachep);
        cachep = NULL;
        goto opps;
    }
    // slab 描述符的总大小,不是 slab 的大小
    slab_size = L1_CACHE_ALIGN(cachep->num * sizeof(kmem_bufctl_t)+ sizeof(slab_t));

    // 有足够的空间给 slab 描述符,则指定为放置 off slab 描述符
    if(flags & CFLGS_OFF_SLAB && left_over >= slab_size){
        flags &= ~CFLGS_OFF_SLAB;
        left_over -= slab_size;
    }
    
    // 偏移量必须是对齐值的倍数
    offset += (align-1);
    offset &= ~(align-1);
    if(!offset)
        offset = L1_CACHE_BYTES;
    cachep->colour_off = offset;
	cachep->colour = left_over/offset;

    // 初始化 cachep 当中剩余的字段
    if(!cachep->gfporder && !(flags & CFLGS_OFF_SLAB))
        flags |= CFLGS_OPTIMIZE;
    
    cachep->flags = flags;
    cachep->gfpflags = 0;
    if(flags & SLAB_CACHE_DMA)
        cachep->gfpflags |= GFP_DMA;
    
    // spin_lock_init(&cachep->slabs);   // TODO: 并发设置
    cachep->firstnotfull = &cachep->slabs;

    if(flags & CFLGS_OFF_SLAB)
        cachep->slabp_cache = kmem_find_general_cachep(slab_size, 0);
    cachep->ctor = ctor;
    cachep->dtor = dtor;
    strcpy(cachep->name, name);

    // 信号量限制并发访问
    down(&cache_chain_sem);
    {
        struct list_head *p;

        list_for_each(p, &cache_chain) {
            kmem_cache_t *pc = list_entry(p, kmem_cache_t, next);

            if(!strcmp(pc->name, name))
                BUG();
        }
    }

    list_add(&cachep->next, &cache_chain);
    up(&cache_chain_sem);

opps:
    return cachep;
}

static inline void kmem_cache_free_one(kmem_cache_t* cachep, void *objp)
{
    slab_t* slabp;
    // 获取slab描述符
    slabp = GET_PAGE_SLAB(virt_to_page(objp));

    {
        unsigned int objnr = (objp-slabp->s_mem)/cachep->objsize;

        slab_bufctl(slabp)[objnr] = slabp->free;
        slabp->free = objnr;
    }

    if(slabp->inuse-- == cachep->num)
        goto moveslab_partial;
    if(!slabp->inuse)
        goto moveslab_free;

moveslab_partial:
    {
        struct list_head *t = cachep->firstnotfull;

		cachep->firstnotfull = &slabp->list;
		if (slabp->list.next == t)
			return;
		list_del(&slabp->list);
		list_add_tail(&slabp->list, t);
		return;
    }

moveslab_free:
    {
        struct list_head *t = cachep->firstnotfull->prev;

		list_del(&slabp->list);
		list_add_tail(&slabp->list, &cachep->slabs);
		if (cachep->firstnotfull == &slabp->list)
			cachep->firstnotfull = t->next;
		return;
    }
}

static inline void __kmem_cache_free(kmem_cache_t *cachep, void* objp)
{
    kmem_cache_free_one(cachep, objp);
}

// 释放对象
void kmem_cache_free(kmem_cache_t *cachep, void *objp)
{
    unsigned int flags;

    local_irq_save(flags);
    __kmem_cache_free(cachep, objp);
    local_irq_restore(flags);
}

void * kmalloc (size_t size, int flags)
{
    cache_sizes_t *csizep = cache_sizes;

    for (; csizep->cs_size; csizep++) {
        if (size > csizep->cs_size)
            continue;
        return __kmem_cache_alloc(flags & GFP_DMA ?
                                  csizep->cs_dmacachep : csizep->cs_cachep, flags);
    }
    BUG(); // too big size
    return NULL;
}

/**
 * kfree - free previously allocated memory
 * @objp: pointer returned by kmalloc.
 *
 * Don't free memory not originally allocated by kmalloc()
 * or you will run into trouble.
 */
void kfree (const void *objp)
{
    kmem_cache_t *c;
    unsigned long flags;

    if (!objp)
        return;
    local_irq_save(flags);
    c = GET_PAGE_CACHE(virt_to_page(objp));
    __kmem_cache_free(c, (void*)objp);
    local_irq_restore(flags);
}