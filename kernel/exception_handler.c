#include <pgdir.h>
#include <pmap.h>
#include <unistd.h>
#include <trap.h>
#include <kernel.h>
#include "syscall_all.h"

extern struct Trapframe *TRAPSP[];
extern struct Env* curenv;

u64 syscall_handlers[32];

void syscall_init() {
    syscall_handlers[SYS_putchar] = sys_putchar;
    syscall_handlers[SYS_getenvid] = sys_getenvid;
    syscall_handlers[SYS_yield] = sys_yield;
    syscall_handlers[SYS_env_destroy] = sys_env_destroy;
    syscall_handlers[SYS_set_pgfault_handler] = sys_set_pgfault_handler;
    syscall_handlers[SYS_mem_alloc] = sys_mem_alloc;
    syscall_handlers[SYS_mem_map] = sys_mem_map;
    syscall_handlers[SYS_mem_unmap] = sys_mem_unmap;
    syscall_handlers[SYS_env_alloc] = sys_env_alloc;
    syscall_handlers[SYS_set_env_status] = sys_set_env_status;
    syscall_handlers[SYS_set_trapframe] = sys_set_trapframe;
    syscall_handlers[SYS_panic] = sys_panic;
    syscall_handlers[SYS_ipc_can_send] = sys_ipc_can_send;
    syscall_handlers[SYS_ipc_recv] = sys_ipc_recv;
    syscall_handlers[SYS_get_envs] = sys_get_envs;
    syscall_handlers[SYS_write_sd] = sys_write_sd;
    syscall_handlers[SYS_read_sd] = sys_read_sd;
    syscall_handlers[SYS_get_pageref] = sys_get_pageref;
    syscall_handlers[SYS_getc] = sys_getc;
}

int cnt = 0;
void synchronous_exception_handler(u64 exc_id, u64 esr_el1, u64 elr_el1, u64 spsr_el1, u64 far_el1) {
    u64 EC = ((esr_el1) & 0xfc000000) >> 26;
    u64 DFSC = ((esr_el1) & 0x3f);
    u64 ISS = ((esr_el1) & 0xffff);
    u64 x0;

    switch (EC)
    {
        case 0b100100:
            switch (DFSC){
                case 0b001101:
                case 0b001110:
                case 0b001111:
                //printf("[KERNEL]Permission error at %lx, the current env is %lx\n", far_el1, curenv -> id);
                page_fault_handler();
                break;
                case 0b000100:
                case 0b000101:
                case 0b000110:
                case 0b000111:
                page_out(far_el1);
                break;
                default:
                    panic("[KERNEL]I don't know what the data abort error is TAT!");
            }
            break;
        case 0b100101:
            page_out(far_el1);
        break;
        case 0b010101:
            x0 = (*TRAPSP)->regs[0];
            handle_syscall(syscall_handlers[x0], (*TRAPSP)->regs[1], (*TRAPSP)->regs[2], (*TRAPSP)->regs[3], (*TRAPSP)->regs[4], (*TRAPSP)->regs[5], *TRAPSP);
        break;
        default:
            panic("[KERNEL]I don't know what the error is TAT!");
    }
    return ;
}

void
page_fault_handler()
{
    struct Trapframe PgTrapFrame;
    struct Trapframe* tf = (*TRAPSP);
    extern struct Env *curenv;

    bcopy(tf, curenv->xstacktop - sizeof(struct Trapframe), sizeof(struct Trapframe));
    tf->sp_el0 = curenv->xstacktop - sizeof(struct Trapframe);
	tf->elr_el1 = curenv->pgfault_handler;
    return;
}
