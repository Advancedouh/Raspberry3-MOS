#include<image.h>
#include<pmap.h>

typedef unsigned long u64;
typedef unsigned int u32;

// Physical memory address space: 0-1G
#define PHYSMEM_START	(0x0UL)
#define PERIPHERAL_BASE (0x3F000000UL)
#define PHYSMEM_END	(0x40000000UL)

// The number of entries in one page table page
#define PTP_ENTRIES 512
// The size of one page table page
#define PTP_SIZE 4096
#define ALIGN(n) __attribute__((__aligned__(n)))

u64 boot_ttbr0_l1[PTP_ENTRIES] ALIGN(PTP_SIZE);
u64 boot_ttbr0_l2[PTP_ENTRIES] ALIGN(PTP_SIZE);
u64 boot_ttbr0_l3[PTP_ENTRIES << 9] ALIGN(PTP_SIZE);

u64 boot_ttbr1_l1[PTP_ENTRIES] ALIGN(PTP_SIZE);
u64 boot_ttbr1_l2[PTP_ENTRIES] ALIGN(PTP_SIZE);
u64 boot_ttbr1_l3[PTP_ENTRIES << 9] ALIGN(PTP_SIZE);
extern u64* User_pgdir;
extern u64* Kern_pgdir;

u64 vis[1 << 12];
struct Page pages[1 << 18];
struct Env envs[1024];

#define SIZE_2M  (2UL*1024*1024)

void init_boot_pt(void) {
	int start_entry_idx;
	int end_entry_idx;
	int i;
	long long kva;
	extern char init_end[];
	extern char img_end[];

	for (i = 0; i < (1 << 18); i++) {
		pages[i].pa = (i << 12) | VALID | TABLE | ACCESSED;
		if ((i << 12) >= PERIPHERAL_BASE)
			pages[i].pa |= OUTER_SHARABLE | DEVICE_MEMORY;
		else
			pages[i].pa |= INNER_SHARABLE | NORMAL_MEMORY;
	}

	boot_ttbr0_l1[0] = ((u64) boot_ttbr0_l2) | TABLE | VALID | ACCESSED;

	for (i = 0; i < ((u64) (init_end)) / SIZE_2M; i++)
		boot_ttbr0_l2[i] = ((u64)(boot_ttbr0_l3 + (i << 9))) | TABLE | VALID | ACCESSED;

	for (i = 0; i < ((u64) (init_end)) / PTP_SIZE; i++) {
		vis[i >> 6] |= (1ul << (i & 63));
		if (i > 0) {
			boot_ttbr0_l3[i] = pages[i].pa;
			pages[i].ref++;
			pages[i].pa |= UXN;
		}
	}

	start_entry_idx = PERIPHERAL_BASE;
	end_entry_idx = PHYSMEM_END;

	//kernel page table init
	boot_ttbr1_l1[0] = ((u64) boot_ttbr1_l2) | TABLE | VALID | ACCESSED;

	for (i = 0; i < 512; i++)
		boot_ttbr1_l2[i] = ((u64)(boot_ttbr1_l3 + (i << 9))) | TABLE | VALID | ACCESSED;

	for (i = 0; i < (1 << 18); i++) {
		if ((((u64) init_end >> 12) <= i && i < ((u64) img_end >> 12)) || i >= (PERIPHERAL_BASE >> 12)) {
			vis[i >> 6] |= (1ul << (i & 63));
			pages[i].ref++;
		}
		if (i > 0)
			boot_ttbr1_l3[i] = pages[i].pa;
	}
	
	kva = KERNBASE + PHYSMEM_END;
	boot_ttbr1_l1[GET_L1_INDEX(kva)] = PHYSMEM_END | UXN	 //Unprivileged execute never
	    | ACCESSED		// Set access flag
	    | DEVICE_MEMORY	// Device memory
	    | VALID;
}
