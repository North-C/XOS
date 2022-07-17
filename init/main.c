#include <asm-i386/types.h>
#include <linux/stdio.h>

void start_kernel(void){
    printk("Start kernel\n");
    setup_arch();    
    while(1);
}
