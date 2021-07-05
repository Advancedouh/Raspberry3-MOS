#ifndef _USER_FD_H_
#define _USER_FD_H_

#include "type.h"
#include <fs.h>

// pre-declare for forward references
struct Fd;
struct Stat;
struct Dev;

struct Dev {
	int dev_id;
	char *dev_name;
	int (*dev_read)(struct Fd *, void *, u32, u32);
	int (*dev_write)(struct Fd *, const void *, u32, u32);
	int (*dev_close)(struct Fd *);
	int (*dev_stat)(struct Fd *, struct Stat *);
	int (*dev_seek)(struct Fd *, u32);
};

struct Fd {
	u32 fd_dev_id;
	u64 fd_offset;
	u32 fd_omode;
};

struct Stat {
	char st_name[MAXNAMELEN];
	u32 st_size;
	u32 st_isdir;
	struct Dev *st_dev;
};

struct Filefd {
	struct Fd f_fd;
	u32 f_fileid;
	struct File f_file;
};

int fd_alloc(struct Fd **fd);
int fd_lookup(int fdnum, struct Fd **fd);
u64 fd2data(struct Fd *);
int fd2num(struct Fd *);
int dev_lookup(int dev_id, struct Dev **dev);
u64 num2fd(int fd);
extern struct Dev devcons;
extern struct Dev devfile;
extern struct Dev devpipe;

#endif
