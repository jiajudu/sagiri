#include<lib.h>
int64_t main(int64_t argc, char** argv){
    int64_t fd = open(":console", 3);
    printf("fd = %d\n", fd);
    write(fd, "Hello World!\n", 14);
    char buf[10];
    for(uint64_t i = 0; i < 10; i++){
        int64_t readret = 0;
        while(readret == 0){
            readret = read(fd, buf + i, 1);
        }
    }
    write(fd, buf, 10);
    return 0;
}