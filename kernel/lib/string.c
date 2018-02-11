#include<lib/string.h>
uint64_t strlen(char* p){
    uint64_t i = 0;
    for(; p[i] != 0; i++);
    return i;
}
int64_t strncmp(char* a, char* b, uint64_t size){
    for(uint64_t i = 0; i < size; i++){
        if(a[i] < b[i]){
            return -1;
        }
        if(a[i] > b[i]){
            return 1;
        }
        if(a[i] == 0 && b[i] == 0){
            return 0;
        }
    }
    return 0;
}
void strncopy(char* t, char* f, uint64_t size){
    for(uint64_t i = 0; i < size; i++){
        t[i] = f[i];
        if(t[i] == 0){
            break;
        }
    }
}
void memset(char* p, char c, uint64_t size){
    for(uint64_t i = 0; i < size; i++){
        p[i] = c;
    }
}
int64_t memcmp(char* a, char* b, uint64_t size){
    for(uint64_t i = 0; i < size; i++){
        if(a[i] < b[i]){
            return -1;
        }
        if(a[i] > b[i]){
            return 1;
        }
    }
    return 0;
}
void memcopy(char* t, char* f, uint64_t size) {
    for(uint64_t i = 0; i < size; i++) {
        t[i] = f[i];
    }
}