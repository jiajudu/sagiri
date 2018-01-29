#pragma once
#include<lib/util.h>
//内核虚拟地址->物理地址
static inline uint64_t k2p(uint64_t k){
    return k - 0xffff800000000000;
}
//物理地址->内核虚拟地址
static inline uint64_t p2k(uint64_t p){
    return p + 0xffff800000000000;
}
extern uint64_t* kpgdir;
extern const uint64_t pte_p;
extern const uint64_t pte_w;
extern const uint64_t pte_u;
extern const uint64_t pte_pwt;
extern const uint64_t pte_pcd;
void setmap(uint64_t* pgdir, uint64_t va, uint64_t pa, uint64_t flag);
void pagerefinit(uint64_t maxmem);
void pagefault(uint64_t addr);
