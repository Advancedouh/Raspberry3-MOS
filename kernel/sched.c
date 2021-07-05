#include <env.h>
#include <pmap.h>
#include <printf.h>
#include <queue.h>

extern struct Env_list env_sched_list[];
extern struct Env* curenv;

void sched_yield(void) {
    static int count = 0;
    static int point = 0;
    struct Env* next_env = curenv;
    while((count == 0) || (next_env == NULL) || (next_env -> status != ENV_RUNNABLE)) {
        if (next_env != NULL)
            LIST_INSERT_TAIL(&env_sched_list[point^1], next_env, sched_link);
        if (LIST_EMPTY(&env_sched_list[point]))
            point = 1 - point;
        if (LIST_EMPTY(&env_sched_list[point]))
            panic("No Env is RUNNABLE\n");
        next_env = LIST_FIRST(&env_sched_list[point]);
        LIST_REMOVE(next_env, sched_link);
        count = next_env->pri;
    }
    count --;
    //printf("[KERNEL]current env is %lx\n", next_env -> id);
    env_run(next_env);
}