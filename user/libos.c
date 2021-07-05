#include "lib.h"
#include <mmu.h>
#include <type.h>

int user_envid = 0;
struct Env *env;
struct Env *envs;

void
exit(void)
{
	syscall_env_destroy(0);
}

void libmain(int argc, char **argv)
{
	// set env to point at our env structure in envs[].
	user_envid = syscall_getenvid();
	envs = syscall_getenvs();
	env = envs + ENVX(user_envid);
	// call user main
	umain(argc, argv);
	// exit
	exit();
}
