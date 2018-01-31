#pragma once
#include<lib/util.h>
#include<sync/spinlock.h>
#include<syscall/syscall.h>
enum threadstate{
    thread_unused, thread_runnable, thread_running, thread_sleeping, thread_zombie
};
enum procstate{
    proc_unused, proc_running, proc_exiting, proc_zombie
};
struct waiter{
    uint64_t space;
};
struct proc{
    uint64_t pid;
    uint64_t state;
    struct proc* parent;
    uint64_t* pgdir;
    uint64_t heaptop;//一个用户进程的地址空间为0x400000-heaptop, stacktop-0x0000800000000000
    uint64_t stacktop;
    int64_t retvalue;
    uint64_t killed;
    struct spinlock pgdirlock;
    struct waiter exitwaiter;
};
struct thread{
    uint64_t tid;
    uint64_t* kstack;
    uint64_t state;   
    struct proc* proc;
    uint64_t rsp;
    int64_t retvalue;
    uint64_t killed;
    uint64_t tick;
    uint64_t needschedule;
    struct waiter exitwaiter;
    struct waiter* waiter;
    struct syscallframe* sf;
};
void procinit();
void exitthread(int64_t retvalue);
void exitproc(int64_t retvalue);
void proctick();
uint64_t getpid();
int64_t fork();
extern struct proc procs[128];
extern struct thread threads[256];
extern struct spinlock ptablelock;
