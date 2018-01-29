#include<syscall/syscall.h>
#include<lib/x64.h>
void sysenter();
void syscall(struct syscallframe* sf){
    
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