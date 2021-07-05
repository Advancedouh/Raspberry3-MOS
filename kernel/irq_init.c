#include "lib.h"

void irq_init() {
	printf("[KERNEL]time_interrupt_init_begin\n");
	int* addr = 0x40000040 + KERNBASE;
	*addr = 0xf; // let pie interrupt
	int x = check_time();
	x = time_init();
	printf("[KERNEL]time_interrupt_init_finish\n");
}