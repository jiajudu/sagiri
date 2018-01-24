#include<mm/mm.h>
#include<lib/x64.h>
#include<lib/stdio.h>
#include<lib/string.h>
#include<debug/debug.h>
uint64_t memstart;
uint64_t memend;
extern char end[];
uint64_t* kpgdir;
static const uint64_t pte_p = 0x001;//是否存在
static const uint64_t pte_w = 0x002;//是否可写
static const uint64_t pte_u = 0x004;//用户是否可用
static const uint64_t pte_pwt = 0x008;//write through
static const uint64_t pte_pcd = 0x010;//cache-disable
//空闲内存页数
static uint64_t freeblocks = 0;
//空闲页链表头
static uint64_t freememory = 0;
//探测到的内存布局
struct e820map {
    int nr_map;
    struct {
        uint64_t addr;//内存块基地址
        uint64_t size;//内存大小, 其中0x1表示可用
        uint32_t type;//内存类型
    } __attribute__((packed)) map[20];
};
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
//把虚拟地址pgdir::va映射到pa, 权限为flag
void setmap(uint64_t* pgdir, uint64_t va, uint64_t pa, uint64_t flag){
    uint64_t pt1 = (va >> 39) & 0x1ff;
    uint64_t pt2 = (va >> 30) & 0x1ff;
    uint64_t pt3 = (va >> 21) & 0x1ff;
    uint64_t pt4 = (va >> 12) & 0x1ff;
    if(!(pgdir[pt1] & pte_p)){
        uint64_t dir2 = alloc();
        memset((char*)dir2, 0, 0x1000);
        pgdir[pt1] = k2p(dir2) | pte_p | pte_u | pte_w;
    }
    uint64_t* pgdir2 = (uint64_t*)(p2k(pgdir[pt1] & 0xfffffffffffff000));
    if(!(pgdir2[pt2] & pte_p)){
        uint64_t dir3 = alloc();
        memset((char*)dir3, 0, 0x1000);
        pgdir2[pt2] = k2p(dir3) | pte_p | pte_u | pte_w;
    }
    uint64_t* pgdir3 = (uint64_t*)(p2k(pgdir2[pt2] & 0xfffffffffffff000));
    if(!(pgdir3[pt3] & pte_p)){
        uint64_t dir4 = alloc();
        memset((char*)dir4, 0, 0x1000);
        pgdir3[pt3] = k2p(dir4) | pte_p | pte_u | pte_w;
    }
    uint64_t* pgdir4 = (uint64_t*)(p2k(pgdir3[pt3] & 0xfffffffffffff000));
    pgdir4[pt4] = pa | flag;
}
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
    for(uint64_t p = 0; p < 0x10000; p += 0x1000) {
        setmap(kpgdir, p, p, pte_p | pte_w);
    }
}