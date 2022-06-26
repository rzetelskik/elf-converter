#include "multiargs.h"
#include <stdio.h>

extern long long validate(int, long, long long, unsigned int, unsigned long, unsigned long long);
extern int validate_ptr(void*);

int real_main() {
	if (validate(I1, L1, LL1, U1, UL1, ULL1) != LL_RET || validate_ptr((void*) (1L << 31)) != 0) 
		return -1;
	printf("OK\n");
	return 0;
}

