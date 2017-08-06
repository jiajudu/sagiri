#include "ctype.h"
#include "asm.h"
static uint64_t COM1 = 0x3f8;
static uint64_t uart;
void microdelay(int64_t us) {
}
void uartputc(int64_t c) {
    if(!uart){
        return;
    }
    for(int64_t i = 0; i < 128 && !(inb(COM1 + 5) & 0x20); i++){
        microdelay(10);
    }
    outb(COM1 + 0, c);
}
void uartearlyinit() {
    // Turn off the FIFO
    outb(COM1 + 2, 0);
    // 9600 baud, 8 data bits, 1 stop bit, parity off.
    outb(COM1 + 3, 0x80);    // Unlock divisor
    outb(COM1 + 0, 115200 / 9600);
    outb(COM1 + 1, 0);
    outb(COM1 + 3, 0x03);    // Lock divisor, 8 data bits.
    outb(COM1 + 4, 0);
    outb(COM1 + 1, 0x01);    // Enable receive interrupts.
    // If status is 0xFF, no serial port.
    if(inb(COM1 + 5) == 0xFF){
        return;
    }
    uart = 1;
    // Announce that we're here.
    for(char* p="hello world!\n"; *p; p++) {
        uartputc(*p);
    }
}
int64_t main() {
    uartearlyinit();
    while(1) {
        ;
    }
}
