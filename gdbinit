file ./hdisk/boot/kernel.bin       
target remote :1234                
define hook-stop                    
x/i $pc
end
b *0x7c00                          
continue
