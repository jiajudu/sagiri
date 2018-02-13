#include<lib.h>
int64_t main(int64_t argc, char** argv){
    if(argc == 1){
        printf("example: cat /license\n");
        return 1;
    }
    int64_t fd = open(argv[1], 1);
    if(fd < 0){
        printf("open error\n");
        return 1;
    }
    char buf[100];
    int64_t readret = 100;
    while(readret == 100){
        readret = read(fd, buf, 100);
        write(1, buf, readret);
    }
    close(fd);
    return 0;
}