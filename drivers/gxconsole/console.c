#include <pi.h>
#include <image.h>

#define PUTCHAR_ADDRESS (((void*)UART0_DR) + KERNBASE)
#define DATA_STATUS_ADDRESS (((void*)UART0_FR) + KERNBASE)

void printcharc(char c) {
    do {
    	asm volatile("nop");
    } while (*((volatile unsigned int*)DATA_STATUS_ADDRESS) & 0x20);

    // write the character to the buffer
    *((volatile unsigned int*)PUTCHAR_ADDRESS) = c;
}

char readcharc() {
    do {
    	asm volatile("nop");
    } while (*((volatile unsigned int*)DATA_STATUS_ADDRESS) & 0x10);

    char c = *((volatile unsigned char*)PUTCHAR_ADDRESS);
    printcharc(c);
    return c;
}


void printstr(char *s) {
    while(*s) {
        // convert newline to carrige return + newline
        if(*s=='\n')
            printcharc('\r');
       	printcharc(*s++);
    }
}
