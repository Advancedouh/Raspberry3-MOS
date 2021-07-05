#include <type.h>
#include "lib.h"
#include <mmu.h>

extern void (*__pgfault_handler)(u64);
extern void __asm_pgfault_handler(void);


void
set_pgfault_handler(void (*fn)(u64 va))
{
	if (__pgfault_handler == 0) {
		if (syscall_mem_alloc(0, UXSTACKTOP - BY2PG, VALID | PTE_RW | PTE_USER) < 0 ||
			syscall_set_pgfault_handler(0, __asm_pgfault_handler, UXSTACKTOP) < 0) {
			writef("cannot set pgfault handler\n");
			return;
		}
    }

	__pgfault_handler = fn;
}