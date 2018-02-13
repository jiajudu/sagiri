#include<lib.h>
int64_t main(int64_t argc, char** argv){
    for(uint64_t i = 0; i < argc; i++){
        printf("arg %d: %s\n", i, argv[i]);
    }
    return 0;
}