OUTPUT_ARCH(aarch64)
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
