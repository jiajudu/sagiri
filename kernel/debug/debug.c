#include<debug/debug.h>
#include<lib/stdio.h>
#include<lib/x64.h>
#include<sync/spinlock.h>
void panic(char* s){
    printf("panic: %s\n", s);
    pushcli();
    while(1){
        ;
    }
}