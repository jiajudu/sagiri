#include<lib.h>
int64_t main(int64_t argc, char** argv){
    uint64_t no = 0;
    for(uint64_t i = 0; i < 5; i++){
        int64_t pid = fork();
        int64_t ret;
        waitproc(pid, &ret);
        no = no * 2;
        if(pid > 0){
            no++;
        }
    }
    printf("no = %d\n", no);
    return 0;
}