#include "lib.h"
#include <args.h>

int debug = 0;

#define WHITESPACE " \t\r\n"
#define SYMOLS "<|>&;()"

int _gettoken(char *s, char **p1, char **p2) {
    int t;

    if (s == 0) {
        return 0;
    }

    *p1 = 0;
    *p2 = 0;

    while (strchr(WHITESPACE, *s))
        *s++ = 0;
    if (*s == 0) {
        return 0;
    }

    if (strchr(SYMOLS, *s)) {
        t = *s;
        *p1 = s;
        *s++ = 0;
        *p2 = s;
        return t;
    }

    *p1 = s;
    while (*s && !strchr(WHITESPACE SYMOLS, *s))
        s++;
    *p2 = s;
    return 'w';
}

int gettoken(char *s, char **p1) {
    static int c, nc;
    static char *np1, *np2;

    if (s) {
        nc = _gettoken(s, &np1, &np2);
        return 0;
    }

    c = nc;
    *p1 = np1;
    nc = _gettoken(np2, &np1, &np2);
    return c;
}

#define MAXARGS 16

void runcmd(char *s) {
    char *argv[MAXARGS], *t;
    int argc, c, r, i, p[2], fd, rightpipe;
    int fdnum;
    rightpipe = 0;
    gettoken(s, 0);
again:
    argc = 0;
    for(;;) {
        c = gettoken(0, &t);
        switch(c) {
            case 0:
                goto runit;
            case 'w':
                if (argc == MAXARGS) {
                    writef("too many arguments\n");
                    exit();
                }
                argv[argc++] = t;
                break;
            case '<':
                if (gettoken(0, &t) != 'w') {
				    writef("syntax error: < not followed by word\n");
				    exit();                
                }
                r = open(t, O_RDONLY);
                if (r < 0) {
                    writef("< open file failed!");
                    exit();
                }
                fd = r;
                dup(fd, 0);
                close(fd);
                break;
            case '>':
                if (gettoken(0, &t) != 'w') {
                    writef("syntax error: > not followed by word\n");
                    exit();
                }
                r = open(t, O_WRONLY);
                if (r < 0) {
                    writef("> open file failed!");
                    exit();
                }
                fd = r;
                dup(fd, 1);
                close(fd);
                break;
            case '|':
                pipe(p);
                if ((rightpipe = fork()) == 0) {
                    dup(p[0], 0);
                    close(p[0]);
                    close(p[1]);
                    goto again;
                } else {
                    dup(p[1], 1);
                    close(p[1]);
                    close(p[0]);
                    goto runit;
                }
            break;
        }
    }

runit:
    if (argc == 0) {
        if (debug) writef("EMPTY COMMAND\n");
        return;
    }
    argv[argc] = 0;
	if (0) {
		writef("[%08x] SPAWN:", env->id);
		for (i=0; argv[i]; i++)
			writef(" %s", argv[i]);
		writef("\n");
	}

    if ((r = spawn(argv[0], argv)) < 0) {
		writef("spawn %s: %e\n", argv[0], r);
        exit();
    }
	close_all();
	if (r >= 0) {
		if (debug) writef("[%08x] WAIT %s %08x\n", env->id, argv[0], r);
		wait(r);
	}
	if (rightpipe) {
		if (debug) writef("[%08x] WAIT right-pipe %08x\n", env->id, rightpipe);
		wait(rightpipe);
	}

	exit();
}

char buf[1024];

#define MAX(a, b) ((a) > (b) ? (a) : (b)) 
void readline(char *buf, u64 n) {
    int i, r;
    r = 0;
    for (i = 0; i < n; i++) {  
        if ((r = read(0, buf+i, 1)) != 1) {
            if (r < 0)
                writef("read error: %e", r);
            exit();
        }
        if (buf[i] == '\b' || buf[i] == 127) {
            i = MAX(i - 2, -1); 
            cursor("Left", 1);
            cursor("Clear", 0);
        }
        else if (buf[i] == '\r' || buf[i] == '\n') {
            buf[i] = 0;
            fwritef(1, "\n");
            return ;
        }
    }
    writef("the read line is too long!!!!\n");
    while((r = read(0, buf, 1)) == 1 && buf[0] != '\n');
    buf[0] = 0;
}

void usage(void)
{
	writef("usage: sh [-dix] [command-file]\n");
	exit();
}

void umain(int argc, char **argv) {
    int r, interactive, echocmds;
    interactive = '?';
    echocmds = 0;
    writef("\n:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::\n");
	writef("::                                                         ::\n");
	writef("::              Super Shell  V2.3.3_3                      ::\n");
	writef("::                                                         ::\n");
	writef(":::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::\n");
    ARGBEGIN{
        case 'd':
            debug ++;
            break;
        case 'i':
            interactive = 1;
            break;
        case 'x':
            echocmds = 1;
            break;
        default:
            usage();
    }ARGEND

    if (argc > 1)
        usage();
    else if (argc == 1) {
        close(0);
        if ((r = open(argv[1], O_RDONLY)) < 0)
            user_panic("open %s: %e", r);
        user_assert(r == 0);
    }

    if (interactive == '?')
        interactive = iscons(0);
    for (;;) {
        if (interactive)
            fwritef(1, "\n$ ");
        readline(buf, sizeof buf);

        if (buf[0] == '#')
            continue;
        if (echocmds)
            fwritef(1, "# %s\n", buf);
        if ((r = fork()) < 0)
            user_panic("fork: %e", r);
        if (r == 0) {
            runcmd(buf);
            exit();
            return ;
        } else 
            wait(r);
    }
}