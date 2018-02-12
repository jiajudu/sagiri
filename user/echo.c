#include<lib.h>
int64_t main(int64_t argc, char** argv){
    for(uint64_t i = 0; i < argc; i++){
        printf("arg %d: %s\n", i, argv[i]);
    }
    char* arg0 = "/hello";
    char* arg1 = "/license";
    uint64_t args[3];
    args[0] = (uint64_t)arg0;
    args[1] = (uint64_t)arg1;
    args[2] = 0;
    exec("/hello", args);
    return 0;
}