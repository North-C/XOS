#ifndef _LINUX_STDIO_H
#define _LINUX_STDIO_H

#include <linux/types.h>
#include <linux/linkage.h>

#define MAX_LENGTH 1024

// C语言标准库的模式
#define va_list char*
#define va_start(ap, v)  ap=(va_list)&v         // 指向函数的第一个固定参数
#define va_arg(ap, type) *((type*)(ap+=4))   // 获取type类型的ap指向的参数
#define va_end(ap)     ap=NULL                  // 将参数指针置位NULL

long simple_strtol(const char* cp, char **endp, unsigned int base);
unsigned long simple_strtoul(const char* cp, char **endp, unsigned int base);

int printf(const char* format, ...);
int sprintf(char *buf, const char* format, ...);
int snprintf(char * buf, size_t size, const char *fmt, ...);
//int vsprintf(char *str, const char* format, va_list ap);
int vsprintf(char *buf, const char *fmt, va_list args);
int vsnprintf(char *buf, size_t size, const char *fmt, va_list args);
// int printk(const char *fmt, ...);
void printk(const char *fmt, ...);

#endif
