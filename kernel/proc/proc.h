#pragma once
#include<lib/util.h>
enum procstate{
    proc_unused, proc_runnable, proc_sleeping, proc_zombie
};
struct proc{
    uint64_t pid;
    uint64_t used;//0表示未占用, 1表示已占用
    struct proc* parent;
    uint64_t* pgdir;
    uint64_t heaptop;//一个用户进程的地址空间为0x400000-heaptop, stacktop-0x0000800000000000
    uint64_t stacktop;
};
struct thread{
    uint64_t tid;
    uint64_t* kstack;
    uint64_t state;   
    struct proc* proc;
    uint64_t rsp;
    struct trapframe* tf;
};
void procinit();

