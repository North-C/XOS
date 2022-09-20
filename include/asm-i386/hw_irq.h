#ifndef _ASM_HW_IRQ_H
#define _ASM_HW_IRQ_H
#include <linux/linkage.h>
#include <asm-i386/atomic.h>
#include <asm-i386/segment.h>

// 用于外部中断的 IDT向量的起始位置
#define FIRST_EXTERNAL_VECTOR 0x20

#define SYSCALL_VECTOR		0x80


extern atomic_t irq_err_count;
extern atomic_t irq_mis_count;

#define __STR(x) #x
#define STR(x) __STR(x)

#define SAVE_ALL \
	"cld\n\t" \
	"pushl %es\n\t" \
	"pushl %ds\n\t" \
	"pushl %eax\n\t" \
	"pushl %ebp\n\t" \
	"pushl %edi\n\t" \
	"pushl %esi\n\t" \
	"pushl %edx\n\t" \
	"pushl %ecx\n\t" \
	"pushl %ebx\n\t" \
	"movl $" STR(__KERNEL_DS) ",%edx\n\t" \
	"movl %edx,%ds\n\t" \
	"movl %edx,%es\n\t"

#define IRQ_NAME2(nr) nr##_interrupt(void)
#define IRQ_NAME(nr) IRQ_NAME2(IRQ##nr)

#define BUILD_IRQ(nr) \
asmlinkage void IRQ_NAME(nr); \
__asm__( \
"\n"__ALIGN_STR"\n" \
SYMBOL_NAME_STR(IRQ) #nr "_interrupt:\n\t" \
    "pushl $"#nr"-256\n\t" \
    "jmp common_interrupt");       
	// 中断处理会全部都进入一段公共中断请求的服务程序总入口common_interrupt

// 处理完后，形成为：
/*
	__asm__ ( \
	"\n" \
	"IRQ0x03_interrupt: \n\t" \
	"pushl $0x03 - 256 \n\t" \
	"jmp common_interrupt");
*/

// 构建总入口 common_interrupt 
#define BUILD_COMMON_IRQ() \
asmlinkage void call_do_IRQ(void); \
__asm__( \
	"\n" __ALIGN_STR"\n" \
	"common_interrupt:\n\t" \
	SAVE_ALL \
	SYMBOL_NAME_STR(call_do_IRQ)":\n\t" \
	"call " SYMBOL_NAME_STR(do_IRQ) "\n\t" \
	"jmp ret_from_intr\n");

/**
  __asm__ (\
  "\n" __ALIGN_STR"\n" \
  "common_interrupt:\n\t" \
  SAVE_ALL \         // SAVE_ALL 保存现场，将寄存器等保存在堆栈中，定义在 entry.S 中
  call_do_IRQ: \   // 调用 do_IRQ() 当中
  "call do_IRQ\n\t" \    
  "jmp ret_from_intr\n");   // 进入 ret_from_intr 当中继续执行
  
 */

#endif