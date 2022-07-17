#ifndef _LINUX_LIST_H
#define _LINUX_LIST_H

/* 双向链表的实现 */
struct list_head { 
    struct list_head *next, *prev;
};

#endif