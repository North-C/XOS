#ifndef __LIB_STDIO_H
#define __LIB_STDIO_H
#include <asm-i386/types.h>

typedef char* va_list;

#define va_start(ap, v) ap = (va_list)&v
#define va_arg(ap, l)  *((l*)(ap+=4))
#define va_end(ap) ap=NULL


extern int vsprintf(char *s, const char *format, va_list args);
extern int sprintf(char *buf, const char *format, ...);
extern int printf(char& str, const char *format, ...);
extern int print_int(int);

#endif


