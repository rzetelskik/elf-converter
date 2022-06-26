#include "multiargs.h"
#include <stdio.h>

long long buf[64];
long long bs = 64;

void check();

int real_main() {
	check();
	for (int i = 0; i < bs; i++)
		if (buf[i] != i)
			return -1;
	printf("OK\n");
	return 0;
}

