#include<lib.h>
int64_t main(int64_t argc, char** argv){
    int64_t stdin = open(":console", 1);
    int64_t stdout = open(":console", 2);
    printf("stdin = %d, stdout = %d\n", stdin, stdout);
    uint64_t args[2];
    args[0] = (uint64_t)"/sh";
    args[1] = 0;
    exec("/sh", args);
    return 0;
}