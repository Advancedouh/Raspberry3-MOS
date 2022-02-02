OUTPUT_ARCH(aarch64)
ENTRY(_start)

BOOT_ADDRESS = 0x80000;
BASE_ADDRESS = 0xffffff8000000000;

SECTIONS {
    . = BOOT_ADDRESS;

    .init : { 
        KEEP(*(.boot))
        boot/main.o
		boot/init.o
		boot/tools.o
	}

    . = ALIGN(1 << 21);

    init_end = ABSOLUTE(.);

    .text BASE_ADDRESS + init_end : AT(init_end) {
        *(.text*)
    }

    . = ALIGN(4096);

	PROVIDE(_sdata = . - BASE_ADDRESS);

    .data : {
        *(.data*)
    }

    . = ALIGN(4096);

    .rodata : {
        *(.rodata*)
    }
    _edata = . - BASE_ADDRESS;

    . = ALIGN(4096);

    __bss_start = . - BASE_ADDRESS;
    .bss : {
        *(.bss*)
    }
    __bss_end = . - BASE_ADDRESS;

    . = ALIGN(4096);
    
    img_end = . - BASE_ADDRESS;
}

__bss_size = (__bss_end - __bss_start) >> 3;
