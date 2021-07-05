#include <env.h>
#include <type.h>
#include <mmu.h>
#include <pmap.h>
#include <error.h>
#include <queue.h>
#include <sd.h>

extern struct Trapframe (*TRAPSP)[];
extern struct Page pages[];

extern struct Env* curenv;

void sys_putchar(int sysno, int c) {
	printcharc((char) c);
	return;
}

void sys_getc(int sysno) {
	(*TRAPSP)->regs[0] = readcharc();
}

void sys_getenvid(int sysno) {
	(*TRAPSP)->regs[0] = curenv->id;
}

void sys_yield() {
	sched_yield();
}

void sys_env_destroy(int sysno, u32 envid) {
	int r;
	struct Env *e;
	
	if ((r = envid2env(envid, &e, 1)) < 0) {
		(*TRAPSP)->regs[0] = r;
		return;
	}

	//printf("[%08x] destroying %08x\n", curenv->id, e->id);
	env_destroy(e);
	(*TRAPSP)->regs[0] = 0;
	return;
}

void sys_set_pgfault_handler(int sysno, u32 envid, u64 func, u64 xstacktop) {
	struct Env* env;
	int r;

	r = envid2env(envid, &env, 1);
	if (r < 0) {
		(*TRAPSP)->regs[0] = r;
		return;
	}

	env->pgfault_handler = func;
	env->xstacktop = xstacktop;
	(*TRAPSP)->regs[0] = 0;
	return;
}

void sys_mem_alloc(int sysno, u32 envid, u64 va, u64 perm) {
	struct Env *env;
	struct Page *ppage;
	int r = 0;

	if (va >= KERNBASE) {
		(*TRAPSP)->regs[0] = -1;
		return;
	}
	r = envid2env(envid, &env, 1);
	if (r < 0) {
		(*TRAPSP)->regs[0] = r;
		return;
	}
	r = page_alloc(&ppage);
	if (r < 0) {
		(*TRAPSP)->regs[0] = r;
		return;
	}
	r = page_insert(env->pgdir, ppage, va, perm);
	if (r < 0) {
		(*TRAPSP)->regs[0] = r;
		return;
	}
	(*TRAPSP)->regs[0] = 0;
	return;
}

void sys_mem_map(int sysno, u32 srcid, u64 srcva, u32 dstid, u64 dstva, u64 perm) {
	//printf("srcva: %lx, dstva: %lx\n", srcva, dstva);
	int r;
	struct Env *srcenv;
	struct Env *dstenv;
	struct Page *ppage = NULL;
	u64 *ppte;
	u64 round_srcva = ROUNDDOWN(srcva, BY2PG);
	u64 round_dstva = ROUNDDOWN(dstva, BY2PG);

	if (srcva >= KERNBASE || dstva >= KERNBASE) {
		(*TRAPSP)->regs[0] = -1;
		return;
	}

	r = envid2env(srcid, &srcenv, 0);
	if (r < 0) {
		(*TRAPSP)->regs[0] = r;
		return;
	}
	r = envid2env(dstid, &dstenv, 0);
	if (r < 0) {
		(*TRAPSP)->regs[0] = r;
		return;
	}

	ppage = page_lookup(srcenv->pgdir, round_srcva, &ppte);
	if (ppage == NULL) {
		(*TRAPSP)->regs[0] = -1;
		return;
	}

	ppage = pa2page(PTE_ADDR(*ppte));
	r = page_insert(dstenv->pgdir, ppage, round_dstva, perm);
	if (r < 0) {
		(*TRAPSP)->regs[0] = r;
		return;
	}
	(*TRAPSP)->regs[0] = 0;
	return;
}

void sys_mem_unmap(int sysno, u32 envid, u64 va) {
	int r;
	struct Env *env;

	if (va >= KERNBASE) {
		(*TRAPSP)->regs[0] = -1;
		return;
	}

	r = envid2env(envid, &env, 0);
	if (r < 0) {
		(*TRAPSP)->regs[0] = r;
		return;
	}

	page_remove(env->pgdir, va);

	return 0;
}

