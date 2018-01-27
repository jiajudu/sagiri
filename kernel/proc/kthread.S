.text
.globl kthread
kthread:
    movabs $ptablelock, %rdi
    movabs $release, %rax
    call *%rax
    movq %rbx, %rdi
    call *%r15
