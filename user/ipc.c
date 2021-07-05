// User-level IPC library routines

#include "lib.h"
#include <mmu.h>
#include <type.h>
#include <error.h>

extern struct Env *env;

// Send val to whom.  This function keeps trying until
// it succeeds.  It should panic() on any error other than
// -E_IPC_NOT_RECV.  
//
// Hint: use syscall_yield() to be CPU-friendly.
void
ipc_send(u32 whom, u64 val, u64 srcva, u64 perm)
{
	int r;

	while ((r=syscall_ipc_can_send(whom, val, srcva, perm)) == -E_IPC_NOT_RECV)
	{
		syscall_yield();
		//writef("QQ");
	}
	if(r == 0)
		return;
	user_panic("error in ipc_send: %d", r);
}

// Receive a value.  Return the value and store the caller's envid
// in *whom.  
//
// Hint: use env to discover the value and who sent it.
u64
ipc_recv(u32 *whom, u64 dstva, u64 *perm)
{
	syscall_ipc_recv(dstva);
	//writef("check !!!! %x\n", envs);
	if (whom)
		*whom = env->ipc_from;
	if (perm)
		*perm = env->ipc_perm;
	return env->ipc_value;
}
