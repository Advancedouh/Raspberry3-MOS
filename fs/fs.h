#ifndef _FS_FS_H_
#define _FS_FS_H_

#include "../include/type.h"
#include "../include/fs.h"

/* IDE disk number to look on for our file system */
#define DISKNO		1

#define BY2BLK      BY2PG
#define BY2SECT		512	/* Bytes per disk sector */
#define SECT2BLK	(BY2BLK/BY2SECT)	/* sectors to a block */

/* Disk block n, when in memory, is mapped into the file system
 * server's address space at DISKMAP+(n*BY2BLK). */
#define DISKMAP		0x200000000

/* Maximum disk size we can handle (4GB) */
#define DISKMAX		0x40000000

/* sd_card.c */
void sd_read(u32 secno, void *dst, u32 nsecs);
void sd_write(u32 secno, void *src, u32 nsecs);

/* fs.c */
int file_open(char *path, struct File **pfile);
int file_get_block(struct File *f, u32 blockno, void **pblk);
int file_set_size(struct File *f, u64 newsize);
void file_close(struct File *f);
int file_remove(char *path);
int file_dirty(struct File *f, u32 offset);
void file_flush(struct File *);

void fs_init(void);
void fs_sync(void);
extern u64 *bitmap;
int map_block(u32);
int alloc_block(void);

#endif