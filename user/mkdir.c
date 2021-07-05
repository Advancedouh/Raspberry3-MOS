#include "lib.h"
#include <args.h>

void mkdir(char *path, char *prefix) {
    int fd, r;
    struct File* file;

    if ((fd = open(path, O_RDONLY)) >= 0) {
        r = fd_lookup(fd, file);
        user_assert(r == 0);
        if (file -> f_type == FTYPE_REG) {
            fwritef(1, "file exist at path: %s\n", path);
            exit();
        }
        return;
    }
    
    if ((r = create(path, FTYPE_DIR)) < 0) {
        usage(-2, path);
    }

    return;
}

void usage(int error_code, char *path) {
    switch (error_code) {
        case -1:
            fwritef(1, "mkdir: missing operand\n");
            break;
        case -2:
            fwritef(1, "directory can't create at path: %s\n", path);
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
            mkdir(argv[i], argv[i]);
        }
    }
}