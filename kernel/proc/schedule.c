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
    struct thread* t = &(threads[target]);
    if(t != cpu->thread){
        struct thread* c = cpu->thread;
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
        if(cpu->thread->proc->killed){
            cpu->thread->tick = 10;
            release(&ptablelock);
            exitproc(-1);
            acquire(&ptablelock);
        }
        if(cpu->thread->killed){
            cpu->thread->tick = 10;
            release(&ptablelock);
            exitthread(-1);
            acquire(&ptablelock);
        }
    }
    cpu->thread->tick = 10;
    release(&ptablelock);
}