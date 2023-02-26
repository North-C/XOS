# GDB

>  参考文档：https://www.cse.unsw.edu.au/~learn/debugging/modules/gdb_init_file/
>
>  《Debugging with GDB: The GNU Source-Level Debugger》
>
>  100个GDB小技巧：https://wizardforcel.gitbooks.io/100-gdb-tips/content/set-print-pretty-on.html

## 疑难

100个GDB小技巧：https://wizardforcel.gitbooks.io/100-gdb-tips/content/set-print-pretty-on.html



## 启动与退出GDB

调用方式：

* `gdb program`： 指定一个 可执行程序 program。

* `gdb program core`： 指定一个 可执行程序 program和core dump文件

* `gdb program 1234`：指定一个 进程ID（此处为1234）作为第二参数.

* `file XXX `选择文件

* `run`  运行 main程序

### 选择文件

* `-symbols file`   `-s file`  从file文件中读取 符号表
* `-exec file` `-e file`  把file文件作为可执行文件来执行
* `-se file` 以上两种的结合
* `-core file` `-c file` 
* `-pid number`  `-p number` 连接到 ID为number的进程。
* `-command file ` `-x file` 执行file文件中的GDB命令

### 选择模式

* `-nx` `-n` 不执行任何初始化文件中的命令
* `-tui` 启动时激活Terminal User Interface。
* `-quiet` `-silent` `-q` 不打GDB的介绍和授权信息
* `-nowindows` `-nw` 不显示GUI

### 退出GDB

`quit` `q`

`Ctrl + C`不会退出GDB。

## GDB命令

* 回车 可以重复运行上一个GDB命令
* Tab 补全命令


* `help` `h` 获取帮助信息
	* `help class` 获取 class 那一类的帮助信息，例如 `help status`
	* `help command` 和命令相关的用法
* `apropos args` 寻找args命令相关的文档
* `compelte args` 和args相似的命令

* `info` 描述程序的状态
* `set` 设置环境变量，例如`set prompt $`
* `show` 描述 GDB本身的状态
	* `show version` `show copying` `show warranty`


## 在GDB下运行程序

### 编译信息

在编译时要生成 debug 信息。
* 指定`-g` 选项，生成调试信息
* `-O` 会对代码进行优化，因此有时可以去掉它

### 运行

* `run` `r` 开始在GDB中运行程序

* `path directory` 将directory加入在PATH环境白能量当中
* `show paths` 展示PATH环境变量
* `show environment [varname]`
* `set environment varname[=value]`
* `unset environment varname`

### debug已经在运行的process

* `attach process-id` 对接已运行的进程.
* `detach`脱离接触
* `kill` 杀死 GDB中运行的程序的child process


### 处理多线程

* 自动对新的线程做出提醒
* `thread threadno` 转换到threadno那个线程
* `info threads` 显示程序中所有的 线程
	* 由GDB 赋予的线程号
	* 目标OS的线程标识符
	* 当前线程的栈帧情况概述
* `thread apply [threadno] [all] args` 对一系列的thread使用一个命令agrs

### 多进程的处理

GDB没有提供对于 使用fork来创建多进程的程序 的debug支持，GDB始终会debug parent process，而新创建的child process会自动畅通无阻的运行。 除非创建一个breakpoint来进行提示。

跟随 child process：
* `set follow-fork-mode mode`,mode的取值如下
	* `parent`
	* `child`
	* `ask`

## Stop and Continuing

`info program` 展示program的状态。

### breakpoint watchpoint 和catchpoint

breakpoint 当程序运行某一个特定的位置(point)时，程序会停下来。

watchpoint 是一种特别的breakpoint，当一个表达式expression的值发生改变时，它会使得程序停下来。

catchpoint 另一种特殊 breakpoint ,当某一类特殊的事件发生时，它会使得程序停下来。

### 设置断点breakpoints

几种不同的设置断点方式

* `break function` 指定函数funtion处断点
* `break +offset` 和 `break -offset`：向前或者向后offset行处断点
* `break linenum` : 在当前源文件中指定 linenum
* `break filename:linenum`
* `break filename:function`
* `break *address` : 在address地址处设置断点，通过这种方式来给没有调试信息的文件打断点
* `break`在下一条命令处打断点

更高级的用法：
* `break ... if cond` ： 设置一个条件
* `tbreak args` : 设置一个只暂停一次的breakpoint，会自动删除
* `hbreak args` : 设置一个 hardware-assisted breakpoint
* `rbreak regex`

