# Makefile for gxconsole module
#
#
INCLUDES 	  := -I../ -I../../ -I../../include/

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $<

.PHONY: clean
all: console.o

clean:
	rm -rf *.o *~



include ../../include.mk
