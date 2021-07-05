#include "lib.h"

extern struct Page pages[];

int
pageref(void *v)
{
	u64 pte;
    u64 va = v;

	if (!((* vpd)[SELF_PD(va)]&VALID)) {
		return 0;
	}

    if (!((* vpm)[SELF_PM(va)] & VALID)) {
        return 0;
    }

    if (!((* vpt)[SELF_PT(va)] & VALID)) {
        return 0;
    }

	pte = (* vpt)[VPN(va)];

	return syscall_get_pageref(PPN(pte)&((1<<27)-1));
}
