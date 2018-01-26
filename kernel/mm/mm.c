#include<mm/mm.h>
#include<mm/malloc.h>
#include<mm/vm.h>
#include<lib/x64.h>
#include<lib/stdio.h>
#include<lib/string.h>
#include<debug/debug.h>
uint64_t memstart;
uint64_t memend;
extern char end[];
//探测到的内存布局
struct e820map {
    int nr_map;
    struct {
        uint64_t addr;//内存块基地址
        uint64_t size;//内存大小, 其中0x1表示可用
        uint32_t type;//内存类型
    } __attribute__((packed)) map[20];
};
void mminit(){
    for(uint64_t p = (uint64_t)end; p < 0xffff800000200000; p += 0x1000){
        free(p);
    }
    kpgdir = (uint64_t*)alloc();
    memset((char*)kpgdir, 0, 0x1000);
    //探测物理内存, 得到可用的物理地址范围
    //设置虚拟地址地址到物理地址的映射: 虚拟地址 = 物理地址 + 0xffff800000000000. 对所有物理地址有效. 
    for(uint64_t p = 0; p < 0x200000; p += 0x1000){
        setmap(kpgdir, p2k(p), p, pte_p | pte_w);
    }
    lcr3(k2p((uint64_t)kpgdir));
    struct e820map* e820 = (struct e820map *)(0xffff800000008000);
    uint64_t maxmem = 0;
    for (int64_t i = 0; i < e820->nr_map; i++) {
        if(e820->map[i].type == 1){
            uint64_t start = e820->map[i].addr;
            uint64_t end = start + e820->map[i].size;
            if(start < 0x200000){
                start = 0x200000;
            }
            for(uint64_t p = start; p < end; p += 0x1000){
                setmap(kpgdir, p2k(p), p, pte_p | pte_w);
                free(p2k(p));
            }
            if(end > maxmem){
                maxmem = end;
            }
        }
        if(e820->map[i].type == 2){
            uint64_t start = e820->map[i].addr;
            uint64_t end = start + e820->map[i].size;
            for(uint64_t p = start; p < end; p += 0x1000){
                setmap(kpgdir, p2k(p), p, pte_p | pte_w | pte_pcd | pte_pwt);
            }
        }
    }
    for(uint64_t p = 0xfe000000; p < 0xff000000; p += 0x1000) {
        setmap(kpgdir, p2k(p), p, pte_p | pte_w | pte_pcd | pte_pwt);
    }
    pagerefinit(maxmem);
    for(uint64_t p = 0; p < 0x10000; p += 0x1000) {
        setmap(kpgdir, p, p, pte_p | pte_w);
    }
}