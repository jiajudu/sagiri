#include<syscall/syscall.h>
#include<lib/x64.h>
#include<lib/stdio.h>
#include<proc/proc.h>
#include<dev/console.h>
void sysenter();
uint64_t sys_exit(uint64_t arg0, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4){
    exitproc(-1);
    return 0;
}
uint64_t sys_print(uint64_t arg0, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4){
    for(uint64_t i = 0; i < arg1; i++){
        consoleput(*((char*)arg0 + i));
    }
    return arg1;
}
uint64_t (*systable[32])(uint64_t, uint64_t, uint64_t, uint64_t, uint64_t) = {
    sys_exit,
    0,
    0, 
    0,
    sys_print,
    0
};
void syscall(struct syscallframe* sf){
    systable[sf->rax](sf->rdi, sf->rsi, sf->rdx, sf->r8, sf->r9);
}
void syscallinit(){
    //设置IA32_FMASK寄存器, 不改变RFLAGS
    wrmsr(0xc0000084, 0, 0);
    //设置IA32_STAR寄存器
    wrmsr(0xc0000081, 0, (24 << 16) | 8);
    //设置IA32_LSTAR寄存器
    uint64_t rip = (uint64_t)sysenter;
    wrmsr(0xc0000082, rip & 0xffffffff, (rip >> 32) & 0xffffffff);
}