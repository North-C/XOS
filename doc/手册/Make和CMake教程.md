

# Make

## 常用函数与常用变量

### `$(subst from, to, text)`

```makefile
$(subst ee,EE,feet on the street)
```
产生结果为： `fEEt on the strEEt`

### `$(patsubst pattern, replacement, text)`

```makefile
$(patsubst %.c,%.o,x.c.c bar.c)
```
产生结果为： `x.c.o bar.o`


### `MAKELIST` 

makefile的列表，按照 make 解析的顺序排列。
例如：

```makefile
name1 := $(lastword $(MAKEFILE_LIST))
include inc.mk
name2 := $(lastword $(MAKEFILE_LIST))
all:
        @echo name1 = $(name1)
        @echo name2 = $(name2)
```

输出的结果为：
```shell
name1 = Makefile
name2 = inc.mk
```



### `$(lastword names...)`

`names`参数是一串通过空白符号分割的一系列名称，返回最后一个name。



`VPATH`变量指定make搜索的前提文件夹列表，通过`:`进行分隔，例如`src:../headers`

```makefile
($subst :, , $(VPATH))
```

由此输出的结果是`src ../headers`。

```makefile
CFLAGS += $(patsubst %, %I, $(subst :, ,$(VPATH)))
```



### `($dir names...)`

获取names文件参数中所有的文件夹。

```makefile
$(dir src/foo.c hacks)
```

输出：`src/ .`



### `($wildcard pattern)`

`pattern` 参数是一个文件，通配符进行匹配



### `($abspath names…)`

返回所有的绝对路径。

### `filter(pattern..., text)`
返回text当中以空格分隔的所有符合 pattern模式的单词。
使用举例：
```makefile
sources := foo.c bar.c baz.s ugh.h
foo: $(sources)
	cc $(filter ^.c %.s, $(sources)) -o foo
```










# CMake教程

> Cmake Cookbook: https://www.bookstack.cn/read/CMake-Cookbook/content-chapter1-1.3-chinese.md
>
> cmake教程：https://riptutorial.com/cmake
>
> 官方教程：https://cmake.org/cmake/help/v3.10/manual

### 基础

文件名必须区分大小写，即必须命名为`CMakeLists.txt`。

#### 从可执行文件到库

编译单个源文件为可执行文件，必要的内容如下：

```cmake
# 设置CMake 所需的最低版本，如果低于该版本，会发出致命错误
cmake_minimum_required(VERSION 3.5 FATAL_ERROR)

# 声明项目的名称(recipe)和支持的编程语言(C)
project(recipe LANGUAGES C)

# 指示CMake创建一个新目标 可执行文件hello-world
# CMake将为编译器使用默认设置，并自动选择生成工具
add_executable(hello-world hello-world.cpp)

```

支持的语言还有：

* CXX 表示 C++，是默认的语言

创建build目录，进入build目录进行配置：

```sh
mkdir -p build
cd build
cmake ..

// 可以直接使用一下命令达到相同效果
cmake -H. -Bbuild
```

* `-H`表示当前目录中搜索根`CMakeLists.txt`文件。`-Bbuild`告诉CMake在一个名为`build`的目录中生成所有的文件。

项目配置将在build生成，编译：

```shell
cmake --build .
```

---

在GNU/Linux上，CMake默认生成Unix Makefile来构建项目：

- `Makefile`: `make`将运行指令来构建项目。
- `CMakefile`：包含临时文件的目录，CMake用于检测操作系统、编译器等。此外，根据所选的生成器，它还包含特定的文件。
- `cmake_install.cmake`：处理安装规则的CMake脚本，在项目安装时使用。
- `CMakeCache.txt`：如文件名所示，CMake缓存。CMake在重新运行配置时使用这个文件。

#### 切换生成器

CMake是一个构建系统生成器，可以使用单个CMakeLists.txt为不同平台上的不同工具集配置项目。

