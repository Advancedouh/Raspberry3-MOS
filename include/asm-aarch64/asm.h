#pragma once

#define __ASM__

#define BEGIN_FUNC(_name)	\
	.global _name;		\
	.type   _name, %function;	\
_name:

#define END_FUNC(_name)		\
	.size _name, .-_name

#define	EXPORT(symbol)		\
	.globl	symbol;		\
	symbol:

#define LOCAL_DATA(x)	\
	.type x,1;	\
x:

#define DATA(x)		\
	.global x;	\
	.hidden x;	\
	LOCAL_DATA(x)

#define END_DATA(x)	\
	.size x, .-x

#define PRINTREG(x) \
	mov x6, #0xFFFF; \
	lsl x6, x6, #16; \
	eor x6, x6, #0xFF80; \
	lsl x6, x6, #8; \
    eor x6, x6, #0x3F; \
	lsl x6, x6, #8; \
	eor x6, x6, #0x20; \
    lsl x6, x6, #16; \
    eor x6, x6, #0x1000; \
    mov x7, x; \
    mov x3, #10; \
6: \
    cbz x7, 7f; \
    udiv x4, x7, x3; \
    msub x5, x4, x3, x7; \
    add x5, x5, #48; \
    str x5, [x6]; \
    mov x7, x4; \
    b 6b; \
7: \
    mov x5, #48; \
    str x5, [x6]; \
    mov x5, #10; \
    str x5, [x6]; \

#define	SAVE_ALL \
	sub	sp, sp, #288; \
	stp	x29, x30, [sp, #16 * 17]; \
	stp	x27, x28, [sp, #16 * 16]; \
	stp	x25, x26, [sp, #16 * 15]; \
	stp	x23, x24, [sp, #16 * 14]; \
	stp	x21, x22, [sp, #16 * 13]; \
	stp	x19, x20, [sp, #16 * 12]; \
	stp	x17, x18, [sp, #16 * 11]; \
	stp	x15, x16, [sp, #16 * 10]; \
	stp	x13, x14, [sp, #16 * 9]; \
	stp	x11, x12, [sp, #16 * 8]; \
	stp	x9, x10, [sp, #16 * 7]; \
	stp	x7, x8, [sp, #16 * 6]; \
	stp	x5, x6, [sp, #16 * 5]; \
	stp	x3, x4, [sp, #16 * 4]; \
	stp	x1, x2, [sp, #16 * 3]; \
	mrs x9, CNTP_TVAL_EL0; \
	stp x9, x0, [sp, #16 * 2]; \
	mrs	x10, sp_el0; \
	mrs	x11, elr_el1; \
	mrs	x12, spsr_el1; \
	stp	x11, x12, [sp, #16 * 1]; \
	mov	x13, sp; \
	mrs x9, far_el1; \
	stp	x9, x10, [sp, #16 * 0]; \
	ldr x14, =TRAPSP; \
	str x13, [x14]

#define CORRECT_NUM 0x3c0
#define RECOVER_ALL \
	mov x0, sp; \
	ldp	x9, x10, [sp, #16 * 0]; \
	ldp	x11, x12, [sp, #16 * 1]; \
	msr	spsr_el1, x12; \
	msr far_el1, x9; \
	ldp	x9, x0, [sp, #16 * 2]; \
	msr elr_el1, x11; \
	msr sp_el0, x10; \
	msr	CNTP_TVAL_EL0, x9; \
	ldp	x1, x2, [sp, #16 * 3]; \
	ldp	x3, x4, [sp, #16 * 4]; \
	ldp	x5, x6, [sp, #16 * 5]; \
	ldp	x7, x8, [sp, #16 * 6]; \
	ldp	x9, x10, [sp, #16 * 7]; \
	ldp	x11, x12, [sp, #16 * 8]; \
	ldp	x13, x14, [sp, #16 * 9]; \
	ldp	x15, x16, [sp, #16 * 10]; \
	ldp	x17, x18, [sp, #16 * 11]; \
	ldp	x19, x20, [sp, #16 * 12]; \
	ldp	x21, x22, [sp, #16 * 13]; \
	ldp	x23, x24, [sp, #16 * 14]; \
	ldp	x25, x26, [sp, #16 * 15]; \
	ldp	x27, x28, [sp, #16 * 16]; \
	add	sp, sp, #288;\
	IRQ_OPEN \
	ldp	x29, x30, [sp, #-16];