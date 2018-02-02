#include<proc/schedule.h>
#include<proc/proc.h>
#include<sync/spinlock.h>
#include<proc/cpu.h>
#include<lib/x64.h>
#include<mm/vm.h>
#include<lib/stdio.h>
#include<lib/string.h>
#include<mm/seg.h>
#include<debug/debug.h>
void switchstack(uint64_t* oldrsp, uint64_t* newrsp);
void schedule(){
    acquire(&ptablelock);
    uint64_t startthread = cpuno;
    uint64_t target = cpu->id;
    cpu->thread->needschedule = 0;
    if(cpu->thread->tid >= cpuno){
        startthread = cpu->thread->tid;
    }
    for(uint64_t i = startthread; i < 256; i++){
        if(threads[i].state == thread_runnable){
            target = i;
            break;
        }
    }
    struct thread* c = cpu->thread;
    struct thread* t = &(threads[target]);
    if(t != c){
        c->cr2 = rcr2();
        if(c->state == thread_running){
            c->state = thread_runnable;
        }
        cpu->thread = t;
        if(c->proc != t->proc){
            lcr3(k2p((uint64_t)(t->proc->pgdir)));
        }
        t->state = thread_running;
        switchstack(&(c->rsp), &(t->rsp));
        settssrsp();
        lcr2(cpu->thread->cr2);
    }
    assert(cpu->thread->state == thread_running);
    cpu->thread->tick = 10;
    release(&ptablelock);
    if(cpu->thread->proc && cpu->thread->proc->killed){
        exitproc(-1);
    }
    if(cpu->thread->killed){
        exitthread(-1);
    }
}