/*
*       首先，我们声明了内核程序的入口地址是符号 "start"
 *      这个脚本告诉 ld 程序 如何构造我们的内核映像文件。
 *      然后，我们声明了第一个段 .text 段（代码段）以及它的起始地址 0x100000（1MB）。
 *      接着是 已初始化数据段 .data 和 未初始化数据段 .bss 以及它们采用的4096的页对齐方式。
 *      Linux GCC 增加了额外的数据段 .rodata，这是一个只读的已初始化数据段，放置常量什么的。
 *      简单起见，我们把和 .data 段放在一起好了。
*/
/* OUTPUT_FORMAT("elf32-i386", "elf32-i386", "elf32-i386")
OUTPUT_ARCH(i386) */
ENTRY(start)       /* 入口地址 */
SECTIONS
{
   . = 0x100000;
    PROVIDE( _start = .);
    .text.init.grub :
	{
		*(.text.init.grub)
		. = ALIGN(4096);
	}
    .text.init :    /* 段名后须有空格 */
    {
        *(.text.init)
        . = ALIGN(4096);
    }

    .data.init :
    {
        *(.data.init)
        . = ALIGN(8192);
    }
    . += 0xC0000000;
    .text : AT(ADDR(.text) - 0xC0000000)   
    {
        *(.text)
        . = ALIGN(4096);
    }
    .data : AT(ADDR(.data) - 0xC0000000)
    {
        *(.data)
        *(.rodata)
        . = ALIGN(8192);
    }

    .bss : AT(ADDR(.bss) - 0xC0000000)
    {
        *(.bss)
        . = ALIGN(4096);
    }

    .stab : AT(ADDR(.stab) - 0xC0000000)
	{
		*(.stab)
		. = ALIGN(8192);
	}

    .data.init_task : AT(ADDR(.data.init_task) - 0xC0000000)
    {
        *(.data.init_task)
        . = ALIGN(8192);
    }
    PROVIDE( __initcall_start = .);
    .initcall.init : AT(ADDR(.initcall.init) - 0xC0000000)
    {
        *(.initcall.init)
        PROVIDE( __initcall_end = .);
        . = ALIGN(8192);
    }
    . += 0x2000;
    .stabstr : AT(ADDR(.stabstr) - 0xC0000000)
	{
		*(.stabstr)
	 	. = ALIGN(4096);
	}
    PROVIDE( _end = . );
    
    /DISCARD/ : { *(.comment) *(.eh_frame) }    
}
