.text
.globl kthread
kthread:
    movq %rbx, %rdi
    call *%r15
    