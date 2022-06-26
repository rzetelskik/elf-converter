.section .rodata
fun_addr_64to32:
.long fun_stub_32
.long 0x23

.section .text

.code64
call fun
movq %rax, %rdx
shrq $0x20, %rdx
ljmpl *fun_addr_64to32

.code32
fun_stub_32:
addl $0x4, %esp
popl %esi
popl %edi
retl


