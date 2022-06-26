.section .rodata
fun_addr_64to32:
.long fun_stub_32
.long 0x23

fun_addr_32to64:
.long fun_stub_64
.long 0x33

.section .text
.code64
ljmpl *fun_addr_64to32

.code32
fun_stub_32:
pushl $0x2b
popl %ds
pushl $0x2b
popl %es
call fun
ljmpl *fun_addr_32to64

.code64
fun_stub_64:
