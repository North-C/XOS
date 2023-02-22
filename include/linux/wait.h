#ifndef _LINUX_WAIT_H
#define _LINUX_WAIT_H

#include <linux/spinlock.h>
#include <linux/list.h>
// 等待队列部分

struct __wait_queue {
    unsigned int flags;
#define WQ_FLAG_EXCLUSIVE	0x01
    // struct task_struct *task;
    struct list_head task_list;
};

typedef struct __wait_queue wait_queue_t;


#define wq_lock_t rwlock_t
# define WAITQUEUE_RW_LOCK_UNLOCKED RW_LOCK_UNLOCKED

struct __wait_queue_head {
    wq_lock_t lock;
	struct list_head task_list;
};

typedef struct __wait_queue_head wait_queue_head_t; 

static inline void init_waitqueue_head(wait_queue_head_t *q)
{
	q->lock = WAITQUEUE_RW_LOCK_UNLOCKED;
	INIT_LIST_HEAD(&q->task_list);
}

typedef struct __wait_queue_head wait_queue_head_t;


#endif