all: test

test: test.c test-64-32.o
	gcc -m32 test.c test-64-32.o -no-pie -o test

test-64.o: test-64.c
	gcc -mcmodel=small -fno-pic -c test-64.c -o test-64.o

test-64-32.o: test-64.o
	./converter test-64.o test.flist test-64-32.o

clean:
	rm -f *.o test
