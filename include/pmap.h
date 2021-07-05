#ifndef _PMAP_H_
#define _PMAP_H_

#include "mmu.h"
#include "printf.h"
#include "type.h"

struct Page
{
	u64 pa;
	u32 ref;
};

extern struct Page* Kern_pages;

#define pa2page(x)	(Kern_pages+(((u32)(x))>>12))
#define page2pa(x)  (((x)-Kern_pages)<<12)
#define page2ppn(x)  (((x)-Kern_pages))
#define page2kva(x)  ((void*)(KERNBASE + ((u64)page2pa(x))))

void page_out(u64 va);
int page_alloc(struct Page** p);
void page_free(struct Page *pp);
int pgdir_walk(u64* pgdir, u64 va, int create, u64 **ppte);
int page_insert(u64 *pgdir, struct Page *pp, u64 va, u64 perm);
struct Page* page_lookup(u64 *pgdir, u64 va, u64 **ppte);
void page_decref(struct Page* pp);
void page_remove(u64 *pgdir, u64 va);
void tlb_invalidate(u64* pgdir, u64 va);

#endif /* _PMAP_H_ */