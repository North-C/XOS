/**
 * @file bitops.h
 * @brief 位图的操作，利用 bts 一类指令实现位的原子操作
 * @date 2022-05-24
 * 
 */
#ifndef _I386_BITOPS_H
#define _I386_BITOPS_H

#define LOCK_PRTEFIX    ""          // 在单处理器系统中，锁的前缀为空
// #define LOCK_PREFIX "lock ;"        // 多处理器系统

#define ADDR    (*(volatile long * ) addr)      

/**
 * @brief 清除一个bit位，并返回其旧值，原子化操作
 * 
 * @param nr 要清除的bit
 * @param addr 开始计算的地址  
 * @return int
 */
static __inline__ int test_and_clear_bit(int nr, volatile void * addr)
{
    int oldbit;

    __asm__ __volatile__(LOCK_PRTEFIX 
        "btrl %2, %1\n\tsbbl %0,%0"
        :"=r" (oldbit), "=m" (ADDR)
        :"Ir" (nr) : "memory");

    return oldbit;
}
/**
 * @brief 清除一个bit位，并返回其旧值，非原子操作
 * 
 * @param nr 要清除的bit
 * @param addr 开始计算的地址  
 * @return int 旧值,原值为0则返回0，原值为1则返回-1(其补码就是全1)
 */
static __inline__ int __test_and_clear_bit(int nr, volatile void * addr)
{
    int oldbit;

	__asm__(
		"btrl %2,%1\n\tsbbl %0,%0"
		:"=r" (oldbit),"=m" (ADDR)
		:"Ir" (nr));
	return oldbit;
}


static __inline__ int test_and_set_bit(int nr, volatile void * addr)
{
    int oldbit;

    __asm__ __volatile__(LOCK_PRTEFIX
    "btsl %2, %1\n\tsbbl %0,%0"
    :"=r"(oldbit),"=m" (ADDR)
    :"Ir" (nr) :"memory");

    return oldbit;
}

#endif
