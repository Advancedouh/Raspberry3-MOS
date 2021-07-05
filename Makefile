# Main makefile
#
# Written by ChenJiYuan GuoYanPei
#

drivers_dir	  := drivers
boot_dir	  := boot
kernel_dir	  := kernel
lib_dir		  := lib
tools_dir	  := tools
user_dir      := user
mm_dir		  := mm
fs_dir		  := fs
vmlinux_elf	  := qemu/kernel8.elf
vmlinux_img	  := qemu/kernel8.img

link_script   := $(tools_dir)/link.ld

modules		  := boot drivers kernel lib mm user fs
objects		  := $(boot_dir)/*.o			  \
				 $(kernel_dir)/*.o			  \
			   	 $(drivers_dir)/gxconsole/console.o \
				 $(lib_dir)/*.o				  \
				 $(mm_dir)/*.o				  \
				 $(user_dir)/*.x			  \
				 $(fs_dir)/*.x

.PHONY: all $(modules) clean

all: $(modules) kernel8

kernel8: $(modules)
	$(LD) -nostdlib -nostartfiles $(objects) -T $(link_script) -o $(vmlinux_elf)
	$(OBJCOPY) -O binary $(vmlinux_elf) $(vmlinux_img)

$(modules): 
	$(MAKE) --directory=$@

clean: 
	for d in $(modules);	\
		do					\
			$(MAKE) --directory=$$d clean; \
		done; \
	rm -rf *.o *~ $(vmlinux_elf)

gdb:
	gdb-multiarch -n -x .gdbinit

qemu-gdb: $(vmlinux_img)
	qemu-system-aarch64 -nographic $(QEMUOPTS) -S

run: clean all
	qemu-system-aarch64 -M raspi3 -kernel $(vmlinux_img) -drive file=qemu/fs.img,if=sd,format=raw -serial stdio

debug: clean all
	qemu-system-aarch64 -M raspi3 -kernel $(vmlinux_img) -drive file=qemu/fs.img,if=sd,format=raw -serial stdio -d int

asm: clean all
	qemu-system-aarch64 -M raspi3 -kernel $(vmlinux_img) -drive file=qemu/fs.img,if=sd,format=raw -serial stdio -d in_asm

decompile: clean all
	aarch64-elf-objdump -D qemu/kernel8.elf >qemu/kernel.txt
	
include include.mk
