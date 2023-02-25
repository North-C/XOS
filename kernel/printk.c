#include <linux/linkage.h>
#include <linux/stdio.h>
#include <linux/console.h>

/* 在任何场景下都可以调用的输出函数
*/
// int printk(const char *fmt, ...)
// {
//     va_list args;           
//     uint32_t flags;         
//     int printed_len;        // 已输出的字符串长度
//     char *p;
//     static char printk_buf[1024];

//     va_start(args, fmt);
//     printed_len = vsnprintf(printk_buf, 1024, fmt, args);
//     print_str(printk_buf);
//     va_end(args);

// out:
//     return printed_len;
// }

void printk(const char* format, ...){
    va_list args;
    va_start(args, format);
    char buf[1024] = {0};
    vsprintf(buf, format, args);
    print_str(buf);

    va_end(args);
}