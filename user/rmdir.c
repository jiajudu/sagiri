#include<lib.h>
int64_t main(int64_t argc, char** argv){
    if(argc == 1){
        printf("usage: rmdir /test/\n");
    }
    int64_t rmdirret = rmdir(argv[1]);
    if(rmdirret < 0){
        printf("rmdir error\n");
        return 1;
    }
    return 0;
}