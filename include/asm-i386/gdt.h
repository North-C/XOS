#ifndef I386_GDT_H
#define I386_GDT_H
#include <asm-i386/types.h>

/* 选择子字段  */
#define RPL0 0
#define RPL1 1
#define RPL2 2
#define RPL3 3

#define TI_GDT 0
#define TI_LDT 1


// 内核GDT描述符选择子
#define SELECTOR_K_CODE ((1<<3) + (TI_GDT<<2)+ RPL0)
#define SELECTOR_K_DATA ((2<<3) + (TI_GDT<<2)+ RPL0)
#define SELECTOR_K_STACK SELECTOR_K_DATA
#define SELECTOR_K_VIDEO ((3<<3) + (TI_GDT<<2)+ RPL0)

// 用户进程GDT描述符选择子
#define SELECTOR_U_CODE ((4<<3) + (TI_GDT<<2)+ RPL3)
#define SELECTOR_U_DATA ((5<<3) + (TI_GDT<<2)+ RPL3)
#define SELECTOR_U_STACK SELECTOR_U_DATA

/* GDT 描述符字段 */
#define DESC_G_BYTE     0
#define DESC_G_4K       1 
#define DESC_DB_16      0
#define DESC_DB_32      1
#define DESC_L          0         // 使用IA-32模式即可，置为1则为 64位代码
#define DESC_AVL        0      // CPU不需要用
#define DESC_P          1
#define DESC_DPL0       0       // 
#define DESC_DPL1       1
#define DESC_DPL2       2
#define DESC_DPL3       3   

/* 段类型的属性 */
#define DESC_S_SYS       0   
#define DESC_S_CODE      1              // 非系统段-代码段
#define DESC_S_DATA      DESC_S_CODE    // 非系统段-数据段
#define DESC_TYPE_CODE   8              // 1000 即 x=1,c=0,r=0,a=0 代码段可执行，非依从，不可读，access位清零
#define DESC_TYPE_DATA   2              // W = 1,可读写 E=0,向上扩展  
#define DESC_TYPE_TSS    9              // 1001 B=0,不忙

/* GDT 的高32位和低32位的部分字段设置 */
#define GDT_ATTR_HIGH ((DESC_G_4K << 7) + (DESC_DB_32 <<6) + (DESC_L << 5 ) +(DESC_AVL << 4))

#define GDT_CODE_ATTR_LOW_DPL0	 ((DESC_P << 7) + (DESC_DPL0 << 5) + (DESC_S_CODE << 4) + DESC_TYPE_CODE)
#define GDT_DATA_ATTR_LOW_DPL0	 ((DESC_P << 7) + (DESC_DPL0 << 5) + (DESC_S_DATA << 4) + DESC_TYPE_DATA)

#define GDT_ATTR_LOW_CODE_DPL3    ((DESC_P << 7) + (DESC_DPL3 << 5) + (DESC_S_CODE << 4) + DESC_TYPE_CODE )
#define GDT_ATTR_LOW_DATA_DPL3    ((DESC_P << 7) + (DESC_DPL3 << 5) + (DESC_S_DATA << 4) + DESC_TYPE_DATA )

/* TSS描述符属性 */
#define TSS_DESC_D  0

#define TSS_ATTR_HIGH ((DESC_G_4K << 7) + (TSS_DESC_D << 6) + (DESC_L << 5) + (DESC_AVL << 4) + 0x0)
#define TSS_ATTR_LOW ((DESC_P << 7) + (DESC_DPL_0 << 5) + (DESC_S_SYS << 4) + DESC_TYPE_TSS)
#define SELECTOR_TSS ((6 << 3) + (TI_GDT << 2 ) + RPL0)

/* IDT描述符 */
#define IDT_DESC_P      1
#define IDT_DESC_DPL0   0
#define IDT_DESC_DPL3   3
#define IDT_DESC_32_TYPE    0xE     // 32位的门

// IDT的高32位中设置P DPL 和TYPE
#define IDT_DESC_ATTR_DPL0  ( (IDT_DESC_P << 7) + (IDT_DESC_DPL0 << 5) + IDT_DESC_32_TYPE )
#define IDT_DESC_ATTR_DPL3  ( (IDT_DESC_P << 7) + (IDT_DESC_DPL3 << 5) + IDT_DESC_32_TYPE )

/* EFLAGS 字段 */
#define EFLAGS_MBS  (1 << 1)
#define EFLAGS_IF_1     (1 << 9)            // IF，0为关中断，1为开中断
#define EFLAGS_IF_0     0
#define EFLAGS_IOPL_3   (3 << 12)           // 用于测试用户程序在非系统调用下进行IO
#define EFLAGS_IOPL_0   (0 << 12)

// GDT的描述符结构
typedef struct gdt_desc {
    uint16_t low_limit_word;
    uint16_t low_base_word;
    uint8_t mid_base_byte;
    uint8_t low_attr_byte;
    uint8_t limit_high_attr_byte;
    uint8_t high_base_byte;
} gdt_desc_t;


#endif