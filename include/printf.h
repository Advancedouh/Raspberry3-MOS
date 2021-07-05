
void printf(char *fmt, ...);

void _panic(const char *, int, const char *, ...)__attribute__((noreturn));

#define panic(...) _panic(__FILE__, __LINE__, __VA_ARGS__)
