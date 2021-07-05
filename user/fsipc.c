#include "lib.h"
#include <fs.h>
#include "error.h"

#define debug 0

extern u8 fsipcbuf[BY2PG];		// page-aligned, declared in entry.S

// Overview:
//	Send an IPC request to the file server, and wait for a reply.
//
// Parameters:
//	@type: request code, passed as the simple integer IPC value.
// 	@fsreq: page to send containing additional request data, usually fsipcbuf.
//		Can be modified by server to return additional response info.
// 	@dstva: virtual address at which to receive reply page, 0 if none.
// 	@*perm: permissions of received page.
//
// Returns:
//	0 if successful,
//	< 0 on failure.
static int
fsipc(u32 type, void *fsreq, u64 dstva, u64 *perm)
{
	u32 whom;
	// NOTEICE: Our file system no.0 process!
	ipc_send(envs[0].id, type, (u64)fsreq, VALID | PTE_RW | PTE_USER);
	return ipc_recv(&whom, dstva, perm);
}

// Overview:
//	Send file-open request to the file server. Includes path and
//	omode in request, sets *fileid and *size from reply.
//
// Returns:
//	0 on success,
//	< 0 on failure.
int
fsipc_open(const char *path, u32 omode, struct Fd *fd)
{
	u64 perm;
	struct Fsreq_open *req;

	req = (struct Fsreq_open *)fsipcbuf;

	// The path is too long.
	if (strlen(path) >= MAXPATHLEN) {
		return -E_BAD_PATH;
	}

	strcpy((char *)req->req_path, path);
	req->req_omode = omode;
	return fsipc(FSREQ_OPEN, req, (u64)fd, &perm);
}

// Overview:
//	Make a map-block request to the file server. We send the fileid and
//	the (byte) offset of the desired block in the file, and the server sends
//	us back a mapping for a page containing that block.
//
// Returns:
//	0 on success,
//	< 0 on failure.
int
fsipc_map(u32 fileid, u32 offset, u64 dstva)
{
	int r;
	u64 perm;
	struct Fsreq_map *req;

	req = (struct Fsreq_map *)fsipcbuf;
	req->req_fileid = fileid;
	req->req_offset = offset;

	if ((r = fsipc(FSREQ_MAP, req, dstva, &perm)) < 0) {
		return r;
	}

	if ((perm & ~(PTE_RW | PTE_LIBRARY | PTE_USER)) != (VALID)) {
		user_panic("fsipc_map: unexpected permissions %lx for dstva %lx", perm, dstva);
	}

	return 0;
}

// Overview:
//	Make a set-file-size request to the file server.
int
fsipc_set_size(u32 fileid, u32 size)
{
	struct Fsreq_set_size *req;

	req = (struct Fsreq_set_size *)fsipcbuf;
	req->req_fileid = fileid;
	req->req_size = size;
	return fsipc(FSREQ_SET_SIZE, req, 0, 0);
}

// Overview:
//	Make a file-close request to the file server. After this the fileid is invalid.
int
fsipc_close(u32 fileid)
{
	struct Fsreq_close *req;

	req = (struct Fsreq_close *)fsipcbuf;
	req->req_fileid = fileid;
	return fsipc(FSREQ_CLOSE, req, 0, 0);
}

// Overview:
//	Ask the file server to mark a particular file block dirty.
int
fsipc_dirty(u32 fileid, u32 offset)
{
	struct Fsreq_dirty *req;

	req = (struct Fsreq_dirty *)fsipcbuf;
	req->req_fileid = fileid;
	req->req_offset = offset;
	return fsipc(FSREQ_DIRTY, req, 0, 0);
}

int 
fsipc_create(const char *path, int filetype)
{
	struct Fsreq_create *req;
	req = (struct Fsreq_create *)fsipcbuf;

	if (strlen(path) >= MAXFILESIZE)
		return -E_BAD_PATH;
		
	strcpy((char *)req->req_path, path);
	req->req_filetype = filetype;
	return fsipc(FSREQ_CREATE, req, 0, 0);
}
// Overview:
//	Ask the file server to delete a file, given its pathname.
int
fsipc_remove(const char *path)
{
	struct Fsreq_remove *req;
	req = (struct Fsreq_map *)fsipcbuf;

	// Step 1: decide if the path is valid.
	
	// The path is too long.
	if (strlen(path) >= MAXPATHLEN) {
		return -E_BAD_PATH;
	}

	// Step 2: Send request to fs server with IPC.
	strcpy((char *)req->req_path, path);
	return fsipc(FSREQ_REMOVE, req, 0 ,0);
}

// Overview:
//	Ask the file server to update the disk by writing any dirty
//	blocks in the buffer cache.
int
fsipc_sync(void)
{
	return fsipc(FSREQ_SYNC, fsipcbuf, 0, 0);
}

