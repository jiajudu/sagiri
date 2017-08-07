#include "driver/uart.h"
#include "lib/x64.h"
#include "lib/stdio.h"
#include "mm/mm.h"
int64_t main() {
    uartearlyinit();
    printf("loading...\n");
    mminit();
    while(1) {
        ;
    }
}
