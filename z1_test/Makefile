all: test-multiargs test-multiargs2 test-retval test-recurse test-data test-data2 test-rodata test-rodata2 test-reg test-syscall

test-%: %-64.c %-32-64.o common.o
	gcc $^ -mcmodel=small -no-pie -o $@

%-32.o: %-32.c
	gcc -m32 -fno-pic -fno-common -fno-stack-protector -c $<

%-32.o: %-32.s
	as --32 -o $@ $^

%-32-64.o: converter %-32.o %.flist
	./converter $(filter-out $<,$^) $@

clean:
	rm -f *.o test-*
