    # bootloader的起始部分. 
    # bios把该代码从磁盘的第一个扇区加载到cs:ip=0:7c00并且从该位置开始执行
.code16
.globl start
start:
    # 关中断
    cli
    # 设置段寄存器ds/es/ss为0
    xorw %ax, %ax
    movw %ax, %ds
    movw %ax, %es
    movw %ax, %ss
    # 加载gdt
    lgdt gdtdesc
    # 开启保护模式
    movl %cr0, %eax
    orl $0x1, %eax
    movl %eax, %cr0
    # 切换到32位模式
    ljmp $0x8, $start32
    # 到此为止, 进入32位模式
.code32
start32:
    # 设置保护模式下的段寄存器, 均设置为gdt中的第2项. fs和gs不设置
    movw    $0x10, %ax
    movw    %ax, %ds
    movw    %ax, %es
    movw    %ax, %ss
    movw    $0, %ax
    movw    %ax, %fs
    movw    %ax, %gs
    # 设置栈指针, 进入bootmain
    movl    $0x8000, %esp
    movl    $0x7200, %eax
    call    *%eax
spin:
    jmp     spin
    # 段描述符结构
    # 每个段描述符长度为8字节, 各部分含义为
    # 段界限: 由bit51-48和bit15-0构成20位段界限, bit55表示单位. 其中0表示单位为1b, 1表示单位为4kb. 根据段类型字段bit43-40中的段扩展方向标志, 处理器可以以两种不同的方式使用limit: e=0: 表示向上扩展的段(简称上扩段), 逻辑地址中的偏移值范围可以从0到limit; e=1: 表示向下扩展的段(简称下扩段), 逻辑地址中的偏移范围可以从limit到0xffff(当b=0时)或者0xffffffff(当b=1时). 
    # 基地址: 由bit63-56, bit39-bit16构成. 基地址把段定位在虚拟地址空间的起点. 
    # 存在性: bit47
    # 特权级: bit46-45
    # 类型标志: bit44, 为1表示系统描述符. 在这里必须设置为1
    # 类型: bit54, bit43-40. bit43为1时表示代码段, 此时bit54表示默认操作数大小, 为1时大小为32位. bit42表示一致性, 非一致性代码段可以被同级代码段调用, 或者通过门调用; 一致性代码段可以从低特权级的程序转移到该段执行(但是低特权级的程序仍然保持自身的特权级). bit41表示是否可读. bit40表示是否访问过. bit43为0时表示数据段, 此时bit54表示栈指针大小, 为1时使用%esp, 为0时使用esp. bit42表示段扩展方向标志. bit41表示是否可写, bit40表示是否访问过. 
    # 其它: bit53-52需置为0
.align 4
gdt:
    # 空段
    .quad 0x0000000000000000
    # 代码段
    .quad 0x00cf9a000000ffff
    # 数据段
    .quad 0x00cf92000000ffff
gdtdesc:
    # 大小 - 1
    .word 0x17
    # gdt位置
    .long gdt
