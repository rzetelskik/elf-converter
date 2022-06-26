#include <sys/mman.h>
#include <stdio.h>

void pstr(const char *str) {
	printf("%s", str);
}

void pnum(int n) {
	printf("%d", n);
}

extern int f(int x, int y);

int real_main() {
	return f(15,7) + f(42, 3);
}

int main() {
	int res = real_main();
	printf("res: %d\n", res);
	return 0;
}
