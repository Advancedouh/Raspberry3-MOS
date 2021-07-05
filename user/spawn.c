#include "lib.h"
#include "error.h"
#include "kerelf.h"

#define TMPPAGE     (BY2PG)
#define TMPPAGE_TOP (TMPPAGE + BY2PG)

char* suffix = ".b\0";
int init_stack(u32 child, char **argv, u64* init_esp) {
    int argc = 0, i, r, tot = 0;
    char *strings;
    u64* args;

    for (argc = 0; ; argc ++) {
        char *str = argv[argc];
        if (str == 0) break;
        tot += strlen(str) + 1;
    }

    //all the string and the place
    if (ROUND(tot, 8) + 8 * (argc + 3) > BY2PG) 
        return -E_NO_MEM;
    
    strings = (u8*)TMPPAGE_TOP - tot;
    args = (u64*)(TMPPAGE_TOP - ROUND(tot, 4) - 8 * (argc + 1));

    if ((r = syscall_mem_alloc(0, TMPPAGE, VALID | PTE_RW | PTE_USER) < 0)) {
        return r;
    }

    int j;
    u8* ctemp, *argv_temp;
    ctemp = strings;
    for (i = 0; i < argc; i++) {
        char *str = argv[i];
        argv_temp = str;
        for (j = 0; str[j]; ++ j) {
            *ctemp = *argv_temp;
            ctemp ++;
            argv_temp ++;
        }
        *ctemp = 0;
        ctemp ++;
    }

    ctemp = (u8*)(USTACKTOP - TMPPAGE_TOP + (u64)strings);
    for (i = 0; i < argc; i++) {
        char *str = argv[i];        
        args[i] = (u64)ctemp;
        ctemp += strlen(str) + 1;
    }

    args[argc] = 0;

    u64* pargv_ptr = args - 1;
    *pargv_ptr = USTACKTOP - TMPPAGE_TOP + (u64)args;
    pargv_ptr --;
    *pargv_ptr = argc;

    *init_esp = USTACKTOP - TMPPAGE_TOP + (u64)pargv_ptr;

    if ((r = syscall_mem_map(0, TMPPAGE, child, USTACKTOP - BY2PG, VALID | PTE_RW | PTE_USER)) < 0) 
        goto error;
    if ((r = syscall_mem_unmap(0, TMPPAGE)) < 0)
        goto error;
    return 0;

error:
    syscall_mem_unmap(0, TMPPAGE);
    return r;
}

int init_stack_valist(u32 child, __builtin_va_list ap, u64* init_esp) {
    int argc, i, r, tot = 0;
    char *strings;
    u64* args;

    __builtin_va_list tmp = ap;
    for (argc = 0; ; argc ++) {
        char *str = __builtin_va_arg(tmp, char*);
        if (str == 0) break;
        tot += strlen(str) + 1;
    }

    //all the string and the place
    if (ROUND(tot, 8) + 8 * (argc + 3) > BY2PG) 
        return -E_NO_MEM;
    
    strings = (u8*)TMPPAGE_TOP - tot;
    args = (u64*)(TMPPAGE_TOP - ROUND(tot, 4) - 8 * (argc + 1));

    if ((r = syscall_mem_alloc(0, TMPPAGE, VALID | PTE_RW | PTE_USER) < 0)) {
        return r;
    }

    int j;
    u8* ctemp, *argv_temp;
    ctemp = strings;
    tmp = ap;
    for (i = 0; i < argc; i++) {
        char *str = __builtin_va_arg(tmp, char*);
        argv_temp = str;
        for (j = 0; str[j]; ++ j) {
            *ctemp = *argv_temp;
            ctemp ++;
            argv_temp ++;
        }
        *ctemp = 0;
        ctemp ++;
    }

    ctemp = (u8*)(USTACKTOP - TMPPAGE_TOP + (u64)strings);
    tmp = ap;
    for (i = 0; i < argc; i++) {
        char *str = __builtin_va_arg(tmp, char*);
        args[i] = (u64)ctemp;
        ctemp += strlen(str) + 1;
    }

    args[argc] = 0;

    u64* pargv_ptr = args - 1;
    *pargv_ptr = USTACKTOP - TMPPAGE_TOP + (u64)args;
    pargv_ptr --;
    *pargv_ptr = argc;

    *init_esp = USTACKTOP - TMPPAGE_TOP + (u64)pargv_ptr;

    if ((r = syscall_mem_map(0, TMPPAGE, child, USTACKTOP - BY2PG, VALID | PTE_RW | PTE_USER)) < 0) 
        goto error;
    if ((r = syscall_mem_unmap(0, TMPPAGE)) < 0)
        goto error;
    return 0;

error:
    syscall_mem_unmap(0, TMPPAGE);
    return r;
}

int usr_is_elf_format(u8 *binary) {
    return 1;
}

