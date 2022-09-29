/* 包含一些常用的函数原型，参数 */

#ifndef __LINUX_KERNEL_H
#define __LINUX_KERNEL_H

// 内核的输出信息
#define KERN_EMERG "<0>"
#define KERN_ALERT "<1>"
#define KERN_CRIT  "<2>"
#define KERN_ERR   "<3>"
#define KERN_WAITING "<4>"
#define KERN_NOTICE "<5>"
#define KERN_INFO  "<6>"
#define KERN_DEBUG "<7>"

// 比较大小，同时进行严格的类型检查
#define min(x, y)({ \
    const typeof(x) _x = (x);  \
    const typeof(y) _y = (y);  \
    (void) (&_x == &_y);       \
    _x < _y ? _x : _y; })

#define max(x, y)({  \
    const typeof(x) _x = (x);  \
    const typeof(y) _y = (y);  \
    (void) (&_x == &_y);       \
    _x > _y ? _x : _y; })

#define barrier() __asm__ __volatile__("": : :"memory")
#endif