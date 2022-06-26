#include "multiargs.h"
void validate_neg(long, long, long long, long long);
void validate(int, long, long long, unsigned int, unsigned long, unsigned long long);
void validate_ptr(void*);

void check() {
	validate_neg(L1, -L1, L1, -L1);
	validate_ptr((void*) (1L<<31));
	validate(I1, L1, LL1, U1, UL1, ULL1);
}
