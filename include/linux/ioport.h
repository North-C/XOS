#ifndef _LINUX_IOPORT_H
#define _LINUX_IOPORT_H

// 描述系统的资源，类似于树形结构，允许嵌套
struct resource{
    const char* name;
    unsigned long start, end;
    unsigned long flags;
    struct resource *parent, *sibling, *child;
};

struct resource_list {
    struct resource_list *next;
    struct resource *res;
    // struct pci_dev *dev;
};


/*
 * IO resources have these defined flags.
 */
#define IORESOURCE_BITS		0x000000ff	/* Bus-specific bits */

#define IORESOURCE_IO		0x00000100	/* Resource type */
#define IORESOURCE_MEM		0x00000200
#define IORESOURCE_IRQ		0x00000400
#define IORESOURCE_DMA		0x00000800

#define IORESOURCE_PREFETCH	0x00001000	/* No side effects */
#define IORESOURCE_READONLY	0x00002000
#define IORESOURCE_CACHEABLE	0x00004000
#define IORESOURCE_RANGELENGTH	0x00008000
#define IORESOURCE_SHADOWABLE	0x00010000
#define IORESOURCE_BUS_HAS_VGA	0x00080000

#define IORESOURCE_UNSET	0x20000000
#define IORESOURCE_AUTO		0x40000000
#define IORESOURCE_BUSY		0x80000000	/* Driver has marked this resource busy */

extern int request_resource(struct resource *root, struct resource *new);

extern struct resource iomem_resource;
extern struct resource ioport_resource;

#endif