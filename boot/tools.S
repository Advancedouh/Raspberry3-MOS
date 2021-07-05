#include <asm-aarch64/asm.h>
#include <registers.h>

.extern boot_ttbr0_l0
.extern boot_ttbr1_l0

#define MAIR_NUM					(0x448804)
#define SCR_NUM						(0x30d51825)
#define TCR_NUM						(0x603a193a19)

BEGIN_FUNC(el1_mmu_activate)
	stp     x29, x30, [sp, #-16]!
	mov     x29, sp

	/* Initialize Memory Attribute Indirection Register */
	ldr		x8, =MAIR_NUM
	msr		mair_el1, x8

	/* Initialize TCR_EL1 */
	/* set cacheable attributes on translation walk */
	/* (SMP extensions) non-shareable, inner write-back write-allocate */
	ldr		x8,	=TCR_NUM
	msr		tcr_el1, x8
	isb

	// Write ttbr with phys addr of the translation table
	adrp    x8, boot_ttbr0_l1
	msr     ttbr0_el1, x8
	adrp    x8, boot_ttbr1_l1
	msr     ttbr1_el1, x8
	isb

	//mrs     x8, sctlr_el1
	/* Enable MMU */
	ldr		x8, =SCR_NUM
	msr		sctlr_el1, x8
    isb

	#define KERNBASE 0xffffff8000000000
	ldr 	x8, =KERNBASE
	add		sp, sp, x8

	ldp     x29, x30, [sp], #16
	ret
END_FUNC(el1_mmu_activate)
