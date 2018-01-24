#include<mm/malloc.h>
//空闲内存页数
uint64_t freeblocks = 0;
//空闲页链表头
uint64_t freememory = 0;
//申请一页内存
uint64_t alloc(){
    uint64_t ret = 0;
    if(freememory != 0){
        freeblocks--;
        ret = freememory;
        freememory = *((uint64_t*)freememory);
    }
    return ret;
}
//释放一页内存
void free(uint64_t p){
    freeblocks++;
    *((uint64_t*)p) = freememory;
    freememory = p;
}