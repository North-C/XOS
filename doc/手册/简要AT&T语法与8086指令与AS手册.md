# AT&T简要介绍

>  AT&T与Intel的简单比较：https://csiflabs.cs.ucdavis.edu/~ssdavis/50/att-syntax.htm
> 
>  更多内容都在官方文档：http://sourceware.org/binutils/docs-2.16/as/index.html

GNU 汇编器是 GNU 二进制实用程序 (binutils) 的一部分，也是 GNU 编译器集合的后端。尽管*as* 不是编写相当大的汇编程序的首选汇编程序，但它是当代类 Unix 系统的重要组成部分，尤其是对于内核级黑客攻击。经常因其神秘的 AT&T 风格语法而受到批评，有人认为，*正如*其所写的那样，它强调用作 GCC 的后端，而很少关注“开发人员友好性”。如果您是来自 INTEL 语法背景的汇编程序员，那么您将在代码可读性和代码生成方面遇到一定程度的窒息。尽管如此，必须指出，许多操作系统的代码库依赖*于*作为生成低级代码的汇编程序。

### 基本格式

AT&T 语法中的程序结构类似于任何其他汇编器语法，由一系列指令、标签、指令组成 - 由一个助记符和最多三个操作数组成。AT&T 语法中最显着的区别在于操作数的顺序。

例如，INTEL 语法中基本数据移动指令的一般格式是：

```
mnemonic destination, source
```

而在 AT&T 的情况下，一般格式是

```
mnemonic source, destination
```

对于某些人（包括我自己）来说，这种格式更直观。以下部分描述了用于 x86 架构的 AT&T 汇编器指令的操作数类型。

### 寄存器

IA-32 架构的所有寄存器名称必须以'%' 符号为前缀，例如。%al、%bx、%ds、%cr0 等。

```
mov %ax, %bx
```

上面的例子是将 16 位寄存器 AX 的值移动到 16 位寄存器 BX 的 mov 指令。

### 字面值

所有文字值必须以“$”符号为前缀。例如，

```
mov $100, %bx
mov $A, %al
```

第一条指令将值 100 移动到寄存器 AX 中，第二条指令将 ascii A 的数值移动到 AL 寄存器中。为了让事情更清楚，请注意下面的示例不是有效的指令，

```
mov %bx, $100
```

因为它只是试图将寄存器 bx 中的值移动到文字值。这没有任何意义。

---

缺少操作数前缀表示它是内存内容；因此 `movl $foo,%eax`将变量`foo`的地址放入寄存器`%eax`，但 `movl foo,%eax`将变量`foo`的内容放入寄存器`%eax`。

### 内存寻址

在 AT&T 语法中，内存的引用方式如下：

```
segment-override：signed-offset(base,index,scale)
```

根据您想要的地址，可以省略其中的部分。

```
%es:100(%eax,%ebx,2)
```

请注意，偏移量和比例不应以“$”为前缀。再举几个与它们等效的 NASM 语法的例子，应该会让事情更清楚，

```
GAS 内存操作数                             NASM 内存操作数
------------------                        -------------------- 

100                                     [100] 
%es: 100                                 [es:100] 
(%eax)                                    [eax] 
(%eax,%ebx)                                [eax+ebx] 
(%ecx,%ebx,2)                             [ecx+ebx*2] 
(,%ebx,2)                                 [ebx *2] 
-10(%eax)                                 [eax-10] 
%ds:-10(%ebp)                             [ds:ebp-10]
```

示例说明，

```
mov %ax, 100 
mov %eax, -100(%eax)
```

第一条指令将寄存器 AX 中的值移动到数据段寄存器的偏移量 100（默认情况下），第二条指令将 eax 寄存器中的值移动到 [eax-100]。

### 操作数大小

有时，尤其是在将文字值移动到内存时，需要指定传输大小或操作数大小。例如指令，

```
mov $10, 100
```

仅指定将值 10 移动到内存偏移量 100，但不指定传输大小。在 NASM 中，这是通过将强制转换关键字 byte/word/dword 等添加到任何操作数来完成的。在 AT&T 语法中，这是通过在指令中添加后缀 - b/w/l - 来完成的。例如，

```
mov $10, %es:(%eax)
```

将字节值 10 移动到内存位置 [ea:eax]，而，

```
movl $10, %es:(%eax)
```

将 long 值 (dword) 10 移动到同一位置。

再举几个例子，

```
movl $100, %ebx 
pushl %eax 
popw %ax
```

### 控制转移指令

