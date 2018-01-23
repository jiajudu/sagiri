#pragma once
#include<lib/util.h>
struct ioapic {
    uint32_t reg;
    uint32_t pad[3];
    uint32_t data;
};
extern volatile struct ioapic* ioapic;
extern uint32_t ioapicid;
void ioapicinit();
void ioapicenable(uint64_t irq, uint64_t cpunum);