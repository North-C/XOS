qemu-system-i386  -serial stdio -s -S -drive file=./hda.img,format=raw,index=0,media=disk -m 512
# qemu-system-i386 -d trace:cpu:int:exec -serial stdio -s -S -hda ./hda.img -m 512
