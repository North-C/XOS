#ifndef _LINUX_INIT_H
#define _LINUX_INIT_H

#define __init __attribute__((__section__ (".text.init")))
#define __initdata __attribute__((__section__ (".data.init")))

#endif