#include<lib/x64.h>
#include<lib/string.h>
#include<lib/stdio.h>
#include<mm/mm.h>
#include<mm/seg.h>
#include<driver/acpi.h>
#include<driver/lapic.h>
uint64_t gdt[7];
uint32_t tss[26];
void seginit() {
    void* local = (void*)alloc();
    memset(local, 0, 4096);
    //设置cpu的fs段基址
    uint64_t fsbase = (uint64_t)local + 2048;
    wrmsr(0xc0000100, fsbase & 0xffffffff, fsbase >> 32);
    struct cpu* cpu = cpus + cpunum();
    cpu->local = local;
    gdt[0] = 0x0000000000000000;
    //1-4代表内核代码, 用户代码, 内核数据, 用户数据, 定义见boot/enable64.S
    gdt[1] = 0x0020980000000000;
    gdt[2] = 0x0020f80000000000;
    gdt[3] = 0x0000920000000000;
    gdt[4] = 0x0000f20000000000;
    //tss描述符结构: 共128bit, bit95-56, bit39-16为tss的基址, bit55表示段界限的粒度, 0表示单位为Byte, 1表示单位为4KByte. bit51-48, bit15-0表示段界限, 设置为0x68 - 1, bit47表示是否存在, bit46-45表示特权级, bit43-40表示类型, 9表示64位tss
    //see https://xem.github.io/minix86/manual/intel-x86-and-64-manual-vol3/o_fe12b1e2a880e0ce-102.html
    gdt[5] = (0x0067) | (((uint64_t) tss & 0xffffff) << 16) | (0xe9ll << 40) | ((((uint64_t) tss >> 24) & 0xff) << 56);
    gdt[6] = ((uint64_t) tss >> 32);
    lgdt((void*)gdt, 56);
    ltr(40);
}