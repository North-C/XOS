#ifndef _I386_STRNG_H
#define _I386_STRNG_H

#include <asm-i386/posix_types.h>
#include <linux/types.h>

/* 
* Linux lib/string.c中使用C语言来实现，而在include/asm-i386/string.h中使用内联汇编在特定架构下的实现是优化版本。
* 在include/linux/string.h中 通过统一接口来调用不同架构的具体实现
*/

// 字符串处理函数 
char * strcpy(char *,const char *);
char * strncpy(char *,const char *, __kernel_size_t);

char * strcat(char *, const char *);
char * strncat(char *, const char *, __kernel_size_t);

int strcmp(const char *,const char *);
int strncmp(const char *,const char *,__kernel_size_t);

char * strchr(const char *,int);
char * strrchr(const char *,int);
char * strstr(const char *,const char *);

__kernel_size_t strlen(const char *);
__kernel_size_t strnlen(const char *,__kernel_size_t);

// 内存处理函数
void * memset(void *,int,__kernel_size_t);
void * memcpy(void *,const void *,__kernel_size_t);
int memcmp(const void *,const void *,__kernel_size_t);
void * memchr(const void *,int,__kernel_size_t);


char * itoa(int number, char* dst, int base);
char * uitoa(unsigned int number, char * dst, int base);
#endif
