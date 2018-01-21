#include<debug/debug.h>
#include<lib/stdio.h>
#include<lib/x64.h>
void panic(char* s){
    printf("panic: %s\n", s);
    cli();
    while(1){
        ;
    }
}