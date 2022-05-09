# cmake_minimum_required(VERSION 3.23)

# # set the project name
# project(OS C ASM)
# set(CMAKE_C_STANDARD 11)


# # set(CMAKE_ASM_COMPILER "/usr/bin/as")
# # # 汇编语言设置
# # enable_language(ASM-ATT)
# # set(CMAKE_ASM_SOURCE_FILE_EXTENSIONS )
# # set(can_use_assembler TRUE)

# # # C 编译器设置
# # set(LIB "-I${PROJECT_PATH}/include")
# # set(CMAKE_C_FLAGS " -W $(LIB) -c -fno-builtin -m32 -fno-stack-protector -nostdinc -fno-pic -gdwarf-2")


# # set(ASMFLAGS "-march=i386 -a -f --gstabs")
# # SET(CMAKE_ASM_FLAGS "${ASM_FLAGS}")


# # # 共享库设置的链接设置
# # set(LINK_FLAGS "-m elf_i386 -T ${PROJECT_PATH}/script/kernel.ld -Map ${PROJECT_PATH}/build/kernel.map -nostdlib")

# # set(CMAKE_SHARED_LINKER_FLAGS "${LINK_FLAGS}")
# # set(CMAKE_EXE_LINKER_FLAGS    "${LINK_FLAGS}")


# include_directories(
#     .
#     include/
#     include/linux/
#     include/asm-i386/
# )


# # 设置编译的源文件
# set(SOURCES 
# boot/boot.c boot/grub_head.S  include/linux/init.h include/linux/multiboot.h include/linux/types.h include/asm-i386/boot.h include/asm-i386/gdt.h include/asm-i386/page.h include/asm-i386/types.h)

# # add the executable
# add_executable(OS ${SOURCES})
