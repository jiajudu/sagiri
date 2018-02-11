#include<lib.h>
int64_t main(){
    printf("Hello World!\n");
    int64_t fd = open("/license", 1);
    char buf[100];
    int64_t readret = 100;
    while(readret == 100){
        readret = read(fd, buf, 100);
        for(uint64_t i = 0; i < readret; i++){
            put(buf[i]);
        }
    }
    close(fd);
    fd = open("/license", 6);
    for(uint64_t i = 0; i < 100; i++){
        int64_t writeret = write(fd, "Hello World ", 12);
    }
    printf("write finish\n");
    close(fd);
    return 0;
}