#include <linux/string.h>
#include <linux/types.h>


/* 有整数转换为字符串 */
char * itoa(int number, char* dst, int base)
{
    char *p = dst;
    unsigned int digval;       // 余数
    char *first = dst;       // 方便后续调整顺序
    char temp;

    if(number < 0){
        *p++ = '-';
        first++;
        number = 0 - number;
    }

    while(number > 0){
        digval = number % base;
        number = number / base;

        if(digval < 10){
            *p++ = digval + '0';
        }
        else{
            *p++ = digval - 10 + 'A';
        }
    }

    *p-- = '\0';

    // 翻转数字的顺序
    while(first < p){
        temp = *first;
        *first = *p;
        *p = temp;

        first++;  p--;
    }

    return dst;
}

/* 无符号整数转换为字符串 */
char * uitoa(unsigned int number, char * dst, int base)
{
    char *p = dst;
    unsigned int digval;
    char *first = dst;       // 方便后续调整顺序
    char temp;

    while(number > 0){
        digval = number % base;
        number = number / base;

        if(digval < 10){
            *p++ = digval + '0';
        }
        else{
            *p++ = digval - 10 + 'A';
        }
    }
    if (p==dst){
        *p++ = '0';
    }
    *p-- = '\0';

    // 翻转数字的顺序
    while(first < p){
        temp = *first;
        *first = *p;
        *p = temp;

        first++;  p--;
    }
    
    return dst;
}


/* 计算字符串 s 的长度 */ 
__kernel_size_t strlen(const char * s)
{
    const char * sc;

    for(sc = s; *sc!='\0'; ++sc)
        ;
    return sc - s;
}

/*  */
__kernel_size_t strnlen(const char *s , __kernel_size_t count){
    const char * sc ;
    for (sc = s ; count-- && *sc != '\0' ; ++sc)
        ;
        
    return sc - s;
}

/* 复制字符串 src 到 dest */
char * strcpy(char *dest, const char *src)
{
    char *tmp = dest;

    while((*dest++ = *src++) != '\0')
        ;
    return tmp;
}

// char * strncpy(char *dest, const char *src, __kernel_size_t count)
// {
//     return NULL;
// }
/* 将字符串 src 拼接到 dest末尾 */
char* strcat(char *dest, const char *src)
{
    char *tmp = dest;

    while(*dest)
        dest++;
    while((*dest++ = *src++)!='\0')
        ;
    return dest;
}

// char* strncat(char *, const char *, __kernel_size_t)
// {
//     return NULL;
// }

/* 比较 cs 和 ct 两个字符串，cs大于ct则为整数，否则为负数 */
int strcmp(const char *cs, const char *ct)
{
    int8_t res;

    while(1){
        if((res = *cs - *ct++) != 0 || !*cs++)
            break;
    }

    return res;
}

// int strncmp(const char *, const char *, __kernel_size_t)
// {
//     return 0;
// }

/* 从左至右，查找字符ch在字符串 s 当中最先出现的地址 */
char * strchr(const char *s, int ch)
{
    while(*s != '\0'){
        if(*s == ch){
            return (char*)s;
        }
        s++;
    }
    return NULL;
}

/* 查找字符ch在字符串 s 当中最后出现的位置 */
char * strrchr(const char *s, int ch)
{
    const char *p = s + strlen(s);
    do {
        if (*p == (char)ch)
            return (char *)p;
    } while (--p >= s);
    return NULL;
}


// 用给定的值填充内存区域
void * memset(void *s, int c, __kernel_size_t count)
{
    char *xs = (char *)s;

    while(count--)
        *xs++ = c;
    
    return s;
}

void * memcpy(void *dest, const void *src, __kernel_size_t count)
{
    char *d =(char *) dest, *s = (char *) src;

    while(count--)
        *d++ = *s++;
    
    return dest;
}

int memcmp(const void *cs, const void *ct, __kernel_size_t count)
{ 
    const unsigned char *s = cs, *t = ct;
    int res = 0;

    while(count--){
        if( (res = *s - *t) != 0)
            break;
        s++; t++;
    }
    return res;
}

// void * memchr(const void *,int,__kernel_size_t)
// {
//     return NULL;
// }