CMake针对不同平台支持本地构建工具列表。同时支持命令行工具(如Unix Makefile和Ninja)和集成开发环境(IDE)工具。

```bash
$ mkdir -p build
$ cd build
$ cmake -G Ninja ..
```

#### 构建和链接静态库与动态库

创建静态库：

```cmake
add_library(message 
	STATIC
		Message.hpp
		Message.cpp)
```

解释：生成必要的构建指令，将指定的源码编译到库中。`add_library`的第一个参数是**目标名**。整个`CMakeLists.txt`中，可**使用相同的名称来引用库**。生成的库的实际名称将由CMake通过在前面添加前缀`lib`和适当的扩展名作为后缀来形成。生成库是根据第二个参数(`STATIC`或`SHARED`)和操作系统确定的。

- **STATIC**：用于创建静态库，即编译文件的打包存档，以便在链接其他目标时使用，例如：可执行文件。
- **SHARED**：用于创建动态库，即可以动态链接，并在运行时加载的库。可以在`CMakeLists.txt`中使用`add_library(message SHARED Message.hpp Message.cpp)`从静态库切换到动态共享对象(DSO)。
- **OBJECT**：可将给定`add_library`的列表中的源码编译到目标文件，不将它们归档到静态库中，也不能将它们链接到共享对象中。如果需要一次性创建静态库和动态库，那么使用对象库尤其有用。
- **MODULE**：又为DSO组。与`SHARED`库不同，它们不链接到项目中的任何目标，不过可以进行动态加载。该参数可以用于构建运行时插件。

创建可执行文件：

```cmake
add_executable(hello-world hello-world.cpp)
```



将目标库链接到可执行目标：

```cmake
target_link_libraries(hello-wolrd message)
```

CMake还能够生成特殊类型的库，这不会在构建系统中产生输出，但是对于组织目标之间的依赖关系，和构建需求非常有用：

* **IMPORTED**：此类库目标表示位于项目外部的库。此类库的主要用途是，对现有依赖项进行构建。因此，`IMPORTED`库将被视为不可变的。

* **INTERFACE**：与`IMPORTED`库类似。不过，该类型库可变，没有位置信息。它主要用于项目之外的目标构建使用。

* **ALIAS**：顾名思义，这种库为项目中已存在的库目标定义别名。不过，不能为`IMPORTED`库选择别名。

**例子：**

```cmake
cmake_minimum_required(VERSION 3.5 FATAL_ERROR)
project(recipe-03 LANGUAGES CXX)
add_library(message-objs
    OBJECT
        Message.hpp
        Message.cpp
    )
# this is only needed for older compilers
# but doesn't hurt either to have it
set_target_properties(message-objs
    PROPERTIES
        POSITION_INDEPENDENT_CODE 1
    )
add_library(message-shared
    SHARED
        $<TARGET_OBJECTS:message-objs>
    )
add_library(message-static
    STATIC
        $<TARGET_OBJECTS:message-objs>
    )
add_executable(hello-world hello-world.cpp)
target_link_libraries(hello-world message-static)
```

#### 指定编译器

CMake将语言的编译器存储在`CMAKE_<LANG>_COMPILER`变量中，其中`<LANG>`是受支持的任何一种语言。

1. 使用CLI中的`-D`选项，例如：

   ```sh
   $ cmake -D CMAKE_CXX_COMPILER=clang++ ..
   ```

2. 通过导出环境变量`CXX`(C++编译器)、`CC`(C编译器)和`FC`(Fortran编译器)。例如，使用这个命令使用`clang++`作为`C++`编译器：

   ```sh
   $ env CXX=clang++ cmake ..
   ```

#### 设置编译器选项

可以选择下面两种方法:

- CMake将编译选项视为目标属性。因此，可以根据每个目标设置编译选项，而不需要覆盖CMake默认值。
- 可以使用`-D`CLI标志直接修改`CMAKE_<LANG>_FLAGS_<CONFIG>`变量。这将影响项目中的所有目标，并覆盖或扩展CMake默认值。

