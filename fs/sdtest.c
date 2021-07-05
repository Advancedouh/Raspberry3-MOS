/*
 * operations on SD card.
 */

#include <mmu.h>
#include "fs.h"

char buf[4096];
void umain() {
    /*for(int i = 0; i < 2048; ++ i)
        buf[i] = 'a';
    //sd_write((4096/512)*2, buf, 2);
    for(int i = 0; i < 2048;++ i)
        buf[i] ++;
    sd_read((4096/512)*3, buf, 1);
    for(int i = 0; i < 2048;++i)
        writef("%c", buf[i]);
    writef("\n");*/
    fs_init();
    fs_test();
}