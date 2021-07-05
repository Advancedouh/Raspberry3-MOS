//#include <stdarg.h>

#define 	LP_MAX_BUF 100
void lp_Print(void (*output)(void *, char *, int), void * arg, char *fmt, __builtin_va_list ap);

void printcharc(char c);

void printstr(char *s);