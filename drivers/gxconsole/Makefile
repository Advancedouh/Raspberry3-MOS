# Makefile for gxconsole module
#
# Written by ChenJiYuan GuoYanPei

INCLUDES 	  := -I../../include/

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $<

.PHONY: clean

all: console.o

clean:
	rm -rf *.o *~

include ../../include.mk
