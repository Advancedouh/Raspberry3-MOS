# Makefile for gxconsole module
#
# Written By ChenJiYuan GuoYanPei

drivers		  := gxconsole

.PHONY:	all $(drivers) 

all: $(drivers)

$(drivers):
	$(MAKE) --directory=$@

clean: 
	for d in $(drivers); \
		do				\
			$(MAKE) --directory=$$d clean; \
		done

include ../include.mk
