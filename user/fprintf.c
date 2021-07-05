#include "lib.h"

static int curlen;

static void user_out2string(void *arg, const char *s, int l)
{
	int i;
	char *b = (char *)arg;

	// special termination call
	if ((l == 1) && (s[0] == '\0')) {
		return;
	}

	for (i = curlen; i < l+curlen; i++) {
		b[i] = s[i-curlen];
	}
	
	curlen += l;
}


int fwritef(int fd, const char *fmt, ...)
{
	char buf[512];
	__builtin_va_list ap;
	__builtin_va_start(ap, fmt);
	user_bzero((void *)buf, 512);
	user_lp_Print(user_out2string, buf, fmt, ap);
	__builtin_va_end(ap);
	buf[curlen] = '\0';
	curlen = 0;
	return write(fd, buf, strlen(buf));
}
