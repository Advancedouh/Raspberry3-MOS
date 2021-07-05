/*SECTIONS
{
    . = 0x80000;
    .init : {
    	KEEP(*(.text.boot))
    	boot/main.o
        boot/init.o
        boot/tools.o
        boot/env.o
    }
    
    . = ALIGN(1 << 21);

    init_end = ABSOLUTE(.);

    .text 0xffffff8000000000 + init_end : AT(init_end) {
        *(.text*)
    }
    . = ALIGN(4096);
    
    PROVIDE(_sdata = . - 0xffffff8000000000);
    .rodata : { *(.rodata .rodata.* .gnu.linkonce.r*) }
   	. = ALIGN(4096);
    .data : { *(.data .data.* .gnu.linkonce.d*) }
    .bss (NOLOAD) : {
        . = ALIGN(16);
        __bss_start = . - 0xffffff8000000000;
        *(.bss .bss.*)
        *(COMMON)
        __bss_end = . - 0xffffff8000000000;
    }
	. = ALIGN(1 << 21);
    img_end = . - 0xffffff8000000000;

   /DISCARD/ : { *(.comment) *(.gnu*) *(.note*) *(.eh_frame*) }

}
__bss_size = (__bss_end - __bss_start)>>3;
*/
/*
SECTIONS
{
    . = 0x80000;
    .text : { KEEP(*(.text.boot)) *(.text .text.* .gnu.linkonce.t*)}
    .rodata : { *(.rodata .rodata.* .gnu.linkonce.r*) }
   	. = ALIGN(4096);
    PROVIDE(_sdata = .);
    .data : { *(.data .data.* .gnu.linkonce.d*) }
    .bss (NOLOAD) : {
        . = ALIGN(16);
        __bss_start = .;
        *(.bss .bss.*)
        *(COMMON)
        __bss_end = .;
    }
	. = ALIGN(4096);
    img_end = .;

   /DISCARD/ : { *(.comment) *(.gnu*) *(.note*) *(.eh_frame*) }

}
__bss_size = (__bss_end - __bss_start)>>3;
*/

ENTRY(_start)
KERNBASE = 0xffffff8000000000;
SECTIONS {
    . = 0x80000;

    .init : { 
        KEEP(*(.text.boot))
        boot/main.o
		boot/init.o
		boot/tools.o
	}

    . = ALIGN(1 << 21);

    init_end = ABSOLUTE(.);

    .text KERNBASE + init_end : AT(init_end) {
        *(.text*)
    }

    . = ALIGN(4096);

	PROVIDE(_sdata = . - KERNBASE);

    .data : {
        *(.data*)
    }

    . = ALIGN(4096);

    .rodata : {
        *(.rodata*)
    }
    _edata = . - KERNBASE;

    . = ALIGN(4096);

    __bss_start = . - KERNBASE;
    .bss : {
        *(.bss*)
    }
    __bss_end = . - KERNBASE;

    . = ALIGN(4096);
    
    img_end = . - KERNBASE;
}

__bss_size = (__bss_end - __bss_start)>>3;
