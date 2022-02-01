INCLUDES = -I../include/

USERLIB := libos.o \
		   syscall_lib.o \
		   syscall_wrap.o \
		   writef.o \
		   ipc.o \
		   fork.o \
		   pgfault.o \
		   string.o \
		   pageref.o \
		   fd.o \
		   file.o \
		   fsipc.o \
		   pipe.o \
		   wait.o \
		   spawn.o \
		   console.o \
		   fprintf.o \
		   cursor.o

USERAPP := init.b \
		   sh.b \
		   cat.b \
		   ls.b \
		   touch.b \
		   mkdir.b \
		   echo.b

all: idle1.x idle2.x pingpong.x pingpong1.x pingpong2.x fktest.x fstest.x testpipe.x testpiperace.x icode.x $(USERAPP) $(USERLIB) entry.o

%.x: %.b.c
	echo cc1 $<
	$(CC) $(CFLAGS) -c -o $@ $<

%.b.c: %.b
	echo create $@
	echo bintoc $* $< > $@~
	chmod +x ./bintoc
	./bintoc $* $< > $@~ && mv -f $@~ $@
	
%.b: entry.o %.o $(USERLIB)
	echo ld $@
	$(LD) -o $@ $(LDFLAGS) -G 0 -static -n -nostdlib -T user.ld $^
	aarch64-none-elf-objdump -D $@ >$@.txt

%.o: %.c
	echo user1 cc $<
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<

%.o: %.S
	echo as $<
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<

%.o: lib.h

.PHONY: clean

clean:
	rm -rf *~ *.o *.b.c *.x *.b *.txt

include ../include.mk