示例：

为目标准备标志列表 flags：

```cmake
list(APPEND flags "-fPIC" "-Wall")
if(NOT WIN32)
  list(APPEND flags "-Wextra" "-Wpedantic")
endif()
```

为这个库目标设置了编译选项:

```cmake
target_compile_options(geometry
	PRIVATE
		${flags}
)
```

为可执行目标 compute_areas 设置编译选项：

```cmake
target_compile_options(compute_areas
	PRIVATE
		"-fPIC"
)
```

编译选项可以添加三个级别的可见性：`INTERFACE`、`PUBLIC`和`PRIVATE`。

可见性的含义如下:

- **PRIVATE**，编译选项会应用于给定的目标，不会传递给与目标相关的目标。我们的示例中， 即使`compute-areas`将链接到`geometry`库，`compute-areas`也不会继承`geometry`目标上设置的编译器选项。
- **INTERFACE**，给定的编译选项将只应用于指定目标，并传递给与目标相关的目标。
- **PUBLIC**，编译选项将应用于指定目标和使用它的目标。

查看编译标志：

```sh
cmake --build . -- VERBOSE=1
```

全局标志，运行一下命令：

```sh
$ cmake -D CMAKE_CXX_FLAGS="-fno-exceptions -fno-rtti" ..
```

#### 为语言设定标准

为目标设置：

```cmake
set_target_properties(animals
  PROPERTIES
    CXX_STANDARD 14
    CXX_EXTENSIONS OFF
    CXX_STANDARD_REQUIRED ON
    POSITION_INDEPENDENT_CODE 1
)
```

- **CXX_STANDARD**会设置我们想要的标准。
- **CXX_EXTENSIONS**告诉CMake，只启用`ISO C++`标准的编译器标志，而不使用特定编译器的扩展。
- **CXX_STANDARD_REQUIRED**指定所选标准的版本。如果这个版本不可用，CMake将停止配置并出现错误。当这个属性被设置为`OFF`时，CMake将寻找下一个标准的最新版本，直到一个合适的标志。这意味着，首先查找`C++14`，然后是`C++11`，然后是`C++98`。

如果语言标准是所有目标共享的全局属性，那么可以将`CMAKE_<LANG>_STANDARD`、`CMAKE_<LANG>_EXTENSIONS`和`CMAKE_<LANG>_STANDARD_REQUIRED`变量设置为相应的值。所有目标上的对应属性都将使用这些设置。

例如：

```cmake
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
```

#### 控制流



```cmake
if(USE_LIBRARY)
    # add_library will create a static library
    # since BUILD_SHARED_LIBS is OFF
    add_library(message ${_sources})
    add_executable(hello-world hello-world.cpp)
    target_link_libraries(hello-world message)
else()
    add_executable(hello-world hello-world.cpp ${_sources})
endif()
```

- 如果将逻辑变量设置为以下任意一种：`1`、`ON`、`YES`、`true`、`Y`或非零数，则逻辑变量为`true`。
- 如果将逻辑变量设置为以下任意一种：`0`、`OFF`、`NO`、`false`、`N`、`IGNORE、NOTFOUND`、空字符串，或者以`-NOTFOUND`为后缀，则逻辑变量为`false`。

`USE_LIBRARY`变量将在第一个和第二个行为之间切换。`BUILD_SHARED_LIBS`是CMake的一个全局标志。

### 检测环境

检测操作系统信息：

```cmake
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    message(STATUS "Configuring on/for Linux")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    message(STATUS "Configuring on/for macOS")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    message(STATUS "Configuring on/for Windows")
elseif(CMAKE_SYSTEM_NAME STREQUAL "AIX")
    message(STATUS "Configuring on/for IBM AIX")
else()
    message(STATUS "Configuring on/for ${CMAKE_SYSTEM_NAME}")
endif()
```

平台相关的源代码：

