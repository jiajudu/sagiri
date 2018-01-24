#pragma once
int64_t acpiinit();
struct cpu{
    uint64_t id;
    uint64_t apicid;
    volatile uint64_t started;
};
extern uint64_t cpuno;
extern struct cpu cpus[8];