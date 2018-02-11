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
        write(fd, "Hello World\n", 12);
    }
    return 0;
}