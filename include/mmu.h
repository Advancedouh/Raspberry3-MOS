#ifndef _MMU_H_
#define _MMU_H_
#include "type.h"

#define BY2PG		4096
#define PTE2PT      512

#define GET_L1_INDEX(x) (((x) >> (12 + 9 + 9)) & 0x1ff)
#define GET_L2_INDEX(x) (((x) >> (12 + 9)) & 0x1ff)
#define GET_L3_INDEX(x) (((x) >> (12)) & 0x1ff)

#define PTE_ADDR(pte)	(((u64)(pte))&(0x3ffff000))
#define GET_PTE_PERM(x) ((x) ^ (PTE_ADDR(x)))

#define PPN(va)		(((u64)(va))>>12)
#define VPN(va)		PPN(va)

#define SELF_PD(va)      ((va) >> (12 + 9 + 9))
#define SELF_PM(va)      ((va) >> (12 + 9))
#define SELF_PT(va)      ((va) >> (12))

#define VA2PFN(va)		(((u64)(va)) & 0xFFFFF000)

/* page table attribute */
#define VALID          (0x1UL << 0)
#define TABLE          (0x1UL << 1)		// if not set, the page table must be a block

#define PXN			   (0x1UL << 53)
#define UXN	       	   (0x1UL << 54)

#define ACCESSED       (0x1UL << 10)

#define OUTER_SHARABLE (0x2UL << 8)
#define INNER_SHARABLE (0x3UL << 8)

#define NORMAL_MEMORY    (0x0UL << 2)
#define DEVICE_MEMORY    (0x1UL << 2)
#define NON_CACHE_MEMORY (0x2UL << 2)

#define PTE_KERN	(0x0UL << 6)
#define PTE_USER	(0x1UL << 6)
#define PTE_RW		(0x0UL << 7)
#define PTE_RO		(0x1UL << 7)

#define PTE_LIBRARY (0x1UL << 55)
#define PTE_DIRTY   (0x1UL << 51)


#define KERNBASE 0xffffff8000000000
#define USTACKTOP (0x3f000000 - BY2PG)
#define UXSTACKTOP 0x7f00000000

#ifndef __ASSEMBLER__

#define VADDR(va)   (va & 0x7fffffffff)

// translates from kernel virtual address to physical address.
#define PADDR(kva)  (((u64)(kva)) - KERNBASE)

// translates from physical address to kernel virtual address.
#define KADDR(pa)   (((u64)(pa)) + KERNBASE)

#define assert(x)   \
    do {    if(!(x)) panic("assertion failed : %s", #x); } while(0)

#define UVPT 0x4000000000
#define UVPM (UVPT + (UVPT >> 9))
#define UVPD (UVPM + (UVPT >> 18))

extern volatile u64 *vpt[];
extern volatile u64 *vpm[];
extern volatile u64 *vpd[];

#endif //!__ASSEMBLER__
#endif // !_MMU_H_
