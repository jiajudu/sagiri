.text
.globl _start
_start:
    call main
    movq %rax, %rdi
    call exit
.globl syscall
syscall:
    movq %rdi, %rax
    movq %rsi, %rdi
    movq %rdx, %rsi
    movq %rcx, %rdx
    syscall
    retq