jmp、call、ret 等指令将控制从程序的一部分转移到另一部分。它们可以分为到相同代码段（近）或不同代码段（远）的控制传输。可能的分支寻址类型是 - 相对偏移（标签）、寄存器、内存操作数和段偏移指针。

*相对偏移量*，使用标签指定，如下所示。

```assembly
    label1: .
            . 
      jmp label1
```

*使用寄存器或内存操作数*的分支寻址必须以“*”为前缀。要指定“远”控制传输，必须以“l”为前缀，如“ljmp”、“lcall”等。例如，

```
GAS 语法                                     NASM 语法
==========                                 =========== 

jmp *100                                 jmp near [100] 
call *100                                 call near [100] 
jmp *%eax                                 jmp near eax 
jmp * %ecx                                 call near ecx 
jmp *(%eax)                             jmp near [eax] 
call *(%ebx)                             call near [ebx] 
ljmp *100                                jmp far [100] 
lcall *100                                call far [100] 
ljmp *(%eax)                             jmp far [eax] 
lcall *(%ebx)                             call far [ebx] 
ret                                     retn 
lret                                     retf 
lret $0x100                             retf 0x100
```

*段偏移*指针使用以下格式指定：

```
jmp $segment, $offset
```

例如：

```
jmp $0x10, $0x100000
```

---

# GNU的汇编伪指令

> 参考：https://sourceware.org/binutils/docs-2.16/as/Pseudo-Ops.html#Pseudo-Ops

`SYMBOL_NAME()`的作用是什么

架构相关：

>  https://sourceware.org/binutils/docs-2.16/as/i386_002dDependent.html#i386_002dDependent

# 8086汇编指令

>  http://www.gabrielececchetti.it/Teaching/CalcolatoriElettronici/Docs/i8086_instruction_set.pdf

## LOCK - Assert LOCK# Signal Prefix

插入 `LOCK#` 前缀，在指令执行过程中插入`LOCK#`信号，使得该指令变成原子指令。

在多处理器环境下， `LOCK#`信号能确保处理器独占 共享内存。

当目的操作数是 memory时，和: ADD, ADC, AND, BTC, BTR, BTS, CMPXCHG, CMPXCH8B,  CMPXCHG16B, DEC, INC, NEG, NOT, OR, SBB, SUB, XOR, XADD, and XCHG.等指令配合使用，会发出异常 `#UD`。

通常 LOCK 前缀和 BTS一起使用，来实现一个 共享内存中的read-modify-write原子操作。

## BT - Bit Test

![image-20220524195031469](https://img-bed-l.oss-cn-beijing.aliyuncs.com/pic_bed/Bit%20Test.png)

在bit串（第一个操作数,Bitbase）中选择一个指定位置(第二个操作数指定，BitOffset)的 bit，保存在 `CF` 标志位。

这一类可以和 **`LOCK` 前缀** 一起使用，达到原子化执行的目的。

等价于：

```C
CF <- Bit(BitBase, BitOffset)
```

CF标志位中保存被选中的bit。

## BTC - Bit Test and Complement

![image-20220524195701183](https://img-bed-l.oss-cn-beijing.aliyuncs.com/pic_bed/BTC.png)

将选中的bit位置反.

```C
CF<-Bit(BitBase, BitOffset)
Bit(BitBase, BitOffset) <- Not Bit(BitBase, BitOffset)
```

CF中保存操作完成前原有的bit值。

## BTS - Bit Test and Set

![image-20220524195842326](https://img-bed-l.oss-cn-beijing.aliyuncs.com/pic_bed/BTS.png)

将选中的bit设置为 1，可以操作 register或者memory。

```C
CF ← Bit(BitBase, BitOffset);
Bit(BitBase, BitOffset) ← 1;
```

## BTR - Bit Test and Reset

![image-20220524195803551](https://img-bed-l.oss-cn-beijing.aliyuncs.com/pic_bed/BTR.png)

将选中的bit清除为 0，可以操作 register或者memory。

```
CF<-Bit(BitBase, BitOffset)
Bit(BitBase, BitOffset) <- 0
```

## SBB - Integer Subtraction with Borrow

借位减法，从 目的操作数中减去 源操作数 和 CF标志位之和。

`DEST <- (DEST - (SRC + CF))`

**该指令不能区分 有符号或者无符号数。**

等价于：

```
DEST ← (DEST – (SRC + CF));
```

根据结果，OF SF(表示有符号结果的符号) ZF AF PF 和 CF都会被置位。

# AS 手册

> http://microelectronics.esa.int/erc32/doc/as.pdf
