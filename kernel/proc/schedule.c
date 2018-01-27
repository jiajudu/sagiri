#include<proc/schedule.h>
#include<proc/proc.h>
#include<sync/spinlock.h>
#include<proc/cpu.h>
#include<lib/x64.h>
#include<mm/vm.h>
#include<lib/stdio.h>
#include<lib/string.h>
void switchstack(uint64_t* oldrsp, uint64_t* newrsp);
void schedule(){
    acquire(&ptablelock);
    uint64_t startthread = cpuno;
    uint64_t target = cpu->id;
    if(cpu->thread->tid >= cpuno){
        startthread = cpu->thread->tid;
    }
    for(uint64_t i = startthread; i < 256; i++){
        if(threads[i].state == proc_runnable){
            target = i;
            break;
        }
    }
    struct thread* t = &(threads[target]);
    if(t != cpu->thread){
        struct thread* c = cpu->thread;
        if(c->state == proc_running){
            c->state = proc_runnable;
        }
        cpu->thread = t;
        if(c->proc != t->proc){
            lcr3(k2p((uint64_t)(t->proc->pgdir)));
        }
        t->state = proc_running;
        switchstack(&(c->rsp), &(t->rsp));
    }
    release(&ptablelock);
}