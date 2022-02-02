# Common includes in Makefile
#
# Written By ChenJiYuan GuoYanPei

# CROSS_COMPILE := aarch64-elf-
CROSS_COMPILE := aarch64-none-elf-
CC			  := $(CROSS_COMPILE)gcc
CFLAGS		  := -Wall -O2 -ffreestanding -fno-stack-protector -nostdinc -nostdlib -nostartfiles -mcmodel=large
LD			  := $(CROSS_COMPILE)ld
LDFLAGS       :=
OBJCOPY		  := $(CROSS_COMPILE)objcopy
OBJDUMP       := $(CROSS_COMPILE)objdump
