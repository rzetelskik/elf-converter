int r64(int, int*);

int r32(int depth, int *verify) {
	int test __attribute__((aligned(16)));
	if (((long) verify) & (0xf))
		return 1;

	/* just pass */
	return r64(depth, &test);
}
