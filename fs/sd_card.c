/*
 * operations on SD card.
 */

#include <mmu.h>
#include <printf.h>
#include "../user/lib.h"

#define sec_offset (2048 + 3000000)
//#define sec_offset 0

// Overview:
// 	read data from IDE disk. First issue a read request through
// 	disk register and then copy data from disk buffer
// 	(512 bytes, a sector) to destination array.
//
// Parameters:
//	diskno: disk number.
// 	secno: start sector number.
// 	dst: destination for data read from IDE disk.
// 	nsecs: the number of sectors to read.
//
// Post-Condition:
// 	If error occurred during read the IDE disk, panic. 
// 	
// Hint: use syscalls to access device registers and buffers
void
sd_read(u32 secno, void *dst, u32 nsecs)
{
	// 0x200: the size of a sector: 512 bytes.
    u64 current_section = secno + sec_offset;
    u64 end_section = secno + nsecs + sec_offset;
	u64 offset = 0;

	writef("@@@@@@@@@@@@@read sd section num: %d\n", current_section);

	while (current_section < end_section) {
		if (syscall_read_sd((u64)(dst + offset), current_section) < 0)
		{
			user_panic("sd read error!");
		}
		offset += 0x200;
        current_section ++;
	}
	//writef("ide_read %x %s\n", offset_begin, dst);
}


// Overview:
// 	write data to IDE disk.
//
// Parameters:
//	diskno: disk number.
//	secno: start sector number.
// 	src: the source data to write into IDE disk.
//	nsecs: the number of sectors to write.
//
// Post-Condition:
//	If error occurred during read the IDE disk, panic.
//	
// Hint: use syscalls to access device registers and buffers
void
sd_write(u32 secno, void *src, u32 nsecs)
{
    u64 current_section = secno + sec_offset;
    u64 end_section = secno + nsecs + sec_offset;
	u64 offset = 0;

	//writef("@@@@@@@@@@@@@write sd section num: %d\n", secno);

	while (current_section < end_section) {
        if (syscall_write_sd((u64)(src + offset), current_section) < 0)
		{
			user_panic("sd write error!");
		}
        offset += 0x200;
        current_section ++;
    }
	//writef("ide_write %x %s\n", offset_begin, src);
}

