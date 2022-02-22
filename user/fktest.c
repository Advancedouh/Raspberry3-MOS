#include "lib.h"


void umain()
{
	int a = 0;
	int id = 0;

	if ((id = fork()) == 0) {
		if ((id = fork()) == 0) {
			a += 3;
			if ((id = fork()) == 0) {
				a += 4;
				for (;;) {
					writef("   this is child3 :a:%d\n", a);
				}
			}
			for (;;) {
				writef("  this is child2 :a:%d\n", a);
			}
		}

		a += 2;

		for (;;) {
			writef(" this is child :a:%d\n", a);
		}
	}

	a++;

	for (;;) {
		writef("this is father: a:%d\n", a);
	}
}
