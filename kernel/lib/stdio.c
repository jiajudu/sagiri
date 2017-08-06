#include "dev/console.h"
#include "lib/ctype.h"
#include "lib/stdio.h"
static void printnum(uint64_t num, uint64_t base){
    char digits[17] = "0123456789abcedf";
    uint64_t buf[20] = {0};
    for(int64_t i = 0; i < 20; i++){
        buf[i] = num % base;
        num = num / base;
    }
    int64_t ptr = 19;
    if(base == 16){
        ptr = 15;
    }else{
        while(buf[ptr] == 0 && ptr > 0){
            ptr--;
        }
    }
    for(int64_t i = ptr; i >= 0; i--){
        consoleput(digits[buf[i]]);
    }
}
void printf(char* fmt, ...){
    __builtin_va_list ap;
    __builtin_va_start(ap, fmt);
    for(int64_t i = 0; fmt[i] != 0; i++){
        if(fmt[i] != '%' || fmt[i + 1] == 0){
            consoleput(fmt[i]);
        }else{
            i++;
            switch(fmt[i]){
                case '%':
                    consoleput('%');
                    break;
                case 'd':
                    printnum(__builtin_va_arg(ap, unsigned long), 10);
                    break;
                case 'x':
                    printnum(__builtin_va_arg(ap, unsigned long), 16);
                    break;
                case 's':
                    for(char* s = __builtin_va_arg(ap, char*); *s; s++){
                        consoleput(*s);
                    }
                    break;
                default:
                    consoleput('%');
                    consoleput(fmt[i]);
                    break;
            }
        }
    }
}