如`hello-world.cpp` 当中具有示例代码如下：

```cpp
#include <cstdlib>
#include <iostream>
#include <string>
std::string say_hello() {
#ifdef IS_WINDOWS
  return std::string("Hello from Windows!");
#elif IS_LINUX
  return std::string("Hello from Linux!");
#elif IS_MACOS
  return std::string("Hello from macOS!");
#else
  return std::string("Hello from an unknown system!");
#endif
}
int main() {
  std::cout << say_hello() << std::endl;
  return EXIT_SUCCESS;
}
```

则 CMakeLists.txt 当中有如下写法：

```cmake
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  target_compile_definitions(hello-world PUBLIC "IS_LINUX")
endif()
if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
  target_compile_definitions(hello-world PUBLIC "IS_MACOS")
endif()
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
  target_compile_definitions(hello-world PUBLIC "IS_WINDOWS")
endif()
```

等等







### 检测外部库

* `find_package()` 命令 用于**发现和设置包的CMake模块**。用于标识系统标准位置中的包。CMake模块文件称为`Find<name>.cmake`。
  * 此外，查找模块还会设置了一些有用的变量，反映实际找到了什么，也可以在自己的`CMakeLists.txt`中使用这些变量。

> *建议在CMake在线文档中查询`Find<package>.cmake`模块，并在使用它们之前详细阅读它们的文档*
>
> 参考文档：*https://cmake.org/cmake/help/v3.5/command/find_ackage.html* 



有四种方式可以找到依赖包：

1. 由包提供商提供CMake文件 `<package>Config.cmake` , `<package>ConfigVersion.cmake` 和`<package>Targets.cmake`，通常会在包的标准安装位置查找。
2. 无论是由CMake还是第三方提供的模块，为所需包使用`find-module`。
3. 使用`pkg-config`。
4. 如果这些都不可行，那么编写自己的`find`模块



#### 检测python解释器

```cmake
cmake_minimum_required(VERSION 3.5 FATAL_ERROR)
project(recipe-01 LANGUAGES NONE)

# 使用find_package命令找到Python解释器
find_package(PythonInterp REQUIRED)

# 执行python命令并获取输出
execute_process(
  COMMAND
      ${PYTHON_EXECUTABLE} "-c" "print('Hello, world!')"
  RESULT_VARIABLE _status
  OUTPUT_VARIABLE _hello_world
  ERROR_QUIET
  OUTPUT_STRIP_TRAILING_WHITESPACE
 )

message(STATUS "RESULT_VARIABLE is: ${_status}")
message(STATUS "OUTPUT_VARIABLE is: ${_hello_world}")

```

对于Python解释器，相关模块为`FindPythonInterp.cmake`附带的设置了一些CMake变量:

- **PYTHONINTERP_FOUND**：是否找到解释器
- **PYTHON_EXECUTABLE**：Python解释器到可执行文件的路径
- **PYTHON_VERSION_STRING**：Python解释器的完整版本信息
- **PYTHON_VERSION_MAJOR**：Python解释器的主要版本号
- **PYTHON_VERSION_MINOR** ：Python解释器的次要版本号
- **PYTHON_VERSION_PATCH**：Python解释器的补丁版本号

如果没有在标准位置安装，则CMake无法准确定位，用户可以使用 `-D` 参数传递相应选项：

```bash
$ cmake -D PYTHON_EXECUTABLE=/custom/location/python ..
```

或者通过`cmake --help-module FindPythonInterp` 来查看具体的模块使用方法。

#### 检测python的库、模块和包

找到Python头文件和库的模块，称为`FindPythonLibs.cmake`，

```cmake
find_package(PythonInterp REQUIRED)
find_package(PythonLibs ${PYTHON_VERSION_MAJOR}.${PYTHON_VERSION_MINOR} EXACT REQUIRED)

# 让可执行文件 包含 Python.h 文件
target_include_directories(hello-embedded-python
  PRIVATE
      ${PYTHON_INCLUDE_DIRS}
)
```

