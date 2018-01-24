#include<proc/cpu.h>
#include<driver/lapic.h>
#include<mm/malloc.h>
#include<lib/string.h>
#include<lib/x64.h>
uint64_t cpuno = 0;
struct cpu cpus[8];
__thread struct cpu* cpu;
void cpuinit(){
    int64_t index = 0;
    int64_t id = lapic[0x20 / 4] >> 24;
    for(int64_t n = 0; n < cpuno; n++) {
        if(id == cpus[n].apicid) {
            index = n;
        }
    }
    void* local = (void*)alloc();
    memset(local, 0, 4096);
    //设置cpu的fs段基址
    uint64_t fsbase = (uint64_t)local + 2048;
    wrmsr(0xc0000100, fsbase & 0xffffffff, fsbase >> 32);
    cpu = cpus + index;
    cpu->local = local;
}