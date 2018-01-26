#pragma once
#include<lib/util.h>
#include<proc/proc.h>
struct cpu{
    uint64_t id;
    uint64_t apicid;
    uint64_t started;
    uint64_t clinum;
    void* local;
    struct thread* thread;
};
extern uint64_t cpuno;
extern struct cpu cpus[8];
extern uint64_t systemstarted;
extern __thread struct cpu* cpu;
extern struct cpu* bsp;
void cpuinit();