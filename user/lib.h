#ifndef _USER_LIB_H_
#define _USER_LIB_H_
#include <pmap.h>
#include <mmu.h>
#include <unistd.h>
#include "fd.h"

/////////////////////////////////////////////////////head
extern void umain();
extern void libmain();
extern void exit();

void syscall_putchar(u8);
int syscall_getenvid(void);
void syscall_yield(void);
int syscall_env_destroy(u64 envid);
int syscall_set_pgfault_handler(u64 envid, void (*func)(void), u64 xstacktop);
int syscall_mem_alloc(u64 envid, u64 va, u64 perm);
int syscall_mem_map(u64 srcid, u64 srcva, u64 dstid, u64 dstva, u64 perm);
int syscall_mem_unmap(u64 envid, u64 va);
int syscall_set_env_status(u64 envid, u64 status);
//int syscall_set_trapframe(u64 envid, struct Trapframe *tf);
int syscall_panic(char *msg);
int syscall_ipc_can_send(u64 envid, u64 value, u64 srcva, u64 perm);
void syscall_ipc_recv(u64 dstva);
u64 syscall_getenvs(void);
int syscall_write_sd(void *src, u32 secno);
int syscall_read_sd(void *dst, u32 secno);
int syscall_get_pageref(int num);
int syscall_getc();

extern struct Env *env;
extern struct Env* envs;


#define USED(x) (void)(x)

//////////////////////////////////////////////////syscall_lib
u64 msyscall(u64, u64, u64, u64, u64, u64);
inline static u64 syscall_env_alloc(void)
{
    return msyscall(SYS_env_alloc, 0, 0, 0, 0, 0);
}

// string.c
int strlen(const char *s);
char *strcpy(char *dst, const char *src);
const char *strchr(const char *s, char c); 
void *memcpy(void *destaddr, void const *srcaddr, u32 len);
int strcmp(const char *p, const char *q);
char *strcat(const char *p, const char *q);

// pageref.c
int	pageref(void *);

// ipc.c
void ipc_send(u32 whom, u64 val, u64 srcva, u64 perm);
u64	ipc_recv(u32 *whom, u64 dstva, u64 *perm);

int	close(int fd);
int	read(int fd, void *buf, u64 nbytes);
int	write(int fd, const void *buf, u64 nbytes);
int	seek(int fd, u64 offset);
void close_all(void);
int	readn(int fd, void *buf, u64 nbytes);
int	dup(int oldfd, int newfd);
int fstat(int fdnum, struct Stat *stat);
int	stat(const char *path, struct Stat*);

void writef(char *fmt, ...);

void _user_panic(const char *, int, const char *, ...)__attribute__((noreturn));

//cursor.c
void cursor(char *s, int n);

#define user_assert(x)	\
	do {	if (!(x)) user_panic("assertion failed: %s", #x); } while (0)
    
#define user_panic(...) _user_panic(__FILE__, __LINE__, __VA_ARGS__)

/* File open modes */
#define	O_RDONLY	0x0000		/* open for reading only */
#define	O_WRONLY	0x0001		/* open for writing only */
#define	O_RDWR		0x0002		/* open for reading and writing */
#define	O_ACCMODE	0x0003		/* mask for above modes */

#define	O_CREAT		0x0100		/* create if nonexistent */
#define	O_TRUNC		0x0200		/* truncate to zero length */
#define	O_EXCL		0x0400		/* error if already exists */
#define O_MKDIR		0x0800		/* create directory, not regular file */

#endif