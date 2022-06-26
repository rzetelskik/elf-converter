#include <stdio.h>

extern char buf[];
extern long long size;
extern int check();

int real_main() {
	for (int i = 0; i < size; i++)
		buf[i] = i;
	if (check() != 0)
		return -1;
	printf("OK\n");
	return 0;
}

