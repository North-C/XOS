#ifndef __ASM_SYSTEM_H
#define __ASM_SYSTEM_H



/* interrupt control.. */
#define __save_flags(x)		__asm__ __volatile__("pushfl ; popl %0":"=g" (x): /* no input */)
#define __restore_flags(x) 	__asm__ __volatile__("pushl %0 ; popfl": /* no output */ :"g" (x):"memory", "cc")
#define __cli() 		__asm__ __volatile__("cli": : :"memory")
#define __sti()			__asm__ __volatile__("sti": : :"memory")
/* used in the idle loop; sti takes one instruction cycle to complete */
#define safe_halt()		__asm__ __volatile__("sti; hlt": : :"memory")
#define __save_and_cli(x)	do { __save_flags(x); __cli(); } while(0);
#define __save_and_sti(x)	do { __save_flags(x); __sti(); } while(0);

#define cli() __cli()
#define sti() __sti()

/* For spinlocks etc */
#if 0
#define local_irq_save(x)	__asm__ __volatile__("pushfl ; popl %0 ; cli":"=g" (x): /* no input */ :"memory")
#define local_irq_set(x)	__asm__ __volatile__("pushfl ; popl %0 ; sti":"=g" (x): /* no input */ :"memory")
#else
#define local_irq_save(x)	__save_and_cli(x)   // 从堆栈中获取 eflags，保存到 x 中
#define local_irq_set(x)	__save_and_sti(x)
#endif

#define local_irq_restore(x)	__restore_flags(x)
#define local_irq_disable()	__cli()
#define local_irq_enable()	__sti()

#define mb() __asm__ __volatile__ ("lock; addl $0, 0(%%esp)": : :"memory")
#define rmb() mb()

#endif