.text
.globl kthread
kthread:
    movabs $ptablelock, %rdi
    movabs $release, %rax
    call *%rax
    movabs $settssrsp, %rax
    call *%rax
    movq %rbx, %rdi
    call *%r15
    movq %rax, %rdi
    call exitthread