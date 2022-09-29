#include <linux/irq.h>
#include <linux/cache.h>
#include <asm-i386/types.h>
#include <linux/linkage.h>
#include <asm-i386/ptrace.h>
#include <linux/stdio.h>
#include  <asm-i386/signal.h>
#include <asm-i386/errno.h>
#include <asm-i386/smp.h>
#include <asm-i386/hardirq.h>
#include <linux/interrupt.h>
#include <linux/kernel_stat.h>

extern struct kernel_stat kstat;

// 不产生控制码的响应
static void enable_none(unsigned int irq) { }
static unsigned int startup_none(unsigned int irq) { return 0; }
static void disable_none(unsigned int irq) { }
static void ack_none(unsigned int irq)
{
    printk("unexpected IRQ trap at vector %02x\n", irq);
}

#define shutdown_none	disable_none
#define end_none	enable_none

struct hw_interrupt_type no_irq_type = {
	"none",
	startup_none,
	shutdown_none,
	enable_none,
	disable_none,
	ack_none,
	end_none
};

irq_desc_t irq_desc[NR_IRQS] __cacheline_aligned =
        { [0 ... NR_IRQS-1] = { 0, &no_irq_type, NULL, 0, SPIN_LOCK_UNLOCKED}};


volatile unsigned long irq_err_count;

int setup_irq(unsigned int irq, struct irqaction * new)
{
	int shared = 0;
	unsigned long flags;
	struct irqaction *old, **p;
	irq_desc_t *desc = irq_desc + irq;

	/*
	 * Some drivers like serial.c use request_irq() heavily,
	 * so we have to be careful not to interfere with a
	 * running system.
	 */
	if (new->flags & SA_SAMPLE_RANDOM) {  // SA_SAMPLE_RANDOM 设置为 1 即可
		/*
		 * This function might sleep, we want to call it first,
		 * outside of the atomic block.
		 * Yes, this might clear the entropy pool if the wrong
		 * driver is attempted to be loaded, without actually
		 * installing a new handler, but is this really a problem,
		 * only the sysadmin is able to do this.
		 */
		// rand_initialize_irq(irq);   	// 伪随机数生成
	}

	/*
	 * The following block of code has to be executed atomically
	 */
	spin_lock_irqsave(&desc->lock,flags);
	p = &desc->action;
	if ((old = *p) != NULL) {
		// 检查是否允许共用一个通道，只有新旧中断都允许才可以共享
		if (!(old->flags & new->flags & SA_SHIRQ)) {  
			spin_unlock_irqrestore(&desc->lock,flags);
			return -EBUSY;
		}

		/* add new interrupt at end of irq queue */
		do {
			p = &old->next;
			old = *p;
		} while (old);
		shared = 1;
	}

	*p = new;

	if (!shared) {			// 第一个加入队列的请求，进行一些初始化即可
		desc->depth = 0;
		desc->status &= ~(IRQ_DISABLED | IRQ_AUTODETECT | IRQ_WAITING);
		desc->handler->startup(irq);
	}
	spin_unlock_irqrestore(&desc->lock,flags);

	// register_irq_proc(irq);
	return 0;
}

/**
 * @brief 分配一个 中断项
 * 
 * @param irq 分配的中断项，中断请求队列的序号，也就是中断请求号
 * @param handler 中断处理程序
 * @param irqflags 中断类型标志
 * @param devname 设备的 ascci 名称
 * @param dev_id 传输给处理程序的设备 id
 * @return int 
 */

