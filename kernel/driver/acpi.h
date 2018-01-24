#pragma once
int64_t acpiinit();
struct cpu{
    uint64_t id;
    uint64_t apicid;
    uint64_t started;
    void* local;
};
extern uint64_t cpuno;
extern struct cpu cpus[8];