/* 1 boot.S --- 启动内核 */

MULTIBOOT_HEADER_MAGIC = 0x1BADB002   # Multiboot 魔数，规范决定默认值

MULTIBOOT_PAGE_ALIGN = 1 << 0      # flags的第0位，设置则表示所有引导模块必须在页面边界对齐
MULTIBOOT_MEM_INFO = 1 << 1    	# flags的 第1位通过 Multiboot 信息结构的 mem_* 域包括可用
# MULTIBOOT_HEADER_OFFSET = 1 << 16       # flags的第16位，设置则表示12-28个字段是有效的

MULTIBOOT_HEADER_FLAGS = MULTIBOOT_PAGE_ALIGN | MULTIBOOT_MEM_INFO

MULTIBOOT_HEADER_CHECKSUM = -(MULTIBOOT_HEADER_MAGIC + MULTIBOOT_HEADER_FLAGS)
# 规范定义，magic flags checksum的三者之和必须为 0

CODE_SELECTOR = 8

#include <linux/linkage.h>
.code32                     # 以32-bit的方式编译
.section .text.init         # 代码段从这里开始

/*
    Multiboot规范中，OS映像使用的头部之后，需要这样一个magic Multiboot header
; Multiboot 头的分布必须如下表所示：
; ----------------------------------------------------------
; 偏移量  类型  域名        备注
;   0     u32   magic       必需
;   4     u32   flags       必需
;   8     u32   checksum    必需
;
;   其他为非必需部分，用不到暂时省略。
*/
.long MULTIBOOT_HEADER_MAGIC         # magic，以此判断该image是否支持
.long MULTIBOOT_HEADER_FLAGS        # flags 
.long MULTIBOOT_HEADER_CHECKSUM     # checksum,校验和


.global start           # 内核代码的入口函数，提供给ld进行设置
.global flush    
.extern kern_entry      # 内核中C语言代码的入口函数
#.extern printf          # 输出方法
start:

    cli         # 在设置好自己的IDT之前，OS映像必须禁用中断
    /* ebx中保存 Multiboot information structure 的指针 */
    movl %ebx, (global_multiboot_info)      # 存入到glb_mboot_ptr变量当中
    movl $0xC0000000, %eax
    /* 初始化栈指针 */
    movl (stack_start), %esp        # multiboot 规范规定OS镜像必须创建一个堆栈
    subl %eax, %esp

    # movl $0x9f000, %esp           # 设置内核栈的地址
    
    movl $0, %ebp                # 修改栈帧指针为0
    call kern_entry

    # /* 输出暂停信息 */
    # pushl $halt_message
    # call printf
    
stop:
    hlt
    jmp stop

# halt_message:
#     .asciz "Halted"

flush:     # 刷新流水线
    ljmp $CODE_SELECTOR, $next
next:
    ret

/* 栈区域*/
.section  .data.init
    .long global_multiboot_info

ENTRY(stack_start)              # 设置栈的位置
#   .long SYMBOL_NAME(init_task_union)+8192    # task_struct和系统堆栈占 两个页面大小
     .rept 8192
     .long 0
     .endr
.previous           # 与section stack最顶上的section进行交换



 




