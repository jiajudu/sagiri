#include<driver/uart.h>
#include<lib/x64.h>
#include<lib/stdio.h>
#include<mm/mm.h>
#include<mm/seg.h>
#include<driver/acpi.h>
#include<driver/lapic.h>
#include<driver/ioapic.h>
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
    seginit();
    idtinit();
    lapicinit();
    ioapicinit();
    uartinit();
    printf("kernel is running...\n");
    sti();
    while(1) {
        ;
    }
}
