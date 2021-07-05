/*
 * File system server main loop -
 * serves IPC requests from other environments.
 */

#include "fs.h"
#include "../user/fd.h"
#include "../user/lib.h"
#include <mmu.h>
#include "error.h"

struct Open {
	struct File *o_file;	// mapped descriptor for open file
	u32 o_fileid;			// file id
	int o_mode;				// open mode
	struct Filefd *o_ff;	// va of filefd page
};

// Max number of open files in the file system at once
#define MAXOPEN			1024
#define FILEVA 			0x60000000

// initialize to force into data section
struct Open opentab[MAXOPEN] = { { 0, 0, 1 } };

// Virtual address at which to receive page mappings containing client requests.
#define REQVA	0x0f000000

// Overview:
//	Initialize file system server process.
void
serve_init(void)
{
	int i;
	u32 va;

	// Set virtual address to map.
	va = FILEVA;

	// Initial array opentab.
	for (i = 0; i < MAXOPEN; i++) {
		opentab[i].o_fileid = i;
		opentab[i].o_ff = (struct Filefd *)va;
		va += BY2PG;
	}
}

// Overview:
//	Allocate an open file.
int
open_alloc(struct Open **o)
{
	int i, r;

	// Find an available open-file table entry
	for (i = 0; i < MAXOPEN; i++) {
		switch (pageref(opentab[i].o_ff)) {
			case 0:
				if ((r = syscall_mem_alloc(0, (u64)opentab[i].o_ff,
										   VALID | PTE_RW | PTE_LIBRARY | PTE_USER)) < 0) {
					return r;
				}
			case 1:
				opentab[i].o_fileid += MAXOPEN;
				*o = &opentab[i];
				user_bzero((void *)opentab[i].o_ff, BY2PG);
				return (*o)->o_fileid;
		}
	}

	return -E_MAX_OPEN;
}

// Overview:
//	Look up an open file for envid.
int
open_lookup(u32 envid, u32 fileid, struct Open **po)
{
	struct Open *o;

	o = &opentab[fileid % MAXOPEN];

	if (pageref(o->o_ff) == 1 || o->o_fileid != fileid) {
		return -E_INVAL;
	}

	*po = o;
	return 0;
}

// Serve requests, sending responses back to envid.
// To send a result back, ipc_send(envid, r, 0, 0).
// To include a page, ipc_send(envid, r, srcva, perm).

void
serve_open(u32 envid, struct Fsreq_open *rq)
{
	//writef("serve_open %x %x 0x%x\n", envid, (int)rq->req_path, rq->req_omode);

	u8 path[MAXPATHLEN];
	struct File *f;
	struct Filefd *ff;
	int fileid;
	int r;
	struct Open *o;

	// Copy in the path, making sure it's null-terminated
	user_bcopy(rq->req_path, path, MAXPATHLEN);
	path[MAXPATHLEN - 1] = 0;

	// Find a file id.
	if ((r = open_alloc(&o)) < 0) {
		//user_panic("open_alloc failed: %d, invalid path: %s", r, path);
		ipc_send(envid, r, 0, 0);
        return ;
	}

	fileid = r;

	// Open the file.
	if ((r = file_open((char *)path, &f)) < 0) {
		//user_panic("file_open failed: %d, invalid path: %s", r, path);
		ipc_send(envid, r, 0, 0);
		return ;
	}

	// Save the file pointer.
	o->o_file = f;

	// Fill out the Filefd structure
	ff = (struct Filefd *)o->o_ff;
	ff->f_file = *f;
	ff->f_fileid = o->o_fileid;
	o->o_mode = rq->req_omode;
	ff->f_fd.fd_omode = o->o_mode;
	ff->f_fd.fd_dev_id = devfile.dev_id;

	ipc_send(envid, 0, (u64)o->o_ff, VALID | PTE_RW | PTE_LIBRARY | PTE_USER);
}

void
serve_map(u32 envid, struct Fsreq_map *rq)
{

	struct Open *pOpen;

	u32 filebno;

	void *blk;

	int r;

	if ((r = open_lookup(envid, rq->req_fileid, &pOpen)) < 0) {
		ipc_send(envid, r, 0, 0);
		return;
	}

	filebno = rq->req_offset / BY2BLK;

	if ((r = file_get_block(pOpen->o_file, filebno, &blk)) < 0) {
		ipc_send(envid, r, 0, 0);
		return;
	}

	u64 tmp = (u64)blk;
	u64 i;
	//writef("%s\n", *(u64*)(blk));
	/*for (i = tmp; i < tmp + BY2PG; i += 8)
		writef("%s\n", *((u64*)(i)));*/
	ipc_send(envid, 0, (u64)blk, VALID | PTE_RW | PTE_LIBRARY | PTE_USER);
}

