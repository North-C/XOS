#ifndef _LINUX_LINKAGE_H
#define _LINUX_LINKAGE_H

// 编译.S文件
#define asmlinkage __attribute__((regparm(0)))  // 告知编译器去CPU堆栈中寻找函数参数,而非寄存器
#define SYMBOL_NAME_STR(X) #X
#define SYMBOL_NAME(X) X
#define __ALIGN .align 4,0x90
#define __ALIGN_STR ".align 4,0x90"
#define ALIGN __ALIGN
#define ENTRY(name) \
  .globl name; \
  ALIGN; \
  name:
#endif
