#include <linux/debug.h>

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