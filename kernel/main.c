#include "lib.h"
#include <env.h>
#include "sd.h"

extern u64 TRAPSP[];
extern void syscall_init();

char buf[256];
void kernel_main() {
    printf("[KERNEL]init.c:aarch64_init() is called\n");

    sd_init();


	memory_management_test();
	
	env_init();
	syscall_init();

	ENV_CREATE(fs_serv);
	ENV_CREATE(user_fstest);
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

//    for(int i = 0; i < 256; ++ i)
//        buf[i] = i;
//    if (sd_writeblock((void*)buf, (8192 + 128000), 1) == 0)
//        panic("error");
//    sd_readblock(130056, buf, 1);
//    for (int i = 0; i < 256; ++ i)
//        printf("%x ", (int)buf[i]);
//    printf("\n");
    panic("");
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
