extern void pnum(int);
extern void pstr(const char *);

int f(int x, int y) {
	int res = x + y;
	pnum(x);
	pstr(" + ");
	pnum(y);
	pstr(" = ");
	pnum(res);
	pstr("\n");
	return res;
}

