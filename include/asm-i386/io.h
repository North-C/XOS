/* 端口的操作函数 */

#ifndef _ASM_IO_H
#define _ASM_IO_H

#include <linux/types.h>

/* 向端口port中写入一个字节data */
static inline void outb(uint16_t port, uint8_t data)
{
    asm volatile("outb %b0, %w1": :"a"(data), "Nd"(port));
}

/* 向端口port中写入一个字data */
static inline void outw(uint16_t port, uint16_t data)
{
    asm volatile("outw %w0, %w1": :"a"(data), "Nd"(port));
}

/* 向端口port中写入从addr地址开始的count个字节 */
static inline void outsw(uint16_t port, void *addr, uint32_t count)
{
    asm volatile("cld; rep outsw ": "+S"(addr), "+c"(count) : "d"(port));
}


/* 从端口port中获取一个字节 */
static inline uint8_t inb(uint16_t port)
{
    uint8_t data;
    asm volatile("inb %w1, %b0": "=a"(data) : "Nd"(port));
    return data;
}

static inline uint16_t inw(uint16_t port)
{
    uint16_t data;
    asm volatile("inw %w1, %w0": "=a"(data) : "Nd"(port));
    return data;
}

/* 从端口port中读入count个字节获取一个字节 */
static inline void insw(uint16_t port, void *addr, uint32_t count)
{
    asm volatile("cld; rep insw": "+D"(addr), "+c"(count) : "d" (port) : "memory");
}


#endif