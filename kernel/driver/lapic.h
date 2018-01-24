#pragma once
extern volatile uint32_t* lapic;
void lapicinit();
void finishintr();
int64_t cpunum();
void lapicstartup(uint8_t apicid, uint32_t addr);