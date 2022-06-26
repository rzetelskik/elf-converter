	.text
	.globl	msg
	.section	.rodata
	.align 4
	.type	msg, @object
	.size	msg, 4
msg:
	.string	"OK\n"
	.text
	.globl	check
	.type	check, @function
check:
	movl $4, %eax
	movl $1, %ebx
	leal msg, %ecx
	movl $3, %edx
	int $0x80

	movl $1, %eax
	movl $0, %ebx
	int $0x80
