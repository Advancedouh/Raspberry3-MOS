# Makefile for boot module
#
# Written by ChenJiYuan GuoYanPei

INCLUDES	  := -I../include/

%.o: %.S
	$(CC) $(CFLAGS) $(INCLUDES) -c $< 

%.o: %.c
	$(CC) $(DEFS) $(CFLAGS) $(INCLUDES) -c $<

.PHONY: clean

all: start.o mmu.o init.o main.o uart.o mailbox.o

clean:
	rm -rf *~ *.o

include ../include.mk
