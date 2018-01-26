#include<lib/util.h>
#include<lib/stdio.h>
#include<driver/acpi.h>
#include<mm/vm.h>
#include<lib/x64.h>
#include<proc/cpu.h>
volatile uint32_t* lapic;
static void microdelay() {

}
static void setlapic(uint64_t index, uint32_t value) {
    lapic[index] = value;
    //通过读取ID寄存器来等待前面的操作完成. ID寄存器的位置为lapic+0x20, 大小为32bit
    lapic[0x20 / 4];
}
void lapicinit() {
    if(!lapic){
        return;
    }
    //为了启用local APIC, 需要设置伪中断向量寄存器. 其位置为lapic+0xf0, 大小为32bit. 其中bit8表示是否启用local APIC, bit7-0表示伪中断的数量. 
    setlapic(0xf0 / 4, 0x1ff);
    //设置timer. 
    //设置除法配置寄存器(位置为lapic + 0x3e0), 使timer的频率等于外来频率除以1. 不同的输入对应的倍率: 0->2, 1->4, 2->8, 3->16, 8->32, 9->64, 10->128, 11->1
    setlapic(0x3e0 / 4, 0xb);
    //设置timer寄存器(位置为lapic + 0x320), bit18-17表示计数器状态(0:一次性, 1: 周期, 2: TSC-Deadline模式), bit16表示mask状态(0:接受中断, 1:拒绝中断), bit12为中断发送的状态(只读)(0: 中断源当前没有活动, 1: 之前发送的中断还没有被处理). bit7-0: 对应的中断向量号, 这里设置为32
    setlapic(0x320 / 4, 0x20020);
    //设置初始计数寄存器(位置为lapic + 0x380), 设置为10000000个周期产生一次中断
    setlapic(0x380 / 4, 10000000);
    //禁止LINT0和LINT1的中断(位置分别为lapic + 0x350 / 0x360), bit16表示mask状态, bit15表示触发模式(0: 边缘触发, 1: 电平触发, 这里采用边缘触发), bit13表示中断输入引脚的极性(0: 高电平表示激活, 1: 低电平表示激活), bit12: 中断发送的状态, bit10-8(中断发送模式, 固定为fixed(0), 发送中断向量号), bit7-0为对应的中断状态号
    setlapic(0x350 / 4, 0x10000);
    setlapic(0x360 / 4, 0x10000);
    //关闭性能计数寄存器的中断(位置为lapic + 0x340), bit16表示mask状态, bit12: 中断发送的状态, bit10-8(中断发送模式, 固定为fixed(0), 发送中断向量号), bit7-0为对应的中断状态号
    setlapic(0x340 / 4, 0x10000);
    //设置ACPI的内部错误对应51号中断, 错误向量的位置为lapic + 0x370, bit16表示mask状态, bit12: 中断发送的状态, bit7-0为对应的中断状态号
    setlapic(0x370 / 4, 0x33);
    //清除错误状态寄存器(lapic + 0x280 / 4). 每次写这个寄存器时, 寄存器的值设置为这一次写操作和上一次写操作之间发生的错误, 两次连续的写能够清除所有的错误. bit7-0分别代表不同的错误
    setlapic(0x280 / 4, 0);
    setlapic(0x280 / 4, 0);
    //设置中断服务已经完成. 这一操作在中断服务完成之后进行, 表示Local APIC可以发出下一个中断. 
    setlapic(0xb0 / 4, 0);
    //设置中断命令寄存器(64bit, bit63-32: lapic + 0x310, bit31-0: lapic + 0x300)
    //bit7-0中断向量号, 设置为0. bit10-8: 设置为5(INIT), 用来启动其他CPU. bit11: 目标地址的寻址模式(0: 物理寻址, 1: 逻辑寻址). bit12: Local APIC是否正在发送IPI(Interprocessor Interrupt). bit14: 在INIT模式中, 表示是否为de-assert发送模式. bit15: 触发模式. bit19-18: 目标速记(0: 没有速记, 1: 发送给自己, 2: 所有CPU, 3: 除自己外的所有CPU). bit63-56: 目标CPU, 仅当没有速记时有效. 
    // Send an Init Level De-Assert to synchronise arbitration ID's.
    setlapic(0x310 / 4, 0);
    setlapic(0x300 / 4, 0x88500);
    while(lapic[0x300 / 4] & 0x1000){
        ;
    }
    //让lapic允许发送所有中断, 所有优先级低于TPR(lapic + 0x80)的bit7-4的中断不会被发送
    setlapic(0x80 / 4, 0);
}
void finishintr() {
    //向lapic发送中断已经处理完成的信息
    setlapic(0xb0 / 4, 0);
}
void lapicstartup(uint8_t apicid, uint32_t addr) {
    //见MultiProcessing Specification B.4
    //在启动一个AP时, 必须把BIOS关闭码初始化为0xa, 热重置向量(0x40::0x67)指向AP的启动代码的位置
    outb(0x70, 0xf);
    outb(0x71, 0xa);
    *((uint32_t*)(p2k(0x467))) = addr;
    //发送init中断
    setlapic(0x310 / 4, apicid << 24);
    setlapic(0x300 / 4, 0xc500);
    microdelay();
    setlapic(0x300 / 4, 0x8500);
    microdelay();
    //发送startup中断
    setlapic(0x300 / 4, 0x600 | (addr >> 12));
    microdelay();
    setlapic(0x300 / 4, 0x600 | (addr >> 12));
    microdelay();
}