* 必须和python的版本匹配
* `EXACT` 限制CMake检测特定的版本

#### 链接 Boost 库

使用 find_package 即可直接搜索 Boost，其中使用 `FindBoost.cmake`模块来实现：

```cmake
find_package(Boost 1.54 REQUIRED COMPONENTS filesystem)

# 添加可执行文件
add_executable(path-info path-info.cpp)

# 链接到 Boost库组件
target_link_libraries(path-info
	PUBLIC
		Boost::filesystem
)
```

如果Boost库安装在非标准位置，可以在配置时使用`BOOST_ROOT`变量传递Boost安装的根目录，以便让CMake搜索非标准路径:

```bash
$ cmake -D BOOST_ROOT=/custom/boost
```

或者，可以同时传递包含头文件的`BOOST_INCLUDEDIR`变量和库目录的`BOOST_LIBRARYDIR`变量:

```bash
$ cmake -D BOOST_INCLUDEDIR=/custom/boost/include -DBOOST_LIBRARYDIR=/custom/boost/lib
```

#### pkg-config

略

#### 自定义 find 模块

略

### 创建和运行测试

#### 简单的单元测试

关键的命令：

- `enable_testing()`，测试这个目录和所有子文件夹(因为我们把它放在主`CMakeLists.txt`)。
- `add_test()`，定义了一个新的测试，并设置测试名称和运行命令。

```cmake
add_test(
  NAME cpp_test
  COMMAND $<TARGET_FILE:cpp_test> # 生成器表达式，使用生成器表达式$<TARGET_FILE:account>来传递库文件的位置
  )
```

> 生成器表达式，是在生成**构建系统生成时**的表达式。

举个例子：

项目基本结构如下：

```
├── CMakeLists.txt
├── main.cpp
├── sum_integers.cpp
├── sum_integers.hpp
├── test.cpp
├── test.py
└── test.sh
```

`CMakeLists.txt` 

```cmake
cmake_minimum_required(VERSION 3.5 FATAL_ERROR)
project(unit_test LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_EXTENSION OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(PythonInterp REQUIRED)  # 查询 python 解释器
find_program(BASH_EXECUTABLE NAMES bash REQUIRED) # 查询 Bash shell

# 库
add_library(sum_integers sum_integers.cpp)
# main 代码
add_executable(sum_up main.cpp) 
target_link_libraries(sum_up sum_integers)

# 添加 测试
add_executable(cpp_test test.cpp)
target_link_libraries(cpp_test sum_integers)

# 打开测试功能
enable_testing()
# 定义四个测试
add_test(
    NAME bash_test
    COMMAND ${BASH_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test.sh $<TARGET_FILE:sum_up>
)

add_test(
    NAME cpp_test
    COMMAND $<TARGET_FILE:cpp_test>
)

add_test(
    NAME python_test_long
    COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test.py --executable $<TARGET_FILE:sum_up>
)

add_test(
    NAME python_test_short
    COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test.py --short --executable $<TARGET_FILE:sum_up>
)
```

编译和测试：

```bash
$ mkdir -p build
$ cd build
$ cmake ..
$ cmake --build .
$ ctest   # 运行测试集
```

#### 使用 Google Test 库进行单元测试

其余部分与上面无差异，主要通过 FetchContent 模块来实现从源代码编译的获取。

`FetchContent`模块支持通过`ExternalProject`模块，在配置时填充内容，并在其**3.11**版本中成为CMake的标准部分。而`ExternalProject_Add()`在构建时进行下载操作，这样`FetchContent`模块使得构建可以立即进行，**这样获取的主要项目和外部项目仅在第一次执行CMake时调用**，使用`add_subdirectory`可以嵌套。

