#ifndef _I386_SEMAPHORE_H
#define _I386_SEMAPHORE_H

#include <asm-i386/atomic.h>
#include <linux/wait.h>
#include <linux/linkage.h>
#include <linux/stdio.h>

struct semaphore {
    atomic_t count;
    int sleepers;
    wait_queue_head_t wait;
};

// asmlinkage void __down_failed(void /* special register calling convention */);
// asmlinkage void __up_wakeup(void /* special register calling convention */);

static inline void sema_init (struct semaphore *sem, int val)
{
/*
 *	*sem = (struct semaphore)__SEMAPHORE_INITIALIZER((*sem),val);
 *
 * i'd rather use the more flexible initialization above, but sadly
 * GCC 2.7.2.3 emits a bogus warning. EGCS doesnt. Oh well.
 */
	atomic_set(&sem->count, val);
	sem->sleepers = 0;
	init_waitqueue_head(&sem->wait);
}

static inline void init_MUTEX (struct semaphore *sem)
{
	sema_init(sem, 1);
}

static inline void init_MUTEX_LOCKED (struct semaphore *sem)
{
	sema_init(sem, 0);
}



// TODO: 有待理解
static inline void up(struct semaphore * sem)
{
	// printk("up semaphore\n");
	// __asm__ __volatile__(
	// 	"# atomic up operation\n\t"
	// 	LOCK "incl %0\n\t"     /* ++sem->count */
	// 	"jle 2f\n"
	// 	"1:\n"
	// 	".section .text.lock,\"ax\"\n"
	// 	"2:\tcall __up_wakeup\n\t"
	// 	"jmp 1b\n"
	// 	".previous"
	// 	:"=m" (sem->count)
	// 	:"c" (sem)
	// 	:"memory");
}

static inline void down(struct semaphore * sem)
{
	// printk("down semaphore\n");
	// __asm__ __volatile__(
	// 	"# atomic down operation\n\t"
	// 	LOCK "decl %0\n\t"     /* --sem->count */
	// 	"js 2f\n"
	// 	"1:\n"
	// 	".section .text.lock,\"ax\"\n"
	// 	"2:\tcall __down_failed\n\t"
	// 	"jmp 1b\n"
	// 	".previous"
	// 	:"=m" (sem->count)
	// 	:"c" (sem)
	// 	:"memory");
}

#endif