查看breankpoint的信息：
* `info breakpoints [n]`
* `info break [n]`
* `info watchpoints [n]`

### 设置 watchpoints

* `watch expr`： 为 表达式 expr设置一个watchpoint
* `rwatch expr` 当expr被读取时，break
* `awatch expr` 当expr被读写时，break
* `info watchpoints` 

### 设置catchpoints

* `catch event`， event可以取值：
	* `throw`
	* `catch`
	* `exec`
	* `fork`
	* `vfork`
	* `load`  `load libname` 动态加载共享胡，或者对libname库的加载.

### 删除breakpoints

* `clear`
* 其他格式斗鱼设置时类似
* `delete [breakpoints] [range...]` 通过range参数指定的范围内的所有breakpoints（包含三种类型）

### Stop & Continue

从上次停止的位置继续执行，`ignore-count` 表示忽略断点被触发的次数。

* `continue [ignore-count]` 
* `c [ignore-count]` 
* `fg [ignore-count]` 

步进：

* `step` 继续运行一条**源程序代码**。
* `stepi` 继续运行一条**机器指令**。
* `step count` 

* `next [count]` 继续执行至当前栈帧当中的下一行源代码
* `nexti` 同理
* `until` 简写为 `u`，表示不同
* `until LOCATION`   指定位置

结束：

* `finish` 结束当前栈帧中函数的运行

### 信号



## 查看 stack

### 栈帧

* `frame ARGS` 允许将一个栈帧移动到另一个，依次打印出选中的那个栈帧。 args可以是地址，或者是 栈帧序号

* `select-frame` 允许在不打印的情况下从一个栈帧移动到另一个地方
* `frame n` 选择第n个frame 
* `frame addr` 选择address当中的frame
* `up n`将n个frame向上移动
* `down n`类似

### 栈

* `backtrace` `bt` 打印整个栈信息
* `backtrace n` 打印内部的n个frames
* `backtrace -n` 打印外部的n个frames

### 栈帧信息
* `frame` `f` 不修改frame，只是展示

* `info frame`解释选中的frame

  ```
  (gdb) info frame
  Stack level 0, frame at 0x7fffffffdd90:
   rip = 0x40110b in phase_6; saved rip = 0x400ecb
   called by frame at 0x7fffffffdda0
   Arglist at 0x7fffffffdd08, args: 
   Locals at 0x7fffffffdd08, Previous frame's sp is 0x7fffffffdd90
   Saved registers:
    rbx at 0x7fffffffdd60, rbp at 0x7fffffffdd68, r12 at 0x7fffffffdd70, r13 at 0x7fffffffdd78,
    r14 at 0x7fffffffdd80, rip at 0x7fffffffdd88
  ```



## 检查源文件

## 打印源文件

`list` 默认打印 10 行：

* `list linenum`
* `list function`
* `list` 打印更多行
* `list -` 打印上次打印的行

通过 `set listsize count` 来设置打印的源文件行数。



## 检查数据

检查数据一般使用的是 print 命令，或者类似的 inspect。它能够推算并打印出 表达式的值。


* `print expr`

* `print /f expr`

  `expr` 是一个被调试的程序语言 所编写的表达式， 可以通过 `/f`  来制定不同的输出格式。

任意的 `value`、`constant` 、`operator` 都可以用到 `expr` 表达式当中来，它包含一些条件表达式，函数调用，转换和字符串常量，但是并不包含 `#define` 等预处理命令。

GDB 也支持 用户自己输入的数组来作为 `expr` 表达式，语法形式为 `{element, element, …}`。

除了编程语言中使用的操作符外，还支持：

* `@` 二进制操作符，将内存当做 数组看待。
* `::` 以 file 或者 function 的角度来指定一个变量。
* `{type} addr` 引用一个 位于 addr 内存地址的 type 类型 对象。

 

打印程序变量：

`expr` 使用的最广泛形式就是变量，在选中的栈帧当中，这些变量要么是 `global`/ `static`  或者是 当前执行的栈帧中可见的。

在下述的函数当中，在 `foo` 函数当中

```C
foo(a)
	int a;
{
	bar(a);
	{
		int b = test();
		bar(b);
	}
}
```

### 检测内存

利用 examine –— `x`  命令来检测内存。

```
x/nfu addr
x addr
x
```

`n`  `f`  `u` 分别是用于指定内存数据格式的可选参数。

