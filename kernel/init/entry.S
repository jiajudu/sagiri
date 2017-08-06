.text
.globl _start
_start:
    # 把bss段清零
    movabs $edata, %rdi
    movabs $end, %rcx
loop:
    movq $0x0, (%rdi)
    add $0x8, %rdi
    cmp %rcx, %rdi
    jb loop
    movq $0xFFFF800000010000, %rsp
    jmp main
