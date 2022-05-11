#!Makefile
mkfile_path := $(abspath $(lastword $(MAKEFILE_LIST)))
PROJECT_PATH := $(patsubst %/,%,$(dir $(mkfile_path)))
BUILD_DIR = ${PROJECT_PATH}/build
C_OBJECTS = $(addprefix ${BUILD_DIR}/, $(notdir $(patsubst %.c, %.o, $(C_SOURCES)))) # 暂时没有修改完成

C_SOURCES = $(shell find . -name "*.c")
C_OBJECTS = $(patsubst %.c, %.o, $(C_SOURCES))
S_SOURCES = $(shell find . -name "*.s")
S_OBJECTS = $(patsubst %.s, %.o, $(S_SOURCES))

CC = gcc
LD = ld
ASM = nasm

C_FLAGS = -I ./include/ -c -fno-builtin -m32 -fno-stack-protector -nostdinc -fno-pic -gdwarf-2
LD_FLAGS = -T scripts/kernel.ld -m elf_i386 -nostdlib -Map ${BUILD_DIR}/kernel.map
ASM_FLAGS = -f elf -g -F stabs

all: $(S_OBJECTS) $(C_OBJECTS) link update_image

%.o: %.c
	@echo 编译代码文件 $< ...
	$(CC) $(C_FLAGS) $< -o $@
	
%.o: %.s
	@echo 编译汇编文件 $< ...
	$(ASM) $(ASM_FLAGS) $<

link:
	@echo 链接内核文件...
	$(LD) $(LD_FLAGS) $(S_OBJECTS) $(C_OBJECTS) -o ${BUILD_DIR}/kernel.bin

.PHONY:clean
clean:
	$(RM) $(S_OBJECTS) $(C_OBJECTS) kernel.bin

.PHONY:update_image
update_image:
	cp ./build/kernel.bin ./hdisk/boot/
	sleep 1

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

