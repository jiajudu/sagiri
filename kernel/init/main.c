#include<driver/uart.h>
#include<lib/x64.h>
#include<lib/stdio.h>
#include<mm/mm.h>
#include<mm/seg.h>
#include<smp/acpi.h>
#include<smp/lapic.h>
#include<trap/trap.h>
extern char begin[];
extern char etext[];
extern char edata[];
extern char end[];
int64_t main() {
    uartearlyinit();
    printf("loading...\n");
    mminit();
    acpiinit();
    lapicinit();
    idtinit();
    seginit();
    printf("kernel is running...\n");
    asm volatile("int $0x4");
    //*((uint64_t*)0) = 5;
    while(1) {
        ;
    }
}
