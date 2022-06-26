#include "multiargs.h"
#include <stdio.h>
#include <stdlib.h>

void validate_neg(long a, long na, long long b, long long nb) {
	if (a != L1 || na != -L1 || b != L1 || nb != -L1)
		exit(-1);
}

void validate_ptr(void *p) {
	if (p != (void*) (1L << 31))
		exit(1);
}

void validate(int i1, long l1, long long ll1,
		unsigned int ui1, unsigned long ul1, unsigned long long ull1) {
	if (i1 == I1 && l1 == L1 && ll1 == LL1 && ui1 == U1 && ul1 == UL1 && ull1 == ULL1)
		printf("OK\n");
	else
		exit(-1);
}

void check();

int real_main() {
	check();
	return 0;
}