void sys_env_alloc(void) {
	int r;

	struct Env* e;
	r = env_alloc(&e, curenv->id);
	if (r < 0) {
		(*TRAPSP)->regs[0] = r;
		return;
	}

	e->status = ENV_NOT_RUNNABLE;
	e->pri = curenv->pri;
	e->parent_id = curenv->id;
	bcopy((*TRAPSP), &(e->tf), sizeof(struct Trapframe));
	e->tf.regs[0] = 0;
	extern struct Env_list env_sched_list[];
	LIST_INSERT_TAIL(&env_sched_list[0], e, sched_link);

	(*TRAPSP)->regs[0] = e->id;
	return;
}

void sys_set_env_status(int sysno, u32 envid, u32 status) {
	struct Env *env;
	int r;

	if (status != ENV_RUNNABLE && status != ENV_NOT_RUNNABLE && status != ENV_FREE) {
		(*TRAPSP)->regs[0] = -1;
		return;
	}

	r = envid2env(envid, &env, 1);
	if (r < 0) {
		(*TRAPSP)->regs[0] = r;
		return;
	}

	if (status == ENV_FREE) {
		env_destroy(env);
	}

	env->status = status;
	(*TRAPSP)->regs[0] = 0;
	return;
}

void sys_set_trapframe(int sysno, u32 envid, struct Trapframe *tf) {
	(*TRAPSP)->regs[0] = 0;
	return;
}

void sys_panic(int sysno, char *msg) {
	panic("%s", msg);
}

void sys_ipc_recv(int sysno, u64 dstva) {
	if (dstva >= KERNBASE) {
		return;
	}
	curenv->ipc_recving = 1;
	curenv->ipc_dstva = dstva;
	curenv->status = ENV_NOT_RUNNABLE;
	sys_yield();
}

void sys_ipc_can_send(int sysno, u32 envid, u64 value, u64 srcva, u64 perm) {
	int r;
	struct Env* e;
	struct Page* p;
	u64* pte;

	if (srcva >= KERNBASE) {
		(*TRAPSP)->regs[0] = -1;
		return;
	}

	r = envid2env(envid, &e, 0);

	if (r < 0) {
		(*TRAPSP)->regs[0] = r;
		return;
	}

	if (e->ipc_recving == 0) {
		(*TRAPSP)->regs[0] = -E_IPC_NOT_RECV;
		return;
	}

	e->ipc_value = value;
	e->ipc_from = curenv->id;
	e->ipc_perm = perm;
	e->ipc_recving = 0;
	e->status = ENV_RUNNABLE;

	if (srcva != 0) {
		p = page_lookup(curenv->pgdir, srcva, &pte);
		if (p == NULL) {
			(*TRAPSP)->regs[0] = -1;
			return;
		}
		page_insert(e->pgdir, p, e->ipc_dstva, perm);
	}
	(*TRAPSP)->regs[0] = 0;
	//printf("Warning!!!!%x\n", envs);
	return;
}

void sys_get_envs(void) {
	(*TRAPSP)->regs[0] = envs;
	return;
}

void sys_write_sd(int sysno, u64* va, u32 secno)
{
	if (va >= KERNBASE)
	{
		(*TRAPSP)->regs[0] = -E_INVAL;
		return;
	}

	if (sd_writeblock(va, secno, 1) == 0)
	{
		(*TRAPSP)->regs[0] = -E_INVAL;
		return;
	}
	(*TRAPSP)->regs[0] = 0;
}

/* Overview:
 * 	This function is used to read data from device, which is
 * 	represented by its mapped physical address.
 *	Remember to check the validity of device address (same as sys_read_dev)
 * 
 * Pre-Condition:
 *      'va' is the startting address of data buffer, 'len' is the
 *      length of data (in bytes), 'dev' is the physical address of
 *      the device
 * 
 * Post-Condition:
 *      copy data from 'dev' to 'va' with length 'len'
 *      Return 0 on success, < 0 on error
 *      
 * Hint: Use ummapped segment in kernel address space to perform MMIO.
 */
void sys_read_sd(int sysno, u64* va, u32 secno) {
	if (va >= KERNBASE) {
		(*TRAPSP)->regs[0] = -E_INVAL;
		return;
	}
	if (sd_readblock(secno, va, 1) == 0) {
		(*TRAPSP)->regs[0] = -E_INVAL;
		return;
	}
	(*TRAPSP)->regs[0] = 0;
}

void sys_get_pageref(int sysno, u32 num) {
	if (num >= (1 << 18)) {
		(*TRAPSP)->regs[0] = -E_INVAL;
		return ;
	}
	(*TRAPSP)->regs[0] = pages[num].ref;
}
