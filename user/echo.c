#include "lib.h"
#include <args.h>

void umain(int argc, char **argv) {
    ARGBEGIN{
    }ARGEND

    int i;
    for (i = 0; i < argc; ++i) {
        fwritef(1, argv[i]);
        fwritef(1, "\n");
    }
}