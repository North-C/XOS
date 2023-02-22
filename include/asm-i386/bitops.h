/**
 * @file bitops.h
 * @brief 位图的操作，利用 bts 一类指令实现位的原子操作
 * @date 2022-05-24
 * 
 */
#ifndef _I386_BITOPS_H
#define _I386_BITOPS_H

#define LOCK_PREFIX    ""          // 在单处理器系统中，锁的前缀为空
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

    __asm__ __volatile__(LOCK_PREFIX 
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

    __asm__ __volatile__(LOCK_PREFIX
    "btsl %2, %1\n\tsbbl %0,%0"
    :"=r"(oldbit),"=m" (ADDR)
    :"Ir" (nr) :"memory");

    return oldbit;
}

// 生成一个nr对应位为1，而其他位为0的字节（共32位），通过按位与 将其应用于包含指定位号的字节
static __inline__ int constant_test_bit(int nr, const volatile void * addr)
{
    return ((1UL << (nr&31)) & (((const volatile unsigned int *) addr)[nr >> 5])) != 0;
}

// bt指令从第二操作数指定的位数组选出第一操作数指定的一个指定位，并且将该位的值存进标志寄存器的 CF 位。第二个指令 sbb 从第二操作数中减去第一操作数，再减去 CF 的值。
// 因此，这里将一个从给定位数组中的给定位号的值写进标志寄存器的 CF 位，并且执行 sbb 指令计算： 00000000 - CF，并将结果写进 oldbit 变量。
static __inline__ int variable_test_bit(int nr, volatile void * addr)
{
    int oldbit;

    __asm__ __volatile__(
        "btl %2,%1\n\tsbbl %0,%0"
        : "=r"(oldbit)
        : "m"(ADDR), "Ir" (nr));
    return oldbit;
}

// 知晓位数组中一个给定的位是否被置位
// __builtin_constant_p 是GCC扩展，可以判断是常量还是变量
#define test_bit(nr, addr) \
(__builtin_constant_p(nr) ? \
constant_test_bit((nr), (addr)) : \
variable_test_bit((nr), (addr)))

// 设置内存中的 bit 位
static __inline__ void set_bit(int nr, volatile void * addr)
{
	__asm__ __volatile__( LOCK_PREFIX
		"btsl %1,%0"
		:"=m" (ADDR)
		:"Ir" (nr));
}

/** 
 * ffz: find first zero in word
 */
static __inline__ unsigned long ffz(unsigned long word)
{
    // bsf --- Bit Scan Forward
    __asm__("bsfl %1,%0"
    :"=r" (word)
    :"r" (~word));
    return word;
}

static __inline__ void change_bit(int nr, volatile void * addr)
{
	__asm__ __volatile__( LOCK_PREFIX
		"btcl %1,%0"
		:"=m" (ADDR)
		:"Ir" (nr));
}

static __inline__ void clear_bit(int nr, volatile void * addr)
{
	__asm__ __volatile__( LOCK_PREFIX
		"btrl %1,%0"
		:"=m" (ADDR)
		:"Ir" (nr));
}

static __inline__ int test_and_change_bit(int nr, volatile void * addr)
{
	int oldbit;

	__asm__ __volatile__( LOCK_PREFIX
		"btcl %2,%1\n\tsbbl %0,%0"
		:"=r" (oldbit),"=m" (ADDR)
		:"Ir" (nr) : "memory");
	return oldbit;
}

#endif
