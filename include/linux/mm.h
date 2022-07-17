#ifndef _LINUX_MM_H
#define _LINUX_MM_H
#include <linux/list.h>

typedef struct page {
    struct list_head list;
    // struct address_space *mapping;
    // unsigned long index;
    // struct page *next_hash;
    // atomic_t count;
    // unsigned long flags;    
    // struct list_head lru;
    // unsigned long age;
    // struct page **pprev_hash;
    // struct buffer_head * buffers;
    // void *virtual;
    // struct zone_struct *zone;
} mem_map_t;

extern void free_area_init(unsigned long * zones_size);

#endif