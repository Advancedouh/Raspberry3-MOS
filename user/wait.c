#include "lib.h"
#include "type.h"

void wait(u32 envid)
{
	struct Env *e;

	//writef("envid:%x  wait()~~~~~~~~~",envid);
	e = &envs[ENVX(envid)];
	while(e->id == envid && e->status != ENV_FREE) 
		syscall_yield();
}
