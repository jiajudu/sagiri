.text
.globl sysenter
sysenter:
    movq %rsp, %r10
    movq %fs:(0xfffffffffffffff8), %rsp
    push %rax
    push %rbx
    push %rcx
    push %rdx
    push %rsi
    push %rdi
    push %rbp
    push %r8
    push %r9
    push %r10
    push %r11
    push %r12
    push %r13
    push %r14
    push %r15
    movq %rsp, %rdi
    call syscall
sysexit:
    pop %r15
    pop %r14
    pop %r13
    pop %r12
    pop %r11
    pop %r10
    pop %r9
    pop %r8
    pop %rbp
    pop %rdi
    pop %rsi
    pop %rdx
    pop %rcx
    pop %rbx
    pop %rax
    movq %r10, %rsp
tdebug:
    sysretq
.globl switchtouser
switchtouser:
    movq %rdi, %rsp
    jmp sysexit
