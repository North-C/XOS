#ifndef _LINUX_SLAB_H
#define _LINUX_SLAB_H
#include <linux/mm.h>
#include <linux/types.h>

typedef struct kmem_cache_s kmem_cache_t;

// 传递给 kmem_cache_create 的标志位
#define	SLAB_DEBUG_FREE		0x00000100UL	/* Peform (expensive) checks on free */
#define	SLAB_DEBUG_INITIAL	0x00000200UL	/* Call constructor (as verifier) */
#define	SLAB_RED_ZONE		0x00000400UL	/* Red zone objs in a cache */
#define SLAB_POISON         0x00000800UL    // 有毒的 objects
#define SLAB_NO_REAP        0x00001000UL
#define SLAB_HWCACHE_ALIGN  0x00002000UL   // cache当中对象对齐
#define SLAB_CACHE_DMA		0x00004000UL	/* use GFP_DMA memory */


// kmem_cache_alloc() 使用的标志位
#define SLAB_BUFFER   GFP_BUFFER
#define SLAB_ATOMIC   GFP_ATOMIC
#define SLAB_USER     GFP_USER
#define SLAB_KERNEL   GFP_KERNEL
#define SLAB_NFS      GFP_NFS
#define SLAB_DMA      GFP_DMA

#define SLAB_LEVEL_MASK   (__GFP_WAIT | __GFP_HIGH | __GFP_IO)
#define SLAB_NO_GROW      0x00001000UL

// 传给构造函数的标志位
#define SLAB_CTOR_CONSTRUCTOR  0x001UL   // 未设置，则调用析构函数
#define SLAB_CTOR_ATOMIC  0x001UL   // 告知构造函数，它不能睡眠


// 函数原型
extern void kmem_cache_init(void);
extern void kmem_cache_sizes_init(void);
extern kmem_cache_t *kmem_find_general_cachep(size_t, int gfpflags);
extern kmem_cache_t *kmem_cache_create(const char *, size_t, size_t, unsigned long,void (*)(void *, kmem_cache_t *, unsigned long),
	void (*)(void *, kmem_cache_t *, unsigned long));
extern int kmem_cache_destroy(kmem_cache_t *);
extern int kmem_cache_shrink(kmem_cache_t *);
extern void *kmem_cache_alloc(kmem_cache_t *, int);
extern void kmem_cache_free(kmem_cache_t *, void *);
extern unsigned int kmem_cache_size(kmem_cache_t *);
extern void * kmalloc (size_t size, int flags);
extern void kfree (const void *objp);
#endif