#include<lib.h>
uint64_t syscall(uint64_t index, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4, uint64_t arg5);
uint64_t put(char s){
    return syscall(4, (uint64_t)s, 0, 0, 0, 0);
}
uint64_t exit(int64_t ret){
    return syscall(0, (uint64_t)ret, 0, 0, 0, 0);
}
uint64_t getpid(){
    return syscall(5, 0, 0, 0, 0, 0);
}
uint64_t strlen(char* p){
    uint64_t i = 0;
    for(; p[i] != 0; i++);
    return i;
}
static void printnum(uint64_t num, uint64_t base){
    char digits[17] = "0123456789abcdef";
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
        put(digits[buf[i]]);
    }
}
void printf(char* fmt, ...){
    __builtin_va_list ap;
    __builtin_va_start(ap, fmt);
    for(int64_t i = 0; fmt[i] != 0; i++){
        if(fmt[i] != '%' || fmt[i + 1] == 0){
            put(fmt[i]);
        }else{
            i++;
            switch(fmt[i]){
                case '%':
                    put('%');
                    break;
                case 'd':
                    printnum(__builtin_va_arg(ap, unsigned long), 10);
                    break;
                case 'x':
                    printnum(__builtin_va_arg(ap, unsigned long), 16);
                    break;
                case 's':
                    for(char* s = __builtin_va_arg(ap, char*); *s; s++){
                        put(*s);
                    }
                    break;
                default:
                    put('%');
                    put(fmt[i]);
                    break;
            }
        }
    }
}

