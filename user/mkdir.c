#include<lib.h>
int64_t main(int64_t argc, char** argv){
    if(argc == 1){
        printf("usage: mkdir /test/\n");
    }
    int64_t mkdirret = mkdir(argv[1]);
    if(mkdirret < 0){
        printf("mkdir error\n");
        return 1;
    }
    return 0;
}