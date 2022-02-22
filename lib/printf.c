#include <printf.h>
#include <print.h>

void printcharc(char ch);

static void myoutput(void *arg, char *s, int l)
{
    int i;

    if ((l==1) && (s[0] == '\0'))
        return;

    for (i=0; i< l; i++) {
        if (s[i] == '\n') {
            printcharc('\r');
        }
        printcharc(s[i]);
    }
}

void printf(char *fmt, ...)
{
    __builtin_va_list ap;
    __builtin_va_start(ap, fmt);
    lp_Print(myoutput, 0, fmt, ap);
    __builtin_va_end(ap);
}

void
_panic(const char *file, int line, const char *fmt,...) {
    __builtin_va_list ap;
    __builtin_va_start(ap, fmt);
    printf("panic at %s:%d: ", file, line);
    lp_Print(myoutput, 0, (char *)fmt, ap);
    printf("\n");
    __builtin_va_end(ap);
    for(;;); //dead loop
}
