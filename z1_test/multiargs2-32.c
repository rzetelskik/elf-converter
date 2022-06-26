#include "multiargs.h"
long long validate(int i1, long l1, long long ll1, unsigned int ui1, unsigned long ul1, unsigned long long ull1) {
	if (i1 == I1 && l1 == L1 && ll1 == LL1 && ui1 == U1 && ul1 == UL1 && ull1 == ULL1)
		return LL_RET;
	else
		return -1;
}

int validate_ptr(void *p) {
	return p == (void*) (1L << 31) ? 0 : -1;
}
