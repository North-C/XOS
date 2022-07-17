#ifndef _LINUX_DEBUG_H
#define _LINUX_DEBUG_H

#include <linux/stdio.h>

void panic(const char *fmt, ...);

// 调试函数
// 内核的BUG出现在 __FILE__文件， __LINE__行
#define BUG() do{ \
    printk("kernel BUG at %s:%d!\n", __FILE__, __LINE__); \
    *((char*)0) = 0;    \
}while(0);\
while(1);


#endif