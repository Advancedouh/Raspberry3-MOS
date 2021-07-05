#include<printf.h>
#include<image.h>

int main() {

    uart_init();
	uart_puts("[BOOT]Install boot page table\n");
	init_boot_pt();

	uart_puts("[BOOT]Enable mmu\n");
	el1_mmu_activate();

	printf("[BOOT]Jump to kernel main\n");

	kernel_main();

    panic("[BOOT]main is over is error!");

    return 0;
}
