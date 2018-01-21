#include<dev/console.h>
#include<driver/uart.h>
void consoleput(int64_t c){
    uartputc(c);
}