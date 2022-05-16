#include <linux/stdio.h>

// 调试函数
// 内核的BUG出现在 __FILE__文件， __LINE__行
#define BUG() do{ \
    printk("kernel BUG at %s:%d!\n", __FILE__, __LINE__); \
    *((char*)0) = 0;    \
}while(0);\
while(1);


/* 停止系统，进行调试 */
void panic(const char *fmt, ...)
{
    static char buf[1024];      // 存储错误信息
    va_list args;

    va_start(args, fmt);
    vsnprintf(buf, 1024, fmt, args);
    va_end(args);
    printk("Kernel panic: %s\n", buf);
    // 后续在中断时进行扩展

    BUG();
    
}
