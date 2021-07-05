# Common includes in Makefile
#
# Written By ChenJiYuan GuoYanPei

vmlinux_img   := qemu/kernel8.img
CROSS_COMPILE :=  aarch64-elf-
CC			  := $(CROSS_COMPILE)gcc
CFLAGS		  := -Wall -O2 -ffreestanding -fno-stack-protector -nostdinc -nostdlib -nostartfiles
LD			  := $(CROSS_COMPILE)ld
OBJCOPY		  := $(CROSS_COMPILE)objcopy
