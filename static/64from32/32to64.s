.section .rodata
fun_addr_32to64:
.long fun_stub_64
.long 0x33

.section .text
.code32
fun_stub:
pushl %edi
pushl %esi
subl $0x4, %esp
ljmpl *fun_addr_32to64

.code64
fun_stub_64:

