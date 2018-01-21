#include<lib/string.h>
uint64_t strlen(char* p){
    uint64_t i = 0;
    for(; p[i] != 0; i++);
    return i;
}
void memset(char* p, char c, uint64_t size){
    for(uint64_t i = 0; i < size; i++){
        p[i] = c;
    }
}
int64_t memcmp(char* a, char* b, uint64_t size){
    for(uint64_t i = 0; i < size; i++){
        if(*a < *b){
            return -1;
        }
        if(*a > *b){
            return 1;
        }
    }
    return 0;
}