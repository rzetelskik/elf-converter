#include <stdio.h>
#include "multiargs.h"

long long var = -42;

long long llret64() {
	return -LL1;
}

long lret64() {
	return -L1;
}

unsigned long luret64() {
	return UL1;
}

void* ptrret64() {
	return (void*) (1L << 31);
}

extern long long llret32();
extern long lret32();
extern unsigned long luret32();
extern long long get_var();
extern void* ptrret32();

int real_main() {
	if (llret32() != -LL1 || lret32() != -L1 || luret32() != UL1 || get_var() != LL1 || ptrret32() != (void*) (1L<<31))
		return -1;
	printf("OK\n");
	return 0;
}

