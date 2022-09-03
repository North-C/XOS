#ifndef __ARCH_I386_ATOMIC_
#define __ARCH_I386_ATOMIC_

typedef struct { volatile int counter; } atomic_t;

#define atomic_set(v,i)		(((v)->counter) = (i))

#endif