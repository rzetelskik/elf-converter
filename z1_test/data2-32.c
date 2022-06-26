extern long long bs;
extern long long buf[];

void check() {
	for (int i = 0; i < bs; i++)
		buf[i] = i;
}
