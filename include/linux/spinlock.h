#ifndef __LINUX_SPINLOCK_H
#define __LINUX_SPINLOCK_H



#if (__GNUC__ > 2 || __GNUC_MINOR__ > 91)
  typedef struct { } rwlock_t;
  #define RW_LOCK_UNLOCKED (rwlock_t) { }
#else
  typedef struct { int gcc_is_buggy; } rwlock_t;
  #define RW_LOCK_UNLOCKED (rwlock_t) { 0 }
#endif

#define read_lock(lock)     (void)(lock)
#define read_unlock(lock)   do { } while(0)
#define write_lock(lock)     (void)(lock)
#define write_unlock(lock)   do { } while(0)


#endif