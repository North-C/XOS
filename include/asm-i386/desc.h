#ifndef _I386_DESC_H
#define _I386_DESC_H

struct desc_struct {
    unsigned long a, b;
};

extern struct desc_struct gdt_table[];
extern struct desc_struct *idt, *gdt;

struct Xgt_desc_struct {
	unsigned short size;
	unsigned long address __attribute__((packed));
};

#define idt_descr (*(struct Xgt_desc_struct *)((char *)&idt - 2))
#define gdt_descr (*(struct Xgt_desc_struct *)((char *)&gdt - 2))

/*
 * This is the ldt that every process will get unless we need
 * something other than this.
 */
extern void set_intr_gate(unsigned int irq, void * addr);


#endif