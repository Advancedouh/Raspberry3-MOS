# Main makefile
#
# Written by ChenJiYuan GuoYanPei
#

# ========= Configuration =========

INCLUDES	  := -I../include/

# ======== End of configuration ===

%.o: %.S
	$(CC) $(CFLAGS) $(INCLUDES) -c $< 

%.o: %.c
	$(CC) $(DEFS) $(CFLAGS) $(INCLUDES) -c $<

.PHONY: clean

all: start.o tools.o init.o main.o

clean:
	rm -rf *~ *.o

include ../include.mk
