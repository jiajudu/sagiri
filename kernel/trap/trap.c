#include<lib/stdio.h>
#include<lib/x64.h>
#include<trap/trap.h>
#include<debug/debug.h>
#include<driver/lapic.h>
#include<driver/uart.h>
#include<proc/proc.h>
#include<sync/spinlock.h>
#include<proc/cpu.h>
#include<proc/schedule.h>
#include<mm/vm.h>
struct idtentry idt[256];
extern uint64_t vectors[256];
struct waiter tick;
struct spinlock ticklock;
static void printtrapframe(struct trapframe* tf) {
    printf("trap frame @%x\n", tf);
    printf("rax: %x ", tf->rax);
    printf("rbx: %x ", tf->rbx);
    printf("rcx: %x ", tf->rcx);
    printf("rdx: %x\n", tf->rdx);
    printf("rbp: %x ", tf->rbp);
    printf("rsi: %x ", tf->rsi);
    printf("rdi: %x ", tf->rdi);
    printf("r8:  %x\n", tf->r8);
    printf("r9:  %x ", tf->r9);
    printf("r10: %x ", tf->r10);
    printf("r11: %x ", tf->r11);
    printf("r12: %x\n", tf->r12);
    printf("r13: %x ", tf->r13);
    printf("r14: %x ", tf->r14);
    printf("r15: %x\n", tf->r15);
    printf("trapno: %d\n", tf->trapno);
    printf("err:    %d\n", tf->err);
    printf("rip:    %x\n", tf->rip);
    printf("cs:     %x\n", tf->cs);
    printf("rflags: %x\n", tf->rflags);
    printf("rsp:    %x\n", tf->rsp);
    printf("ss:     %x\n", tf->ss);
}
void interrupt(struct trapframe* tf){
    switch(tf->trapno) {
        case 14: { //缺页异常
            //printtrapframe(tf);
            uint64_t addr = rcr2();
            //printf("addr: %x\n", addr);
            pagefault(addr);
        }
        case 32: { //定时器中断
            acquire(&ticklock);
            tick.space++;
            proctick();
            if(tick.space % 100 == 0) {
                //printf("tick: %d\n", tick);
            }
            release(&ticklock);
            finishintr();
            break;
        }
        case 36: { //串口中断
            uartintr();
            finishintr();
            break;
        }
        default: {
            printtrapframe(tf);
            panic("trap!");
        }
    }
    if(cpu->thread->needschedule){
        schedule();
    }
}
void idtinit(){
    //设置中断向量表
    //每个中断向量128bit, 共256个. bit95-bit48, bit15-0为目标地址, bit31-16为目标段选择子, bit34-32为中断堆栈表偏移, bit43-40表示类型, 固定为14(中断门), bit46-45表示调用这个中断门需要的特权级, bit47表示这个向量是否存在. bit44需要为0
    for(uint64_t i = 0; i < 256; i++){
        idt[i].off15_0 = (vectors[i] & 0xffff);
        idt[i].off31_16 = (vectors[i] & 0xffff0000) >> 16;
        idt[i].off63_32 = (vectors[i] & 0xffffffff00000000) >> 32;
        idt[i].seg = 8;
        idt[i].ist_resv1 = 0;
        idt[i].p_dpl_s_type = 0x8f;
        idt[i].resv2 = 0;
    }
    lidt(idt, 4096);
}