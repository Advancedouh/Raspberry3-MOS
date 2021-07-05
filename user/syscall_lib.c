#include "lib.h"
#include <unistd.h>
#include <type.h>
#include <mmu.h>

void syscall_putchar(u8 ch)
{
	msyscall(SYS_putchar, (u64)ch, 0, 0, 0, 0);
}

int syscall_getenvid(void)
{
	return msyscall(SYS_getenvid, 0, 0, 0, 0, 0);
}

void syscall_yield(void)
{
	msyscall(SYS_yield, 0, 0, 0, 0, 0);
}

int syscall_env_destroy(u64 envid)
{
	return msyscall(SYS_env_destroy, envid, 0, 0, 0, 0);
}

int syscall_set_pgfault_handler(u64 envid, void (*func)(void), u64 xstacktop)
{
	return msyscall(SYS_set_pgfault_handler, envid, (u64)func, xstacktop, 0, 0);
}

int syscall_mem_alloc(u64 envid, u64 va, u64 perm)
{
	return msyscall(SYS_mem_alloc, envid, va, perm, 0, 0);
}

int syscall_mem_map(u64 srcid, u64 srcva, u64 dstid, u64 dstva, u64 perm)
{
	return msyscall(SYS_mem_map, srcid, srcva, dstid, dstva, perm);
}

int syscall_mem_unmap(u64 envid, u64 va)
{
	return msyscall(SYS_mem_unmap, envid, va, 0, 0, 0);
}

int syscall_set_env_status(u64 envid, u64 status)
{
	return msyscall(SYS_set_env_status, envid, status, 0, 0, 0);
}

/*int syscall_set_trapframe(u64 envid, struct Trapframe *tf)
{
	return msyscall(SYS_set_trapframe, envid, (u64)tf, 0, 0, 0);
}*/

int syscall_panic(char *msg)
{
	msyscall(SYS_panic, (u64)msg, 0, 0, 0, 0);
}

int syscall_ipc_can_send(u64 envid, u64 value, u64 srcva, u64 perm)
{
	return msyscall(SYS_ipc_can_send, envid, value, srcva, perm, 0);
}

void syscall_ipc_recv(u64 dstva)
{
	msyscall(SYS_ipc_recv, dstva, 0, 0, 0, 0);
}

int syscall_cgetc()
{
	return msyscall(SYS_cgetc, 0, 0, 0, 0, 0);
}

u64 syscall_getenvs(void)
{
	msyscall(SYS_get_envs, 0, 0, 0, 0, 0);
}

int syscall_write_sd(void *src, u32 secno)
{
	return msyscall(SYS_write_sd, src, secno, 0, 0, 0);
}

int syscall_read_sd(void *dst, u32 secno)
{
	return msyscall(SYS_read_sd, dst, secno, 0, 0, 0);
}

int syscall_get_pageref(int num)
{
	return msyscall(SYS_get_pageref, (u64)num, 0, 0, 0, 0);
}

int syscall_getc()
{
	return msyscall(SYS_getc, 0, 0, 0, 0, 0);
}