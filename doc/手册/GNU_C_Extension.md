# Extensions to the C Language Family

>  参考：https://gcc.gnu.org/onlinedocs/gcc/C-Extensions.html#C-Extensions

## 引用类型`typeof`

引用表达式类型的另一种方法是使用`typeof`. 使用这个关键字的语法看起来像`sizeof`，但该结构在语义上的行为类似于用 定义的类型名称`typedef`。

将参数写入 有两种方法`typeof`：使用表达式或使用类型。这是一个带有表达式的示例：

```
typeof (x[0](1))
```

这假设这`x`是一个指向函数的指针数组；所描述的类型是函数值的类型。
这是一个以类型名作为参数的示例：
```
typeof (int *)
```
这里描述的类型是指向`int`.

如果您正在编写包含在 ISO C 程序中时必须工作的头文件，请编写`__typeof__`而不是`typeof`. 请参阅[备用关键字](https://gcc.gnu.org/onlinedocs/gcc/Alternate-Keywords.html#Alternate-Keywords)。

`typeof`可以在任何可以使用 typedef 名称的地方使用构造。例如，您可以在声明、强制转换或 or 内部使用`sizeof`它`typeof`。

`typeof`当且仅当它是可变修改类型的表达式或此类类型的名称时，才评估其副作用 的操作数。

`typeof`通常与语句表达式结合使用（请参阅[Statement Exprs](https://gcc.gnu.org/onlinedocs/gcc/Statement-Exprs.html#Statement-Exprs)）。以下是如何将两者一起用于定义一个安全的“最大”宏，该宏可对任何算术类型进行操作，并对每个参数只计算一次：

```
#定义最大值（a，b）\
  ({ typeof (a) _a = (a); \
      typeof (b) _b = (b); \
    _a > _b ? _a : _b; })
```
为局部变量使用以下划线开头的名称的原因是为了避免与替换表达式中出现的变量名称发生冲突。最终，我们希望设计一种新形式的声明语法，允许您声明其范围仅在其初始值设定项之后开始的变量；这将是防止此类冲突的更可靠方法。

---
更多的使用示例`typeof`：

- 将y声明为x指向的类型:
```C
typeof (*x) y;
```
- 将y声明为x指向类型的数组:
```C
typeof (*x) y[4];
```

- 将y 声明为为一个指向字符的指针数组：

```C
typeof (typeof (char *)[4]) y;
// 它等价于以下传统的 C 声明：
char *y[4]];
```

要查看 using 声明的含义`typeof`，以及为什么它可能是一种有用的编写方式，请使用以下宏重写它：

```C
#define pointer(T) typeof(T *)
#define array(T, N) typeof(T [N])
```

现在可以这样重写声明：

```C
array(pointer (char), 4) y;
```

因此，`array (pointer (char), 4)`是 4 个指向 的指针数组的类型`char`。

在 GNU C 而不是 GNU C++ 中，您也可以将变量的类型声明为`__auto_type`. 在这种情况下，声明必须只声明一个变量，其声明符必须只是一个标识符，声明必须被初始化，变量的类型由初始化器确定；直到初始化器之后，变量的名称才在范围内。`auto`（在 C++ 中，您应该为此使用 C++11 。）使用`__auto_type`，上面的“最大”宏可以写成：

```
#定义最大值（a，b）\
  ({ __auto_type _a = (a); \
      __auto_type _b = (b); \
    _a > _b ? _a : _b; })
```

使用`__auto_type`而不是`typeof`有两个优点：

- 宏的每个参数在宏的展开中只出现一次。当对此类宏的调用嵌套在此类宏的参数中时，这可以防止宏扩展的大小呈指数增长。
- 如果宏的参数具有可变修改类型，则在使用时只计算一次`__auto_type`，但在使用时计算两次 `typeof`。

## Attribute属性语义

一个attribute specifier的格式为`__attribute__((attribute-list))`，一个属性列表可能是为空的逗号分隔的属性序列，其中每个属性是以下之一：

* 空
* 属性名称（可以是标识符，例如unused，或者保留字，例如const）
* 属性名称，其后跟着一个`,`和一个以`,`分隔的列表：
* 属性名后跟一个带括号的属性参数列表。这些参数采用以下形式之一：
  - 一个标识符。例如，`mode`属性使用这种形式。
  - 一个标识符，后跟一个逗号和一个以逗号分隔的非空表达式列表。例如，`format`属性使用这种形式。
  - 一个可能为空的逗号分隔的表达式列表。例如， `format_arg`属性使用这种形式，列表是单个整数常量表达式，`alias`属性使用这种形式，列表是单个字符串常量。

可以选择在名称之前和之后添加`__`。这允许您在头文件中使用它们，而不必担心可能的同名宏。例如，您可以使用属性名称`__noreturn__`而不是`noreturn`.

### Function Attributes

在 GNU C 和 C++ 中，您可以**使用函数属性来指定某些函数属性，这些属性可以帮助编译器优化调用或更仔细地检查代码的正确性。**例如，您可以使用属性来指定函数从不返回 ( `noreturn`)、仅根据其参数的值返回值 ( `const`) 或具有`printf`-style 参数 ( `format`)。

还可以使用属性来**控制被注释函数内的内存放置、代码生成选项或调用/返回约定**。其中许多属性是特定于目标的。

函数属性由`__attribute__`函数声明中的关键字引入，后跟用双括号括起来的属性说明。您可以在声明中指定多个属性，方法是在双括号内用逗号分隔它们，或者在一个属性规范之后紧跟另一个。

#### Common Function Attributes

* access (access-mode, ref-index) 或 access(access-mode, ref-index, size-index)
	
* weak
	**该weak属性导致外部符号的声明作为弱符号而不是全局符号发出。**这**主要用于定义可以在用户代码中覆盖的库函数，**尽管它也可以与非函数声明一起使用。覆盖符号必须与弱符号具有相同的类型。此外，如果它指定一个变量，它还必须具有与弱符号相同的大小和对齐方式。ELF 目标支持弱符号，使用 GNU 汇编器和链接器时也支持 a.out 目标。

* unused
  附加到函数的此属性意味着该函数可能未被使用。

* section ("section-name")

  通常，编译器将它生成的代码放在`text`节中。但是，有时您需要额外的部分，或者您需要某些特定功能出现在特殊部分中。**该`section` 属性指定函数位于特定部分中**。例如，声明：

  ```
  extern void foobar (void) __attribute__ ((section ("bar")));
  ```

  将功能`foobar`放在该`bar`部分中。

  某些文件格式不支持任意部分，因此该`section` 属性并非在所有平台上都可用。如果您需要将模块的全部内容映射到特定部分，请考虑改用链接器的工具。

* noreturn
  一些标准库函数，例如abortand exit，不能返回。
  
* nothrow
  该nothrow属性用于通知编译器一个函数不能抛出异常。例如，标准 C 库中的大多数函数都可以保证不会抛出异常，其中值得注意的异常是qsort和bsearch采用函数指针参数。

* leaf 
  使用此属性调用外部函数必须**仅通过返回或异常处理返回到当前编译单元**。特别是，叶函数不允许调用从当前编译单元传递给它的回调函数，直接调用单元导出的函数，或longjmp进入单元。

* hot
  函数的hot属性用于通知编译器该函数是编译程序的热点。该功能被更积极地优化，并且在许多目标上它被放置在文本部分的一个特殊小节中，因此所有热门功能看起来很接近，从而提高了局部性。

* format (archetype, string-index, first-to-check)
  该format属性指定函数采用应根据格式字符串进行类型检查的、printf或 样式参数。
  * 参数archetype确定格式字符串的解释方式，应该是printf, scanf, strftime, gnu_printf, gnu_scanf,gnu_strftime或 strfmon. 
  * 参数string-index 指定哪个参数是格式字符串参数（从 1 开始）
  * first-to-check是要检查格式字符串的第一个参数的编号。对于无法检查参数的函数（例如 vprintf），将第三个参数指定为零。在这种情况下，编译器只检查格式字符串的一致性。
  例如，声明：
  ```C
  extern int 
  my_printf(void *my_object, const char *my_format, ...)
  	__attribute__((format(printf, 2, 3)));
  ```

> x86函数属性： https://gcc.gnu.org/onlinedocs/gcc/x86-Function-Attributes.html#x86-Function-Attributes



### Variable Attributes
x86变量属性：
`ms_struct`和`gcc_struct`，如果packed在结构上使用 ，或者如果使用位域，则可能是 Microsoft ABI 对结构的布局与 GCC 通常的布局方式不同。

### Label Attributes
此示例使用cold标签属性来指示 ErrorHandling不太可能采用分支并且 ErrorHandling标签未使用：
```C
   asm goto ("some asm" : : : : NoError); 
/* 这个分支（来自 asm 的分支）不太常用 */ 
ErrorHandling: 
   __attribute__((cold, used)); /* 这里需要分号 */ 
   printf("error\n"); 
   返回0；
NoError: 
   printf("没有错误\n"); 
   返回 1；
```
* unused
	此功能适用于程序生成的代码，这些代码可能包含未使用的标签，但编译时使用`-Wall`。通常不适合在其中使用人工编写的代码，尽管在跳转到标签的代码包含在#ifdef条件中的情况下它可能很有用。

* hot
  标签上的hot属性用于**通知编译器标签后面的路径比没有注释的路径更有可能执行**。此属性用于`__builtin_expect` 无法使用的情况，例如计算的 goto 或asm goto.

* cold
  标签上的cold属性用于**通知编译器标签后面的路径不太可能被执行。**此属性用于__builtin_expect无法使用的情况，例如计算的 goto 或asm goto.

### Statement Attributes

GCC 允许在空语句上设置属性。



* fallthrough
	    带有 null 语句的fallthrough属性用作 fallthrough 语句。**它向编译器暗示一条语句落入另一个 case 标签或 switch 语句中的用户定义标签是故意的**，因此 `-Wimplicit-fallthrough`警告不得触发。
	* **fallthrough 属性在每个属性列表中最多出现一次，并且不能与其他属性混合。**它**只能用在 switch 语句中（**否则编译器将发出错误），在前面的语句之后和逻辑上连续的 case 标签之前，或用户定义的标签。
	

该例子中，fallthrough指示  -Wimplicit-fallthrough警告应该被忽略
```C
switch(cond)
{
	case 1:
		bar(1):
		__attribute__((fallthrough));
	case 2:
	...
}
```



### Type Attributes

#### Common Type Attributes

* aligned 或 aligned(alignment)：

  该`aligned`属性为指定类型的变量指定**最小对齐方式**（以字节为单位）。指定时，<u>对齐必须是 2 的幂</u>。不指定对齐参数意味着目标的最大对齐，通常但不总是 8 或 16 个字节。例如：

  ```C
  struct __attribute__ ((aligned (8))) S { short f[3];}
  typedef int more_aligned_int __attribute__((aligned (8)))
  ```


* deprecated或 deprecated (msg)
  如果在源文件中的任何位置使用该类型，则该`deprecated`属性会**导致警告**。这在识别预计将在未来版本的程序中删除的类型时很有用。如果可能，警告还包括不推荐使用类型的声明位置，以使用户能够轻松找到有关不推荐使用类型的原因的更多信息，或者他们应该做什么。请注意，**警告仅在使用时出现，并且仅在类型应用于本身未被声明为已弃用的标识符时才会出现。**该`deprecated`属性**也可用于函数和变量**。
  
* packed

  * 此属性附加到struct 、union或 C++ class类型定义，**指定放置其每个成员（零宽度位字段除外）以最小化所需的内存**。

  * 当附加到`enum`定义时，该`packed`属性**指示应使用最小的整数类型。**

  * 只能指定packed属性到enum,struct,union或class上，不能在未定义枚举类型,struct,union或class的`typedef`上使用。
```C
struct my_unpacked_struct
{
	char c;
	int i;
}

struct  __attribute__ ((__packed__)) my_packed_struct
{
	char c;
	int i;
	struct my_unpacked_struct s;
}
```
* unused
   当附加到一个类型（包括union或 struct）时，该属性意味着该类型的变量可能看起来可能未使用。GCC 不会对该类型的任何变量产生警告，即使该变量似乎什么都不做。

* mode (mode)
	该属性指定声明的数据类型——无论哪种对应于模式mode的类型。这实际上使您可以根据其宽度请求整数或浮点类型。
	具体的和[Machine Modes](http://gcc.gnu.org/onlinedocs/gccint/Machine-Modes.html#Machine-Modes)相关，例如byte，__byte__,word,__word__等。



## 常见

### asmlinkage

```C
if defined __i386__
#define asmlinkage CPP_LINKAGE __attribute__((regparm(0)))
#elif defined __ia64__
#define asmlinkage CPP_LINKAGE __attribute__((syscall_linkage))
#else
#define asmlinkage CPP_ASMLIKAGE
#endif
    
   #ifdef __cplusplus
#define CPP_ASMLINKAGE extern "C"
#else
#define CPP_ASMLINKAGE
#endif
```

 asmlinkage 标记。这是一个用于某些 gcc 魔术的 `#define`，它告诉编译器函数**不应该期望在寄存器中找到它的任何参数**(一种常见的优化) ，而**只能在 CPU 堆栈中找到它的参数。**









