#define BUFPAGE     BY2PG

int usr_load_elf(int fd, Phdr* ph, int child_envid) {
    u64 va = ph -> vaddr;
    u64 sgSize = ph -> memsz;
    u64 binSize = ph -> filesz;
    u64 fileOffset = ph -> offset;
    u8 buf[BY2PG];
    int i = 0, r;
    u64 offset = va + i - ROUNDDOWN(va + i, BY2PG);
    u64 size;
    if (offset > 0) {
        r = syscall_mem_alloc(child_envid, va + i, VALID | PTE_RW | PTE_USER);
        if (r < 0) return r;
        size = MIN(binSize - i, BY2PG - offset);
        r = seek(fd, fileOffset + i);
        if (r < 0) return r;
        r = readn(fd, buf, size);
        if (r < 0) return r;
        r = syscall_mem_map(child_envid, va + i, 0, BUFPAGE, VALID | PTE_RW | PTE_USER);
        if (r < 0) return r;
        user_bcopy((void*)buf, (void*)(BUFPAGE + offset), size);
        r = syscall_mem_unmap(0, BUFPAGE);
        if (r < 0) return r;
        i += size;
    }
    while (i < binSize) {
        size = MIN(BY2PG, binSize - i);
        r = syscall_mem_alloc(child_envid, va + i, VALID | PTE_RW | PTE_USER);
        if (r < 0) return r;
        size = MIN(binSize - i, BY2PG);
        r = seek(fd, fileOffset + i);
        if (r < 0) return r;
        r = readn(fd, buf, size);
        if (r < 0) return r;
        r = syscall_mem_map(child_envid, va + i, 0, BUFPAGE, VALID | PTE_RW | PTE_USER);
        if (r < 0) return r;
        user_bcopy((void*)buf, (void*)BUFPAGE, size);
        r = syscall_mem_unmap(0, BUFPAGE);
        if (r < 0) return r;
        i += size;
    }
    offset = va + i - ROUNDDOWN(va + i, BY2PG);
    if (offset > 0) {
        size = MIN(sgSize - i, BY2PG - offset);
        r = syscall_mem_map(child_envid, va + i, 0, BUFPAGE, VALID | PTE_RW | PTE_USER);
        if (r < 0) return r;
        user_bzero((void*)(BUFPAGE + offset), size);
        r = syscall_mem_unmap(0, BUFPAGE);
        if (r < 0) return r;
        i += size;
    }
    while (i < sgSize) {
        size = MIN(BY2PG, sgSize - i);
        r = syscall_mem_alloc(child_envid, va + i, VALID | PTE_RW | PTE_USER);
        if (r < 0) return r;
        i += size;
    }
    return 0;
}

int spawn(char *prog, char **argv) {
    u8 elfbuf[512];
    int r, fdnum;
    u32 child_envid;
    u64 entry_size, text_start;
    int count;

    u64 i, j, k;
    u64 esp;
    Ehdr *ehdr;
    Phdr *phdr;

    char *name;
    int len = strlen(prog);
    if (len >= 2 && prog[len-2] == '.' && prog[len-1] == 'b') {
        name = prog;
    } else {
        name = strcat(prog, suffix);
        //writef("name: %s\n", name);
    }

    if ((r = open(name, O_RDONLY)) < 0) {
        writef("spawn : open RDONLY error!!!!!\n");
        return r;
    }

    fdnum = r;
    r = readn(fdnum, elfbuf, sizeof(Ehdr));
    if (r < 0) {
        writef("read Ehdr error!!!!!\n");
        return r;
    }

	ehdr = (Ehdr*)elfbuf;
    if (!usr_is_elf_format((u8*)ehdr)) {
        writef("not elf at all!!!!!\n");
        return r;
    }

    entry_size = ehdr -> phentsize;
    text_start = ehdr -> phoff;
    count = ehdr -> phnum;

    child_envid = syscall_env_alloc();
    if (child_envid < 0) {
        writef("alloc a new env error!!!\n");
        return child_envid;
    }

    init_stack(child_envid, argv, &esp);

    for (i = 0; i < count; i++) {
        r = seek(fdnum, text_start);
        if (r < 0) {
            writef("seek error!!!!\n");
            return r;
        }
        r = readn(fdnum, elfbuf, entry_size);
        if (r < 0) {
            writef("readn error!!!!\n");
            return r;
        }
        phdr = (Phdr *)elfbuf;
        if (phdr -> type == PT_LOAD) {
            r = usr_load_elf(fdnum, phdr, child_envid);
            if (r < 0) {
                writef("load elf error %d!!!!!\n", r);
                return r;
            }
        }
        text_start += entry_size;
    }

    u64 res = 0;
    for (i = 0; i < count; i++) 
        res += entry_size;
    struct Trapframe *tf;
    //writef("\n::::::::::spawn size : %lx  sp : %lx::::::::\n", res, esp);    
    tf = &(envs[ENVX(child_envid)].tf);
    tf -> elr_el1 = 0x00a00000;
    tf -> sp_el0 = esp;

    for (i = 0; i < 512; ++ i) {
        if (!(((*vpd)[i]) & VALID)) continue;
        for (j = 0; j < 512; ++ j) {
            if (!(((*vpm)[(i << 9) + j]) & VALID)) continue;
            for (k = 0; k < 512; ++ k) {
                u64 pte = (*vpt)[(i << 18) + (j << 9) + k];
                if (!(pte & VALID) || !(pte & PTE_LIBRARY)) continue;
                u64 va = ((i << 18) + (j << 9) + k) << 12;
                r = syscall_mem_map(0, va, child_envid, va, VALID | PTE_RW | PTE_USER | PTE_LIBRARY);
                if (r < 0) {
					writef("error when map at va: %x   child_envid: %x   \n",va,child_envid);
					//user_panic("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@");
					return r;                
                }
            }
        }
    }

    if ((r = syscall_set_env_status(child_envid, ENV_RUNNABLE)) < 0) {
        writef("set child runnable is wrong\n");
        return r;
    }

    return child_envid;    
}

