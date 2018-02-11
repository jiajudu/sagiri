#include<lib.h>
int64_t main(){
    printf("Hello World!\n");
    int64_t fd = open("/license", 1);
    printf("fd = %d\n", fd);
    return 0;
}