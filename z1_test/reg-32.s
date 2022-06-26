	.text
	.globl	check
	.type	check, @function
check:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%edi
	pushl	%esi
	/* set esi and edi to magic values */
	movl	$13, %esi
	movl	$31, %edi

	/* call 64-bit code */
	pushl	$0
	pushl	$42
	pushl	$0
	pushl	$24
	call	noop
	addl	$16, %esp

	/* check esi and edi */
	cmpl	$13, %esi
	jne	.do_exit
	cmpl	$31, %edi
	jne	.do_exit

	leal	-8(%ebp), %esp
	popl	%esi
	popl	%edi
	popl	%ebp
	ret


.do_exit:
	movl $1, %eax
	movl $1, %ebx
	int $0x80