void
serve_set_size(u32 envid, struct Fsreq_set_size *rq)
{
	struct Open *pOpen;
	int r;
	if ((r = open_lookup(envid, rq->req_fileid, &pOpen)) < 0) {
		ipc_send(envid, r, 0, 0);
		return;
	}

	if ((r = file_set_size(pOpen->o_file, rq->req_size)) < 0) {
		ipc_send(envid, r, 0, 0);
		return;
	}
	
	ipc_send(envid, 0, 0, 0);
}

void
serve_close(u32 envid, struct Fsreq_close *rq)
{
	struct Open *pOpen;

	int r;

	if ((r = open_lookup(envid, rq->req_fileid, &pOpen)) < 0) {
		ipc_send(envid, r, 0, 0);
		return;
	}
	
	file_close(pOpen->o_file);
	ipc_send(envid, 0, 0, 0);
}

// Overview:
//	fs service used to delete a file according path in `rq`.
void
serve_remove(u32 envid, struct Fsreq_remove *rq)
{
	int r;
	u8 path[MAXPATHLEN];

	// Step 1: Copy in the path, making sure it's terminated.
	user_bcopy(rq->req_path, path, MAXPATHLEN);
	path[MAXPATHLEN - 1] = '\0';

	// Step 2: Remove file from file system and response to user-level process.
	r = file_remove(path);
	if (r < 0)
	{
		ipc_send(envid, r, 0, 0);
        return ;
	}
	ipc_send(envid, 0, 0, 0);
}

void 
serve_create(u32 envid, struct Fsreq_create *rq)
{
	struct File *f;
	int r;
	u8 path[MAXPATHLEN];

	user_bcopy(rq->req_path, path, MAXPATHLEN);
	path[MAXPATHLEN - 1] = '\0';

	r = file_create(path, &f, rq->req_filetype);
	if (r < 0) {
		ipc_send(envid, r, 0, 0);
		return ;
	}
	ipc_send(envid, 0, 0, 0);
}

void
serve_dirty(u32 envid, struct Fsreq_dirty *rq)
{

	// Your code here
	struct Open *pOpen;
	int r;

	if ((r = open_lookup(envid, rq->req_fileid, &pOpen)) < 0) {
		ipc_send(envid, r, 0, 0);
		return;
	}

	if ((r = file_dirty(pOpen->o_file, rq->req_offset)) < 0) {
		ipc_send(envid, r, 0, 0);
		return;
	}

	ipc_send(envid, 0, 0, 0);
}

void
serve_sync(u32 envid)
{
	fs_sync();
	ipc_send(envid, 0, 0, 0);
}

void
serve(void)
{
	u64 req, whom;
	u64 perm;

	for (;;) {
		perm = 0;

		req = ipc_recv(&whom, REQVA, &perm);

		// All requests must contain an argument page
		if (!(perm & VALID)) {
			writef("Invalid request from %08x: no argument page\n", whom);
			continue; // just leave it hanging, waiting for the next request.
		}

		switch (req) {
			case FSREQ_OPEN:
				serve_open(whom, (struct Fsreq_open *)REQVA);
				break;

			case FSREQ_MAP:
				serve_map(whom, (struct Fsreq_map *)REQVA);
				break;

			case FSREQ_SET_SIZE:
				serve_set_size(whom, (struct Fsreq_set_size *)REQVA);
				break;

			case FSREQ_CLOSE:
				serve_close(whom, (struct Fsreq_close *)REQVA);
				break;

			case FSREQ_DIRTY:
				serve_dirty(whom, (struct Fsreq_dirty *)REQVA);
				break;

			case FSREQ_REMOVE:
				serve_remove(whom, (struct Fsreq_remove *)REQVA);
				break;

			case FSREQ_SYNC:
				serve_sync(whom);
				break;

			case FSREQ_CREATE:
				serve_create(whom, (struct Fsreq_create *)REQVA);
				break;

			default:
				writef("Invalid request code %d from %08x\n", whom, req);
				break;
		}

		syscall_mem_unmap(0, REQVA);
	}
}

void
umain(void)
{
	user_assert(sizeof(struct File) == BY2FILE);

	writef("FS is running\n");

	writef("FS can do I/O\n");

	serve_init();
	fs_init();
	fs_test();

	serve();
}

