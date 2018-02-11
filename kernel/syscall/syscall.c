#include<syscall/syscall.h>
#include<lib/x64.h>
#include<lib/stdio.h>
#include<proc/proc.h>
#include<dev/console.h>
#include<debug/debug.h>
#include<proc/cpu.h>
#include<fs/fs.h>
void sysenter();
uint64_t sys_fork(uint64_t arg0, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4){
    return fork();
}
uint64_t sys_exit(uint64_t arg0, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4){
    exitproc(arg0);
    return 0;
}
uint64_t sys_thread(uint64_t arg0, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4){
    return userthread(arg0, arg1);
}
uint64_t sys_threadexit(uint64_t arg0, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4){
    exitthread(arg0);
    return 0;
}
uint64_t sys_waitproc(uint64_t arg0, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4){
    if(arg1 > 0x800000000000){
        return -1;
    }
    int64_t ret = 0;
    int64_t retv = 0;
    ret = waitproc(arg0, &retv);
    *((int64_t*)arg1) = retv;
    return ret;
}
uint64_t sys_waitthread(uint64_t arg0, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4){
    if(arg1 > 0x800000000000){
        return -1;
    }
    int64_t ret = 0;
    int64_t retv = 0;
    ret = waitthread(arg0, &retv);
    *((int64_t*)arg1) = retv;
    return ret;
}
uint64_t sys_getpid(uint64_t arg0, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4){
    return getpid();
}
uint64_t sys_gettid(uint64_t arg0, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4){
    return gettid();
}
uint64_t sys_killproc(uint64_t arg0, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4){
    return killproc(arg0);
}
uint64_t sys_killthread(uint64_t arg0, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4){
    return killthread(arg0);
}
uint64_t sys_sleep(uint64_t arg0, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4){
    sleep(arg0 * cpuno);
    return 0;
}
uint64_t sys_open(uint64_t arg0, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4){
    return fileopen((char*)arg0, arg1);
}
uint64_t sys_close(uint64_t arg0, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4){
    return fileclose(arg0);
}
uint64_t sys_put(uint64_t arg0, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4){
    consoleput(arg0);
    return arg1;
}
uint64_t (*systable[32])(uint64_t, uint64_t, uint64_t, uint64_t, uint64_t) = {
    sys_fork,
    sys_exit,
    sys_thread, 
    sys_threadexit,
    sys_waitproc,
    sys_waitthread,
    sys_getpid,
    sys_gettid,
    sys_killproc,
    sys_killthread,
    sys_sleep,
    sys_open,
    sys_close,
    sys_put
};
void syscall(struct syscallframe* sf){
    cpu->thread->sf = sf;
    sf->rax = systable[sf->rax](sf->rdi, sf->rsi, sf->rdx, sf->r8, sf->r9);
    if(cpu->thread->proc && cpu->thread->proc->killed){
        exitproc(-1);
    }
    if(cpu->thread->killed){
        exitthread(-1);
    }
}
void syscallinit(){
    //设置IA32_FMASK寄存器, 不改变RFLAGS
    wrmsr(0xc0000084, 0, 0);
    //设置IA32_STAR寄存器
    wrmsr(0xc0000081, 0, (24 << 16) | 8);
    //设置IA32_LSTAR寄存器
    uint64_t rip = (uint64_t)sysenter;
    wrmsr(0xc0000082, rip & 0xffffffff, (rip >> 32) & 0xffffffff);
    //设置syscall发生时禁用中断
    wrmsr(0xc0000084, 0x200, 0);
}