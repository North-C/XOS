#include <asm-i386/hw_irq.h>
#include <linux/linkage.h>
#include <asm-i386/desc.h>
#include <linux/stdio.h>

asmlinkage int system_call(void);
asmlinkage void lcall7(void);
asmlinkage void lcall27(void);

struct desc_struct default_ldt[] = { {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} };

/* 创建 IDT，必须进行页对齐 */
struct desc_struct idt_table[256] __attribute__((__section__(".data.idt"))) = { {0, 0}, };

asmlinkage void divide_error(void);
asmlinkage void debug(void);
asmlinkage void nmi(void);
asmlinkage void int3(void);
asmlinkage void overflow(void);
asmlinkage void bounds(void);
asmlinkage void invalid_op(void);
asmlinkage void device_not_available(void);
asmlinkage void double_fault(void);
asmlinkage void coprocessor_segment_overrun(void);
asmlinkage void invalid_TSS(void);
asmlinkage void segment_not_present(void);
asmlinkage void stack_segment(void);
asmlinkage void general_protection(void);
asmlinkage void page_fault(void);
asmlinkage void coprocessor_error(void);
asmlinkage void simd_coprocessor_error(void);
asmlinkage void alignment_check(void);
asmlinkage void spurious_interrupt_bug(void);
asmlinkage void machine_check(void);


#define _set_gate(gate_addr,type,dpl,addr) \
do { \
  int __d0, __d1; \
  __asm__ __volatile__ ("movw %%dx,%%ax\n\t" \
	"movw %4,%%dx\n\t" \
	"movl %%eax,%0\n\t" \
	"movl %%edx,%1" \
	:"=m" (*((long *) (gate_addr))), \
	 "=m" (*(1+(long *) (gate_addr))), "=&a" (__d0), "=&d" (__d1) \
	:"i" ((short) (0x8000+(dpl<<13)+(type<<8))), \
	 "3" ((char *) (addr)),"2" (__KERNEL_CS << 16)); \
} while (0)

void set_intr_gate(unsigned int n, void *addr)
{
    _set_gate(idt_table+n, 14, 0, addr);
}

void set_trap_gate(unsigned int n, void *addr)
{
    _set_gate(idt_table+n, 15, 0, addr);
}

static void set_system_gate(unsigned int n, void *addr)
{
	_set_gate(idt_table+n,15,3,addr);
}

static void set_call_gate(void *a, void *addr)
{
	_set_gate(a,12,3,addr);
}

void common_temp0() {
    printk("0 error!!!!!!!!!!!!!!!!!!!\n");
    while(1);
}
void common_temp1() {
    printk("1 error!!!!!!!!!!!!!!!!!!!\n");
    while(1);
}
void common_temp2() {
    printk("2 error!!!!!!!!!!!!!!!!!!!\n");
    while(1);
}
void common_temp3() {
    printk("3 error!!!!!!!!!!!!!!!!!!!\n");
    while(1);
}
void common_temp4() {
    printk("4 error!!!!!!!!!!!!!!!!!!!\n");
    while(1);
}
void common_temp5() {
    printk("5 error!!!!!!!!!!!!!!!!!!!\n");
    while(1);
}
void common_temp6() {
    printk("6 error!!!!!!!!!!!!!!!!!!!\n");
    while(1);
}
void common_temp7() {
    printk("7 error!!!!!!!!!!!!!!!!!!!\n");
    while(1);
}
void common_temp8() {
    printk("8 error!!!!!!!!!!!!!!!!!!!\n");
    while(1);
}
void common_temp9() {
    printk("9 error!!!!!!!!!!!!!!!!!!!\n");
    while(1);
}
void common_temp10() {
    printk("10 error!!!!!!!!!!!!!!!!!!!\n");
    while(1);
}
void common_temp11() {
    printk("11 error!!!!!!!!!!!!!!!!!!!\n");
    while(1);
}
void common_temp12() {
    printk("12 error!!!!!!!!!!!!!!!!!!!\n");
    while(1);
}
void common_temp13() {
    printk("13 error!!!!!!!!!!!!!!!!!!!\n");
    while(1);
}
void common_temp14() {
    printk("\n14 error!!!!!!!!!!!!!!!!!!!\n");
    while(1);
}
void common_temp15() {
    printk("15 error!!!!!!!!!!!!!!!!!!!\n");
    while(1);
}
void common_temp16() {
    printk("16 error!!!!!!!!!!!!!!!!!!!\n");
    while(1);
}
void common_temp17() {
    printk("17 error!!!!!!!!!!!!!!!!!!!\n");
    while(1);
}
void common_temp18() {
    printk("18 error!!!!!!!!!!!!!!!!!!!\n");
    while(1);
}
void common_temp19() {
    printk("19 error!!!!!!!!!!!!!!!!!!!\n");
    while(1);
}

void trap_init(void)
{	
	// 暂时先用临时的处理程序替代
	set_trap_gate(0,common_temp0);
    set_trap_gate(1,common_temp1);
    set_intr_gate(2,common_temp2);
    set_system_gate(3,common_temp3);	/* int3-5 can be called from all */
    set_system_gate(4,common_temp4);
    set_system_gate(5,common_temp5);
    set_trap_gate(6,common_temp6);
    set_trap_gate(7,common_temp7);
    set_trap_gate(8,common_temp8);
    set_trap_gate(9,common_temp9);
    set_trap_gate(10,common_temp10);
    set_trap_gate(11,common_temp11);
    set_trap_gate(12,common_temp12);
    set_trap_gate(13,common_temp13);
    set_intr_gate(14,common_temp14);
    set_trap_gate(15,common_temp15);
    set_trap_gate(16,common_temp16);
    set_trap_gate(17,common_temp17);
    set_trap_gate(18,common_temp18);
    set_trap_gate(19,common_temp19);

//     set_system_gate(SYSCALL_VECTOR,&system_call);   // 系统调用向量的初始化

    /*
	 * 兼容使用调用门的其他系统，default LDT is a single-entry callgate to lcall7 for iBCS
	 * and a callgate to lcall27 for Solaris/x86 binaries
	 */
	// set_call_gate(&default_ldt[0],lcall7);     
	// set_call_gate(&default_ldt[4],lcall27);

	/*
	 * Should be a barrier for any external CPU state.
	 */
	// cpu_init();
    printk("trap_init done\n");
}


