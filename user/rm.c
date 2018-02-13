#include<lib.h>
int64_t main(int64_t argc, char** argv){
    if(argc == 1){
        printf("usage: rm /test\n");
    }
    int64_t rmret = unlink(argv[1]);
    if(rmret < 0){
        printf("rm error\n");
        return 1;
    }
    return 0;
}