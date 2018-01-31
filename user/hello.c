#include<lib.h>
int64_t a;
int64_t b = 3;
int64_t main(){
    printf("hello! this is proc %d\n", getpid());
    int t = 0;
    int64_t pid = fork();
    if(pid == 0){
        a = 4;
        printf("This is child. a = %d, pid = %d\n", a, getpid());
    }else{
        t = 8;
        a = 10;
        printf("This is parent. a = %d, pid = %d\n", a, getpid());
    }
    return 0;
}