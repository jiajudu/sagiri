# void switchstack(uint64_t* oldrsp, uint64_t* newrsp);
.global switchstack
switchstack:
    push %rbp
    push %rbx
    push %r12
    push %r13
    push %r14
    push %r15
    movq %rsp, (%rdi)
    movq (%rsi), %rsp
    pop %r15
    pop %r14
    pop %r13
    pop %r12
    pop %rbx
    pop %rbp
    ret