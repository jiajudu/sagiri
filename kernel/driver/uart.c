#include "lib/util.h"
#include "lib/x64.h"
static uint64_t com1 = 0x3f8;
static uint64_t uart;
static void microdelay(int64_t us) {
}
void uartputc(int64_t c) {
    if(!uart){
        return;
    }
    for(int64_t i = 0; i < 128 && !(inb(com1 + 5) & 0x20); i++){
        microdelay(10);
    }
    outb(com1 + 0, c);
}
void uartearlyinit() {
    // turn off the fifo
    outb(com1 + 2, 0);
    // 9600 baud, 8 data bits, 1 stop bit, parity off.
    outb(com1 + 3, 0x80);    // unlock divisor
    outb(com1 + 0, 115200 / 9600);
    outb(com1 + 1, 0);
    outb(com1 + 3, 0x03);    // lock divisor, 8 data bits.
    outb(com1 + 4, 0);
    outb(com1 + 1, 0x01);    // enable receive interrupts.
    // if status is 0xff, no serial port.
    if(inb(com1 + 5) == 0xff){
        return;
    }
    uart = 1;
}