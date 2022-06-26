#include <sys/mman.h>
#include <stdio.h>

#define STACK_SIZE 0x10000


extern int real_main();

__asm__(
	"call_with_stack:\n"
	"pushq %rbp\n"
	"movq %rsp, %rbp\n"
	"movq %rdi, %rsp\n"
	"call real_main\n"
	"movq %rbp, %rsp\n"
	"popq %rbp\n"
	"ret\n"
);

int call_with_stack(void *ptr);

int main() {
	void *stack = mmap(0, STACK_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_32BIT, -1, 0);
	return call_with_stack(stack + STACK_SIZE);
}
