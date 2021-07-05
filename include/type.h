#ifndef _TYPE_H_
#define _TYPE_H_

#include "queue.h"

typedef unsigned long u64;
typedef unsigned int u32;
typedef unsigned char u8;
typedef unsigned short u16;

struct Trapframe {
	u64 far_el1;
	u64 sp_el0;
    u64 elr_el1;
    u64 spsr_el1;
	u64 count_down;	
	u64 regs[31];
};

struct Env {
	struct Trapframe tf;
	LIST_ENTRY(Env) link;
	u32 id;                   // Unique environment identifier
	u32 parent_id;            // env_id of this env's parent
	u32 status;               // Status of the environment
	u64 *pgdir;                // Kernel virtual address of page dir
	u64 pgdir_pa;
	LIST_ENTRY(Env) sched_link;
    u32 pri;
	
	u64 ipc_value;            // data value sent to us 
	u64 ipc_from;             // envid of the sender  
	u64 ipc_recving;          // env is blocked receiving
	u64 ipc_dstva;		// va at which to map received page
	u64 ipc_perm;		// perm of page mapping received

	u64 pgfault_handler;      // page fault state
	u64 xstacktop;            // top of exception stack

	u64 env_runs;			// number of times been env_run'ed
	u64 env_nop;            // align to avoid mul instruction
};

#define ENV_FREE	0
#define ENV_RUNNABLE		1
#define ENV_NOT_RUNNABLE	2

#define LOG2NENV	10
#define NENV		(1<<LOG2NENV)
#define ENVX(envid)	((envid) & (NENV - 1))
#define GET_ENV_ASID(envid) (((envid)>> 11)<<6)

#define MIN(_a, _b)	\
	({		\
		typeof(_a) __a = (_a);	\
		typeof(_b) __b = (_b);	\
		__a <= __b ? __a : __b;	\
	})

#define ROUND(a, n)	(((((u64)(a))+(n)-1)) & ~((n)-1))
#define ROUNDDOWN(a, n)	(((u64)(a)) & ~((n)-1))

#endif