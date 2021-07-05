#include "lib.h"
#include "fd.h"

static int pipeclose(struct Fd*);
static int piperead(struct Fd *fd, void *buf, u64 n, u64 offset);
static int pipestat(struct Fd*, struct Stat*);
static int pipewrite(struct Fd* fd, const void *buf, u64 n, u64 offset);

struct Dev devpipe = 
{
    .dev_id = 'p',
    .dev_name = "pipe",
    .dev_read = piperead,
    .dev_write = pipewrite,
    .dev_close = pipeclose,
    .dev_stat = pipestat,
};

#define BY2PIPE 32

struct Pipe {
    u64 p_rpos;
    u64 p_wpos;
    u8 p_buf[BY2PIPE];
};

int pipe(int pfd[2])
{
    u64 r, va;
    struct Fd *fd0, *fd1;

    if ((r = fd_alloc(&fd0)) < 0
    ||  (r = syscall_mem_alloc(0, (u64)fd0, VALID | PTE_RW | PTE_LIBRARY | PTE_USER) < 0)) {
        goto err1;
    }

    if ((r = fd_alloc(&fd1)) < 0
    ||  (r = syscall_mem_alloc(0, (u64)fd1, VALID | PTE_RW | PTE_LIBRARY | PTE_USER) < 0)) {
        goto err2;
    }

    va = fd2data(fd0);
    if ((r = syscall_mem_alloc(0, va, VALID | PTE_RW | PTE_LIBRARY | PTE_USER)) < 0) {
        goto err3;
    }

    if ((r = syscall_mem_map(0, va, 0, fd2data(fd1), VALID | PTE_RW | PTE_LIBRARY | PTE_USER)) < 0) {
        goto err4;
    }

    fd0->fd_dev_id = devpipe.dev_id;
    fd0->fd_omode = O_RDONLY;

    fd1->fd_dev_id = devpipe.dev_id;
    fd1->fd_omode = O_WRONLY;

    //writef("[%08x] pipecreate \n", env->id, (* vpt)[VPN(va)]);

    pfd[0] = fd2num(fd0);
    pfd[1] = fd2num(fd1);
    return 0;

err4: syscall_mem_unmap(0, va);
err3: syscall_mem_unmap(0, (u64)fd1);
err2: syscall_mem_unmap(0, (u64)fd0);
err1: return r;
}

static int _pipeisclosed(struct Fd *fd, struct Pipe *p) {
    int fdref, pref, runs;
    do {
        runs = env -> env_runs;
        fdref = pageref(fd);
        pref = pageref(p);
    } while(runs != env -> env_runs);

    if (fdref == pref) {
        return 1;
    } else {
        return 0;
    }
}

int pipeisclosed(int fdnum) {
    struct Fd* fd;
    struct Pipe *p;
    int r;

    if (r = fd_lookup(fdnum, &fd) < 0)
        return r;
    p = (struct Pipe*)fd2data(fd);
    return _pipeisclosed(fd, p);
}

static int piperead(struct Fd *fd, void *vbuf, u64 n, u64 offset) {
    int i;
    struct Pipe* p;
    char *rbuf;
    p = (struct Pipe*)fd2data(fd);

    while (p -> p_rpos == p -> p_wpos) {
        if (_pipeisclosed(fd, p))
            return 0;
        syscall_yield();
    }
    rbuf = (char*)vbuf;
    for (i = 0; i < n; i++) {
        while (p -> p_rpos == p -> p_wpos) {
            if (i > 0 || _pipeisclosed(fd, p)) {
                return i;
            }
            syscall_yield();
        }
        rbuf[i] = p -> p_buf[p -> p_rpos % BY2PIPE];
        p -> p_rpos++;
    }
    return n;
}

static int pipewrite(struct Fd* fd, const void *vbuf, u64 n, u64 offset) {
    int i;
    struct Pipe *p;
    char *wbuf;

    p = fd2data(fd);
    
    wbuf = (char *)vbuf;
    
    for (i = 0; i < n; i++) {
        while (p -> p_wpos - p -> p_rpos == BY2PIPE) {
            if (_pipeisclosed(fd, p)) return 0;
            syscall_yield();
        }
        p -> p_buf[p -> p_wpos % BY2PIPE] = wbuf[i];
        p -> p_wpos ++;
    }
    return n;
}

static int pipestat(struct Fd *fd, struct Stat *stat) {
    
}

static int pipeclose(struct Fd *fd) {
    struct Fd *tmp = fd;
    syscall_mem_unmap(0, (u64)fd);
    syscall_mem_unmap(0, fd2data(tmp));
    return 0;
}