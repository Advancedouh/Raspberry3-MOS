#include "lib.h"
#include <env.h>
#include "sd.h"

extern u64 TRAPSP[];
extern void syscall_init();

char buf[256];
void kernel_main() {
    printf("[KERNEL]init.c:\taarch64_init() is called\n");

    while(1) {
    }

    /*
	memory_management_test();
	
	env_init();
	syscall_init();

	sd_init();

	ENV_CREATE(fs_serv);
	//ENV_CREATE(user_fstest);
	//ENV_CREATE(user_testpipe);
	//ENV_CREATE(user_testpiperace);
	//ENV_CREATE(user_fktest);
	//ENV_CREATE(user_pingpong);
	//ENV_CREATE(user_pingpong1);
	//ENV_CREATE(user_pingpong2);
	//ENV_CREATE(user_idle1);
	//ENV_CREATE(user_idle2);
	ENV_CREATE(user_icode);

	irq_init();
     */
}

void bcopy(const void *src, void *dst, int len)
{
	void *max;

	max = dst + len;

	while (dst + 3 < max) {
		*(int *)dst = *(int *)src;
		dst += 4;
		src += 4;
	}

	while (dst < max) {
		*(char *)dst = *(char *)src;
		dst += 1;
		src += 1;
	}
}

void bzero(void *b, int len)
{
	void *max;

	max = b + len;

	while (b + 3 < max) {
		*(int *)b = 0;
		b += 4;
	}

	while (b < max) {
		*(char *)b++ = 0;
	}

}
