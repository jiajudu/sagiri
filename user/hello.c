#include<lib.h>
int64_t a;
int64_t b = 3;
void threadstart(void* args){
    printf("this is parent thread. a = %d, pid = %d, tid = %d\n", a, getpid(), gettid());
    uint64_t c = 0;
    for(uint64_t i = 0; i < 10000; i++){
        c += i;
    }
    //sleep(500);
    threadexit(c);
}
int64_t main(){
    printf("hello! pid = %d, tid = %d\n", getpid(), gettid());
    int64_t pid = fork();
    if(pid == 0){
        a = 4;
        printf("this is child. a = %d, pid = %d, tid = %d\n", a, getpid(), gettid());
        //sleep(500);
        return 22;
    }else{
        int64_t pret = 0;
        waitproc(pid, &pret);
        printf("child exit with %d\n", pret);
        a = 10;
        printf("this is parent. a = %d, pid = %d, tid = %d\n", a, getpid(), gettid());
        int64_t tid = thread((uint64_t)threadstart, 0);
        //killthread(tid);
        printf("new tid: %d\n", tid);
        int64_t c = 0;
        waitthread(tid, &c);
        printf("c = %d\n", c);
    }
    return 0;
}