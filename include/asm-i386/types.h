#ifndef _I386_TYPES_H
#define _I386_TYPES_H

/* 1. _xx 不会影响POSIX 的命名空间，在导出到用户空间的头文件中使用 
    2. 与传统的旧的编译器兼容， -traditional */
typedef __signed__ char __s8;
typedef unsigned char __u8;

typedef __signed__ short __s16;
typedef unsigned short __u16;

typedef __signed__ int __s32;
typedef unsigned int __u32;

typedef __signed__ long long __s64;
typedef unsigned long long __u64;


/* 内核空间中使用 */
typedef signed char int8_t;
typedef unsigned char uint8_t;

typedef signed short int16_t;
typedef unsigned short uint16_t;

typedef signed int int32_t;
typedef unsigned int uint32_t;

typedef signed long long int64_t;
typedef unsigned long long  uint64_t;

#define true 1
#define false 0

#define NULL ((void*)0)
#define bool

#define DIV_ROUND_UP(X, STEP)   ((X + STEP - 1) / (STEP))   // 向上整除

#define PG_SIZE 4096

#endif