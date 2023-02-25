losetup_device=`losetup -f`
losetup $losetup_device ./hda.img
mkfs -q $losetup_device

if [ ! -d "./hdisk" ]; then
	mkdir ./hdisk
fi

mount -o loop ./hda.img ./hdisk

if [ ! -d "./hdisk/boot" ]; then
	mkdir ./hdisk/boot
	grub-install --boot-directory=./hdisk/boot/ --force --allow-floppy $losetup_device
fi

# .S compile: gcc -I./include -c -fno-builtin -m32 -fno-stack-protector -nostdinc -fno-pic -gdwarf-2 ./boot/grub_head.S -o grub_head.o

# .s nasm -f elf32 -g -F stabs grub_head.s -o grub_head.o