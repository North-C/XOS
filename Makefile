#!Makefile
# 文件目录设置
BUILD_DIR = build# 生成的.o文件目录
SOURCE_DIR = boot init arch/i386/kernel kernel lib mm # 源文件目录 
ASSEMBLY_DIR = boot 				# 汇编文件目录

vpath %.c $(SOURCE_DIR)       # 寻找.c文件依赖时，自动到 $(SOURCE_DIR)下寻找
# 递归查找，暂时失败
# rwildcard=$(foreach d,$(wildcard $(addsuffix *,$(1))),$(call rwildcard,$(d)/,$(2))$(filter $(subst *,%,$(2)),$(d)))

# .c 源文件和目标文件,递归查找
C_SOURCES = $(foreach dir, $(SOURCE_DIR), $(wildcard $(dir)/*.c))
#C_SOURCES = $(shell find . -name "*.c")
C_OBJECTS = $(addprefix $(BUILD_DIR)/, $(patsubst %.c, %.o, $(notdir $(C_SOURCES))))

# .s 源文件和目标文件
S_SOURCES = $(foreach dir, $(ASSEMBLY_DIR), $(wildcard $(dir)/*.s))
# S_SOURCES = $(shell find . -name "*.s")
S_OBJECTS = $(addprefix $(BUILD_DIR)/, $(patsubst %.s, %.o, $(notdir $(S_SOURCES))))


# 编译
CC = gcc
LD = ld
ASM = nasm

C_FLAGS = -I ./include/ -c -fno-builtin -m32 -fno-stack-protector -nostdinc -fno-pic -gdwarf-2
LD_FLAGS = -T scripts/kernel.ld -m elf_i386 -nostdlib -Map ${BUILD_DIR}/kernel.map
ASM_FLAGS = -f elf -g -F stabs

all: $(S_OBJECTS) $(C_OBJECTS) link update_image

# 编译 .c 文件 
$(BUILD_DIR)/%.o: %.c
	@echo 编译代码文件 $< ...
	$(CC) $(C_FLAGS) $< -o $@

# 编译 .s文件
$(BUILD_DIR)/%.o: %.s
	@echo 编译汇编文件 $< ...
	$(ASM) $(ASM_FLAGS) $< -o $@		

link:
	@echo 链接内核文件...
	$(LD) $(LD_FLAGS) $(S_OBJECTS) $(C_OBJECTS) -o ${BUILD_DIR}/kernel.bin

.PHONY:grub
grub:
	$(ASM) $(ASM_FLAGS) ./boot/grub_head.s -o ./build/grub_head.o

.PHONY:clean
clean:
	$(RM) $(S_OBJECTS) $(C_OBJECTS) kernel.bin

.PHONY:update_image
update_image:
	cp ./build/kernel.bin ./hdisk/boot/
#	sleep 1

.PHONY:mount_image
mount_image:
	mount -o loop ./hda.img ./hdisk/

.PHONY:umount_image
umount_image:
	umount ./hdisk

.PHONY:qemu
qemu:
	qemu-system-i386  -serial stdio -s -S -drive file=./hda.img,format=raw,index=0,media=disk -m 512

# .PHONY:bochs
# bochs:
#     bochs -f tools/bochsrc.txt

.PHONY:debug
debug:
    #qemu-system-i386  -serial stdio -s -S -drive file=./hda.img,format=raw,index=0,media=disk -m 512
    #sleep 1
	gdb -x ./gdbinit

