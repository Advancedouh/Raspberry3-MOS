# Makefile for kernel module
#
# Written by ChenJiYuan GuoYanPei

INCLUDES := -I../include

%.o: %.S
	$(CC) $(CFLAGS) $(INCLUDES) -c $<

%.o: %.c
	$(CC) $(DEFS) $(CFLAGS) $(INCLUDES) -c $<

.PHONY: clean 

all: main.o exception.o time.o irq_init.o exception_handler.o env.o \
     env_asm.o kernel_elfloader.o sched.o syscall_all.o

clean:
	rm -rf *~ *.o

include ../include.mk