* `n` 表示 读取的内存数据个数
* `f` 展示的格式, `s` 表示无终结符的字符串， `i` 表示机器指令，默认是 `x` 表示 16进制数字。每次使用 `x` 命令都会改变默认格式。
* `u` 单位大小, `b ` 字节，`h` – half word(两个字节), `w` 一个字（四字节），`g` Giant words(8字节)

例如： 

```
x/3uh 0x54320
```

反汇编 `disassemble` ，简写为 `disass`。



### 寄存器

* `info registers` ，简写为`i r`， 输出所有寄存器信息，不包含浮点寄存器
* `info all-registers` 输出所有寄存器信息，包含浮点寄存器
* `info registers RENAME …`

利用 `print` 指令也可以查看，例如：`p $eip`，加上一个 `$` 符号即可。

有四个标准的寄存器名称：

* `$pc` 表示指令计数器
* `$sp` 表示栈指针寄存器
* `$fp` 表示当前栈帧的寄存器
* `$ps` 表示 处理器状态的寄存器，x86当中指 EFLAGS

修改寄存器的值：

* `set $eax = 1`



### 自动显示

每次运行都会自动展示相应的数据。

* `display EXPRESSION` 
* `display /FMT EXPRESSION`  FMT 指定格式
* `display /FMT ADDR` 

取消自动展示：

`undisplay dnums...`

`delete display dnums…`

`disable display dnums…`

`enable display dnums…`



### 查看结构体

有结构体定义如下：

```c
struct A{
    char *name;//姓名,指针在64位系统占用8个字节
    int num;//学号
    char group;//小组，考察字节对齐
    int age;//年龄
    double score;//分数
    // float score;//分数
}A1,A2;//stu1是结构体变量，有点类似enum类型
```

用gdb查看如下：

<img src="https://img-bed-l.oss-cn-beijing.aliyuncs.com/pic_bed/image-20230221170131647.png" alt="image-20230221170131647" style="zoom:50%;" />





## 检测符号表

可以检测程序中 变量、函数、类型的名称，这些都是在文本中定义且不会更改的，GDB 从可以从符号表中找到他们。

* `info address SYMBOL` 描述 符号 SYMBOL 的数据的地址。对于寄存器变量，会输出对应的寄存器。对于非寄存器变量，输出栈帧的偏移量。
* `info symbol ADDR` 打印保存在 ADDR 处的符号。例如 `info symbol 0x54320` 
* `whatis EXPR` 打印 表达式 EXPR 的数据类型
* `ptype TYPENAME`  打印数据类型名 TYPENAME 对应的数据类型和结构
* `info types [REGEXP]`   所有匹配的类型
* `info source` 展示当前源文件中的名称
* `info sources` 展示所有源文件中的名称 
* `info functions` 展示所有已定义的函数名和数据类型
* `info variables` 函数外部变量的信息



## tracepoint

用于程序实时运行时难以跟踪的问题。

通过 `trace` 和 `collect` 命令，在程序中指定位置和任意的表达式来判断，是否 tracepoints 被触发。接着使用 `tfind` 命令，检测程序触发 `tracepoints` 时表达式的值。

设置 Tracepoints：



## 修改执行顺序

GDB 可以修改程序执行的顺序，通常配合变量设置来验证自己的修改是否正确。

```
(gdb) whatis width
type = double
(gdb) p width
$4 = 13
(gdb) set var width=47
```

然后使用 `jump` 命令进行跳转：

* `jump LINESPEC` 

* `jump *ADDRESS` 

给程序发送信号：

* `signal SIGNAL`

从函数当中返回:
* `return` 
* `return expression`

调用程序函数:
* `call EXPRESSION`





## 调试远程程序

### 使用gdbserver

gdbserver是一个Unix-like系统中的控制程序，听过使用 `target remote`命令来连接远程的GDB。

gdbserver和GDB之间通过serial line或者 TCP连接进行通信。

在远程目标机器上：
告诉gdbserver如何连接 GDB：
`target > gdbserver comm program [ args ...]`
* `comm` 是设备名，或TCP主机名与端口号
举个例子： `target gdbserver /dev/com1 emacs foo.txt`或
`target gdbserver host:2345 emacs foo.txt`

`gdbserver comm --attach pid` 连接到运行中的program。


在GDB宿主机上连接：
`(gdb) target remote /dev/ttyb` 通过serial line `/dev/ttyb`进行信息传输。

或者利用TCP连接进行：
`(gdb) target remote the-target:2345`

























