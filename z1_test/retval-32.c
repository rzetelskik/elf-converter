#include "multiargs.h"

extern long long llret64();
extern long lret64();
extern unsigned long luret64();
extern void* ptrret64();
static long long var = LL1;

long long llret32() {
	return llret64();
}

long lret32() {
	return lret64();
}

unsigned long luret32() {
	return luret64();
}

void* ptrret32() {
	return ptrret64();
}

long long get_var() {
	return var;
}