int request_irq(unsigned int irq, 
		void (*handler)(int, void *, struct pt_regs *), 
		unsigned long irqflags, 
		const char * devname, 
		void *dev_id)
{
	int retval;
	struct irqaction * action;

#if 1
	/*
	 * 完整性检查 Sanity-check: shared interrupts should REALLY pass in
	 * a real dev-ID, otherwise we'll have trouble later trying
	 * to figure out which interrupt is which (messes up the
	 * interrupt freeing logic etc).
	 */
	if (irqflags & SA_SHIRQ) {
		if (!dev_id)
			printk("Bad boy: %s (at 0x%x) called us without a dev_id!\n", devname, (&irq)[-1]);   // irq 是最后入栈的参数，在它的后面入栈的是 函数的返回地址
	}
#endif

	if (irq >= NR_IRQS)
		return -EINVAL;
	if (!handler)
		return -EINVAL;
	// TODO: 分配内存
	// action = (struct irqaction *)
	// 		kmalloc(sizeof(struct irqaction), GFP_KERNEL);
	if (!action)
		return -ENOMEM;

	action->handler = handler;
	action->flags = irqflags;
	action->mask = 0;
	action->name = devname;
	action->next = NULL;
	action->dev_id = dev_id;

	retval = setup_irq(irq, action);   // 将 action 加入到中断请求队列
	// if (retval)
	// 	kfree(action);
	return retval;
}

int handle_IRQ_event(unsigned int irq, struct pt_regs * regs, struct irqaction * action)
{
	int status;
	int cpu = smp_processor_id();

	irq_enter(cpu, irq);

	status = 1; 

	if(!(action->flags & SA_INTERRUPT))
		__sti();   // 开中断
	
	do{
		status |= action->flags;
		action->handler(irq, action->dev_id, regs);
		action = action->next;
	} while(action);

	// if(status & SA_SAMPLE_RANDOM)
	// 	add_interrupt_randomness(irq);
	__cli();	  // 关中断

	irq_exit(cpu, irq);

	return status;
}

/**
 * @brief 
 * 
 * @param regs 系统堆栈中返回地址以上的数据结构映像
 * @return 
 */
asmlinkage unsigned int do_IRQ(struct pt_regs regs)
{
	printk("do_IRQ...\n");

	int irq = regs.orig_eax & 0xff;
	int cpu = smp_processor_id();    // 专门为 SMP 结构设计，单处理器时只返回 0
	irq_desc_t *desc = irq_desc + irq;
	struct irqaction * action;
	unsigned int status;

	kstat.irqs[cpu][irq]++;
	spin_lock(&desc->lock); 	// 为多处理器考虑
	desc->handler->ack(irq);    // 向 CPU 确认，表示正在处理

	// REPLAY 是指当CPU开启该队列的服务时，看到这个标志位而补上一次中断服务。重新发送之前的中断请求
	// WAITING 是 标记已经被检测到的 IRQ
	status = desc->status & ~(IRQ_REPLAY | IRQ_WAITING);
	status |= IRQ_PENDING;   // 表示希望处理该 IRQ
	
	// IRQ 被禁止,则无法调用 action 执行动作
	action = NULL;
	if(!(status & (IRQ_DISABLED | IRQ_INPROGRESS))){   // 没有被禁止，也不在进行中（后者为多处理器结构设置）
		action = desc->action;
		status &= ~IRQ_PENDING;   // 希望处理 IRQ
		status |= IRQ_INPROGRESS;	// 正在处理
	}
	desc->status = status;

	if(!action)
		goto out;
	// 边缘触发的中断需要记住未决的事件，在do_IRQ 或者 handler 当中时，允许第二个中断到达，但仅处理第二个，其后的会被抛弃。
	for(;;){
		spin_unlock(&desc->lock);
		handle_IRQ_event(irq, &regs, action);   // 具体的中断服务
		spin_lock(&desc->lock);

		if(!(desc->status & IRQ_PENDING))
			break;
		desc->status &= ~IRQ_PENDING;
	}
	desc->status &= ~IRQ_INPROGRESS;

out:
	desc->handler->end(irq);   // 处理那些中断被禁用，但是其处理程序还在运行的情况
	spin_unlock(&desc->lock);

	if(softirq_pending(cpu))   // 检查是否有软中断在等待执行
		do_softirq();
	
	return 1;
}
