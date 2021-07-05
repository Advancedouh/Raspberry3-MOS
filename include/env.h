#ifndef _ENV_H_
#define _ENV_H_

#include "type.h"
#include "queue.h"
#include "trap.h"

#define ENV_FREE	0
#define ENV_RUNNABLE		1
#define ENV_NOT_RUNNABLE	2

extern struct Env envs[];

LIST_HEAD(Env_list, Env);

extern struct Env* curenv;

#define ENV_CREATE_PRIORITY(x, y) \
{\
        extern u8 binary_##x##_start[]; \
        extern int binary_##x##_size;\
        env_create_priority(binary_##x##_start, \
                (int)binary_##x##_size, y);\
}
#define ENV_CREATE(x) \
{ \
	extern u8 binary_##x##_start[];\
	extern int binary_##x##_size; \
	env_create(binary_##x##_start, \
		(int)binary_##x##_size); \
}

void sched_yield(void);

void env_init(void);
u32 mkenvid(struct Env* e);
int envid2env(u32 envid, struct Env **penv, int checkperm);
int env_setup_vm(struct Env* e);
int env_alloc(struct Env**new, u64 parent_id);
void env_create_priority(u8* binary, int size, int priority);
void env_create(u8 *binary, int size);
void env_free(struct Env* e);
void env_destroy(struct Env *e);
void env_run(struct Env* e);

#endif