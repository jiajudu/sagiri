#include<lib.h>
int64_t main(){
    printf("Hello World!\n");
    int64_t fd = open("/license", 1);
    printf("fd = %d\n", fd);
    int64_t ret = close(fd);
    printf("ret = %d\n", ret);
    ret = close(fd);
    printf("ret = %d\n", ret);
    return 0;
}