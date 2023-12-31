# Makefile for file system module
#
# Written by ChenJiYuan GuoYanPei

INCLUDES = -I../include/

user_dir := ../user

USERLIB := $(user_dir)/libos.o \
		   $(user_dir)/syscall_lib.o \
		   $(user_dir)/syscall_wrap.o \
		   $(user_dir)/writef.o \
		   $(user_dir)/ipc.o \
		   $(user_dir)/fork.o \
		   $(user_dir)/pgfault.o \
		   $(user_dir)/string.o \
		   $(user_dir)/pageref.o \
		   $(user_dir)/file.o \
		   $(user_dir)/fd.o \
		   $(user_dir)/fsipc.o \
		   $(user_dir)/pipe.o \
		   $(user_dir)/wait.o \
		   $(user_dir)/spawn.o \
		   $(user_dir)/console.o \
		   $(user_dir)/fprintf.o \
		   $(user_dir)/cursor.o

FSLIB := sd_card.o fs.o test.o

USERAPPS := $(user_dir)/init.b \
			$(user_dir)/sh.b \
			$(user_dir)/cat.b \
			$(user_dir)/ls.b \
			$(user_dir)/touch.b \
			$(user_dir)/mkdir.b \
			$(user_dir)/echo.b

FSIMGFILES := motd newmotd $(USERAPPS)

FSIMG      := ../target/fs.img

all: sdtest.x serv.x

%.x: %.b.c
	echo cc1 $<
	$(CC) $(CFLAGS) -c -o $@ $<

%.b.c: %.b
	echo create $@
	echo bintoc $* $< > $@~
	chmod +x ./bintoc
	./bintoc $* $< > $@~ && mv -f $@~ $@
	
%.b: ../user/entry.o %.o $(USERLIB) $(FSLIB)
	echo ld $@
	$(LD) -o $@ $(LDFLAGS) -G 0 -static -n -nostdlib -T ../user/user.ld $^
	$(OBJDUMP) -D $@ >$@.txt

%.o: %.c
	echo user1 cc $<
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<

%.o: %.S
	echo as $<
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<

%.o: $(user_dir)/lib.h

image: $(FSIMGFILES)
	dd if=/dev/zero of=$(FSIMG) bs=4096 count=1024 2>/dev/null
	gcc fsformat.c -o fsformat -m64
	chmod +x fsformat
	./fsformat $(FSIMG) $(FSIMGFILES) >fs_debug.txt
	# ./fsformat $(FSIMG) $(FSIMGFILES)
	sudo dd if=$(FSIMG) of=/dev/sdb1 bs=512 skip=0 seek=3000000

.PHONY: clean

clean:
	rm -rf *~ *.o *.b.c *.x *.b *.txt

include ../include.mk
