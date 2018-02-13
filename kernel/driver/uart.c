#include<lib/util.h>
#include<lib/x64.h>
#include<driver/ioapic.h>
#include<lib/stdio.h>
#include<dev/console.h>
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
int64_t uartgetc() {
    if(!(inb(com1 + 5) & 0x1)) {
        return -1;
    }else{
        return inb(com1 + 0);
    }
}
void uartearlyinit() {
    // 见https://en.wikibooks.org/wiki/Serial_Programming/8250_UART_Programming#UART_Registers
    // 关闭先进先出
    outb(com1 + 2, 0);
    // 开始设置波特率
    outb(com1 + 3, 0x80);
    // 设置波特率为9600
    outb(com1 + 0, 115200 / 9600);
    outb(com1 + 1, 0);
    // 完成设置波特率, 设置字长为8bit, 1个停止位, 无校验位
    outb(com1 + 3, 0x03);
    // 设置调试控制寄存器
    outb(com1 + 4, 0);
    // 启用收到数据时产生中断
    outb(com1 + 1, 0x01);
    uart = 1;
}
void uartinit() {
    ioapicenable(4, 0);
}
void uartintr() {
    char c = uartgetc();
    if(c >= 0){
        consolewritec(c);
    }
}