#include <asm-i386/types.h>
#include <linux/stdio.h>

void start_kernel(void){
    int i ;
    for (i = 0; i < 100; i++){
        printk("Hello OUROS\n");
    }
    printk("God123\n");
    int n = 8;
    printk("%d",n);
    panic("GG");
    printk("%d",n);
    while(1);
}
