#ifndef __I386_DIV64
#define __I386_DIV64

/* 除法计算： n 除数， base 进制基数(2,8,10,16等)
* 1. 将 n 拆分为 高__high 和 低__low 两部分
* 2. 调用 divl 除法指令
* 3. 重新将 商 整合为结果，返回 余数
*/
#define do_div(n,base) ({ \
    unsigned long __upper, __low, __high, __mod; \
    asm("":"=a" (__low), "=d" (__high): "A" (n)); \
    __upper = __high; \
    if(__high) { \
        __upper = __high % (base); \
        __high = __high / (base); \
    } \
    asm("divl %2" : "=a" (__low), "=d" (__mod) : "rm" (base), "0" (__low), "1"(__upper)); \
    asm("" : "=A" (n) : "a" (__low), "d" (__high)); \
    __mod; \
})

#endif