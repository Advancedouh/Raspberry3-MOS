#include<macro.h>
#include<image.h>
#include<mmu.h>
#include<pmap.h>
#include<printf.h>
#include<kernel.h>
#include<pgdir.h>
#include<env.h>

extern u64 vis[];
extern struct Page pages[];
extern u64 boot_ttbr0_l1[];
extern u64 boot_ttbr1_l1[];
u64* User_pgdir = boot_ttbr0_l1;
u64* Kern_pgdir = boot_ttbr1_l1;
u64* Kern_vis = (u64)vis + KERNBASE;
struct Page* Kern_pages = (u64)pages + KERNBASE;

int page_alloc(struct Page **pp)
{
    struct Page* tmp;
    int i, j;
    for (i = 0; i < (1 << 12); i++) {
        if (Kern_vis[i] != -1ll) {
            for (j = 0; j < 64; j++) {
                if ((Kern_vis[i] & (1 << j)) == 0) {
                    tmp = Kern_pages + (i << 6 | j);
                    bzero(page2kva(tmp), BY2PG, 0);
                    Kern_vis[i] |= (1 << j);
                    *pp = tmp;
                    return 0;
                }
            }
        }
    }
    return -1;
}

void page_free(struct Page *pp) {
	if (pp->ref > 0) {
		return;
	}
    if (pp->ref == 0) {
        u64 idx = page2ppn(pp);
        Kern_vis[idx >> 6] &= ~(1ul << (idx & 63));
		return;
	}

    panic("gyp:pp->pp_ref is less than zero\n");
}

int time = 0;
int pgdir_walk(u64* pgdir, u64 va, int create, u64 **ppte) {
    u64 *addr = pgdir;
    u64 *pgdir_entryp;
    u64 *pgtable;
    struct Page *ppage;

    int level = 0;
    while(level < 2) {
        level ++;
        switch (level) {
            case 1:
                addr = addr + GET_L1_INDEX(va);
                break;
            case 2:
                addr = addr + GET_L2_INDEX(va);
                break;
            default:
                break;
        }
        if (!((*addr) & VALID)) {
            if (create) {
                struct Page* pp;
                int ret = page_alloc(&pp);
                if (ret < 0) {
                    return -1;
                }
                (*addr) = pp->pa;
                if (va < KERNBASE) {
                    (*addr) |= PTE_USER;
                }
                pp->ref++;
            } else {
                *ppte = 0;
                return 0;
            }
        }
        addr = KADDR(PTE_ADDR(*(addr)));
    }
    *ppte = addr + GET_L3_INDEX(va);
	return 0;
}

int page_insert(u64 *pgdir, struct Page *pp, u64 va, u64 perm) {
	u64 PERM = perm | VALID;
	u64 *pgtable_entry;
	pgdir_walk(pgdir, va, 0, &pgtable_entry);
	if (pgtable_entry != 0 && (*pgtable_entry & VALID)) {
		if (pa2page(*pgtable_entry) != pp) {
			page_remove(pgdir, va);
		} else	{
			tlb_invalidate(pgdir, va);
			*pgtable_entry = (pp->pa | PERM);
			return 0;
		}
	}

	tlb_invalidate(pgdir, va);

	if(pgdir_walk(pgdir, va, 1, &pgtable_entry) != 0) {
		return -1;
	}

	*pgtable_entry = pp->pa | PERM;
	pp->ref++;

	return 0;
}

struct Page* page_lookup(u64 *pgdir, u64 va, u64 **ppte) {
	struct Page *ppage;
	u64 *pte;

	pgdir_walk(pgdir, va, 0, &pte);

	if (pte == 0 || (*pte & VALID) == 0) {
		return 0;
	}

	ppage = pa2page(*pte);
	if (ppte) {
		*ppte = pte;
	}

	return ppage;
}

void page_decref(struct Page* pp) {
    pp->ref--;
    page_free(pp);
}

void page_remove(u64 *pgdir, u64 va) {
	u64 *pagetable_entry;
	struct Page *ppage;

	ppage = page_lookup(pgdir, va, &pagetable_entry);

	if (ppage == 0) {
		return;
	}

	ppage->ref--;
    page_free(ppage);

	*pagetable_entry = 0;
	tlb_invalidate(pgdir, va);
	return;
}

void tlb_invalidate(u64* pgdir, u64 va) {
    flush_tlb();
}

void memory_management_test() {
    printf("[KERNEL]memory_management_test begin\n");
    extern u64* User_pgdir, *Kern_pgdir;
    int *j = 1ll << 35;
	u64* ppte;
	*((int*)KADDR(PTE_ADDR((Kern_pages[60000].pa)))) = 19260817;
	page_insert(User_pgdir, Kern_pages + 50000, j, 0);
	*j = 10000;
    assert((*j) == 10000);
	printf("should be 10000 ok!\n");
	page_insert(User_pgdir, Kern_pages + 60000, j, 0);
	assert((*j) == 19260817);
    printf("should be 19260817 ok!\n");
	page_insert(User_pgdir, Kern_pages + 50000, j, 0);
	assert((*j) == 10000);
    printf("should be 10000 ok!\n");
    /*unsigned long i;
    for (i = 1; i < 1024; ++ i) {
        struct Page* tempage;
        int r = page_alloc(&tempage);
        u64* addr1 = ((1ul << 35) | (i << 20)), *addr2 = KERNBASE + (u64)addr1;
        page_insert(User_pgdir, tempage, addr1, 0);
        page_insert(Kern_pgdir, tempage, addr2, 0);
        (*addr1) = i;
        assert((*addr1) == (*addr2));
        assert((*addr1) == i);
        page_remove(User_pgdir, addr1);
        page_remove(Kern_pgdir, addr2);
        int ppn = page2ppn(tempage);
        assert((Kern_vis[ppn>>6]&(1ll<<(ppn&63)))==0);
    }*/
    printf("[KERNEL]memory_management_test succeed!\n");
}

extern struct Env *curenv;
void page_out(u64 va) {
    //printf("[KERNEL]pageout at %lx , the current env is %lx\n", va, curenv != NULL ? curenv->id : -1);
    if (VADDR(va) < BY2PG) {
        panic("^^^^^^^^^TOO LOW^^^^^^^^");
    }
    u64 *Upgdir = (u64*)((u64)User_pgdir + KERNBASE), *Kpgdir = (u64*)((u64)Kern_pgdir + KERNBASE);
    u64 perm = 0;
    if (va < KERNBASE) {
        perm = PTE_USER;
    }
    u64* addr = (u64)((va & (1ll << 39)) ? (Kpgdir) : (Upgdir));
    int level = 0;
    while(level < 3) {
        level ++;
        switch (level) {
            case 1:
                addr = addr + GET_L1_INDEX(va);
                break;
            case 2:
                addr = addr + GET_L2_INDEX(va);
                break;
            case 3:
                addr = addr + GET_L3_INDEX(va);
                break;
            default:
                break;
        }
        if ((!((*addr) & VALID)) || (level == 3 && va < KERNBASE && (!((*addr) & PTE_USER)))) {
            struct Page* pp;
            int ret = page_alloc(&pp);
            pp->ref++;
            if (ret < 0)
                panic("page_out error: no free page");
            (*addr) = pp->pa | TABLE | VALID | perm;
            //if (level == 3)
            //    (*addr) |= INNER_SHARABLE | NORMAL_MEMORY | ACCESSED | perm;
        }
        addr = KADDR(PTE_ADDR(*(addr)));
    }
    return;
}
