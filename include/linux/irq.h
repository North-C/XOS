#ifndef _LINUX_IRQ_H
#define _LINUX_IRQ_H

#include <asm-i386/irq.h>
#include <linux/cache.h>
#include <linux/spinlock.h>

// IRQ 的状态
#define IRQ_INPROGRESS 1
#define IRQ_DISABLED   2
#define IRQ_PENDING    4
#define IRQ_REPLAY     8
#define IRQ_AUTODETECT 16
#define IRQ_WAITING    32
#define IRQ_LEVEL      64
#define IRQ_MASKED     128
#define IRQ_PER_CPU    256

// 中断控制器描述符，描述底层的硬件
struct hw_interrupt_type {
	const char * typename;
	unsigned int (*startup)(unsigned int irq);
	void (*shutdown)(unsigned int irq);
	void (*enable)(unsigned int irq);   // 开启中断通道
	void (*disable)(unsigned int irq);  // 关闭中断通道
	void (*ack)(unsigned int irq);      // 对中断控制器的响应
	void (*end)(unsigned int irq);		// 用于中断服务返回之前，表示结束
	void (*set_affinity)(unsigned int irq, unsigned long mask);
};

typedef struct hw_interrupt_type  hw_irq_controller;

// 中断向量描述符，包含了不同的中断信息。将其嵌入 32 bytes 的缓存当中
typedef struct {
    unsigned int status;   // IRQ 状态
    hw_irq_controller *handler;  // 
    struct irqaction *action;    // IRQ 动作队列
    unsigned int depth;         // 允许嵌入的深度
    spinlock_t lock;
} ____cacheline_aligned irq_desc_t;

extern irq_desc_t irq_desc [NR_IRQS];

extern hw_irq_controller no_irq_type;

#endif