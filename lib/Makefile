# Makefile for library module
#
# Written by ChenJiYuan GuoYanPei

INCLUDES 	  := -I./ -I../include/

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $<
	$(OBJDUMP) -D $@ >$@.txt

%.o: %.S
	$(CC) $(CFLAGS) $(INCLUDES) -c $<

.PHONY: clean

all: print.o printf.o delay.o sd.o cache.o

clean:
	rm -rf *~ *.o

include ../include.mk
