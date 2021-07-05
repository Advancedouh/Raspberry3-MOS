#include "lib.h"
#include <mmu.h>
#include <type.h>

void user_bcopy(const void *src, void *dst, u64 len) {
	void *max;

	max = dst + len;

	if (((u64)src % 4 == 0) && ((u64)dst % 4 == 0)) {
		while (dst + 3 < max) {
			*(int *)dst = *(int *)src;
			dst += 4;
			src += 4;
		}
	}

	while (dst < max) {
		*(char *)dst = *(char *)src;
		dst += 1;
		src += 1;
	}
}

void user_bzero(void *v, u64 n) {
	char *p;
	int m;

	p = v;
	m = n;

	while (--m >= 0) {
		*p++ = 0;
	}
}

static void pgfault(u64 va) {
	u64 tmp = USTACKTOP;
	int ret;
    if ((((u64*)(*vpt))[VPN(va)] & PTE_RO) == 0) {
		user_panic("User pgfault face a not Read Only page!");
	}
	va = ROUNDDOWN(va, BY2PG);

	ret = syscall_mem_alloc(0, tmp, VALID | PTE_RW | PTE_USER);
	if (ret < 0) {
		user_panic("User pgfault alloc fail!");
	}

	user_bcopy(va, tmp, BY2PG);
	
	ret = syscall_mem_map(0, tmp, 0, va, VALID | PTE_RW | PTE_USER);
	if (ret < 0) {
		user_panic("User pgfault map fail!");
	}
	ret = syscall_mem_unmap(0, tmp);
	if (ret < 0) {
		user_panic("User pgfault umap faild!");
	}

	return;
}

static void duppage(u32 envid, u64 va, u64 pte)
{
	if (va == UXSTACKTOP - BY2PG) {
		return ;
	}
	u64 perm = GET_PTE_PERM(pte);
	if (perm & PTE_LIBRARY) {
		if(syscall_mem_map(0, va, envid, va, perm) < 0) {
			user_panic("user panic mem map error!");
		}
	} else {
		if(syscall_mem_map(0, va, envid, va, perm | PTE_RO) < 0)
		{
			user_panic("user panic mem map error!3");
		}
		if ((perm & PTE_RO) == 0) {
			if(syscall_mem_map(0, va, 0, va, perm | PTE_RO) < 0)
			{
				user_panic("user panic mem map error!2");
			}
		}
	}
}

extern void __asm_pgfault_handler(void);

int
fork(void)
{
	int newenvid;
	extern struct Env *envs;
	extern struct Env *env;
	u64 i,j,k;
	int ret;

	set_pgfault_handler(pgfault);

	newenvid = syscall_env_alloc();
	if (newenvid == 0)
	{
		env = envs + ENVX(syscall_getenvid());
		return 0;
	}

    for (i = 0; i < 512; i++) {
		if (i == GET_L1_INDEX(UVPT)) continue;
        if ((((*vpd)[i]) & VALID) == 0) continue;
        for (j = 0; j < 512; j++) {
            if (((*vpm)[(i << 9) + j] & VALID) == 0) continue;
            for (k = 0; k < 512; k++) {
                u64 pte = (*vpt)[(i << 18) + (j << 9) + k];
                if ((pte & VALID) == 0 || (pte & PTE_USER) == 0) continue;
                u64 va = ((i << 18) + (j << 9) + k) << 12;
                duppage(newenvid, va, pte);
            }
        }
    }

	ret = syscall_mem_alloc(newenvid, UXSTACKTOP - BY2PG, VALID | PTE_RW | PTE_USER);
	if (ret < 0) {
		user_panic("fork alloc mem fail");
	}
	ret = syscall_set_pgfault_handler(newenvid, __asm_pgfault_handler, UXSTACKTOP);
	if (ret < 0) {
		user_panic("fork set pgfault fail");
	}

	ret = syscall_set_env_status(newenvid, ENV_RUNNABLE);
	if (ret < 0) {
		user_panic("fork set status faild");
	}

	return newenvid;
}