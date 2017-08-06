#include "driver/uart.h"
#include "lib/x64.h"
#include "lib/stdio.h"
#include "mm/pmm.h"
int64_t main() {
    uartearlyinit();
    printf("loading...\n");
    pmminit();
    while(1) {
        ;
    }
}
