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
