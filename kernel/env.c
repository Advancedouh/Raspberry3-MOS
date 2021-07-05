#include <env.h>
#include <queue.h>
#include <pmap.h>
#include <mmu.h>
#include <pgdir.h>
#include <type.h>

struct Env *curenv = NULL;
static struct Env_list env_free_list;
struct Env_list env_sched_list[2];
extern struct Page pages[];
struct Page* Kern_page = (u64)pages + KERNBASE;
struct Env* Kern_envs = (u64)envs + KERNBASE;

void env_init(void) {
    int i;
    LIST_INIT(&env_free_list);

    LIST_INIT(&env_sched_list[0]);
    LIST_INIT(&env_sched_list[1]);

    for (i = NENV - 1; i >= 0; i--) {
        Kern_envs[i].status = ENV_FREE;
        LIST_INSERT_HEAD(&env_free_list, &Kern_envs[i], link);
    }
}

u32 mkenvid(struct Env* e) {
    static u32 next_env_id = 0;

    u32 idx = e - Kern_envs;

    return (++next_env_id << (1 + LOG2NENV)) | idx;
}

int envid2env(u32 envid, struct Env **penv, int checkperm) {
    struct Env *e;

    if (envid == 0) {
        *penv = curenv;
        return 0;
    }
    e = Kern_envs + ENVX(envid);

    if (e->status == ENV_FREE || e->id != envid) {
        *penv = NULL;
        return -1;
    }

    if (checkperm) {
        if (e != curenv && e->parent_id != curenv->id) {
            *penv = 0;
            return -1;
        }
    }

    *penv = e;
    return 0;
}

int env_setup_vm(struct Env* e) {
    int i, r;
    struct Page* p = NULL;
    u64 *pgdir;

    r = page_alloc(&p);
    if (r < 0) {
        panic("env_setup_vm - page alloc error\n");
        return r;
    }

    p->ref++;
    pgdir = (u64*) page2kva(p);

    for (i = 0; i < PTE2PT; i++) {
        pgdir[i] = 0;
    }

    pgdir[GET_L1_INDEX(UVPT)] = p->pa | PTE_RW | PTE_USER;

    e->pgdir = pgdir;
    e->pgdir_pa = PADDR(pgdir);

    extern char init_end[];
    for (i = 0; i < ((u64) (init_end)) / 4096; i++)
        page_insert(e->pgdir, &Kern_page[i], (i << 12), PTE_RW | PTE_USER | PTE_LIBRARY);
}

int env_alloc(struct Env**new, u64 parent_id) {
    int r;
    struct Env *e;

    if (LIST_EMPTY(&env_free_list)) {
        *new = NULL;
        return -1;
    }
    e = LIST_FIRST(&env_free_list);
    LIST_REMOVE(e, link);

    env_setup_vm(e);

    e->id = mkenvid(e);
    e->status = ENV_RUNNABLE;
    e->parent_id = parent_id;

    e->tf.spsr_el1 = 0x340;
    e->tf.sp_el0 = USTACKTOP;

    *new = e;
    return 0;
}

static int load_icode_mapper(u64 va, u32 sgsize, u8* bin, u32 bin_size, void* user_data) {
    struct Env *env = (struct Env *)user_data;
    struct Page *p = NULL;
    u64 i;
    int r = 0;
    u64 offset = va - ROUNDDOWN(va, BY2PG);
    u64* j;

    if (offset > 0) {
        p = page_lookup(env->pgdir, va, &j);
        if (p == NULL) {
            if (page_alloc(&p) != 0) {
                return -1;
            }
            page_insert(env->pgdir, p, va, PTE_RW | PTE_USER);
        }
        r = MIN(bin_size, BY2PG - offset);
        bcopy(bin, page2kva(p) + offset, r);
    }
    for (i = r; i < bin_size; i += r) {
        if (page_alloc(&p) != 0) {
            return -1;
        }
        page_insert(env->pgdir, p, va + i, PTE_RW | PTE_USER);
        r = MIN(BY2PG, bin_size - i);
        bcopy(bin + i, page2kva(p), r);
    }

    offset = va + i - ROUNDDOWN(va + i, BY2PG);
    if (offset > 0) {
        p = page_lookup(env->pgdir, va + i, &j);
        if (p == NULL) {
            if (page_alloc(&p) != 0) {
                return -1;
            }
            page_insert(env->pgdir, p, va + i, PTE_RW | PTE_USER);
        }
        r = MIN(sgsize - i, BY2PG - offset);
        bzero(page2kva(p) + offset, r);
    }
    for (i += r; i < sgsize; i += r) {
        if (page_alloc(&p) != 0) {
            return -1;
        }
        page_insert(env->pgdir, p, va + i, PTE_RW | PTE_USER);
        r = MIN(BY2PG, sgsize - i);
        bzero(page2kva(p), r);
    }
    return 0;
}

