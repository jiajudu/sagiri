#include "mm/pmm.h"
#include "lib/x64.h"
#include "lib/stdio.h"
uint64_t memstart;
uint64_t memend;
void pmminit(){
    //探测物理内存, 得到可用的物理地址范围
    struct e820map* memmap = (struct e820map *)(0xFFFF800000008000);
    uint64_t maxsize = 0;
    for (int64_t i = 0; i < memmap->nr_map; i ++) {
        if(memmap->map[i].type == 1 && maxsize < memmap->map[i].size){
            memstart = memmap->map[i].addr;
            memend = memstart + memmap->map[i].size;
        }
    }
    printf("memory: 0x%x -> 0x%x\n", memstart, memend - 1);
}