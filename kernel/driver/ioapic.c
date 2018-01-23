#include<driver/ioapic.h>
#include<lib/stdio.h>
volatile struct ioapic* ioapic;
uint32_t ioapicid = 0;
//ioapic对cpu表现为一系列的寄存器, 其中有一个控制寄存器, 一个数据寄存器. 读一个寄存器时, 先把index写到reg中, 然后在data中读取. 写一个寄存器时, 先把index写到reg中, 然后在data中读取. 
static uint32_t ioapicread(uint32_t reg) {
    ioapic->reg = reg;
    return ioapic->data;
}
static void ioapicwrite(uint32_t reg, uint32_t data) {
    ioapic->reg = reg;
    ioapic->data = data;
}
void ioapicinit(){
    //1号寄存器的bit23-16表示支持的中断数量
    uint64_t intrnum = (ioapicread(1) >> 16) & 0xff;
    for(uint64_t i = 0; i <= intrnum; i++) {
        //从0x10号寄存器开始, 每两个寄存器代表一个中断(64bit)
        //bit16表示是否禁用这个中断, bit15表示是电平触发(1)还是边沿触发(0), bit7-0表示对应的中断号
        ioapicwrite(0x10 + 2 * i, 0x10000 + 32 + i);
        ioapicwrite(0x10 + 2 * i + 1, 0);
    }
}