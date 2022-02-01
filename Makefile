# Main makefile
#
# Written by ChenJiYuan GuoYanPei

QEMU          := qemu-system-aarch64
GDB           := gdb-multiarch

drivers_dir	  := drivers
boot_dir	  := boot
kernel_dir	  := kernel
lib_dir		  := lib
tools_dir	  := tools
user_dir      := user
mm_dir		  := mm
fs_dir		  := fs
kernel_elf	  := qemu/kernel.elf
kernel_img	  := qemu/kernel.img

link_script   := $(tools_dir)/qemu.ld
# link_script   := $(tools_dir)/raspi3.ld

modules		  := boot drivers kernel lib mm user fs
objects		  := $(boot_dir)/*.o \
				 $(kernel_dir)/*.o \
			   	 $(drivers_dir)/gxconsole/console.o \
				 $(lib_dir)/*.o \
				 $(mm_dir)/*.o \
				 $(user_dir)/*.x \
				 $(fs_dir)/*.x

.PHONY: all $(modules) build clean

all: $(modules) build

build: $(modules)
	$(LD) -nostdlib $(objects) -T $(link_script) -o $(kernel_elf)
	$(OBJCOPY) -O binary $(kernel_elf) $(kernel_img)

$(modules): 
	$(MAKE) --directory=$@

clean: 
	for d in $(modules);	\
		do					\
			$(MAKE) --directory=$$d clean; \
		done; \
	rm -rf *.o *~ $(kernel_elf)
	cd ./qemu && rm -rf *.img *.txt *.elf && cd ..

gdb:
	gdb-multiarch -n -x .gdbinit

qemu-gdb: $(kernel_img)
	$(QEMU) -nographic $(QEMUOPTS) -S

run: clean all
	$(QEMU) -M raspi3 -kernel $(kernel_img) -drive file=qemu/fs.img,if=sd,format=raw -serial stdio

int: clean all
	$(QEMU) -M raspi3 -kernel $(kernel_img) -drive file=qemu/fs.img,if=sd,format=raw -serial stdio -d int

asm: clean all
	$(QEMU) -M raspi3 -kernel $(kernel_img) -drive file=qemu/fs.img,if=sd,format=raw -serial stdio -d in_asm

decompile: clean all
	$(OBJDUMP) -D qemu/kernel.elf >qemu/kernel.txt

include include.mk
