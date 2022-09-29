#ifndef __LINUX_TYPES_H
#define __LINUX_TYPES_H

#include <asm-i386/types.h>
#include <asm-i386/posix_types.h>

typedef unsigned int size_t;

typedef __kernel_suseconds_t  suseconds_t;

#ifndef _TIME_T
#define _TIME_T
typedef __kernel_time_t time_t;
#endif


#endif