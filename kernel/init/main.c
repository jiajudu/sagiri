#include "driver/uart.h"
#include "lib/stdio.h"
int64_t main() {
    uartearlyinit();
    printf("loading...\n");
    while(1) {
        ;
    }
}
