#include <linux/irq.h>
#include <linux/timex.h>
#include <linux/spinlock.h>
#include <linux/timex.h>

#include <asm-i386/irq.h>
#include <asm-i386/io.h>
#include <asm-i386/hw_irq.h>
#include <asm-i386/delay.h>
#include <asm-i386/atomic.h>
#include <asm-i386/desc.h>

BUILD_COMMON_IRQ()

#define BI(x,y) \
	BUILD_IRQ(x##y)

#define BUILD_16_IRQS(x) \
	BI(x,0) BI(x,1) BI(x,2) BI(x,3) \
	BI(x,4) BI(x,5) BI(x,6) BI(x,7) \
	BI(x,8) BI(x,9) BI(x,a) BI(x,b) \
	BI(x,c) BI(x,d) BI(x,e) BI(x,f)

/*
 * ISA PIC or low IO-APIC triggered (INTA-cycle or APIC) interrupts:
 * (these are usually mapped to vectors 0x20-0x2f)
 */
BUILD_16_IRQS(0x0)

#define IRQ(x,y) IRQ##x##y##_interrupt     // ## 用于连接字符串，在 gcc 预处理之后替换为所需的文字,例如 IRQ0x00_interrupt

#define IRQLIST_16(x) \
	IRQ(x,0), IRQ(x,1), IRQ(x,2), IRQ(x,3), \
	IRQ(x,4), IRQ(x,5), IRQ(x,6), IRQ(x,7), \
	IRQ(x,8), IRQ(x,9), IRQ(x,a), IRQ(x,b), \
	IRQ(x,c), IRQ(x,d), IRQ(x,e), IRQ(x,f)

// 中断服务程序的入口地址
void (*interrupt[NR_IRQS])(void) = {
    IRQLIST_16(0x0),
};

// 8259A irq 控制器的掩码
static unsigned int cached_irq_mask = 0xffff;

#define __byte(x, y) (((unsigned char *)&(y))[x])
#define cached_21    (__byte(0,cached_irq_mask))
#define cached_A1	 (__byte(1,cached_irq_mask))

/**
 * @brief 传统的 8259A 编程控制器
 * 
 */
spinlock_t i8259A_lock = SPIN_LOCK_UNLOCKED;

void disable_8259A_irq(unsigned int irq)
{
	unsigned int mask = 1 << irq;
	unsigned long flags;

	spin_lock_irqsave(&i8259A_lock, flags);
	cached_irq_mask |= mask;
	if (irq & 8)
		outb(cached_A1,0xA1);
	else
		outb(cached_21,0x21);
	spin_unlock_irqrestore(&i8259A_lock, flags);
}

void enable_8259A_irq(unsigned int irq)
{
	unsigned int mask = ~(1 << irq);
	unsigned long flags;

	spin_lock_irqsave(&i8259A_lock, flags);
	cached_irq_mask &= mask;
	if (irq & 8)
		outb(cached_A1,0xA1);
	else
		outb(cached_21,0x21);
	spin_unlock_irqrestore(&i8259A_lock, flags);
}

static void end_8259A_irq (unsigned int irq)
{
	if (!(irq_desc[irq].status & (IRQ_DISABLED|IRQ_INPROGRESS)))
		enable_8259A_irq(irq);
}

#define shutdown_8259A_irq	disable_8259A_irq

void mask_and_ack_8259A(unsigned int);

static unsigned int startup_8259A_irq(unsigned int irq)
{ 
	enable_8259A_irq(irq);
	return 0; /* never anything pending */
}

// 该函数假定被访问的很少，它在 i8259A 的寄存器间的切换是很慢的
static inline int i8259A_irq_real(unsigned int irq)
{
	int value;
	int irqmask = 1<<irq;

	if(irq < 8){
		outb(0x0B,0x20);
		value = inb(0x20) & irqmask;
		outb(0x0A,0x20);
		return value;
	}
	outb(0x0B, 0xA0);
	value = inb(0xA0) & (irqmask >> 8);
	outb(0x0A,0xA0);
	return value;
}

// 8259A 有一些必须要执行的步骤，先使用掩码，其次发送 EOI信号，且顺序很重要
void mask_and_ack_8259A(unsigned int irq)
{
	unsigned int irqmask = 1 << irq;
	unsigned long flags;

	spin_lock_irqsave(&i8259A_lock, flags);

	if(cached_irq_mask & irqmask)
		goto spurious_8259A_irq;
	cached_irq_mask |= irqmask;

handle_real_irq:
	if(irq & 8){
		inb(0xA1);
		outb(cached_A1,0xA1);
		outb(0x60+(irq&7),0xA0);
		outb(0x62, 0x20);
	} else{
		inb(0x21);
		outb(cached_21,0x21);
		outb(0x60+irq,0x20);
	}
	spin_unlock_irqrestore(&i8259A_lock, flags);
	return;

spurious_8259A_irq:   // 找不到中断源的中断
	if(i8259A_irq_real(irq))
		goto handle_real_irq; 
	{
		static int spurious_irq_mask;
		// 确认中断是 spurious 
		if(!(spurious_irq_mask & irqmask)) {
			printk("spurious 8259A interrupt: IRQ%d.\n", irq);
			spurious_irq_mask |= irqmask;
		}
		atomic_inc(&irq_err_count);

		goto handle_real_irq;  // 理论上不需要这一步，但是这样做不会有问题，且简单
	}
	
}

static struct hw_interrupt_type i8259A_irq_type = {
	"XT-PIC",
	startup_8259A_irq,
	shutdown_8259A_irq,
	enable_8259A_irq,
	disable_8259A_irq,
	mask_and_ack_8259A,
	end_8259A_irq,
	NULL
};

/**
 * @brief 初始化 8259A 中断控制器
 * 
 * @param auto_eoi 
 */
void init_8259A(int auto_eoi)
{
    unsigned long flags;

	spin_lock_irqsave(&i8259A_lock, flags);

	outb(0xff, 0x21);	/* mask all of 8259A-1 ，送数据到 OCW1，屏蔽所有外部中断*/
	outb(0xff, 0xA1);	/* mask all of 8259A-2 ，送数据到 OCW1，屏蔽所有外部中断*/

	/*
	 * outb_p - this has to work on a wide range of PC hardware.
	 */
	outb_p(0x11, 0x20);	/* ICW1: select 8259A-1 init ，0x11 = 0001_0001，外部中断请求信号为上升沿有效，系统中有多片 8259A 级联，还表示要向 ICW4 送数据 */
	outb_p(0x20 + 0, 0x21);	/* ICW2: 8259A-1 IR0-7 mapped to 0x20-0x27 ，0x20 = 0010_0000，设置中断向量 */
	outb_p(0x04, 0x21);	/* 8259A-1 (the master) has a slave on IR2，0000_0100，主片使用 IRQ2 连接从片 */
	if (auto_eoi)
		outb_p(0x03, 0x21);	/* master does Auto EOI */
	else
		outb_p(0x01, 0x21);	/* master expects normal EOI */

	outb_p(0x11, 0xA0);	/* ICW1: select 8259A-2 init，初始化从片 */
	outb_p(0x20 + 8, 0xA1);	/* ICW2: 8259A-2 IR0-7 mapped to 0x28-0x2f */
	outb_p(0x02, 0xA1);	/* 8259A-2 is a slave on master's IR2，0x20 = 0000_0010，指定主片的哪个接口连接从片 */
	outb_p(0x01, 0xA1);	/* (slave's support for AEOI in flat mode
				    is to be investigated)，表示 x86 处理器 */

	if (auto_eoi)
		/*
		 * in AEOI mode we just have to mask the interrupt
		 * when acking.
		 */
		i8259A_irq_type.ack = disable_8259A_irq;
	else
		i8259A_irq_type.ack = mask_and_ack_8259A;

	udelay(100);		/* wait for 8259A to initialize */

	outb(cached_21, 0x21);	/* restore master IRQ mask */
	outb(cached_A1, 0xA1);	/* restore slave IRQ mask */

	spin_unlock_irqrestore(&i8259A_lock, flags);
}


void init_ISA_irqs(void)
{
    int i;

    init_8259A(0);
    
    for(i = 0; i < NR_IRQS; i++) {
        irq_desc[i].status = IRQ_DISABLED;
        irq_desc[i].action = 0;
        irq_desc[i].depth = 1;

        if(i < 16){     // 将开头的 16个中断向量的 处理程序指向 i8259A_irq_type
            irq_desc[i].handler = &i8259A_irq_type;
        } else {
            irq_desc[i].handler = &no_irq_type;
        }
    }
}

void init_IRQ(void)
{
    int i;

// #ifndef CONFIG_X86_VISWS_APIC
 	init_ISA_irqs();
// #else
//	init_VISWS_APIC_irqs();
// #endif

    /**
     * 覆盖所有的向量空间，设置其对应的服务程序
     */
    for (i = 0; i < NR_IRQS; i++) {
		int vector = FIRST_EXTERNAL_VECTOR + i;
		if (vector != SYSCALL_VECTOR) 
			set_intr_gate(vector, interrupt[i]);
	}
    /*
	 * 时钟速率设置：Set the clock to HZ Hz, we already have a valid
	 * vector now:
	 */
	outb_p(0x34,0x43);		/* binary, mode 2, LSB/MSB, ch 0 */
	outb_p(LATCH & 0xff , 0x40);	/* LSB */
	outb(LATCH >> 8 , 0x40);	/* MSB */

// #ifndef CONFIG_VISWS
	// setup_irq(2, &irq2);
// #endif

	/*
	 * External FPU? Set up irq13 if so, for
	 * original braindamaged IBM FERR coupling.
	 */
	// if (boot_cpu_data.hard_math && !cpu_has_fpu)
		// setup_irq(13, &irq13);
}