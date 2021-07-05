#ifndef UNISTD_H
#define UNISTD_H

#define __NR_SYSCALLS 20


#define SYS_putchar 		(0)
#define SYS_getenvid 		(1)
#define SYS_yield			(2)
#define SYS_env_destroy		(3)
#define SYS_set_pgfault_handler	(4)
#define SYS_mem_alloc		(5)
#define SYS_mem_map			(6)
#define SYS_mem_unmap		(7)
#define SYS_env_alloc		(8)
#define SYS_set_env_status	(9)
#define SYS_set_trapframe		(10)
#define SYS_panic			(11)
#define SYS_ipc_can_send		(12)
#define SYS_ipc_recv		(13)
#define SYS_cgetc			(14)
#define SYS_get_envs        (15)
#define SYS_write_sd        (16)
#define SYS_read_sd         (17)
#define SYS_get_pageref     (18)
#define SYS_getc            (19)

#endif
