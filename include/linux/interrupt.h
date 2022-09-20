#ifndef _LINUX_INTERRUPT_H
#define _LINUX_INTERRUPT_H

#include <asm-i386/ptrace.h>
#include <linux/linkage.h>

struct irqaction{
    void (*handler)(int, void *, struct pt_regs *);
    unsigned long flags;
    unsigned long mask;
    const char *name;
    void *dev_id;
    struct irqaction *next;
};

asmlinkage void do_softirq(void);

#endif