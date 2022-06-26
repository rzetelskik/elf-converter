#include <stdio.h>

extern long long answer;

int real_main() {
	if (answer != 42)
		return -1;
	printf("OK\n");
	return 0;
}

