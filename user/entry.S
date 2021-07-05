#include "time.h"
#include <asm-aarch64/asm.h>

#define USTACKTOP (0x3f000000 - 4096)

#define UVPT 0x4000000000
#define UVPM (UVPT + (UVPT >> 9))
#define UVPD (UVPM + (UVPT >> 18))

	.p2align 12
.data
	.global fsipcbuf
fsipcbuf:
	.space 4096

	.global fdtab
fdtab:
	.space 4096

	.global vpt
vpt:
	.dword UVPT

	.global vpm
vpm:
	.dword UVPM

	.global vpd
vpd:
	.dword UVPD

	.global __pgfault_handler
__pgfault_handler:
	.dword 0

.text
	.global user_start
user_start:
	//msr		daifclr, #2
	ldr	x0, =USTACKTOP
	cmp  sp, x0
	beq  1f
	ldr x0, [sp, #0]
	ldr x1, [sp, #8]
1:
    bl	libmain

.text
	.global __asm_pgfault_handler
__asm_pgfault_handler:
	ldr x0, [sp, #0]
	ldr x10, __pgfault_handler
	blr x10
	mov x17, sp
	ldp	xzr, x10, [sp, #16 * 0]
	mov sp, x10
	ldp	xzr, x0, [x17, #16 * 2] 
	ldp	x1, x2, [x17, #16 * 3] 
	ldp	x3, x4, [x17, #16 * 4] 
	ldp	x5, x6, [x17, #16 * 5] 
	ldp	x7, x8, [x17, #16 * 6] 
	ldp	x9, x10, [x17, #16 * 7] 
	ldp	x11, x12, [x17, #16 * 8] 
	ldp	x13, x14, [x17, #16 * 9] 
	ldp	x15, x16, [x17, #16 * 10] 
	ldp	xzr, x18, [x17, #16 * 11] 
	ldp	x19, x20, [x17, #16 * 12] 
	ldp	x21, x22, [x17, #16 * 13] 
	ldp	x23, x24, [x17, #16 * 14] 
	ldp	x25, x26, [x17, #16 * 15] 
	ldp	x27, x28, [x17, #16 * 16]
	ldp	x29, x30, [x17, #16 * 17]
	ldr x17, [x17, #16]
	br  x17
