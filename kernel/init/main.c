#include<driver/uart.h>
#include<lib/x64.h>
#include<lib/stdio.h>
#include<mm/mm.h>
#include<smp/acpi.h>
#include<smp/lapic.h>
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
    printf("kernel is running...\n");
    while(1) {
        ;
    }
}
