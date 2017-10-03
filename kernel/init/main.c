#include "driver/uart.h"
#include "lib/x64.h"
#include "lib/stdio.h"
#include "mm/mm.h"
#include "smp/acpi.h"
int64_t main() {
    uartearlyinit();
    printf("loading...\n");
    mminit();
    acpiinit();
    
    while(1) {
        ;
    }
}
