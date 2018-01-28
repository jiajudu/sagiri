#include<proc/proc.h>
#include<proc/cpu.h>
#include<mm/vm.h>
#include<mm/malloc.h>
#include<sync/spinlock.h>
#include<lib/string.h>
#include<lib/stdio.h>
#include<debug/debug.h>
#include<lib/x64.h>
#include<proc/schedule.h>
struct context{
    uint64_t r15;
    uint64_t r14;
    uint64_t r13;
    uint64_t r12;
    uint64_t rbx;
    uint64_t rbp;
    uint64_t rip;
    uint64_t padding;
};
struct proc procs[128];
struct thread threads[256];
struct spinlock ptablelock;
void kthread();
static void setidleprocess(){
    //把0号进程和0-(cpuno-1)号线程设置为空闲进程/线程
    procs[0].pgdir = kpgdir;
    procs[0].state = proc_running;
    for(uint64_t i = 0; i < cpuno; i++){
        threads[i].kstack = 0;
        threads[i].proc = &(procs[0]);
        threads[i].state = thread_running;
        cpus[i].thread = &(threads[i]);
    }
}
int64_t allocthread(uint64_t newproc){
    acquire(&ptablelock);
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
    struct proc* p = 0;
    if(newproc){
        uint64_t curpid = cpu->thread->proc->pid;
        for(uint64_t i = curpid; i < 128; i++){
            if(procs[i].state == proc_unused){
                procs[i].state = proc_running;
                p = &(procs[i]);
                break;
            }
        }
        if(!p){
            for(uint64_t i = 0; i < curpid; i++){
                if(procs[i].state == proc_unused){
                    procs[i].state = proc_running;
                    p = &(procs[i]);
                    break;
                }
            }
        }
        if(p == 0){
            free((uint64_t)kstack);
            free((uint64_t)pgdir);
            return -1;
        }
    }else{
        p = cpu->thread->proc;
    }
    struct thread* t = 0;
    uint64_t curtid = cpu->thread->tid;
    for(uint64_t i = curtid; i < 256; i++){
        if(!(threads[i].state)){
            threads[i].state = thread_sleeping;
            t = &(threads[i]);
            break;
        }
    }
    if(!t){
        for(uint64_t i = 0; i < curtid; i++){
            if(!(threads[i].state)){
                threads[i].state = thread_sleeping;
                t = &(threads[i]);
                break;
            }
        }
    }
    if(t == 0){
        if(newproc){
            p->state = proc_unused;
            free((uint64_t)pgdir);
        }
        free((uint64_t)kstack);
        return -1;
    }
    if(newproc){
        p->heaptop = 0;
        p->pgdir = pgdir;
        memcopy((char*)pgdir, (char*)kpgdir, 4096);
        p->stacktop = 0x0000800000000000;
        p->killed = 0;
    }
    t->proc = p;
    t->kstack = kstack;
    t->killed = 0;
    memset((char*)kstack, 0, 4096);
    release(&ptablelock);
    return t->tid;
}
int64_t createthread(uint64_t (*fn)(void *), void *args, uint64_t newproc){
    int64_t newtid = allocthread(newproc);
    if(newtid < 0){
        return newtid;
    }
    struct thread* t = &(threads[newtid]);
    struct context c;
    c.r15 = (uint64_t)fn;
    c.rbx = (uint64_t)args;
    c.rip = (uint64_t)kthread;
    t->rsp = (uint64_t)t->kstack + 4096 - sizeof(struct context);
    memcopy((char*)(t->rsp), (char*)(&c), sizeof(struct context));
    if(!newproc){
        t->proc = cpu->thread->proc;
    }else{
        t->proc->parent = cpu->thread->proc;
    }
    t->state = thread_runnable;
    return 0;
}
void exitthread(int64_t retvalue){
    acquire(&ptablelock);
    printf("exitthread\n");
    cpu->thread->retvalue = retvalue;
    cpu->thread->state = thread_zombie;
    release(&ptablelock);
    schedule();
}
void exitproc(int64_t retvalue){
    acquire(&ptablelock);
    printf("exitproc\n");
    for(uint64_t i = 0; i < 256; i++){
        if(threads[i].proc == cpu->thread->proc){
            threads[i].killed = 1;
        }
    }
    cpu->thread->proc->state = proc_exiting;
    cpu->thread->proc->retvalue = retvalue;
    release(&ptablelock);
    schedule();
}
uint64_t secondthread(void* args){
    printf("This is second thread.\n");
    exitproc(12);
    return 0;
}
uint64_t firstthread(void* args){
    static int64_t cpuid = -1;
    static int64_t c = 0;
    createthread(secondthread, 0, 0);
    while(1){
        if(cpu->id != cpuid){
            printf("cpu %d in thread %d, proc %d\n", cpu->id, cpu->thread->tid, cpu->thread->proc->pid);
            c++;
        }
        if(c == 10){
            break;
        }
        cpuid = cpu->id;
        schedule();
    }
    exitproc(34);
    return 10;
}
void procinit(){
    for(uint64_t i = 0; i < 256; i++){
        threads[i].tid = i;
        threads[i].kstack = 0;
        threads[i].proc = 0;
        threads[i].state = thread_unused;
        threads[i].rsp = 0;
        threads[i].killed = 0;
    }
    for(uint64_t i = 0; i < 128; i++){
        procs[i].heaptop = 0;
        procs[i].parent = 0;
        procs[i].pgdir = 0;
        procs[i].pid = i;
        procs[i].state = proc_unused;
        procs[i].stacktop = 0x0000800000000000;
        procs[i].killed = 0;
    }
    setidleprocess();
    createthread(firstthread, 0, 1);
}