```cmake

# 使用网络获取 Google Test 源代码
option(ENABLE_UNIT_TESTS "Enable unit tests" ON)
message(STATUS "Enable testing: ${ENABLE_UNIT_TESTS}")

if(ENABLE_UNIT_TESTS)
    include(FetchContent)

    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG        release-1.8.0
    )

    # 添加可执行测试文件
    add_executable(cpp_test "")
    target_sources(cpp_test
        PRIVATE
            test.cpp
    )
    target_link_libraries(cpp_test
        PRIVATE
            sum_integers
            gtest_main
    )

    # 测试项目
    enable_testing()

    add_test(
        NAME google_test
        COMMAND $<TARGET_FILE:cpp_test>
    )
endif()
```









### 配置与构建时操作

CMake 完整的工作流：

![image-20221202153926016](https://img-bed-l.oss-cn-beijing.aliyuncs.com/pic_bed/image-20221202153926016.png)



- **execute_process**，从CMake中执行任意进程，并检索它们的输出。
- **add_custom_target**，创建执行自定义命令的目标。
- **add_custom_command**，指定必须执行的命令，以生成文件或在其他目标的特定生成事件中生成。





### 构建项目







```cmake
add_subdirectory(src)
enable_testing()
add_subdirectory(tests)
```

定义了源码目标：

```cmake
set(CMAKE_INCLUDE_CURRENT_DIR_IN_INTERFACE ON)
add_library(sum_integers sum_integers.cpp)
add_executable(sum_up main.cpp)
target_link_libraries(sum_up sum_integers)
```



### 模块化构建

1. 定义一个函数或宏，并将其放入模块中
2. 包含模块
3. 调用函数或宏

自己构建 `XXX.cmake` 文件，在主`CMakeLists.txt` 当中

```cmake
# 告知 CMake 查找宏
list(APPEND CMKAE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include(colors)  
# 显示的包含
include(cmake/colors.cmake)
define_color()  # 调用其中的宏
```

通过编写函数来测试和设置编译器，例如：

```cmake
include(CheckCCompilerFlag)
include(CheckCXXCompilerFlag)
include(CheckFortranCompilerFlag)
function(set_compiler_flag _result _lang)
  # build a list of flags from the arguments
  set(_list_of_flags)
  # also figure out whether the function
  # is required to find a flag
  set(_flag_is_required FALSE)
  foreach(_arg IN ITEMS ${ARGN})
      string(TOUPPER "${_arg}" _arg_uppercase)
      if(_arg_uppercase STREQUAL "REQUIRED")
          set(_flag_is_required TRUE)
      else()
          list(APPEND _list_of_flags "${_arg}")
      endif()
  endforeach()
  set(_flag_found FALSE)
  # loop over all flags, try to find the first which works
  foreach(flag IN ITEMS ${_list_of_flags})
      unset(_flag_works CACHE)
      if(_lang STREQUAL "C")
          check_c_compiler_flag("${flag}" _flag_works)
      elseif(_lang STREQUAL "CXX")
          check_cxx_compiler_flag("${flag}" _flag_works)
      elseif(_lang STREQUAL "Fortran")
          check_Fortran_compiler_flag("${flag}" _flag_works)
      else()
          message(FATAL_ERROR "Unknown language in set_compiler_flag: ${_lang}")
          endif()
    # if the flag works, use it, and exit
    # otherwise try next flag
    if(_flag_works)
      set(${_result} "${flag}" PARENT_SCOPE)
      set(_flag_found TRUE)
      break()
    endif()
  endforeach()
  # raise an error if no flag was found
  if(_flag_is_required AND NOT _flag_found)
      message(FATAL_ERROR "None of the required flags were supported")
  endif()
endfunction()
```

通过调用这个函数来实现编译器标志的设置：

```cmake
set_compiler_flag(
	working_compile_flag C REQUIRED
	"-foo" "-Wall" "-warn all"
)
```

#### add_subdirectory 隔离模块范围

例如，在 `src/evolution/CMakeLists.txt` 当中：

```cmake
add_library(evolution "")  # 定义库名
 
# 定义它的源和包含目录，以及它们的目标可见性
target_sources(evolution
  PRIVATE
      evolution.cpp
  PUBLIC
      ${CMAKE_CURRENT_LIST_DIR}/evolution.hpp
)

target_link_directories(evolution
	PUBLIC
		${CMAKE_CURRENT_LIST_DIR}
)
```

在更上一层的srcCMakeLists.txt 当中：

```cmake
add_executable(automata main.cpp)

add_subdirectory(evolution)
add_subdirectory(initial)
add_subdirectory(io)
add_subdirectory(parser)

target_link_libraries(automata
  PRIVATE
    conversion
    evolution
    initial
    io
    parser
  )
```







### 打包项目



### 文档构建









## 常用命令

### 源文件


```cmake
include_directories([AFTER|BEFORE] [SYSTEM] dir1 [dir2 ...])
```

  将包含目录添加到构建中，将给定目录添加到编译器用于搜索包含文件的目录。相对路径被解释为相对于当前源目录。

### 可执行文件

```cmake
add_executable(<name> [WIN32] [MACOSX_BUNDLE]
               [EXCLUDE_FROM_ALL]
               source1 [source2 ...])
```

通过指定的源文件来添加执行文件到项目中。

其他形式：`add_executable(<name> ALIAS <target>)` 和  `add_executable(<name> IMPORTED [GLOBAL])`



### 添加和链接库

`add_library`主要作用是将指定的源文件生成链接文件，然后添加到工程当中。

语法如下：

```cmake
add_library(<name> [STATIC | SHARED | MODULE ] [EXCLUDE_FROM_ALL] [source1] [source2][...])
```

* `STATIC` `SHARED` `MODULE` 用于指定库的不同形式。

`target_linkl_libraries` 将目标文件与库文件进行链接：

```cmake
target_link_libraries(<target> [item1] [item2] [...] [[debug | optimized | general] <item>] ...)
```

`<target>`是指通过`add_executable()`和`add_library()`指令生成已经创建的目标文件。而`[item]`表示库文件没有后缀的名字。



**使用实例：**

在子目录中添加：

```cmake
add_library(MathFunction mysqrt.cxx)
```

在顶层的目录中添加

```cmake
# add the MathFunctions library
add_subdirectory(MathFunctions)

# add the executable
add_executable(Tutorial tutorial.cxx)

target_link_libraries(Tutorial PUBLIC MathFunctions)

# add the binary tree to the search path for include files
# so that we will find TutorialConfig.h
target_include_directories(Tutorial PUBLIC
                          "${PROJECT_BINARY_DIR}"
                          "${PROJECT_SOURCE_DIR}/MathFunctions"
                          )
```



安装：

```cmake
# 在MathFunction的子目录下添加
install(TARGETS MathFunctions DESTINATION lib)
install(FILES MathFunctions.h DESTINATION include)
# 在顶层目录上添加
install(TARGETS Tutorial DESTINATION bin)
install(FILES "${PROJECT_BINARY_DIR}/TutorialConfig.h"
	DESTINATION include)
```

> 三个关键字的理解：
>
> 参考：
>
> https://kubasejdak.com/modern-cmake-is-like-inheritance
>
> https://leimao.github.io/blog/CMake-Public-Private-Interface/

![image-20220419101020362](https://img-bed-l.oss-cn-beijing.aliyuncs.com/pic_bed/cmake%E5%85%B3%E9%94%AE%E5%AD%97.png)

![image-20220419101053158](https://img-bed-l.oss-cn-beijing.aliyuncs.com/pic_bed/cmake%E9%93%BE%E6%8E%A5%E7%BB%A7%E6%89%BF.png)



* `find_package_handle_standard_args`提供了，用于处理与查找相关程序和库的标准工具。

```cmake
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(NumPy
  FOUND_VAR NumPy_FOUND
  REQUIRED_VARS NumPy
  VERSION_VAR _numpy_version
  )
```











