#include "lib.h"

char buf[127];

void cat(int f, char *s) {
    long n;
    int r;

    while((n = read(f, buf, sizeof buf)) > 0) {
        if ((r = write(1, buf, n)) != n) {
            writef("write error copying %s: %d", s, n);
            exit();
        }
    }
    if (n < 0) {
        writef("error reading %s: %d\n", s, n);
        exit();
    }
}

void umain(int argc, char **argv) {
    int f, i;
    if (argc == 1)
        cat(0, "<stdin>");
    else {
        for (i = 1; i <argc; i++) {
            f = open(argv[i], O_RDONLY);
            if (f < 0) {
                writef("can't open %s: %d", argv[i], f);
                exit();
            }
            else {
                cat(f, argv[i]);
                close(f);
            }
        }
    }
}