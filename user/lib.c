#include<lib.h>
uint64_t syscall(uint64_t arg0, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4, uint64_t arg5){
    asm volatile("movq %rcx, %rax");
    asm volatile("syscall");
}
uint64_t sys_print(char* s, uint64_t len){
    return syscall((uint64_t)s, len, 0, 4, 0, 0);
}
uint64_t sys_exit(int64_t ret){
    return syscall((uint64_t)ret, 0, 0, 0, 0, 0);
}
uint64_t strlen(char* p){
    uint64_t i = 0;
    for(; p[i] != 0; i++);
    return i;
}
void print(char* s){
    uint64_t i = strlen(s);
    sys_print(s, i);
}
void exit(int64_t ret){
    sys_exit(ret);
}