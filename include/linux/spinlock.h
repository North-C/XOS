#ifndef __LINUX_SPINLOCK_H
#define __LINUX_SPINLOCK_H

#include <asm-i386/system.h>

#define spin_lock_irqsave(lock, flags)  do{ local_irq_save(flags); spin_lock(lock); } while(0)
#define spin_lock_irq(lock)  do { local_irq_disable(); spin_lock(lock); } while(0)
#define spin_lock_bh(lock)  do { local_bh_disable(); spin_lock(lock); } while(0)

#define read_lock_irqsave(lock, flags)  do { local_irq_save(flags); read_lock(lock); } while(0)
#define read_lock_irq(lock)  do { local_irq_disable();  read_lock(lock); }while(0)
#define read_lock_bh(lock) do { local_bh_disable(); read_lock(); }while(0)

#define write_lock_irqsave(lock, flags)  do { local_irq_save(flags); write_lock(lock); } while(0)
#define write_lock_irq(lock)  do { local_irq_disable();  write_lock(lock); }while(0)
#define write_lock_bh(lock) do { local_bh_disable(); write_lock(); } while(0)

#define spin_unlock_irqrestore(lock, flags) do { spin_unlock(lock); local_irq_restore(flags); } while(0)
#define spin_unlock_irq(lock)  do { spin_lock(lock); local_irq_enable(); } while(0)
#define spin_unlock_bh(lock) do { spin_unlock(lock); local_bh_enable(); } while(0)

#define read_unlock_irqrestore(lock, flags)	do { read_unlock(lock);  local_irq_restore(flags); } while (0)
#define read_unlock_irq(lock)			do { read_unlock(lock);  local_irq_enable();       } while (0)
#define read_unlock_bh(lock)			do { read_unlock(lock);  local_bh_enable();        } while (0)

#define write_unlock_irqrestore(lock, flags)	do { write_unlock(lock); local_irq_restore(flags); } while (0)
#define write_unlock_irq(lock)			do { write_unlock(lock); local_irq_enable();       } while (0)
#define write_unlock_bh(lock)			do { write_unlock(lock); local_bh_enable();        } while (0)
#define spin_trylock_bh(lock)			({ int __r; local_bh_disable();\
						__r = spin_trylock(lock);      \
						if (!__r) local_bh_enable();   \
						__r; })


typedef struct {} rwlock_t;
#define RW_LOCK_UNLOCKED (rwlock_t) {}

#define rwlock_init(lock) do { } while(0)
#define read_lock(lock)     (void)(lock)
#define read_unlock(lock)   do { } while(0)
#define write_lock(lock)     (void)(lock)
#define write_unlock(lock)   do { } while(0)

// 单核锁
typedef struct {
	volatile unsigned long lock;
} spinlock_t;
#define SPIN_LOCK_UNLOCKED (spinlock_t) { 0 }

#define spin_lock_init(x)	do { (x)->lock = 0; } while (0)
#define spin_is_locked(lock)	(test_bit(0,(lock)))
#define spin_trylock(lock)	(!test_and_set_bit(0,(lock)))

#define spin_lock(x)		do { (x)->lock = 1; } while (0)
#define spin_unlock_wait(x)	do { } while (0)
#define spin_unlock(x)		do { (x)->lock = 0; } while (0)

#endif