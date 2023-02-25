#ifndef __ARHC_I386_POSIX_TYPES_H
#define __ARHC_I386_POSIX_TYPES_H

/* 
 * 该文件给用户级程序使用，注意命名空间的污染
 */

typedef unsigned int __kernel_size_t;
typedef long __kernel_time_t;
typedef long __kernel_suseconds_t;
typedef int		__kernel_pid_t;
typedef unsigned short	__kernel_uid_t;
typedef unsigned short	__kernel_gid_t;
typedef unsigned int	__kernel_uid32_t;
typedef unsigned int	__kernel_gid32_t;
#endif