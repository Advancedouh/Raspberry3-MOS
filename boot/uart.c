//
// Created by chenjiyuan on 2022/2/22.
//

#include <pi.h>
#include <mailbox.h>

void uart_init()
{
    register unsigned int r;

    /* initialize UART */
    *UART0_CR = 0;         // turn off UART0

    /* set up clock for consistent divisor values */
    mbox[0] = 9*4;
    mbox[1] = MBOX_REQUEST;
    mbox[2] = MBOX_TAG_SETCLKRATE; // set clock rate
    mbox[3] = 12;
    mbox[4] = 8;
    mbox[5] = 2;           // UART clock
    mbox[6] = 4000000;     // 4Mhz
    mbox[7] = 0;           // clear turbo
    mbox[8] = MBOX_TAG_LAST;
    mbox_call(MBOX_CH_PROP);

    /* map UART0 to GPIO pins */
    r=*GPFSEL1;
    r&=~((7<<12)|(7<<15)); // gpio14, gpio15
    r|=(4<<12)|(4<<15);    // alt0
    *GPFSEL1 = r;
    *GPPUD = 0;            // enable pins 14 and 15
    r=150; while(r--) { asm volatile("nop"); }
    *GPPUDCLK0 = (1<<14)|(1<<15);
    r=150; while(r--) { asm volatile("nop"); }
    *GPPUDCLK0 = 0;        // flush GPIO setup

    *UART0_ICR = 0x7FF;    // clear interrupts
    *UART0_IBRD = 2;       // 115200 baud
    *UART0_FBRD = 0xB;
    *UART0_LCRH = 0x7<<4;  // 8n1, enable FIFOs
    *UART0_CR = 0x301;     // enable Tx, Rx, UART
}

void uart_send(unsigned int c) {
    // wait until we can send
    do{asm volatile("nop");}while(*UART0_FR&0x20);
    // write the character to the buffer
    *UART0_DR=c;
}

void uart_puts(char *s) {
    while(*s) {
        // convert newline to carrige return + newline
        if(*s=='\n')
            uart_send('\r');
        uart_send(*s++);
    }
}