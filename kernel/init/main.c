#include<driver/uart.h>
#include<lib/x64.h>
#include<lib/stdio.h>
#include<lib/string.h>
#include<mm/mm.h>
#include<mm/seg.h>
#include<mm/vm.h>
#include<mm/malloc.h>
#include<driver/acpi.h>
#include<driver/lapic.h>
#include<driver/ioapic.h>
#include<trap/trap.h>
#include<proc/cpu.h>
#include<sync/spinlock.h>
#include<proc/proc.h>
#include<proc/schedule.h>
#include<syscall/syscall.h>
char bspstack[0x8000];
void _startmp();
static void startothers() {
    extern char _binary_out_entrymp_start[], _binary_out_entrymp_size[];
    memcopy((char*)p2k(0x7000), _binary_out_entrymp_start, (uint64_t)_binary_out_entrymp_size);
    extern char _binary_out_entrymp64_start[], _binary_out_entrymp64_size[];
    memcopy((char*)p2k(0x7200), _binary_out_entrymp64_start, (uint64_t)_binary_out_entrymp64_size);
    bsp = cpu;
    for(struct cpu* c = cpus; c < cpus + cpuno; c++) {
        //不需要启动自身
        if(c == cpu) {
            continue;
        }
        uint64_t kstack = (uint64_t)threads[c - cpus].kstack;
        *(uint64_t*)(p2k(0x7000) - 8) = (uint64_t)_startmp;
        *(uint64_t*)(p2k(0x7000) - 16) = (uint64_t)(kstack + 0x8000);
        *(uint64_t*)(p2k(0x7000) - 24) = k2p((uint64_t)kpgdir);
        lapicstartup(c->apicid, 0x7000);
        while(c->started == 0) {
            ;
        }
    }
}
void mpstart() {
    cpuinit();
    seginit();
    idtinit();
    lapicinit();
    syscallinit();
    printf("cpu %d starting, thread %d, proc %d\n", cpu->id, cpu->thread->tid, cpu->thread->proc->pid);
    xchg(&(cpu->started), 1);
    while(bsp->started == 0){
        ;
    }
    popcli();
    while(1){
        cpu->thread->needschedule = 1;
    }
}
int64_t main() {
    uartearlyinit();
    printf("loading...\n");
    mminit();
    acpiinit();
    cpuinit();
    seginit();
    idtinit();
    lapicinit();
    ioapicinit();
    uartinit();
    procinit();
    syscallinit();
    startothers();
    printf("cpu %d starting, thread %d, proc %d\n", cpu->id, cpu->thread->tid, cpu->thread->proc->pid);
    xchg(&(cpu->started), 1);
    systemstarted = 1;
    popcli();
    while(1) {
        cpu->thread->needschedule = 1;
    }
}
