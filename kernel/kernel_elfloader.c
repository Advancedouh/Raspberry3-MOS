#include <type.h>
#include <kerelf.h>
#include <printf.h>

int is_elf_format(u8 *binary) {

    return 1;
}

int load_elf(u8* binary, int size, u64* entry_point, void* user_data,
    int (*map)(u64 va, u32 sgsize, u8* bin, u32 bin_size, void* user_data)) {
    
    Ehdr *ehdr = (Ehdr*) binary;
    Phdr *phdr = 0;

    u8 *ph_table = 0;
    u16 entry_cnt;
    u16 entry_size;
    int r;

    if (size < 4 || !is_elf_format(binary)) {
        return -1;
    }

    ph_table = binary + ehdr->phoff;
    entry_cnt = ehdr->phnum;
    entry_size = ehdr->phentsize;

    while (entry_cnt--) {
        phdr = ph_table;
        if (phdr->type == PT_LOAD) {
            r = map(phdr->vaddr, phdr->memsz, binary + phdr->offset, phdr->filesz, user_data);
            if (r < 0) {
                return r;
            }
        }
        ph_table += entry_size;
    }
    
    *entry_point = ehdr->entry;
    return 0;
}