#include "lib.h"

void cursor(char *s, int n) {
    switch (s[0]) {
        case 'U':
            writef("\33[%dA", n);
            break;
        case 'D':
            writef("\33[%dB", n);
            break;
        case 'R':
            writef("\33[%dC", n);
            break;
        case 'L':
            writef("\33[%dD", n);
            break;
        case 'C':
            writef("\33[K");
            break;
        default:
            user_panic("error!!!\n");
            break;
    }
}