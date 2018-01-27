#include<dev/console.h>
#include<driver/uart.h>
#include<sync/spinlock.h>
#include<proc/cpu.h>
struct spinlock consolelock;
void consoleput(int64_t c){
    if(systemstarted){
        acquire(&consolelock);
    }
    uartputc(c);
    if(systemstarted){
        release(&consolelock);
    }   
}