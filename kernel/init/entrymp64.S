.code32
.globl start
start:
    # 启动物理地址扩展 cr4.pae=1
    mov %cr4, %eax
    bts $5, %eax
    mov %eax, %cr4
    # 使能长模式活动状态位 efer.lme=1
    mov $0xc0000080, %ecx
    rdmsr
    bts $8, %eax
    wrmsr
    # 设置页表
    mov 0x6fe8, %eax
    mov %eax, %cr3
    # 使能分页
    mov %cr0, %eax
    bts $31, %eax
    mov %eax, %cr0
    movl $gdtdesc, %edi
    lgdt (%edi)
    ljmp $0x8, $longmode
    # 正式进入长模式
.align 16
.code64
longmode:
    # 设置段寄存器
    mov $0x0, %ax
    mov %ax, %ds
    mov %ax, %ss
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs
    mov 0x6ff8, %rax
    call *%rax
.align 16
    # 长模式段描述符. 分段在长模式下是禁用的. 
    # 代码段中bit53表示位于长模式, bit54必须为0, 表示32位操作数尺寸. bit47表示存在性. bit46-45表示特权级, bit42表示一致性
    # 数据段中bit47表示存在性
    # 其余字段忽略
gdt:
    # 空段
    .quad 0x0000000000000000
    # 代码段
    .quad 0x0020980000000000
    # 数据段
    .quad 0x0000920000000000
gdtdesc:
    # gdt大小 - 1
    .word 0x17
    .quad gdt
