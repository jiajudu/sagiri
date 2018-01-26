#include<sync/spinlock.h>
#include<proc/cpu.h>
#include<lib/x64.h>
#include<lib/stdio.h>
void acquire(struct spinlock* s) {
    pushcli();
    while(xchg(&(s->lock), 1) != 0){
        ;
    }
}
void release(struct spinlock* s) {
    xchg(&(s->lock), 0);
    popcli();
}
void pushcli(){
    if(cpu->clinum == 0){
        cli();
    }
    cpu->clinum++;
}
void popcli(){
    cpu->clinum--;
    if(cpu->clinum == 0){
        sti();
    }
}