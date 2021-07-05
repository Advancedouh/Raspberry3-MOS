#include "lib.h"

int x;
void umain()
{
	if (x) {
		user_panic("bss not cleared");
	}
	x++;
	while (1) {
		writef("%d ", 2);
		x++;
	}
}