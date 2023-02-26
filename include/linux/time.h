#ifndef __LINUX_TIME_H
#define __LINUX_TIME_H

#include <linux/types.h>

struct timeval {
    time_t      tv_sec;     // 秒
    suseconds_t tv_usec;    // 微妙
};



#endif