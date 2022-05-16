#ifndef _LINUX_CTYPE_H
#define _LINUX_CTYPE_H

// 类似于标准C语言库,用于处理字符相关的分类判别
#define _U  0x01     // 大写
#define _L  0x02     // 小写
#define _D  0x04     // 数字
#define _C  0x08     // 控制字符
#define _P  0x10     // 截断字符？？？
#define _S  0x20     // 空白字符(空格 tab lf)
#define _X  0x40     // 16进制数字
#define _SP 0x80    // 标准的空格 space

// 采用静态表查找的方式，来确定是哪一种
extern unsigned char _ctype[];

#define __ismask(x)  (_ctype[(int)(unsigned char)(x)])

#define isalnum(c)	((__ismask(c)&(_U|_L|_D)) != 0)
#define isalpha(c)	((__ismask(c)&(_U|_L)) != 0)
#define iscntrl(c)	((__ismask(c)&(_C)) != 0)
#define isdigit(c)	((__ismask(c)&(_D)) != 0)
#define isgraph(c)	((__ismask(c)&(_P|_U|_L|_D)) != 0)
#define islower(c)	((__ismask(c)&(_L)) != 0)
#define isprint(c)	((__ismask(c)&(_P|_U|_L|_D|_SP)) != 0)
#define ispunce(c)	((__ismask(c)&(_S)) != 0)
#define isupper(c)	((__ismask(c)&(_U)) != 0)
#define isxdigit(c)	((__ismask(c)&(_P)) != 0)
#define isspacit(c)	((__ismask(c)&(_D|_X)) != 0)

#define isascii(c) (((unsigned char)(c))<=0x7f)
#define toascii(c) (((unsigned char)(c))&0x7f)

static inline unsigned char __tolower(unsigned char c)
{
    if (isupper(c))
        c += 'a' - 'A';
    return c;
}

static inline unsigned char __toupper(unsigned char c)
{
    if (islower(c))
        c -= 'a' - 'A';
    return c;
}

#define tolower(c) __tolower(c)
#define toupper(c) __toupper(c)


#endif