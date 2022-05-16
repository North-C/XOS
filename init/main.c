#include <asm-i386/types.h>
#include <linux/stdio.h>

void start_kernel(void){
    // uint8_t* input = (uint8_t*)0xB8000;
    // // uint8_t color = 0x07;       // 黑底白字
    // uint8_t color = (0 << 4) | (15 & 0x0F);
    // *input++ = 'H'; *input++ = color;
    // *input++ = 'e'; *input++ = color;
    // *input++ = 'l'; *input++ = color;
    // *input++ = 'l'; *input++ = color;
    // *input++ = 'o'; *input++ = color;
    // *input++ = ','; *input++ = color;
    // *input++ = ' '; *input++ = color;
    // *input++ = 'O'; *input++ = color;
    // *input++ = 'U'; *input++ = color;
    // *input++ = 'R'; *input++ = color;
    // *input++ = 'O'; *input++ = color;
    // *input++ = 'S'; *input++ = color;
    // *input++ = '.'; *input++ = color;
    
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
