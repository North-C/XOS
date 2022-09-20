/* 端口的操作函数 */

#ifndef _ASM_IO_H
#define _ASM_IO_H

#include <linux/types.h>
#include <asm-i386/io.h>
#include <asm-i386/page.h>
/**
 * @brief 该文件包含 x86 的 IO 指令
 * 
 */

#define IO_SPACE_LIMIT 0xffff

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

// 通过 重复的跳转，达到延迟时间的效果
static inline void outb_p(uint8_t value, uint16_t port)
{
    asm volatile("outb %b0,%w1;\njmp 1f;\n1:\tjmp 1f;\n1:": : "a"(value), "Nd"(port));
}

static inline void outw_p(uint8_t value, uint16_t port)
{
    asm volatile("outw %w0,%w1;\njmp 1f;\n1:\tjmp 1f;\n1:": : "a"(value), "Nd"(port));
}

static inline void outl_p(uint8_t value, uint16_t port)
{
    asm volatile("outl %0,%w1;\njmp 1f;\n1:\tjmp 1f;\n1:": : "a"(value), "Nd"(port));
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

/**
 * @brief 将物理地址映射为虚拟地址，只用于有内核映射的地址
 * 
 * @param address 映射地址
 * @return void* 返回虚拟地址，是当前CPU对给定内存地址的映射
 */
static inline void * phys_to_virt(unsigned long address)
{
    return __va(address);
}

/**
 * @brief 将虚拟地址映射到物理地址
 * 
 * @param address 虚拟地址
 * @return unsigned long 物理地址
 */
static inline unsigned long virt_to_phys(volatile void * address)
{
    return __pa(address);
}


#ifdef CONFIG_HIGHMEM64G
#define page_to_phys(page)	((u64)(page - mem_map) << PAGE_SHIFT)
#else
#define page_to_phys(page)	((page - mem_map) << PAGE_SHIFT)
#endif

#define virt_to_bus virt_to_phys
#define bus_to_virt phys_to_virt
#define page_to_bus page_to_phys


#endif