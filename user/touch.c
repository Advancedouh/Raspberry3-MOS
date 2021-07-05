#include "lib.h"
#include <args.h>

void touch(char *path, char *prefix) {
    int fd, r;

    if ((fd = open(path, O_RDONLY)) >= 0)
        return;
    
    if ((r = create(path, FTYPE_REG)) < 0) {
        usage(-2, path);
    }

    return;
}

void usage(int error_code, char *path) {
    switch (error_code) {
        case -1:
            fwritef(1, "touch: missing file operand\n");
            break;
        case -2:
            fwritef(1, "file can't create for path: %s\n", path);
            return;
        default:
            break;
    }
    exit();
}

void umain(int argc, char **argv) {
    int i;
    ARGBEGIN{
	}ARGEND
    
    if (argc == 0)
        usage(-1, NULL);
    else {
        for (i = 0; i < argc; i++) {
            touch(argv[i], argv[i]);
        }
    }
}