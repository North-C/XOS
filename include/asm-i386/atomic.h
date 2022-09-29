#ifndef __ARCH_I386_ATOMIC_
#define __ARCH_I386_ATOMIC_


#ifdef CONFIG_SMP
#define LOCK "lock ; "
#else
#define LOCK ""
#endif

typedef struct { volatile int counter; } atomic_t;

#define atomic_set(v,i)		(((v)->counter) = (i))

#define atomic_read(v)		((v)->counter)

static __inline__ void atomic_inc(atomic_t *v)
{
    __asm__ __volatile__(
        LOCK "incl %0"
        : "=m" (v->counter)
        : "m" (v->counter));
}

#endif