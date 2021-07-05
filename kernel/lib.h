#include <printf.h>
#include <mmu.h>
#include <image.h>
#include <pmap.h>

extern void irq_init();
extern int time_init();
extern int get_time_frequency();
extern int check_time();