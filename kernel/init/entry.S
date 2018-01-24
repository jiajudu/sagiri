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
    movabs $bspstack, %rsp
    addq $4096, %rsp
    jmp main
.globl _startmp
_startmp:
    movq 0x6ff0, %rsp
    jmp mpstart
