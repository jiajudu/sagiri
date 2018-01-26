#include<proc/proc.h>
#include<proc/cpu.h>
#include<mm/vm.h>
#include<mm/malloc.h>
#include<sync/spinlock.h>
#include<lib/string.h>
struct context{

};
struct proc procs[128];
struct thread threads[256];
struct spinlock ptablelock;
void switchstack(uint64_t* oldrsp, uint64_t* newrsp);
static void setidleprocess(){
    //把0号进程和0-(cpuno-1)号线程设置为空闲进程/线程
    procs[0].pgdir = kpgdir;
    procs[0].used = 1;
    for(uint64_t i = 0; i < cpuno; i++){
        threads[i].kstack = 0;
        threads[i].proc = &(procs[0]);
        threads[i].state = proc_runnable;
        cpus[i].thread = &(threads[i]);
    }
}
void procinit(){
    for(uint64_t i = 0; i < 256; i++){
        threads[i].tid = i;
        threads[i].kstack = 0;
        threads[i].proc = 0;
        threads[i].state = 0;
        threads[i].rsp = 0;
        threads[i].tf = 0;
    }
    for(uint64_t i = 0; i < 128; i++){
        procs[i].heaptop = 0;
        procs[i].parent = 0;
        procs[i].pgdir = 0;
        procs[i].pid = i;
        procs[i].used = 0;
        procs[i].stacktop = 0x0000800000000000;
    }
    setidleprocess();
}
struct proc* allocproc(){
    struct proc* ret = 0;
    uint64_t curpid = cpu->thread->proc->pid;
    acquire(&ptablelock);
    for(uint64_t i = curpid; i < 128; i++){
        if(!(procs[i].used)){
            procs[i].used = 1;
            ret = &(procs[i]);
        }
    }
    if(!ret){
        for(uint64_t i = 0; i < curpid; i++){
            if(!(procs[i].used)){
                procs[i].used = 1;
                ret = &(procs[i]);
            }
        }
    }
    release(&ptablelock);
    return ret;
}
void freeproc(struct proc* p){
    p->used = 0;
}
struct thread* allocthread(){
    struct thread* ret = 0;
    uint64_t curtid = cpu->thread->tid;
    acquire(&ptablelock);
    for(uint64_t i = curtid; i < 256; i++){
        if(!(threads[i].state)){
            threads[i].state = proc_sleeping;
            ret = &(threads[i]);
        }
    }
    if(!ret){
        for(uint64_t i = 0; i < curtid; i++){
            if(!(threads[i].state)){
                threads[i].state = proc_sleeping;
                ret = &(threads[i]);
            }
        }
    }
    release(&ptablelock);
    return ret;
}
void freethread(struct thread* t){
    t->state = proc_unused;
}
int64_t fork(uint64_t newproc){
    uint64_t* kstack = (uint64_t*)alloc();
    if(kstack == 0){
        return -1;
    }
    uint64_t* pgdir = 0;
    if(newproc){
        pgdir = (uint64_t*)alloc();
        if(pgdir == 0){
            free((uint64_t)kstack);
            return -1;
        }
    }
    struct proc* p;
    if(newproc){
        p = allocproc();
        if(p == 0){
            free((uint64_t)kstack);
            free((uint64_t)pgdir);
            return -1;
        }
    }else{
        p = cpu->thread->proc;
    }
    struct thread* t = allocthread();
    if(t == 0){
        if(newproc){
            freeproc(p);
            free((uint64_t)pgdir);
        }
        free((uint64_t)kstack);
        return -1;
    }
    if(newproc){
        p->heaptop = 0;
        p->parent = cpu->thread->proc;
        p->pgdir = pgdir;
        memset((char*)pgdir, 0, 4096);
        p->stacktop = 0x0000800000000000;
    }
    t->kstack = kstack;
    memset((char*)kstack, 0, 4096);
    t->proc = p;
    t->rsp = 0;//????
    return 0;
}
int64_t kernelthread(int (*fn)(void *), void *arg, uint64_t clone_flags){
    
    return 0;
}