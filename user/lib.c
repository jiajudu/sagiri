#include<lib.h>
uint64_t syscall(uint64_t index, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4, uint64_t arg5);
uint64_t fork(){
    return syscall(0, 0, 0, 0, 0, 0);
}
uint64_t exit(int64_t ret){
    return syscall(1, (uint64_t)ret, 0, 0, 0, 0);
}
uint64_t thread(uint64_t fn, uint64_t* args){
    return syscall(2, fn, (uint64_t)args, 0, 0, 0);
}
uint64_t threadexit(int64_t ret){
    return syscall(3, ret, 0, 0, 0, 0);
}
uint64_t waitproc(uint64_t pid, int64_t* addr){
    return syscall(4, pid, (uint64_t)addr, 0, 0, 0);
}
uint64_t waitthread(uint64_t tid, int64_t* addr){
    return syscall(5, tid, (uint64_t)addr, 0, 0, 0);
}
uint64_t getpid(){
    return syscall(6, 0, 0, 0, 0, 0);
}
uint64_t gettid(){
    return syscall(7, 0, 0, 0, 0, 0);
}
uint64_t killproc(uint64_t pid){
    return syscall(8, pid, 0, 0, 0, 0);
}
uint64_t killthread(uint64_t tid){
    return syscall(9, tid, 0, 0, 0, 0);
}
uint64_t sleep(uint64_t tick){
    return syscall(10, tick, 0, 0, 0, 0);
}
uint64_t open(char* name, uint64_t flag){
    return syscall(11, (uint64_t)name, flag, 0, 0, 0);
}
uint64_t close(uint64_t fd){
    return syscall(12, fd, 0, 0, 0, 0);
}
uint64_t read(uint64_t fd, char* buf, uint64_t size){
    return syscall(13, fd, (uint64_t)buf, size, 0, 0);
}
uint64_t write(uint64_t fd, char* buf, uint64_t size){
    return syscall(14, fd, (uint64_t)buf, size, 0, 0);
}
uint64_t unlink(char* name){
    return syscall(15, (uint64_t)name, 0, 0, 0, 0);
}
uint64_t readdir(char* name, struct dircontent* buf){
    return syscall(16, (uint64_t)name, (uint64_t)buf, 0, 0, 0);
}
uint64_t stat(char* name, struct stat* buf){
    return syscall(17, (uint64_t)name, (uint64_t)buf, 0, 0, 0);
}
uint64_t mkdir(char* name){
    return syscall(18, (uint64_t)name, 0, 0, 0, 0);
}
uint64_t rmdir(char* name){
    return syscall(19, (uint64_t)name, 0, 0, 0, 0);
}
uint64_t lseek(uint64_t fd, int64_t off, uint64_t base){
    return syscall(20, fd, off, base, 0, 0);
}
uint64_t exec(char* name, uint64_t* args){
    return syscall(21, (uint64_t)name, (uint64_t)args, 0, 0, 0);
}
void put(char c){
    write(0, &c, 1);
}
static void printnum(int64_t num, uint64_t base){
    if(num < 0 && base == 10){
        put('-');
        num = -num;
    }
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