int spawn_valist(char *prog, __builtin_va_list ap) {
    u8 elfbuf[512];
    int r, fdnum;
    u32 child_envid;
    u64 entry_size, text_start;
    int count;

    u64 i, j, k;
    u64 esp;
    Ehdr *ehdr;
    Phdr *phdr;

    if ((r = open(prog, O_RDONLY)) < 0) {
        user_panic("spawn : open RDONLY error!!!!!\n");
        return r;
    }

    fdnum = r;
    r = readn(fdnum, elfbuf, sizeof(Ehdr));
    if (r < 0) {
        user_panic("read Ehdr error!!!!!\n");
    }

	ehdr = (Ehdr*)elfbuf;
    if (!usr_is_elf_format((u8*)ehdr)) {
        user_panic("not elf at all!!!!!\n");
    }

    entry_size = ehdr -> phentsize;
    text_start = ehdr -> phoff;
    //writef("text start: %lx\n", text_start);
    count = ehdr -> phnum;
    //writef("phdr %d\n", count);

    child_envid = syscall_env_alloc();
    if (child_envid < 0) {
        user_panic("alloc a new env error!!!\n");
    }

    init_stack_valist(child_envid, ap, &esp);

    for (i = 0; i < count; i++) {
        r = seek(fdnum, text_start);
        if (r < 0) {
            user_panic("seek error!!!!\n");
        }
        r = readn(fdnum, elfbuf, entry_size);
        if (r < 0) {
            user_panic("readn error!!!!\n");
        }
        phdr = (Phdr *)elfbuf;
        if (phdr -> type == PT_LOAD) {
            r = usr_load_elf(fdnum, phdr, child_envid);
            if (r < 0) {
                user_panic("load elf error %d!!!!!\n", r);
            }
        }
        text_start += entry_size;
    }

    u64 res = 0;
    for (i = 0; i < count; i++) 
        res += entry_size;
    struct Trapframe *tf;
    writef("\n::::::::::spawn size : %lx  sp : %lx::::::::\n", res, esp);    
    tf = &(envs[ENVX(child_envid)].tf);
    tf -> elr_el1 = 0x00a00000;
    tf -> sp_el0 = esp;

    for (i = 0; i < 512; ++ i) {
        if (!(((*vpd)[i]) & VALID)) continue;
        for (j = 0; j < 512; ++ j) {
            if (!(((*vpm)[(i << 9) + j]) & VALID)) continue;
            for (k = 0; k < 512; ++ k) {
                u64 pte = (*vpt)[(i << 18) + (j << 9) + k];
                if (!(pte & VALID) || !(pte & PTE_LIBRARY)) continue;
                u64 va = ((i << 18) + (j << 9) + k) << 12;
                r = syscall_mem_map(0, va, child_envid, va, VALID | PTE_RW | PTE_USER | PTE_LIBRARY);
                if (r < 0) {
					writef("error when map at va: %x   child_envid: %x   \n",va,child_envid);
					user_panic("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@");
					return r;                
                }
            }
        }
    }

    if ((r = syscall_set_env_status(child_envid, ENV_RUNNABLE)) < 0) {
        user_panic("set child runnable is wrong\n");
        return r;
    }

    return child_envid;
}

int spawnl(char *prog, ...) {
   __builtin_va_list ap;
    __builtin_va_start(ap, prog);
    char *name;
    int len = strlen(prog);
    if (len >= 2 && prog[len-2] == '.' && prog[len-1] == 'b') {
        name = prog;
    } else {
        name = strcat(name, suffix);
    }
    return spawn_valist(name, ap);
}