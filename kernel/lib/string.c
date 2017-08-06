#include "lib/string.h"
uint64_t strlen(char* p){
    uint64_t i = 0;
    for(; p[i] != 0; i++);
    return i;
}