static void load_icode(struct Env* e, char* binary, u32 size) {
    struct Page* p = NULL;
    u64 entry_point;
    int r;

    load_elf(binary, size, &entry_point, e, load_icode_mapper);

    e->tf.elr_el1 = entry_point;
}

void env_create_priority(u8* binary, int size, int priority) {
    struct Env *e;

    int r = env_alloc(&e, 0);
    if (r < 0) {
        return;
    }

    e->pri = priority;
    load_icode(e, binary, size);

    LIST_INSERT_TAIL(&env_sched_list[0], e, sched_link);
}

void env_create(u8 *binary, int size) {
    env_create_priority(binary, size, 1);
}

extern void sched_yield();
void env_free(struct Env* e) {
    //printf("[%08x] free env %08x\n", curenv ? curenv->id : 0, e->id);
    u64 i, j, k;
    e->pgdir[GET_L1_INDEX(UVPT)] = 0;
    for (i = 0; i < PTE2PT; i++) {
        if (!(e->pgdir[i] & VALID))
            continue;
        u64 pa1 = PTE_ADDR(e->pgdir[i]);
        u64* pt1 = KADDR(pa1);
        for (j = 0; j < PTE2PT; j++) {
            if (!(pt1[j] & VALID))
                continue;
            u64 pa2 = PTE_ADDR(pt1[j]);
            u64* pt2 = KADDR(pa2);
            for (k = 0; k < PTE2PT; k++) {
                if (pt2[k] & VALID) {
                    u64 addr = (i << 30) | (j << 21) | (k << 12);
                    //printf("now got to %x\n", addr);
                    page_remove(e->pgdir, addr);
                    //printf("now finish to %x\n", addr);
                }
            }
            pt1[j] = 0;
            page_decref(pa2page(pa2));
            //panic("reach gggggg");
        }
        e->pgdir[i] = 0;
        page_decref(pa2page(pa1));
    }
    u64 pa = e->pgdir_pa;
    e->pgdir = 0;
    e->pgdir_pa = 0;
    page_decref(pa2page(pa));
    e->status = ENV_FREE;
    LIST_INSERT_HEAD(&env_free_list, e, link);
    //printf("hello env destroy\n");
}

void env_destroy(struct Env *e) {
    env_free(e);
    if (curenv == e) {
        curenv = NULL;
        //bcopy();
        //printf("[KERNEL]I am killed ... \n");
        sched_yield();
    }
}

extern void env_pop_tf(u64);

extern u64 TRAPSP[];
extern u64* lcontext(u64 pgdir);
extern void flush_tlb();

void env_run(struct Env* e) {
    if (curenv) {
        bcopy(*TRAPSP, &(curenv->tf), sizeof(struct Trapframe));
    }

    curenv = e;
    curenv -> env_runs ++;
    User_pgdir = lcontext(e->pgdir_pa);

    flush_tlb();

    e->tf.count_down = 62500 * 2;
    
    bcopy(&(e->tf), *TRAPSP, sizeof(struct Trapframe));

    env_pop_tf(*TRAPSP);
}

void save_sp(u64 sp) {
    (*TRAPSP) = sp;
}