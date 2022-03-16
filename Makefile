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
kernel_elf	  := target/kernel8.elf
kernel_img	  := target/kernel8.img

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

#QEMUOPTS      := -serial stdio
QEMUOPTS      := -drive file=target/fs.img,if=sd,format=raw -serial stdio

.PHONY: all $(modules) build clean

all: $(modules) build

image:
	$(MAKE) --directory=./fs/ image

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
	cd ./target && rm -rf *.img *.txt *.elf && cd ..

gdb: $(kernel_img)
	gdb-multiarch $(kernel_img)

qemu-gdb: $(kernel_img)
	$(QEMU) -M raspi3 -kernel $(kernel_img) -nographic -drive file=target/fs.img,if=sd,format=raw -s -S

run:
	$(QEMU) -M raspi3 -kernel $(kernel_img) $(QEMUOPTS)

int: clean all
	$(QEMU) -M raspi3 -kernel $(kernel_img) $(QEMUOPTS) -d int

asm: clean all
	$(QEMU) -M raspi3 -kernel $(kernel_img) $(QEMUOPTS) -d in_asm

decompile: clean all
	$(OBJDUMP) -D target/kernel.elf >target/kernel.txt

include include.mk
